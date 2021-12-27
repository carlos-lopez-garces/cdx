// Windows registry.
#include <winreg.h>

#include "cdx.h"
#include "graphics.h"
#include "game.h"
#include "texture.h"
#include "utils.h"

#pragma comment(lib, "d3d12.lib") 

using namespace Math;

namespace Graphics {
  // Can the GPU do typed UAV loads of R11G11B10_FLOAT or R16G16B16A16_FLOAT?
  bool g_bTypedUAVLoadSupport_R11G11B10_FLOAT = false;
  bool g_bTypedUAVLoadSupport_R16G16B16A16_FLOAT = false;

  ID3D12Device* gDevice = nullptr;
  CommandListManager g_CommandManager;
  ContextManager g_ContextManager;
  
  // Target features supported by Direct3D 11.0, including shader model 5.
  D3D_FEATURE_LEVEL g_D3DFeatureLevel = D3D_FEATURE_LEVEL_11_0;

  DescriptorAllocator gDescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {
    // Constant buffer views, shader resource views, and unordered access views.
    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
    // Render target views.
    D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
    // Depth-stencil views.
    D3D12_DESCRIPTOR_HEAP_TYPE_DSV
  };

  // Static samplers.
  SamplerDesc SamplerLinearWrapDesc;
  D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearWrap;
  SamplerDesc SamplerAnisoWrapDesc;
  D3D12_CPU_DESCRIPTOR_HANDLE SamplerAnisoWrap;
  SamplerDesc SamplerShadowDesc;
  D3D12_CPU_DESCRIPTOR_HANDLE SamplerShadow;
  SamplerDesc SamplerLinearClampDesc;
  D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearClamp;
  SamplerDesc SamplerVolumeWrapDesc;
  D3D12_CPU_DESCRIPTOR_HANDLE SamplerVolumeWrap;
  SamplerDesc SamplerPointClampDesc;
  D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointClamp;
  SamplerDesc SamplerPointBorderDesc;
  D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointBorder;
  SamplerDesc SamplerLinearBorderDesc;
  D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearBorder;

  Texture DefaultTextures[kNumDefaultTextures];

  void InitializeCommonState() {
    // Static samplers.
    SamplerLinearWrapDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerLinearWrap = SamplerLinearWrapDesc.CreateSampler();

    SamplerAnisoWrapDesc.MaxAnisotropy = 4;
    SamplerAnisoWrap = SamplerAnisoWrapDesc.CreateSampler();

    SamplerShadowDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    SamplerShadowDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    SamplerShadowDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    SamplerShadow = SamplerShadowDesc.CreateSampler();

    SamplerLinearClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerLinearClampDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    SamplerLinearClamp = SamplerLinearClampDesc.CreateSampler();

    SamplerVolumeWrapDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    SamplerVolumeWrap = SamplerVolumeWrapDesc.CreateSampler();

    SamplerPointClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    SamplerPointClampDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    SamplerPointClamp = SamplerPointClampDesc.CreateSampler();

    SamplerLinearBorderDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerLinearBorderDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_BORDER);
    SamplerLinearBorderDesc.SetBorderColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
    SamplerLinearBorder = SamplerLinearBorderDesc.CreateSampler();

    SamplerPointBorderDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    SamplerPointBorderDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_BORDER);
    SamplerPointBorderDesc.SetBorderColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
    SamplerPointBorder = SamplerPointBorderDesc.CreateSampler();
  }

  void Initialize() {
    Microsoft::WRL::ComPtr<ID3D12Device> pDevice;
    DWORD dxgiFactoryFlags = 0;

    uint32_t useDebugLayers = 0;
    CommandLineArgs::GetInteger(L"debug", useDebugLayers);
#if _DEBUG
    // Override for debug builds.
    useDebugLayers = 1;
#endif

    if (useDebugLayers) {
      // An interface used to turn on the debug layer.
      Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
      if (SUCCEEDED(D3D12GetDebugInterface(MY_IID_PPV_ARGS(&debugInterface)))) {
        debugInterface->EnableDebugLayer();

        // GPU-based validation, GBV, enables validation scenarios on the GPU timeline 
        // that are not possible during API calls on the CPU. GBV helps to identify the 
        // following errors:
        //
        // - Use of uninitialized or incompatible descriptors in a shader.
        // - Use of descriptors referencing deleted Resources in a shader.
        // - Validation of promoted resource states and resource state decay.
        // - Indexing beyond the end of the descriptor heap in a shader.
        // - Shader accesses of resources in incompatible state.
        // - Use of uninitialized or incompatible Samplers in a shader.
        uint32_t useGPUBasedValidation = 0;
        CommandLineArgs::GetInteger(L"gpu_debug", useGPUBasedValidation);
        if (useGPUBasedValidation) {
          // ID3D12Debug1,2,3,4,5 add features to the debug layer. ID3D12Debug1 adds
          // GPU-Based Validation and Dependent Command Queue Synchronization.
          Microsoft::WRL::ComPtr<ID3D12Debug1> debugInterface1;
          if (SUCCEEDED((debugInterface->QueryInterface(IID_PPV_ARGS(&debugInterface1))))) {
            debugInterface1->EnableDebugLayer(true);
          }
        }
      } else {
        Utils::Print("WARNING: unable to enable D3D12 debug validation layer\n");
      }

#if _DEBUG
      // Controls the debug information queue of the debug layer. An information queue
      // interface stores, retrieves, and filters debug messages.
      Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
      if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf())))) {
        // For CreateDXGIFactory2 to request DXGIDebug.dll.
        dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
        // Break when error or corruption messages enter the information queue.
        dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
        dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

        // Deny these messages.
        DXGI_INFO_QUEUE_MESSAGE_ID hide[] = {
          80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
        };
        DXGI_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = _countof(hide);
        filter.DenyList.pIDList = hide;
        dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
      }
#endif
    }

    // Used to create other DXGI objects and query device characteristics. Factory 6,
    // in particular, lets you enumerates graphics adapters based on a given GPU
    // preference: unspecified, minimum power, high performance.
    Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory;
    ASSERT_SUCCEEDED(CreateDXGIFactory2(dxgiFactoryFlags), MY_IID_PPV_ARGS(&dxgiFactory));

    // Represents the GPU or video card.
    Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter;

    uint32_t bUseWarpDriver = false;
    CommandLineArgs::GetInteger(L"warp", bUseWarpDriver);
    D3D12EnableExperimentalFeatures(0, nullptr, nullptr, nullptr);

    if (!bUseWarpDriver) {
      // GPU memory size.
      SIZE_T videoMemorySize = 0;

      for (uint32_t idx = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(idx, &pAdapter); ++idx) {
        DXGI_ADAPTER_DESC1 adapterDesc;
        pAdapter->GetDesc1(&adapterDesc);
        if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
          // Ignore graphics emulators (software display adapters).
          continue;
        }

        // Fail if the Direct3D capability set specified by g_D3DFeatureLevel isn't supported.
        if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), g_D3DFeatureLevel, MY_IID_PPV_ARGS(&pDevice)))) {
          pAdapter->GetDesc1(&adapterDesc);
          Utils::Printf(
            L"D3D12-capable hardware found:  %s (%u MB)\n", 
            adapterDesc.Description, 
            adapterDesc.DedicatedVideoMemory >> 20
          );

          videoMemorySize = adapterDesc.DedicatedVideoMemory;
          break;
        }
      }

      if (videoMemorySize > 0) {
        // Move the raw pointer to the device to the other ComPtr.
        gDevice = pDevice.Detach();
      }
    }

    if (gDevice == nullptr) {
      // A GPU wasn't found.
      
      // Try with a software display adapter (graphics emulator) or use one if requested.
      // WARP is the Windows Advanced Rasterization Platform.
      if (bUseWarpDriver) {
        Utils::Print("WARP software adapter requested. Initializing ...\n");
      } else {
        Utils::Print("GPU wasn't found. Falling back to WARP ...\n");
      }

      ASSERT_SUCCEEDED(dxgiFactory->EnumWarpAdapter(MY_IID_PPV_ARGS(&pAdapter)));
      ASSERT_SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), g_D3DFeatureLevel, MY_IID_PPV_ARGS(&pDevice)));

      // Move the raw pointer to the device to the other ComPtr.
      gDevice = pDevice.Detach();
    }
