//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BufferVk.cpp:
//    Implements the class methods for BufferVk.
//

#include "libANGLE/renderer/vulkan/BufferVk.h"

#include "common/debug.h"
#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/trace.h"

namespace rx
{

namespace
{
// Vertex attribute buffers are used as storage buffers for conversion in compute, where access to
// the buffer is made in 4-byte chunks.  Assume the size of the buffer is 4k+n where n is in [0, 3).
// On some hardware, reading 4 bytes from address 4k returns 0, making it impossible to read the
// last n bytes.  By rounding up the buffer sizes to a multiple of 4, the problem is alleviated.
constexpr size_t kBufferSizeGranularity = 4;
static_assert(gl::isPow2(kBufferSizeGranularity), "use as alignment, must be power of two");

// Start with a fairly small buffer size. We can increase this dynamically as we convert more data.
constexpr size_t kConvertedArrayBufferInitialSize = 1024 * 8;

// Base size for all staging buffers
constexpr size_t kStagingBufferBaseSize = 1024;
// Fix the staging buffer size multiplier for unpack buffers, for now
constexpr size_t kUnpackBufferStagingBufferMultiplier = 1024;

size_t CalculateStagingBufferSize(gl::BufferBinding target, size_t size, size_t alignment)
{
    size_t alignedSize = rx::roundUp(size, alignment);
    int multiplier     = std::max(gl::log2(alignedSize), 1);

    switch (target)
    {
        case gl::BufferBinding::Array:
        case gl::BufferBinding::DrawIndirect:
        case gl::BufferBinding::ElementArray:
        case gl::BufferBinding::Uniform:
            return kStagingBufferBaseSize * multiplier;

        case gl::BufferBinding::PixelUnpack:
            return std::max(alignedSize,
                            (kStagingBufferBaseSize * kUnpackBufferStagingBufferMultiplier));

        default:
            return kStagingBufferBaseSize;
    }
}

// Buffers that have a static usage pattern will be allocated in
// device local memory to speed up access to and from the GPU.
// Dynamic usage patterns or that are frequently mapped
// will now request host cached memory to speed up access from the CPU.
ANGLE_INLINE VkMemoryPropertyFlags GetPreferredMemoryType(gl::BufferBinding target,
                                                          gl::BufferUsage usage)
{
    constexpr VkMemoryPropertyFlags kDeviceLocalFlags =
        (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    constexpr VkMemoryPropertyFlags kHostCachedFlags =
        (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
         VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
    constexpr VkMemoryPropertyFlags kHostUncachedFlags =
        (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (target == gl::BufferBinding::PixelUnpack)
    {
        return kHostCachedFlags;
    }

    switch (usage)
    {
        case gl::BufferUsage::StaticCopy:
        case gl::BufferUsage::StaticDraw:
        case gl::BufferUsage::StaticRead:
            // For static usage, request a device local memory
            return kDeviceLocalFlags;
        case gl::BufferUsage::DynamicDraw:
        case gl::BufferUsage::StreamDraw:
            // For non-static usage where the CPU performs a write-only access, request
            // a host uncached memory
            return kHostUncachedFlags;
        case gl::BufferUsage::DynamicCopy:
        case gl::BufferUsage::DynamicRead:
        case gl::BufferUsage::StreamCopy:
        case gl::BufferUsage::StreamRead:
            // For all other types of usage, request a host cached memory
            return kHostCachedFlags;
        default:
            UNREACHABLE();
            return kHostCachedFlags;
    }
}
}  // namespace

// ConversionBuffer implementation.
ConversionBuffer::ConversionBuffer(RendererVk *renderer,
                                   VkBufferUsageFlags usageFlags,
                                   size_t initialSize,
                                   size_t alignment,
                                   bool hostVisible)
    : dirty(true), lastAllocationOffset(0)
{
    data.init(renderer, usageFlags, alignment, initialSize, hostVisible);
}

ConversionBuffer::~ConversionBuffer() = default;

ConversionBuffer::ConversionBuffer(ConversionBuffer &&other) = default;

// BufferVk::VertexConversionBuffer implementation.
BufferVk::VertexConversionBuffer::VertexConversionBuffer(RendererVk *renderer,
                                                         angle::FormatID formatIDIn,
                                                         GLuint strideIn,
                                                         size_t offsetIn,
                                                         bool hostVisible)
    : ConversionBuffer(renderer,
                       vk::kVertexBufferUsageFlags,
                       kConvertedArrayBufferInitialSize,
                       vk::kVertexBufferAlignment,
                       hostVisible),
      formatID(formatIDIn),
      stride(strideIn),
      offset(offsetIn)
{}

BufferVk::VertexConversionBuffer::VertexConversionBuffer(VertexConversionBuffer &&other) = default;

BufferVk::VertexConversionBuffer::~VertexConversionBuffer() = default;

// BufferVk implementation.
BufferVk::BufferVk(const gl::BufferState &state) : BufferImpl(state) {}

BufferVk::~BufferVk() {}

void BufferVk::destroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    release(contextVk);
}

void BufferVk::release(ContextVk *contextVk)
{
    RendererVk *renderer = contextVk->getRenderer();
    mBuffer.release(renderer);
    mStagingBuffer.release(renderer);
    mShadowBuffer.release();

    for (ConversionBuffer &buffer : mVertexConversionBuffers)
    {
        buffer.data.release(renderer);
    }
}

void BufferVk::initializeStagingBuffer(ContextVk *contextVk, gl::BufferBinding target, size_t size)
{
    RendererVk *rendererVk = contextVk->getRenderer();

    constexpr VkImageUsageFlags kBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    size_t alignment =
        static_cast<size_t>(rendererVk->getPhysicalDeviceProperties().limits.minMemoryMapAlignment);
    size_t stagingBufferSize = CalculateStagingBufferSize(target, size, alignment);

    mStagingBuffer.init(rendererVk, kBufferUsageFlags, alignment, stagingBufferSize, true);
}

angle::Result BufferVk::initializeShadowBuffer(ContextVk *contextVk,
                                               gl::BufferBinding target,
                                               size_t size)
{
    // For now, enable shadow buffers only for pixel unpack buffers.
    // If usecases present themselves, we can enable them for other buffer types.
    if (target == gl::BufferBinding::PixelUnpack)
    {
        // Initialize the shadow buffer
        mShadowBuffer.init(size);

        // Allocate required memory. If allocation fails, treat it is a non-fatal error
        // since we do not need the shadow buffer for functionality
        ANGLE_TRY(mShadowBuffer.allocate(size));
    }

    return angle::Result::Continue;
}

void BufferVk::updateShadowBuffer(const uint8_t *data, size_t size, size_t offset)
{
    if (mShadowBuffer.valid())
    {
        mShadowBuffer.updateData(data, size, offset);
    }
}

angle::Result BufferVk::setData(const gl::Context *context,
                                gl::BufferBinding target,
                                const void *data,
                                size_t size,
                                gl::BufferUsage usage)
{
    ContextVk *contextVk = vk::GetImpl(context);

    if (size > static_cast<size_t>(mState.getSize()))
    {
        // Release and re-create the memory and buffer.
        release(contextVk);

        // We could potentially use multiple backing buffers for different usages.
        // For now keep a single buffer with all relevant usage flags.
        VkImageUsageFlags usageFlags =
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

        if (contextVk->getFeatures().supportsTransformFeedbackExtension.enabled)
        {
            usageFlags |= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
        }

        VkBufferCreateInfo createInfo    = {};
        createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.flags                 = 0;
        createInfo.size                  = roundUpPow2(size, kBufferSizeGranularity);
        createInfo.usage                 = usageFlags;
        createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;

        // Assume host visible/coherent memory available.
        VkMemoryPropertyFlags memoryPropertyFlags = GetPreferredMemoryType(target, usage);

        ANGLE_TRY(mBuffer.init(contextVk, createInfo, memoryPropertyFlags));

        // Initialize the staging buffer
        initializeStagingBuffer(contextVk, target, size);

        // Initialize the shadow buffer
        ANGLE_TRY(initializeShadowBuffer(contextVk, target, size));
    }

    if (data && size > 0)
    {
        ANGLE_TRY(setDataImpl(contextVk, static_cast<const uint8_t *>(data), size, 0));
    }

    return angle::Result::Continue;
}

angle::Result BufferVk::setSubData(const gl::Context *context,
                                   gl::BufferBinding target,
                                   const void *data,
                                   size_t size,
                                   size_t offset)
{
    ASSERT(mBuffer.valid());

    ContextVk *contextVk = vk::GetImpl(context);
    ANGLE_TRY(setDataImpl(contextVk, static_cast<const uint8_t *>(data), size, offset));

    return angle::Result::Continue;
}

angle::Result BufferVk::copySubData(const gl::Context *context,
                                    BufferImpl *source,
                                    GLintptr sourceOffset,
                                    GLintptr destOffset,
                                    GLsizeiptr size)
{
    ASSERT(mBuffer.valid());

    ContextVk *contextVk = vk::GetImpl(context);
    auto *sourceBuffer   = GetAs<BufferVk>(source);
    ASSERT(sourceBuffer->getBuffer().valid());

    // If the shadow buffer is enabled for the destination buffer then
    // we need to update that as well. This will require us to complete
    // all recorded and in-flight commands involving the source buffer.
    if (mShadowBuffer.valid())
    {
        ANGLE_TRY(sourceBuffer->getBuffer().waitForIdle(contextVk));

        // Update the shadow buffer
        uint8_t *srcPtr;
        ANGLE_TRY(sourceBuffer->getBuffer().mapWithOffset(contextVk, &srcPtr, sourceOffset));

        updateShadowBuffer(srcPtr, size, destOffset);

        // Unmap the source buffer
        sourceBuffer->getBuffer().unmap(contextVk->getRenderer());
    }

    vk::CommandBuffer *commandBuffer = nullptr;

    ANGLE_TRY(contextVk->onBufferRead(VK_ACCESS_TRANSFER_READ_BIT, &sourceBuffer->getBuffer()));
    ANGLE_TRY(contextVk->onBufferWrite(VK_ACCESS_TRANSFER_WRITE_BIT, &mBuffer));
    ANGLE_TRY(contextVk->endRenderPassAndGetCommandBuffer(&commandBuffer));

    // Enqueue a copy command on the GPU.
    const VkBufferCopy copyRegion = {static_cast<VkDeviceSize>(sourceOffset),
                                     static_cast<VkDeviceSize>(destOffset),
                                     static_cast<VkDeviceSize>(size)};

    commandBuffer->copyBuffer(sourceBuffer->getBuffer().getBuffer(), mBuffer.getBuffer(), 1,
                              &copyRegion);

    // The new destination buffer data may require a conversion for the next draw, so mark it dirty.
    onDataChanged();

    return angle::Result::Continue;
}

angle::Result BufferVk::map(const gl::Context *context, GLenum access, void **mapPtr)
{
    ASSERT(mBuffer.valid());

    return mapImpl(vk::GetImpl(context), mapPtr);
}

angle::Result BufferVk::mapRange(const gl::Context *context,
                                 size_t offset,
                                 size_t length,
                                 GLbitfield access,
                                 void **mapPtr)
{
    return mapRangeImpl(vk::GetImpl(context), offset, length, access, mapPtr);
}

angle::Result BufferVk::mapImpl(ContextVk *contextVk, void **mapPtr)
{
    return mapRangeImpl(contextVk, 0, static_cast<VkDeviceSize>(mState.getSize()), 0, mapPtr);
}

angle::Result BufferVk::mapRangeImpl(ContextVk *contextVk,
                                     VkDeviceSize offset,
                                     VkDeviceSize length,
                                     GLbitfield access,
                                     void **mapPtr)
{
    if (!mShadowBuffer.valid())
    {
        ASSERT(mBuffer.valid());

        if ((access & GL_MAP_UNSYNCHRONIZED_BIT) == 0)
        {
            ANGLE_TRY(mBuffer.waitForIdle(contextVk));
        }

        ANGLE_TRY(mBuffer.mapWithOffset(contextVk, reinterpret_cast<uint8_t **>(mapPtr),
                                        static_cast<size_t>(offset)));
    }
    else
    {
        // If the app requested a GL_MAP_UNSYNCHRONIZED_BIT access, the spec states -
        //      No GL error is generated if pending operations which source or modify the
        //      buffer overlap the mapped region, but the result of such previous and any
        //      subsequent operations is undefined
        // To keep the code simple, irrespective of whether the access was GL_MAP_UNSYNCHRONIZED_BIT
        // or not, just return the shadow buffer.
        mShadowBuffer.map(static_cast<size_t>(offset), mapPtr);
    }

    return angle::Result::Continue;
}

angle::Result BufferVk::unmap(const gl::Context *context, GLboolean *result)
{
    ANGLE_TRY(unmapImpl(vk::GetImpl(context)));

    // This should be false if the contents have been corrupted through external means.  Vulkan
    // doesn't provide such information.
    *result = true;

    return angle::Result::Continue;
}

angle::Result BufferVk::unmapImpl(ContextVk *contextVk)
{
    ASSERT(mBuffer.valid());

    if (!mShadowBuffer.valid())
    {
        mBuffer.unmap(contextVk->getRenderer());
        mBuffer.onExternalWrite(VK_ACCESS_HOST_WRITE_BIT);
    }
    else
    {
        bool writeOperation = ((mState.getAccessFlags() & GL_MAP_WRITE_BIT) != 0);
        size_t offset       = static_cast<size_t>(mState.getMapOffset());
        size_t size         = static_cast<size_t>(mState.getMapLength());

        // If it was a write operation we need to update the GPU buffer.
        if (writeOperation)
        {
            // We do not yet know if thie data will ever be used. Perform a staged
            // update which will get flushed if and when necessary.
            const uint8_t *data = getShadowBuffer(offset);
            ANGLE_TRY(stagedUpdate(contextVk, data, size, offset));
        }

        mShadowBuffer.unmap();
    }

    markConversionBuffersDirty();

    return angle::Result::Continue;
}

angle::Result BufferVk::getIndexRange(const gl::Context *context,
                                      gl::DrawElementsType type,
                                      size_t offset,
                                      size_t count,
                                      bool primitiveRestartEnabled,
                                      gl::IndexRange *outRange)
{
    ContextVk *contextVk = vk::GetImpl(context);
    RendererVk *renderer = contextVk->getRenderer();

    // This is a workaround for the mock ICD not implementing buffer memory state.
    // Could be removed if https://github.com/KhronosGroup/Vulkan-Tools/issues/84 is fixed.
    if (renderer->isMockICDEnabled())
    {
        outRange->start = 0;
        outRange->end   = 0;
        return angle::Result::Continue;
    }

    ANGLE_TRACE_EVENT0("gpu.angle", "BufferVk::getIndexRange");

    uint8_t *mapPointer;

    if (!mShadowBuffer.valid())
    {
        // Needed before reading buffer or we could get stale data.
        ANGLE_TRY(mBuffer.finishRunningCommands(contextVk));

        ASSERT(mBuffer.valid());

        ANGLE_TRY(mBuffer.mapWithOffset(contextVk, &mapPointer, offset));
    }
    else
    {
        mapPointer = getShadowBuffer(offset);
    }

    *outRange = gl::ComputeIndexRange(type, mapPointer, count, primitiveRestartEnabled);

    mBuffer.unmap(renderer);
    return angle::Result::Continue;
}

angle::Result BufferVk::directUpdate(ContextVk *contextVk,
                                     const uint8_t *data,
                                     size_t size,
                                     size_t offset)
{
    uint8_t *mapPointer = nullptr;

    ANGLE_TRY(mBuffer.mapWithOffset(contextVk, &mapPointer, offset));
    ASSERT(mapPointer);

    memcpy(mapPointer, data, size);

    mBuffer.unmap(contextVk->getRenderer());
    mBuffer.onExternalWrite(VK_ACCESS_HOST_WRITE_BIT);

    return angle::Result::Continue;
}

angle::Result BufferVk::stagedUpdate(ContextVk *contextVk,
                                     const uint8_t *data,
                                     size_t size,
                                     size_t offset)
{
    // Acquire a "new" staging buffer
    bool needToReleasePreviousBuffers = false;
    uint8_t *mapPointer               = nullptr;
    VkDeviceSize stagingBufferOffset  = 0;

    ANGLE_TRY(mStagingBuffer.allocate(contextVk, size, &mapPointer, nullptr, &stagingBufferOffset,
                                      &needToReleasePreviousBuffers));
    if (needToReleasePreviousBuffers)
    {
        // Release previous staging buffers
        mStagingBuffer.releaseInFlightBuffers(contextVk);
    }
    ASSERT(mapPointer);

    memcpy(mapPointer, data, size);

    // Enqueue a copy command on the GPU.
    VkBufferCopy copyRegion = {stagingBufferOffset, offset, size};
    ANGLE_TRY(mBuffer.copyFromBuffer(contextVk, mStagingBuffer.getCurrentBuffer(),
                                     VK_ACCESS_HOST_WRITE_BIT, copyRegion));
    mStagingBuffer.getCurrentBuffer()->retain(&contextVk->getResourceUseList());

    return angle::Result::Continue;
}

angle::Result BufferVk::setDataImpl(ContextVk *contextVk,
                                    const uint8_t *data,
                                    size_t size,
                                    size_t offset)
{
    // Update shadow buffer
    updateShadowBuffer(data, size, offset);

    // If the buffer is currently in use, stage the update. Otherwise update the buffer directly.
    if (mBuffer.isCurrentlyInUse(contextVk->getLastCompletedQueueSerial()))
    {
        ANGLE_TRY(stagedUpdate(contextVk, data, size, offset));
    }
    else
    {
        ANGLE_TRY(directUpdate(contextVk, data, size, offset));
    }

    // Update conversions
    markConversionBuffersDirty();

    return angle::Result::Continue;
}

angle::Result BufferVk::copyToBufferImpl(ContextVk *contextVk,
                                         vk::BufferHelper *destBuffer,
                                         uint32_t copyCount,
                                         const VkBufferCopy *copies)
{
    vk::CommandBuffer *commandBuffer;
    ANGLE_TRY(contextVk->onBufferWrite(VK_ACCESS_TRANSFER_WRITE_BIT, destBuffer));
    ANGLE_TRY(contextVk->onBufferRead(VK_ACCESS_TRANSFER_READ_BIT, &mBuffer));
    ANGLE_TRY(contextVk->endRenderPassAndGetCommandBuffer(&commandBuffer));

    commandBuffer->copyBuffer(mBuffer.getBuffer(), destBuffer->getBuffer(), copyCount, copies);

    return angle::Result::Continue;
}

ConversionBuffer *BufferVk::getVertexConversionBuffer(RendererVk *renderer,
                                                      angle::FormatID formatID,
                                                      GLuint stride,
                                                      size_t offset,
                                                      bool hostVisible)
{
    for (VertexConversionBuffer &buffer : mVertexConversionBuffers)
    {
        if (buffer.formatID == formatID && buffer.stride == stride && buffer.offset == offset)
        {
            return &buffer;
        }
    }

    mVertexConversionBuffers.emplace_back(renderer, formatID, stride, offset, hostVisible);
    return &mVertexConversionBuffers.back();
}

void BufferVk::markConversionBuffersDirty()
{
    for (VertexConversionBuffer &buffer : mVertexConversionBuffers)
    {
        buffer.dirty = true;
    }
}

void BufferVk::onDataChanged()
{
    markConversionBuffersDirty();
}

}  // namespace rx
