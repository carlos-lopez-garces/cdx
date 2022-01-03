SamplerState pointClampSampler: register(s0);
SamplerState linearClampSampler: register(s1);

Texture2D<float4> depthBufferTexture : register(t0);

cbuffer Constants: register(b0)
{
	float2 pixelSize;
	// x coordinate is the (3,3) entry of the projection matrix;
	// y coordinate is the (4,3) entry (row-major order).
	float2 projParams;
};

struct PSInput
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};

// Converts depth value in NDC space to z camera-space coordinate.
// See http://wojtsterna.blogspot.com/2013/11/recovering-camera-position-from-depth.html.
float DepthNDCToView(float depthNDC)
{
	return -projParams.y / (depthNDC + projParams.x);
}

float4 main(PSInput input) : SV_Target0
{
	float2 texCoord = input.texCoord + float2(-0.25f, -0.25f) * pixelSize;
	// Obtain the pixel's depth value from the depth buffer.
	float depthNDC = depthBufferTexture.Sample(pointClampSampler, texCoord).x;
	float linearDepth = DepthNDCToView(depthNDC);
	return linearDepth;
}