struct VSInput
{
	uint index: SV_VertexID;
};

struct VSOutput
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	if (input.index == 0)
	{
		output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
		output.texCoord = float2(0.0f, 1.0f);
	}
	else if (input.index == 1)
	{
		output.position = float4(1.0f, -1.0f, 0.0f, 1.0f);
		output.texCoord = float2(1.0f, 1.0f);
	}
	else if (input.index == 2)
	{
		output.position = float4(1.0f, 1.0f, 0.0f, 1.0f);
		output.texCoord = float2(1.0f, 0.0f);
	}
	else if (input.index == 3)
	{
		output.position = float4(-1.0f, 1.0f, 0.0f, 1.0f);
		output.texCoord = float2(0.0f, 0.0f);
	}

	return output;
}