#ifndef RELEASE
    else {
      bool developerModeEnabled = false;
      HKEY hKey;
      LSTATUS result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AppModelUnlock", 0, KEY_READ, &hKey);
      if (result == ERROR_SUCCESS) {
        DWORD keyValue, keySize = sizeof(DWORD);
        result = RegQueryValueEx(hKey, L"AllowDevelopmentWithoutDevLicense", 0, NULL, (byte*)&keyValue, &keySize);
        if (result == ERROR_SUCCESS && keyValue == 1) {
          developerModeEnabled = true;
        }
        RegCloseKey(hKey);
      }

      WARN_ONCE_IF_NOT(developerModeEnabled, "Enable Developer Mode on Windows 10 to get consistent profiling results");

      // Prevent the GPU from overclocking or underclocking to get consistent timings.
      if (developerModeEnabled) {
        gDevice->SetStablePowerState(TRUE);
      }
    }
#endif

#if _DEBUG
    ID3D12InfoQueue* pInfoQueue = nullptr;
    // QueryInterface is a method of IUnknown: does the ID3D12Device support 
    // the ID3D12InfoQueue interface?
    if (SUCCEEDED(gDevice->QueryInterface(MY_IID_PPV_ARGS(&pInfoQueue)))) {
      // Suppress messages with severity "info".
      D3D12_MESSAGE_SEVERITY severitiesToDeny[] = {
        D3D12_MESSAGE_SEVERITY_INFO
      };

      // Suppress annoying messages.
      D3D12_MESSAGE_ID idsToDeny[] =
      {
        // Occurs when there are uninitialized descriptors in a descriptor table, even when a
        // shader does not access the missing descriptors. This is common when switching
        // shader permutations and not wanting to change much code to reorder resources.
        D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,

        // Occurs when a shader does not export all color components of a render target, such as
        // when only writing RGB to an R10G10B10A2 buffer, ignoring alpha.
        D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_PS_OUTPUT_RT_OUTPUT_MISMATCH,

        // Occurs when a descriptor table is unbound even when a shader does not access the missing
        // descriptors. This is common with a root signature shared between disparate shaders that
        // don't all need the same types of resources.
        D3D12_MESSAGE_ID_COMMAND_LIST_DESCRIPTOR_TABLE_NOT_SET,

        // RESOURCE_BARRIER_DUPLICATE_SUBRESOURCE_TRANSITIONS.
        (D3D12_MESSAGE_ID)1008,
      };

      D3D12_INFO_QUEUE_FILTER filter = {};
      filter.DenyList.NumSeverities = _countof(severitiesToDeny);
      filter.DenyList.pSeverityList = severitiesToDeny;
      filter.DenyList.NumIDs = _countof(idsToDeny);
      filter.DenyList.pIDList = idsToDeny;
      pInfoQueue->PushStorageFilter(&filter);
      pInfoQueue->Release();
    }
#endif

    // Check if the GPU is able to do typed UAV loads of R11G11B10_FLOAT or R16G16B16A16_FLOAT,
    // which is required for doing read-modify-write operations on UAVs during post processing.
    // Otherwise, we are forced to manually decode an R32_UINT representation of the same buffer.
    D3D12_FEATURE_DATA_D3D12_OPTIONS featureData = {};
    if (SUCCEEDED(gDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &featureData, sizeof(featureData)))) {
      if (featureData.TypedUAVLoadAdditionalFormats) {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT desiredFormats = {
          DXGI_FORMAT_R11G11B10_FLOAT, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE
        };

        if (
          SUCCEEDED(gDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &desiredFormats, sizeof(desiredFormats)))
          && (desiredFormats.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0
        ) {
          g_bTypedUAVLoadSupport_R11G11B10_FLOAT = true;
        }

        desiredFormats.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

        if (
          SUCCEEDED(gDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &desiredFormats, sizeof(desiredFormats))) 
          && (desiredFormats.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0
        ) {
          g_bTypedUAVLoadSupport_R16G16B16A16_FLOAT = true;
        }
      }
    }

    g_CommandManager.Create(gDevice);

    InitializeCommonState();
    Display::Initialize();
  }
}