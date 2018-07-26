Texture2D diffuseTexture : register(t0);
SamplerState smplr : register(s0);

struct MaterialAttributes
{
	float KsR, KsG, KsB, Ns;			//specular color + power
	float KdR, KdG, KdB, UseTexture;	//Diffuse color + useTexture 'boolean'
	float KaR, KaG, KaB, pad2;			//Ambient color
};

cbuffer Material : register(b0)
{
	MaterialAttributes matAttr;
}

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float4 PosW : POSITION;
	float4 NorW : NORMAL;
	float2 UV : TEXCOORD;
};

struct PSOutput
{
	float4 Normal    : SV_Target0;
	float4 Diffuse   : SV_Target1;
	float4 Specular  : SV_Target2;
	float4 Position  : SV_Target3;
};


PSOutput PS_main(in VS_OUT input)
{
	PSOutput output;

	//Normal ( SV_Target0.rgb )
	float3 NorW = normalize(input.NorW).xyz;
	output.Normal = float4(NorW, 0.0f);

	//Diffuse ( t2 )
	float3 diffuse = float3(0.0, 0.0, 0.0);
	if (matAttr.UseTexture > 0.0)
	{
		diffuse = diffuseTexture.Sample(smplr, float2(input.UV.x, 1.0 - input.UV.y)).rgb;
	}
	else
	{
		diffuse = float3(matAttr.KdR, matAttr.KdG, matAttr.KdB);
	}
	output.Diffuse = float4(diffuse, 1.0f);

	//Specular
	output.Specular = float4(matAttr.KsR, matAttr.KsG, matAttr.KsB, matAttr.Ns);

	output.Position = input.PosW;

	return output;
}