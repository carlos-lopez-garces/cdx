#include "sampler.h"
#include "graphics.h"
#include "utils.h"

#include <map>

// Declarations in an anonymous namespace are restricted to the translation unit.
namespace {
  std::map<size_t, D3D12_CPU_DESCRIPTOR_HANDLE> sSamplerCache;
}

D3D12_CPU_DESCRIPTOR_HANDLE SamplerDesc::CreateSampler() {
  size_t hash = Utils::GetHash(this);
  auto iter = sSamplerCache.find(hash);
  if (iter != sSamplerCache.end()) {
    return iter->second;
  }

  D3D12_CPU_DESCRIPTOR_HANDLE handle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  Graphics::g_Device->CreateSampler(this, handle);
  sSamplerCache.insert(hash, handle);
  return handle;
}