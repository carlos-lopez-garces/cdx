#pragma once

#include "sampler.h"
#include "descriptor.h"

class CommandListManager;
class ContextManager;

namespace Graphics {
  using namespace Microsoft::WRL;

#ifndef RELEASE
  extern const GUID WKPDID_D3DDebugObjectName;
#endif
  extern ID3D12Device* gDevice;
  extern CommandListManager g_CommandManager;
  extern ContextManager g_ContextManager;
  extern D3D_FEATURE_LEVEL g_D3DFeatureLevel;
  // Can the shader read from a UAV with R11G11B10_FLOAT or R16G16B16A16_FLOAT formats?
  // Three partial-precision s10e5 floating-point numbers encoded into a single 32-bit value.
  extern bool g_bTypedUAVLoadSupport_R11G11B10_FLOAT;
  extern bool g_bTypedUAVLoadSupport_R16G16B16A16_FLOAT;
  extern DescriptorAllocator gDescriptorAllocator[];

  // Static samplers don't need to be allocated from a sampler heap. Instead, you may
  // define an array of static samplers once, when initializing the root signature.
  extern SamplerDesc SamplerLinearWrapDesc;
  extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearWrap;
  extern SamplerDesc SamplerAnisoWrapDesc;
  extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerAnisoWrap;
  extern SamplerDesc SamplerShadowDesc;
  extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerShadow;
  extern SamplerDesc SamplerLinearClampDesc;
  extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearClamp;
  extern SamplerDesc SamplerVolumeWrapDesc;
  extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerVolumeWrap;
  extern SamplerDesc SamplerPointClampDesc;
  extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointClamp;
  extern SamplerDesc SamplerPointBorderDesc;
  extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointBorder;
  extern SamplerDesc SamplerLinearBorderDesc;
  extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearBorder;

  enum DefaultTextureId {
    kMagenta2D,
    kBlackOpaque2D,
    kBlackTransparent2D,
    kWhiteOpaque2D,
    kWhiteTransparent2D,
    kDefaultNormalMap,
    kBlackCubeMap,
    kNumDefaultTextures
  };

  void Initialize();

  // Allocates a number of descriptors from the specified type of descriptor heap.
  inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(
    D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count = 1
  ) {
    return gDescriptorAllocator[type].Allocate(count);
  }

  D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultTexture(DefaultTextureId texId);
}