SamplerState pointClampSampler: register(s0);
SamplerState linearClampSampler: register(s1);

Texture2D<float4> depth16Texture: register(t0);
Texture2D<float4> gbufferNormalTexture: register(t1);

#define PI 3.1415f
#define TWO_PI 2.0f * PI
#define GOLDEN_ANGLE 2.4f
#define SAMPLES_COUNT 16

static float2 alchemySpiralOffsets16[16] = {
	float2(0.19509f, 0.980785f),
	float2(-0.55557f, -0.83147f),
	float2(0.831469f, 0.555571f),
	float2(-0.980785f, -0.195091f),
	float2(0.980785f, -0.19509f),
	float2(-0.83147f, 0.555569f),
	float2(0.555571f, -0.831469f),
	float2(-0.195092f, 0.980785f),
	float2(-0.195089f, -0.980786f),
	float2(0.555569f, 0.83147f),
	float2(-0.831469f, -0.555572f),
	float2(0.980785f, 0.195092f),
	float2(-0.980786f, 0.195088f),
	float2(0.831471f, -0.555568f),
	float2(-0.555572f, 0.831468f),
	float2(0.195093f, -0.980785f)
};

cbuffer Constants: register(b0)
{
	float2 pixelSize;
	float2 nearPlaneSize_normalized;
	float4x4 viewTransform;
	float aspect;
	float radius_world;
	float maxRadius_screen;
	float contrast;
}

struct PSInput
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};

// Reconstruct the pixel's position in view space from its depth value in the depth buffer.
float3 Position_View(float2 texCoord)
{
	float3 position = float3(texCoord, -1.0f);
	position.xy = position.xy - 0.5f;
	position.y *= -1.0f;
	position.xy *= nearPlaneSize_normalized;
	position *= -depth16Texture.Sample(pointClampSampler, texCoord).x;
	return position;
}

// Rotates one sample point.
float2 RotatePoint(float2 pt, float angle)
{
	float sine, cosine;
	sincos(angle, sine, cosine);

	float2 rotatedPoint;
	rotatedPoint.x = cosine * pt.x - sine * pt.y;
	rotatedPoint.y = sine * pt.x + cosine * pt.y;

	return rotatedPoint;
}

// Computes the ith Alchemy spiral sample.
float2 AlchemySpiralOffset(int sampleIndex, float phi)
{
	float tau = 7.0f;
	float alpha = float(sampleIndex + 0.5f) / SAMPLES_COUNT;
	float theta = TWO_PI * alpha * tau + phi;
	float sine, cosine;
	sincos(theta, sine, cosine);
	return float2(cosine, sine);
}

float AlchemyNoise(int2 positionScreen)
{
	return 30.0f * (positionScreen.x ^ positionScreen.y) + 10.0f * positionScreen.x * positionScreen.y;
}

float4 main(PSInput input) : SV_Target0
{
	float2 texCoord00 = input.texCoord + float2(-0.25f, -0.25f) * pixelSize;
	float3 position = Position_View(input.texCoord);
	float3 normal = gbufferNormalTexture.Sample(pointClampSampler, texCoord00).xyz;
	normal = 2.0f * normal - 1.0f;
	normal = mul((float3x3)viewTransform, normal);

	float alchemyNoise = AlchemyNoise((int2)input.position.xy);
	float2 radiusScreen = radius_world / position.z;
	radiusScreen = min(radiusScreen, maxRadius_screen);
	radiusScreen.y *= aspect;

	float ao = 0.0f;
	for (int i = 0; i < SAMPLES_COUNT; i++)
	{
		float2 sampleOffset = 0.0f;
		sampleOffset = AlchemySpiralOffset(i, alchemyNoise);

		float2 sampleTexCoord = input.texCoord + radiusScreen * sampleOffset;
		float3 samplePosition = Position_View(sampleTexCoord);
		float3 v = samplePosition - position;
		ao += max(0.0f, dot(v, normal) + 0.002f * position.z) / (dot(v, v) + 0.001f);
	}

	ao = saturate(ao / SAMPLES_COUNT);
	ao = 1.0f - ao;
	ao = pow(ao, contrast);

	return ao;
}
