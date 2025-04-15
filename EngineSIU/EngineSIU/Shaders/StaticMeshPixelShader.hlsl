// staticMeshPixelShader.hlsl

Texture2D Textures : register(t0);
Texture2D NormalTextures : register(t1);
Texture2D MetallicTextures : register(t2);
Texture2D SpecularTextures : register(t3);
Texture2D EmessiveTextures : register(t4);
SamplerState Sampler : register(s0);


cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 Model;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};
cbuffer CameraConstants : register(b1)
{
    row_major float4x4 View;
    row_major float4x4 Projection;
    float3 CameraPosition;
    float pad;
};

struct FMaterial
{
    float3 DiffuseColor;
    float TransparencyScalar;
    
    float3 AmbientColor;
    float DensityScalar;
    
    float3 SpecularColor;
    float SpecularScalar;
    
    float3 EmissiveColor;
    int TextureInfo; // 0b0001: Diffuse, 0b0010: Ambient, 0b0100: Specular, 0b1000: Bump,0b10000: metallic, 0b100000: Emissive
};
cbuffer MaterialConstants : register(b3)
{
    FMaterial Material;
}

cbuffer SubMeshConstants : register(b5)
{
    bool IsSelectedSubMesh;
    float3 SubMeshPad0;
}

cbuffer TextureConstants : register(b6)
{
    float2 UVOffset;
    float2 TexturePad0;
}

#include "Light.hlsl"



struct PS_INPUT
{
    float4 position : SV_POSITION; // 클립 공간 화면 좌표
    float3 worldPos : TEXCOORD0; // 월드 공간 위치
    float4 color : COLOR; // 전달된 베이스 컬러
    float normalFlag : TEXCOORD1; // 노멀 유효 플래그
    float2 texcoord : TEXCOORD2; // UV 좌표
    int materialIndex : MATERIAL_INDEX; // 머티리얼 인덱스
    float3x3 TBN : TANGENT; // 탄젠트 공간 (tangent, bitangent, normal)
    float4 GouraudColor : COLOR2; // 구로우드 색상
    float4 GouraudSpecular : COLOR3; // 구로우드 스펙큘러 색상
};

struct PS_OUTPUT
{
    float4 color : SV_Target0;
    float4 UUID : SV_Target1;
};

float3 GetNormalFromMap(PS_INPUT input)
{
    // 노말 맵에서 RGB 가져오기 (값 범위: [0, 1])
    float3 normalMap = NormalTextures.Sample(Sampler, input.texcoord).rgb;

    // [-1, 1] 범위로 변환
    normalMap = normalMap * 2.0f - 1.0f;

    // 노말 맵의 노말을 월드 공간으로 변환
    return normalize(mul(normalMap, input.TBN));
}


PS_OUTPUT mainPS(PS_INPUT input)
{
    PS_OUTPUT output;
    output.UUID = UUID;

    // 기본 색상 처리 (알베도)
    float3 albedo = Textures.Sample(Sampler, input.texcoord).rgb;
    bool HasDiffuseTexture = (Material.TextureInfo & 0x1) != 0;
    bool HasSpecularTexture = (Material.TextureInfo & 0x4) != 0;  
    bool HasBumpTexture = (Material.TextureInfo & 0x8) != 0;
    bool HasMetallicTexture = (Material.TextureInfo & 0x10) != 0;
    bool HasEmissiveTexture = (Material.TextureInfo & 0x20) != 0;
    
    
    float3 baseColor = HasDiffuseTexture ? albedo : Material.DiffuseColor;
    float3 normalWS = input.TBN[2];
    
    
    if (HasBumpTexture)
    {
        normalWS = GetNormalFromMap(input);
    }

#if LIGHTING_MODEL_UNLIT
    output.color = float4(baseColor, 1.0f);

#elif LIGHTING_MODEL_NORMAL
    output.color = float4(normalWS * 0.5f + 0.5f, 1.0f);

#elif LIGHTING_MODEL_GOURAUD
{
    float3 gouraudDiffuse = input.GouraudColor.rgb * baseColor;
    float3 gouraudSpecular = input.GouraudSpecular.rgb;

    float3 litColor = gouraudDiffuse + gouraudSpecular;

    // Emissive 적용
    float3 emissive = Material.EmissiveColor;
    if (HasEmissiveTexture)
    {
        emissive += EmessiveTextures.Sample(Sampler, input.texcoord).rgb;
    }

    output.color = float4(litColor + emissive, 1.0f);
}



#else 
    LightingResult lighting = Lighting(input.worldPos, normalWS);

    float specularFactor = Material.SpecularScalar;
    if (HasSpecularTexture)
    {
        float texSpec = SpecularTextures.Sample(Sampler, input.texcoord).r;
        specularFactor *= texSpec;
    }

    float3 lightRgb = lighting.Diffuse + lighting.Specular * specularFactor;

    // Metallic 텍스처 적용
    float metallic = 0.0f;
    if (HasMetallicTexture)
    {
        metallic = MetallicTextures.Sample(Sampler, input.texcoord).r;
    }

    // Emissive 텍스처 적용
    float3 emissive = Material.EmissiveColor;
    if (HasEmissiveTexture)
    {
        emissive += EmessiveTextures.Sample(Sampler, input.texcoord).rgb;
    }

    //metalic 적용
    float3 litColor = lerp(baseColor * lightRgb, lightRgb, metallic);
    litColor += emissive;

    output.color = float4(litColor, 1.0f);
#endif

    if (isSelected)
    {
        output.color += float4(0.02f, 0.02f, 0.02f, 0.0f);
    }

    return output;
}


