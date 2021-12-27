#include "command_context.h"
#include "graphics.h"
#include "linear_allocator.h"

using namespace Graphics;

CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE cmdListType)
  : mType(cmdListType),
    mDynamicViewDescriptorHeap(*this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
    mDynamicSamplerDescriptorHeap(*this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),
    mCPULinearAllocator(kCPUWritable),
    mGPULinearAllocator(kGPUExclusive) {

}