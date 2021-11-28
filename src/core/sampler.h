#pragma once

#include "cdx.h"
#include "color.h"

// TODO: rename to Sampler.
class SamplerDesc : public D3D12_SAMPLER_DESC {
public:
  // Set sampler descriptor default values.
  SamplerDesc() {
    // Anisotropic filtering for minification, magnification, and mipmapping.
    Filter = D3D12_FILTER_ANISOTROPIC;
    AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    MipLODBias = 0.0f;
    MaxAnisotropy = 16;
    // For a comparison sampler, such as those used in shadow mapping.
    ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    // R.
    BorderColor[0] = 1.0f;
    // G.
    BorderColor[1] = 1.0f;
    // B.
    BorderColor[2] = 1.0f;
    // A.
    BorderColor[3] = 1.0f;
    MinLOD = 0.0f;
    MaxLOD = D3D12_FLOAT32_MAX;
  }

  void SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE mode) {
    AddressU = mode;
    AddressV = mode;
    AddressW = mode;
  }

  void SetBorderColor(Color color) {
    BorderColor[0] = color.R();
    BorderColor[1] = color.G();
    BorderColor[2] = color.B();
    BorderColor[3] = color.A();
  }

  // Creates a sampler using this sampler descriptor and returns a CPU handle to it.
  D3D12_CPU_DESCRIPTOR_HANDLE CreateSampler();
};