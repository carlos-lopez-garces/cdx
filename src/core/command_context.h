#pragma once

#include "gpu_resource.h"

struct NonCopyable {
  NonCopyable() = default;
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
};

class CommandContext : NonCopyable {
private:
  CommandContext(D3D12_COMMAND_LIST_TYPE type);

protected:
  D3D12_COMMAND_LIST_TYPE mType;
  // TODO: implement data types.
  DynamicDescriptorHeap mDynamicViewDescriptorHeap;
  DynamicDescriptorHeap mDynamicSamplerDescriptorHeap;
  LinearAllocator mCPULinearAllocator;
  LinearAllocator mGPULinearAllocator;

public:
  static void InitializeTexture(GPUResource& dest, UINT numSubresources, D3D12_SUBRESOURCE_DATA subData[]);
};