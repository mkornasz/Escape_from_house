Texture2D diffuseMap : register(t0);
Texture2D specularMap : register(t1);
SamplerState texSampler : register(s0);

cbuffer vs_projBuffer : register(b0)
{
	matrix proj;
}

cbuffer vs_viewBuffer : register(b1)
{
	matrix view;
}

cbuffer vs_modelBuffer : register(b2)
{
	matrix model;
	matrix modelit;
}

cbuffer ps_materialBuffer : register(b0)
{
	struct Material
	{
		float4 diffuseColor;
		float4 specular;
	} material;
}

struct VS_INPUT
{
	float3 pos : POSITION0;
	float3 norm : NORMAL0;
	float2 tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float3 norm : NORMAL;
	float2 tex : TEXCOORD0;
	float3 viewVec : TEXCOORD1;
	float3 lightVec1 : TEXCOORD2;
	float3 lightVec2 : TEXCOORD3;
};

static const float4 lightpos1 = float4(0, 2.2f, 0, 1.0f);
static const float4 lightpos2 = float4(-50.0f, 100.0f, 50.0f, 1.0f);

PS_INPUT VS_Main( VS_INPUT i )
{
	PS_INPUT o = (PS_INPUT)0;
	float4 viewPos = float4(i.pos, 1.0f);
	viewPos = mul(model, viewPos);
	viewPos = mul(view, viewPos);
	o.pos = mul(proj, viewPos);
	o.norm = normalize(mul(view, mul(model, float4(i.norm, 0.0f))).xyz);
	o.viewVec = normalize(-viewPos.xyz);
	o.lightVec1 = normalize((mul(view, lightpos1) - viewPos).xyz);
	o.lightVec2 = normalize((mul(view, lightpos2) - viewPos).xyz);
	o.tex = i.tex;
	return o;
}

static const float ka = 0.2;
static const float3 light1Color = float3(0.8f, 0.76f, 0.54f);
static const float3 light2Color = float3(0.8f, 0.8f, 0.8f);
float4 PS_Main( PS_INPUT i ) : SV_TARGET
{
	float3 viewVec = normalize(i.viewVec);
	float3 normal = normalize(i.norm);
	float3 lightVec = normalize(i.lightVec1);
	float3 halfVec = normalize(viewVec + lightVec);
	//float4 diffuseColor = material.diffuseColor;
	float4 diffuseColor = diffuseMap.Sample(texSampler, i.tex);
	float4 specularColor = float4(specularMap.Sample(texSampler, i.tex).xyz , material.specular.a);
	float3 color = diffuseColor.xyz*ka;
	color += light1Color*diffuseColor.xyz * saturate(dot(normal, lightVec)) +
			 light1Color*specularColor.xyz * pow(saturate(dot(normal, halfVec)), specularColor.a);
	lightVec = normalize(i.lightVec2);
	halfVec = normalize(viewVec + lightVec);
	color += light2Color*diffuseColor.xyz * saturate(dot(normal, lightVec)) +
			 light2Color*specularColor.xyz * pow(saturate(dot(normal, halfVec)), specularColor.a);
	return float4(saturate(color), diffuseColor.a);
	//return diffuseColor;
	//return specularColor;
	//return float4(-normal, 1.0f);
}