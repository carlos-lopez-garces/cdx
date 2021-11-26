#include pragma

class CommandListManager;
class ContextManager;

namespace Graphics {
  using namespace Microsoft::WRL;

#ifndef RELEASE
  extern const GUID WKPDID_D3DDebugObjectName;
#endif
  extern ID3D12Device* g_Device;
  extern CommandListManager g_CommandManager;
  extern ContextManager g_ContextManager;
  extern D3D_FEATURE_LEVEL g_D3DFeatureLevel;
  // Can the shader read from a UAV with R11G11B10_FLOAT or R16G16B16A16_FLOAT formats?
  // Three partial-precision s10e5 floating-point numbers encoded into a single 32-bit value.
  extern bool g_bTypedUAVLoadSupport_R11G11B10_FLOAT;
  extern bool g_bTypedUAVLoadSupport_R16G16B16A16_FLOAT;
  extern DescriptorAllocator g_DescriptorAllocator[];

  void Initialize();

  // Allocates a number of descriptors from the specified type of descriptor heap.
  inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(
    D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count = 1
  ) {
    return g_DescriptorAllocator[type].Allocate(count);
  }
}