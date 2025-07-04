
struct MATERIAL
{
    float4 m_cAmbient;
    float4 m_cDiffuse;
    float4 m_cSpecular; //a = power
    float4 m_cEmissive;
	
	
    //matrix gmtxTexture;
    //int2 gi2TextureTiling;
    //float2 gf2TextureOffset;
    //float gf1Deltatime;
};
struct SPRITEANIMATIONMATERIAL
{
    float4 m_cAmbient;
    float4 m_cDiffuse;
    float4 m_cSpecular; //a = power
    float4 m_cEmissive;
	
    matrix				gmtxTexture;
    int2				gi2TextureTiling;
    float2				gf2TextureOffset;
    float				gf1Deltatime;

};
cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
   // matrix		gmtxidentityView : packoffset(c12);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix		gmtxGameObject : packoffset(c0);
	MATERIAL	gMaterial : packoffset(c4);
	uint		gnTexturesMask : packoffset(c8);
};

cbuffer cbChildGameObjectInfo : register(b3)
{
    matrix gmtxChildGameObject : packoffset(c0);//worldmatrix
    SPRITEANIMATIONMATERIAL gChildMaterial : packoffset(c4);
};

cbuffer cbTimerinfo : register(b5)
{
    float Timer;
    float Alpha;
};

#include "Light.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#define _WITH_VERTEX_LIGHTING

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

#define _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS

#ifdef _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);
#else
Texture2D gtxtStandardTextures[7] : register(t6);
#endif

SamplerState gWrapSamplerState : register(s0);
SamplerState gClampSamplerState : register(s1);

struct VS_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = (float3)mul(float4(input.tangent, 1.0f), gmtxGameObject);
	output.bitangentW = (float3)mul(float4(input.bitangent, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

#ifdef _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gWrapSamplerState, input.uv);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gWrapSamplerState, input.uv);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gWrapSamplerState, input.uv);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gWrapSamplerState, input.uv);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gWrapSamplerState, input.uv);
#else
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtStandardTextures[0].Sample(gWrapSamplerState, input.uv);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtStandardTextures[1].Sample(gWrapSamplerState, input.uv);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtStandardTextures[2].Sample(gWrapSamplerState, input.uv);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtStandardTextures[3].Sample(gWrapSamplerState, input.uv);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtStandardTextures[4].Sample(gWrapSamplerState, input.uv);
#endif

	float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 cColor = cAlbedoColor + cSpecularColor + cEmissionColor;
	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3 normalW = input.normalW;
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] �� [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
		cIllumination = Lighting(input.positionW, normalW);
		cColor = lerp(cColor, cIllumination, 0.5f);
	}

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionL = input.position;

	return(output);
}

TextureCube gtxtSkyCubeTexture : register(t13);

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtSkyCubeTexture.Sample(gClampSamplerState, input.positionL);

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SPRITE_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_SPRITE_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_SPRITE_TEXTURED_OUTPUT VSTextured(VS_SPRITE_TEXTURED_INPUT input)
{
	VS_SPRITE_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

/*
float4 PSTextured(VS_SPRITE_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float4 cColor;
	if (nPrimitiveID < 2)
		cColor = gtxtTextures[0].Sample(gWrapSamplerState, input.uv);
	else if (nPrimitiveID < 4)
		cColor = gtxtTextures[1].Sample(gWrapSamplerState, input.uv);
	else if (nPrimitiveID < 6)
		cColor = gtxtTextures[2].Sample(gWrapSamplerState, input.uv);
	else if (nPrimitiveID < 8)
		cColor = gtxtTextures[3].Sample(gWrapSamplerState, input.uv);
	else if (nPrimitiveID < 10)
		cColor = gtxtTextures[4].Sample(gWrapSamplerState, input.uv);
	else
		cColor = gtxtTextures[5].Sample(gWrapSamplerState, input.uv);
	float4 cColor = gtxtTextures[NonUniformResourceIndex(nPrimitiveID/2)].Sample(gWrapSamplerState, input.uv);

	return(cColor);
}
*/
Texture2D gtxtTexture : register(t0);


struct VS_TEXTURED_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_TEXTURED_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSBillBoardTextured(VS_TEXTURED_INPUT input)
{
    VS_TEXTURED_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxChildGameObject), gmtxView), gmtxProjection);
    output.uv = input.uv;
   // output.uv.y += Timer;
    return (output);
}

float4 PSBillBoardTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtTexture.Sample(gWrapSamplerState, input.uv);
    
    //cColor.a = Alpha;
	
    return (cColor);
}

VS_TEXTURED_OUTPUT VSDebugbox(VS_TEXTURED_INPUT input)
{
    VS_TEXTURED_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxChildGameObject), gmtxView), gmtxProjection);
    output.uv = input.uv;
   // output.uv.y += Timer;
    return (output);
}

float4 PSDebugbox(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtTexture.Sample(gWrapSamplerState, input.uv);
    
    cColor = gMaterial.m_cAmbient;
	
    return (cColor);
}

VS_TEXTURED_OUTPUT VSSpriteAnimation(VS_TEXTURED_INPUT input)
{
    VS_TEXTURED_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxChildGameObject), gmtxView), gmtxProjection);
	output.uv = mul(float3(input.uv, 1.0f), (float3x3) (gChildMaterial.gmtxTexture)).xy;
    //output.uv = input.uv;
    return (output);
}
float4 PSSpriteAnimation(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtTexture.Sample(gWrapSamplerState, input.uv);
    
    //cColor.a = Alpha;
	
    return (cColor);
}


VS_TEXTURED_OUTPUT VSUITextured(VS_TEXTURED_INPUT input)
{
    VS_TEXTURED_OUTPUT output;

    output.position = mul(mul(float4(input.position, 1.0f), gmtxChildGameObject), gmtxProjection);
    output.uv = input.uv;
   // output.uv.y += Timer;
    return (output);
}

float4 PSUITextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtTexture.Sample(gWrapSamplerState, input.uv);
   // if (cColor.x > 0.99)
   // {
		 //cColor.a = 0;
   // }
    //cColor.a = Alpha;
	
        return (cColor);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTerrainBaseTexture : register(t1);

struct VS_TERRAIN_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

struct VS_TERRAIN_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
    VS_TERRAIN_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxChildGameObject), gmtxView), gmtxProjection);
    output.color = input.color;
    output.uv = input.uv;

    return (output);
}

VS_TERRAIN_OUTPUT VSWaterTerrain(VS_TERRAIN_INPUT input)
{
    VS_TERRAIN_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxChildGameObject), gmtxView), gmtxProjection);
    output.color = input.color;
    output.uv = input.uv;
    output.uv.y += Timer/100;
    return (output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
    float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gWrapSamplerState, input.uv);
    float4 cColor =  cBaseTexColor;
 //   cColor.a = 0.3;
    return (cColor);
}

float4 PSWaterTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
    float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gWrapSamplerState, input.uv);
    float4 cColor = cBaseTexColor;
    cColor.a = 0.3;
    return (cColor);
}
