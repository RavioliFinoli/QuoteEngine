struct PS_IN
{
	float4 Pos : SV_POSITION;
	float4 Nor : NORMAL;
	float2 UV : TEXCOORD0;
};

//no attenuation
float DiffuseFactor(float4 normal, float3 lightPos)
{
	return dot(normal.xyz, lightpos;
}

float4 main() : SV_TARGET
{
	float diffuseFactor = DiffuseFactor(normal, 0.0f, 0.0f, -2.0f); //no attenuation
	return diffuseFactor * float4(1.0f, 1.0f, 1.0f, 1.0f);
}