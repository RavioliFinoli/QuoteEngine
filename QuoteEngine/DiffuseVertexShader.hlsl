struct VS_IN
{
	float3 Pos : POSITION;
	float3 Nor : NORMAL;
	float2 UV : TEXCOORD;
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float4 PosW : POSITION;
	float4 NorW : NORMAL;
	float2 UV : TEXCOORD;
};


cbuffer PerModel : register(b0)
{
	row_major float4x4 WVP;
	row_major float4x4 W;
}

VS_OUT VS_main( VS_IN input )
{
	VS_OUT result;
	result.Pos = mul(float4(input.Pos, 1.0f), WVP);
	result.PosW = mul(float4(input.Pos, 1.0f), W);
	result.NorW = mul(float4(input.Nor, 0.0f), W); //TODO: inverse transpose
	result.UV = input.UV;

	return result;
}