#include "pch.h"
#include "ASSAO.h"
#include "Camera.h"
#include "Math/Common.h"
#include "Math/Matrix4.h"
#include "Math/Vector.h"
#include "CommandContext.h"
#include "BufferManager.h"
#include "PipelineState.h"

#include "CompiledShaders/ASSAOVS.h"
#include "CompiledShaders/ASSAODownsamplePS.h"
#include "CompiledShaders/ASSAOComputePS.h"
#include "CompiledShaders/ASSAOBlurPS.h"
#include "CompiledShaders/ASSAOUpsamplePS.h"

using namespace Math;

const float Pi = 3.141593f;
float ssaoRadius_world = 1.0f;
float ssaoMaxRadius_screen = 0.1f;
float ssaoContrast = 4.0f;

namespace ASSAO
{
  BoolVar Enable("Graphics/ASSAO/Enable", true);
}

namespace
{
    RootSignature s_RootSignature;
    GraphicsPSO s_DownsamplePSO(L"ASSAO: Downsample PSO");
    GraphicsPSO s_ComputePSO(L"ASSAO: Compute PSO");
    GraphicsPSO s_BlurPSO(L"ASSAO: Blur PSO");
    GraphicsPSO s_UpsamplePSO(L"ASSAO: Upsample PSO");
}

INLINE Vector3 PlaneSize(float distance, float fovY, float aspect) {
    Vector3 size;
    size.SetY(2.0f * distance * Tan(0.5f * fovY));
    size.SetX(aspect * size.GetY());
    return size;
}

