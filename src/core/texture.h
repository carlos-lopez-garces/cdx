#pragma once

#include "cdx.h"
#include "gpu_resource.h"

class Texture : public GPUResource {
public:
  Texture() {
    mCPUDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
  }

  uint32_t GetWidth() const {
    return mWidth;
  }
  
  uint32_t GetHeight() const {
    return mHeight;
  }

  uint32_t GetDepth() const {
    return mDepth;
  }

  void Create2D(size_t rowPitchBytes, size_t width, size_t height, DXGI_FORMAT format, const void* data);
  void CreateCube(size_t rowPitchBytes, size_t width, size_t height, DXGI_FORMAT format, const void* data);

  virtual void Destroy() override {
    GPUResource::Destroy();
    mCPUDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
  }

protected:
  uint32_t mWidth;
  uint32_t mHeight;
  uint32_t mDepth;
  D3D12_CPU_DESCRIPTOR_HANDLE mCPUDescriptorHandle;
};