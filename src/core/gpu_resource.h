#pragma

#include "cdx.h"

// Resources are essentially typed/typeless memory buffers.
class GPUResource {
public:
  GPUResource()
    : mUsageState(D3D12_RESOURCE_STATE_COMMON),
      mTransitioningState((D3D12_RESOURCE_STATES)-1),
      mGPUVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL)
  {}

  ~GPUResource() {
    Destroy();
  }

  virtual void Destroy() {
    m_pResource = nullptr;
    m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
    ++m_VersionID;
  }

protected:
  Microsoft::WRL::ComPtr<ID3D12Resource> mpResource;

  // State that describes what the resource is currently being used for:
  // as a render target? as a vertex buffer? as an index buffer? as constant buffer?
  // A given resource may be bound to the pipeline for different usages; for example,
  // first as a render target to write to it and then as a shader resource to read from it.
  D3D12_RESOURCE_STATES mUsageState;
  
  D3D12_RESOURCE_STATES mTransitioningState;

  D3D12_GPU_VIRTUAL_ADDRESS mGPUVirtualAddress;
};