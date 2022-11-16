Texture2D objTextureY : register(t0);
Texture2D objTextureU : register(t1);
Texture2D objTextureV : register(t2);
SamplerState objSamplerState : register(s0);

struct PS_INPUT
{
	float4 inPosition : SV_POSITION;
	float2 inTexCoord : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float3 yuv;
	float4 Output;

	const float3 offset = { -0.0627451017, -0.501960814, -0.501960814 };
	const float3 Rcoeff = { 1.164,  0.000,  1.596 };
	const float3 Gcoeff = { 1.164, -0.391, -0.813 };
	const float3 Bcoeff = { 1.164,  2.018,  0.000 };

	yuv.x = objTextureY.Sample(objSamplerState, input.inTexCoord).r;
	//input.inTexCoord = input.inTexCoord / 2.0f;
	yuv.y = objTextureU.Sample(objSamplerState, input.inTexCoord).r;
	yuv.z = objTextureV.Sample(objSamplerState, input.inTexCoord).r;

	yuv += offset;
	Output.r = dot(yuv, Rcoeff);
	Output.g = dot(yuv, Gcoeff);
	Output.b = dot(yuv, Bcoeff);
	Output.a = 1.0f;
	return Output; // * input.color;
}
