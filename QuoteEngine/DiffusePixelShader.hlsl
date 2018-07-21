struct PS_IN
{
	float4 Pos : SV_POSITION;
	float4 PosW : POSITION;
	float4 NorW : NORMAL;
	float2 UV : TEXCOORD;
};

//no attenuation
float DiffuseFactor(float4 normal, float3 lightPos)
{
	return dot(lightPos, normal.xyz);
}

float4 PS_main(PS_IN input) : SV_TARGET
{
	input.NorW.xyz = normalize(input.NorW.xyz);
	float3 lightPosition = float3(0.0f, 0.0f, -2.0f);
	float3 lightVector = normalize(lightPosition - input.PosW.xyz);
	float diffuseFactor = DiffuseFactor(input.NorW, lightVector); //no attenuation
	return diffuseFactor * float4(1.0f, 1.0f, 1.0f, 1.0f) + float4(0.15f, 0.15f, 0.15f, 0.0f);
}