struct VS_IN
{
	float3 Pos : POSITION;
	float3 Nor : NORMAL;
	float2 UV : TEXCOORD0;
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float4 Nor : NORMAL;
	float2 UV : TEXCOORD0;
};


cbuffer PerModel : register(b0)
{
	row_major float4x4 WVP;
}

VS_OUT main( VS_IN input )
{
	VS_OUT result;
	result.Pos = mul(input.Pos, WVP);
	result.Nor = input.Nor;
	result.UV = input.UV;

	return result;
}