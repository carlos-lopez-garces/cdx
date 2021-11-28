#include "cdx.h"
#include "descriptor.h"
#include "graphics.h"
#include "utils.h"

#include <mutex>

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::Allocate(uint32_t count) {
  if (mCurrentHeap == nullptr || mRemainingFreeHandles < count) {
    // No heap has been created before or the current one doesn't have enough
    // free handles to satisfy the request.
    mCurrentHeap = RequestNewHeap(mHeapType);
    mCurrentHandle = mCurrentHeap->GetCPUDescriptorHandleForHeapStart();
    mRemainingFreeHandles = smNumDescriptorsPerHeap;
    if (mDescriptorSize == 0) {
      mDescriptorSize = Graphics::gDevice->GetDescriptorHandleIncrementSize(mHeapType);
    }
  }

  // The allocated space starts where the handle was left after the last allocation.
  D3D12_CPU_DESCRIPTOR_HANDLE handle = mCurrentHandle;
  handle.ptr += count * mDescriptorSize;
  mRemainingFreeHandles -= count;
  return handle;
}

ID3D12DescriptorHeap* Descriptor:Allocator::RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType) {
  // A std::lock_guard tries to acquire the mutex during construction. Then,
  // when it goes out of scope, its destructor releases the mutex. See Resource
  // Acquisition Is Initialization or RAII.
  std::lock_guard<std::mutex> lockGuard(smAllocationMutex);

  D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
  heapDesc.Type = heapType;
  heapDesc.NumDescriptors = smNumDescriptorsPerHeap;
  heapDesc.Flgs = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  heapDesc.NodeMask = 1;

  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pHeap;
  ASSERT_SUCCEEDED(Graphics::gDevice->CreateDescriptorHeap(&heapDesc, MY_IID_PPV_ARGS(&pHeap)));
  smDescriptorHeapPool.emplace_back(pHeap);
  return pHeap.Get();
}
