Texture2D diffuse : register(t0);

SamplerState sampAni; //default sampler

struct PS_IN
{
	float4 Pos : SV_POSITION;
	float4 PosW : POSITION;
	float4 NorW : NORMAL;
	float2 UV : TEXCOORD;
};

struct MaterialAttributes
{
	float KsR, KsG, KsB, Ns;			//specular color + power
	float KdR, KdG, KdB, UseTexture;	//Diffuse color + useTexture 'boolean'
	float KaR, KaG, KaB, pad2;			//Ambient color
};

cbuffer PerFrame : register(b0)
{
	float3 CamPos;
}

cbuffer PerModel : register(b1)
{
	MaterialAttributes matAttr;
}
//no attenuation
float DiffuseFactor(float4 normal, float3 lightPos)
{
	return dot(lightPos, normal.xyz);
}

float4 PS_main(PS_IN input) : SV_TARGET
{
	input.NorW.xyz = normalize(input.NorW.xyz);

	float4 sampledColor = float4(0.0f, 0.0, 0.0, 1.0f);
	//get color of fragment from texture (if model uses texture)
	if (matAttr.UseTexture > 0.0)
	{
		sampledColor.xyz = diffuse.Sample(sampAni, float2(input.UV.x, 1.0f - input.UV.y)).xyz;
	}
	else //else get from material
	{
		sampledColor.xyz = float3(matAttr.KdR, matAttr.KdG, matAttr.KdB);
	}

	//Ambient
	float3 ambientColor = max(float3(matAttr.KaR, matAttr.KaG, matAttr.KaB), float3(0.1, 0.1, 0.1)); //Never pitch black color
	//-------

	//Specular
	float3 lightPosition = float3(0.0f, 2.0f, -12.0f);
	float3 N = input.NorW.xyz;
	float3 L = normalize(lightPosition - input.PosW);
	float3 V = normalize(CamPos.xyz - input.PosW.xyz);
	float3 R = 2 * dot(N, L) * N - L;
	float specularFactor = 0.0f;
	if (matAttr.Ns > 0.0)
	{
		specularFactor = pow(max(dot(V, R), 0.0f), matAttr.Ns);
	}

	float4 specularColor = float4(matAttr.KsR, matAttr.KsG, matAttr.KsB, 1.0);
	//--------

	//Diffuse
	input.NorW.xyz = normalize(input.NorW.xyz);
	float3 lightVector = normalize(lightPosition - input.PosW.xyz);
	float diffuseFactor = max(DiffuseFactor(input.NorW, lightVector), 0.0); //no attenuation
	//-------

	float4 ambient = float4(ambientColor, 1.0f); //hm

	//Init final with ambient
	float4 final = float4(ambient.xyz, 1.0);
	//Add diffuse and specular
	final = diffuseFactor * sampledColor + (specularFactor * specularColor);
	
	return final;
}