struct VS_IN
{
	float3 Pos : POSITION;
	float3 Color : COLOR;
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float3 Color : COLOR;
};

cbuffer PerModel : register(b0)
{
	row_major float4x4 WVP;
}

//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	output.Pos = mul(WVP, float4(input.Pos, 1.0f));
	//output.Pos = float4(input.Pos, 1.0f);
	output.Color = input.Color;

	return output;
}