void ASSAO::Initialize(void)
{
    s_RootSignature.Reset(4, 2);
    s_RootSignature.InitStaticSampler(0, Graphics::SamplerPointClampDesc);
    s_RootSignature.InitStaticSampler(1, Graphics::SamplerLinearClampDesc);
    s_RootSignature[0].InitAsConstantBuffer(0);
    s_RootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    s_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    s_RootSignature[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    s_RootSignature.Finalize(L"ASSAO");

    // g_pASSAOVS and g_pASSAODownsamplePS are compiled shaders that are compiled
    // at build time by Visual Studio's FX compiler. They are found in Build\x64\Debug\Output\Core\CompiledShaders
    // in the form of header files (included here at the top).
    s_DownsamplePSO.SetRootSignature(s_RootSignature);
    s_DownsamplePSO.SetVertexShader(g_pASSAOVS, sizeof(g_pASSAOVS));
    s_DownsamplePSO.SetPixelShader(g_pASSAODownsamplePS, sizeof(g_pASSAODownsamplePS));
    s_DownsamplePSO.SetRasterizerState(Graphics::RasterizerDefault);
    s_DownsamplePSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    s_DownsamplePSO.Finalize();

    s_ComputePSO.SetRootSignature(s_RootSignature);
    s_ComputePSO.SetVertexShader(g_pASSAOVS, sizeof(g_pASSAOVS));
    s_ComputePSO.SetPixelShader(g_pASSAOComputePS, sizeof(g_pASSAOComputePS));
    s_ComputePSO.SetRasterizerState(Graphics::RasterizerDefault);
    s_ComputePSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    s_ComputePSO.Finalize();

    s_BlurPSO.SetRootSignature(s_RootSignature);
    s_BlurPSO.SetVertexShader(g_pASSAOVS, sizeof(g_pASSAOVS));
    s_BlurPSO.SetPixelShader(g_pASSAOBlurPS, sizeof(g_pASSAOBlurPS));
    s_BlurPSO.SetRasterizerState(Graphics::RasterizerDefault);
    s_BlurPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    s_BlurPSO.Finalize();

    s_UpsamplePSO.SetRootSignature(s_RootSignature);
    s_UpsamplePSO.SetVertexShader(g_pASSAOVS, sizeof(g_pASSAOVS));
    s_UpsamplePSO.SetPixelShader(g_pASSAOUpsamplePS, sizeof(g_pASSAOUpsamplePS));
    s_UpsamplePSO.SetRasterizerState(Graphics::RasterizerDefault);
    s_UpsamplePSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    s_UpsamplePSO.Finalize();
}

void ASSAO::Render( GraphicsContext& GfxContext, const Camera& camera )
{
    // Guessing that screen size is that of the scene color buffer.
    uint32_t screenWidth = Graphics::g_SceneColorBuffer.GetWidth();
    uint32_t screenHeight = Graphics::g_SceneColorBuffer.GetHeight();

    const Matrix4& viewTransform = camera.GetViewMatrix();
    const Matrix4& projTransform = camera.GetProjMatrix();
    
    GfxContext.SetViewport(0.0f, 0.0f, screenWidth/2, screenHeight/2);

    // Downsample.
    {
        GfxContext.SetRenderTarget(Graphics::depth16RT_x4.GetRTV());
        GfxContext.SetPipelineState(s_DownsamplePSO);
        // TODO: This is a guess.
        GfxContext.SetDynamicDescriptors(0, 0, 1, &Graphics::g_SceneDepthBuffer.GetDepthSRV());

        __declspec(align(16)) struct DownsampleConstantBuffer
        {
            Vector3 pixelSize;
            Vector3 projParams;
        } cbuffer;
        cbuffer.pixelSize.SetX(2.0f / (float)screenWidth);
        cbuffer.pixelSize.SetY(2.0f / (float)screenHeight);
        // Entry (3,3), 0-based.
        cbuffer.projParams.SetX(projTransform.GetZ().GetZ());
        // Entry (4,3), 0-based.
        cbuffer.projParams.SetX(projTransform.GetW().GetZ());
        GfxContext.SetDynamicConstantBufferView(0, sizeof(cbuffer), &cbuffer);
        // TODO: 6?
        GfxContext.DrawIndexed(6, 0, 0);
    }

    // Computation.
    {
        GfxContext.SetRenderTarget(Graphics::ssaoRT_x4.GetRTV());
        GfxContext.SetPipelineState(s_ComputePSO);
        GfxContext.SetDynamicDescriptors(0, 0, 1, &Graphics::depth16RT_x4.GetSRV());
        GfxContext.SetDynamicDescriptors(1, 0, 1, &Graphics::gbufferNormalRT->GetSRV());
        
        __declspec(align(16)) struct ComputeConstantBuffer
        {
            Vector3 pixelSize;
            Vector3 nearPlaneSize_normalized;
            // TODO: 3x3?
            Matrix4 viewTransform;
            float aspect;
            float radius_world;
            float maxRadius_screen;
            float contrast;
        } cbuffer;
        cbuffer.pixelSize.SetX(2.0f/(float)screenWidth);
        cbuffer.pixelSize.SetY(2.0f/(float)screenHeight);
        cbuffer.nearPlaneSize_normalized = PlaneSize(1.0f, Pi/3.0f, (float)screenWidth/(float)screenHeight);
        cbuffer.viewTransform = viewTransform;
        cbuffer.aspect = (float)screenWidth / (float)screenHeight;
        // TODO: should increase with INSERT and decrease with DELETE.
        cbuffer.radius_world = ssaoRadius_world;
        cbuffer.maxRadius_screen = ssaoMaxRadius_screen;
        cbuffer.contrast = ssaoContrast;
        GfxContext.SetDynamicConstantBufferView(0, sizeof(cbuffer), &cbuffer);
        // TODO: 6?
        GfxContext.DrawIndexed(6, 0, 0);
    }

    // Blur X.
    {
        GfxContext.SetPipelineState(s_BlurPSO);
        GfxContext.SetDynamicDescriptors(0, 0, 1, &Graphics::depth16RT_x4.GetSRV());
        GfxContext.SetDynamicDescriptors(1, 0, 1, &Graphics::ssaoRT_x4.GetSRV());

        __declspec(align(16)) struct BlurConstantBuffer {
            Vector3 pixelOffset;
            Vector3 padding;
        } cbuffer;
        cbuffer.pixelOffset.SetX(2.0f/(float)screenWidth);
        cbuffer.pixelOffset.SetY(0.0f);
        GfxContext.SetDynamicConstantBufferView(0, sizeof(cbuffer), &cbuffer);
        // TODO: 6?
        GfxContext.DrawIndexed(6, 0, 0);
    }

    // Blur Y.
    {
        GfxContext.SetPipelineState(s_BlurPSO);
        GfxContext.SetDynamicDescriptors(0, 0, 1, &Graphics::depth16RT_x4.GetSRV());
        GfxContext.SetDynamicDescriptors(1, 0, 1, &Graphics::ssaoRT_x4.GetSRV());

        __declspec(align(16)) struct BlurConstantBuffer {
            Vector3 pixelOffset;
            Vector3 padding;
        } cbuffer;
        cbuffer.pixelOffset.SetX(0.0f);
        cbuffer.pixelOffset.SetY(2.0f/(float)screenHeight);
        GfxContext.SetDynamicConstantBufferView(0, sizeof(cbuffer), &cbuffer);
        // TODO: 6?
        GfxContext.DrawIndexed(6, 0, 0);
    }

    // Upsample.
    {
        GfxContext.SetPipelineState(s_UpsamplePSO);
        // TODO: This is a guess.
        GfxContext.SetDynamicDescriptors(0, 0, 1, &Graphics::g_SceneDepthBuffer.GetDepthSRV());
        GfxContext.SetDynamicDescriptors(1, 0, 1, &Graphics::depth16RT_x4.GetSRV());
        GfxContext.SetDynamicDescriptors(2, 0, 1, &Graphics::ssaoBlurRT_x4.GetSRV());

        __declspec(align(16)) struct UpsampleConstantBuffer
        {
            Vector3 pixelSize;
            Vector3 projParams;
        } cbuffer;
        cbuffer.pixelSize.SetX(1.0f / (float)screenWidth);
        cbuffer.pixelSize.SetY(1.0f / (float)screenHeight);
        // Entry (3,3), 0-based.
        cbuffer.projParams.SetX(projTransform.GetZ().GetZ());
        // Entry (4,3), 0-based.
        cbuffer.projParams.SetX(projTransform.GetW().GetZ());
        GfxContext.SetDynamicConstantBufferView(0, sizeof(cbuffer), &cbuffer);
        // TODO: 6?
        GfxContext.DrawIndexed(6, 0, 0);
    }
}

// void ASSAO::Render( GraphicsContext& GfxContext, const float* ProjMat, float NearClipDist, float FarClipDist )
// {

// }