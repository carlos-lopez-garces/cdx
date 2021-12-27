#include "texture.h"
#include "graphics.h"
#include "utils.h"

using namespace Graphics;

// Row pitch is the physical size in bytes of one row of a texture.
void Texture::Create2D(size_t rowPitchBytes, size_t width, size_t height, DXGI_FORMAT format, const void* initialData) {
  Destroy();

  mUsageState = D3D12_RESOURCE_STATE_COPY_DEST;
  mWidth = (uint32_t)width;
  mHeight = (uint32_t)height;
  mDepth = 1;

  D3D12_RESOURCE_DESC texDesc = {};
  texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  texDesc.Width = width;
  // Width is UINT64, but Height is UINT.
  texDesc.Height = (UINT)height;
  texDesc.DepthOrArraySize = 1;
  texDesc.MipLevels = 1;
  texDesc.Format = format;
  texDesc.SampleDesc.Count = 1;
  texDesc.SampleDesc.Quality = 0;
  texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

  D3D12_HEAP_PROPERTIES heapProperties;
  heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
  heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  heapProperties.CreationNodeMask = 1;
  heapProperties.VisibleNodeMask = 1;

  ASSERT_SUCCEEDED(gDevice->CreateCommittedResource(
    &heapProperties,
    D3D12_HEAP_FLAG_NONE,
    &texDesc,
    mUsageState,
    nullptr,
    MY_IID_PPV_ARGS(mpResource.ReleaseAndGetAddressOf())
  ));

  mpResource->SetName(L"Texture");

  D3D12_SUBRESOURCE_DATA texSubresource;
  texSubresource.pData = initialData;
  texSubresource.RowPitch = rowPitchBytes;
  texSubresource.SlicePitch = rowPitchBytes * height;
}