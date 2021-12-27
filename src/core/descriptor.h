#pragma once

#include "cdx.h"

#include <mutex>
#include <vector>

class DescriptorAllocator {
public:
  DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    : mHeapType(heapType), mCurrentHeap(nullptr), mDescriptorSize(0) {

    mCurrentHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
  }

  // Allocates the requested number of descriptors and returns a handle to the
  // start of the allocated space in the descriptor heap.
  D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32_t count);

protected:
  // Type of descriptor to allocate.
  D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
  // CPU-side handle to the start of the next available block in the current heap.
  D3D12_CPU_DESCRIPTOR_HANDLE mCurrentHandle;
  ID3D12DescriptorHeap* mCurrentHeap;
  // The size of descriptors depends on the type of descriptor and is determined
  // by querying the device, ID3D12Device::GetDescriptorHandleIncrementSize.
  uint32_t mDescriptorSize;
  uint32_t mRemainingFreeHandles;

  // Each heap can allocate 256 descriptors. A new heap is created whenever
  // the current heap can't allocate the number of descriptors requested through
  // an Allocate call.
  static const uint32_t smNumDescriptorsPerHeap = 256;

  static std::mutex smAllocationMutex;
  static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> smDescriptorHeapPool;
  static ID3D12DescriptorHeap* RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
};