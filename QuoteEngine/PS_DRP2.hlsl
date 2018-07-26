Texture2D NormalTexture   : register(t0);
Texture2D DiffuseTexture  : register(t1);
Texture2D SpecularTexture : register(t2);
Texture2D PositionTexture : register(t3);

void GetAttributes
(
	in float2 screenPosition,
	out float3 normal,
	out float3 position,
	out float3 diffuse,
	out float3 specular,
	out float specularPower
)
{
	int3 sampledIndex = int3(screenPosition, 0);

	normal = NormalTexture.Load(sampledIndex.xyz).xyz;
	position = PositionTexture.Load(sampledIndex.xyz).xyz;
	diffuse = DiffuseTexture.Load(sampledIndex.xyz).xyz;
	float4 spec = SpecularTexture.Load(sampledIndex.xyz);

	specular = spec.xyz;
	specularPower = spec.w;
}

float ComputeDiffuseFactor(float3 lightVector, float3 normal)
{
	return dot(lightVector, normal);
}

cbuffer Camera : register(b0)
{
	float4 CamPos; //in world (ignore .w)
}

float4 PS_main(in float4 screenPos : SV_Position) : SV_TARGET0
{
	//Hard coded light
	float4 lightPosition = float4(0.0f, 50.0f, -6.0f, 1.0f);
	float4 lightColor = float4(1.0f, 1.0f, 1.0f, 1.0f);	  //.a is intensity

	float3 NorW;
	float3 PosW;
	float3 diffuse;
	float3 specular;
	float specularPower;
	GetAttributes(screenPos.xy, NorW, PosW, diffuse, specular, specularPower);
	//normalize normal
	NorW.xyz = normalize(NorW.xyz);

	float4 final = float4(0.0, 0.0, 0.0, 1.0);

	float3 lightPos = lightPosition.xyz;
	float4 lightCol = lightColor;
	//normalized vector from fragment in world space to light
	float3 lightVector = normalize(lightPos - PosW.xyz);

	//---------------------------
	//             Specular
	//---------------------------

	float3 N = NorW.xyz;
	float3 L = normalize(lightPos - PosW);
	float3 V = normalize(CamPos.xyz - PosW.xyz);
	float3 R = 2 * dot(N, L) * N - L;


	//float3 R = normalize((2 * dot(input.NorW.xyz, lightVector) * input.NorW.xyz) - lightVector);

	float specularFactor = pow(max(dot(V, R), 0.0f), specularPower);

	//---------------------------

	//cos(angle) between light vector and normal of fragment (since normalized this is just dot prod)
	float diffuseFactor = max(ComputeDiffuseFactor(lightVector, NorW.xyz), 0.0f);


	//initialize final with ambient as ambientColor*lightcol*fragmentCol*0.1
	float3 ambientColor = diffuse * 0.2; //Never pitch black color
	float4 thisColor = float4(ambientColor*lightCol*diffuse * 0.3f, 1.0f);

	//calculate color from this light
	thisColor = float4(((diffuseFactor * diffuse * lightCol.xyz)), 1.0f); // Diffuse factor(no attenuation)				//Specular factor

	if (specularFactor > 0.0)
		thisColor.xyz += float3(specular.xyz * specularFactor);

	final += float4(thisColor.xyz, 0.0);

	return float4(final.xyz, 1.0f);
};