// staticMeshPixelShader.hlsl

Texture2D Textures : register(t0);
Texture2D NormalTextures : register(t1);
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
    int TextureInfo; // 0b0001: Diffuse, 0b0010: Ambient, 0b0100: Specular, 0b1000: Bump
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

    float3 albedo = Textures.Sample(Sampler, input.texcoord).rgb;
    float3 matDiffuse = Material.DiffuseColor.rgb;
    bool hasTexture = any(albedo != float3(0, 0, 0));
    float3 baseColor = hasTexture ? albedo : matDiffuse;
    
    bool HasDiffuseTexture = (Material.TextureInfo & 0x1) != 0;
    bool HasBumpTexture = (Material.TextureInfo & 0x8) != 0;

    float3 normalWS = input.TBN[2].xyz;

    if (HasBumpTexture) // 노말맵이 유효한 경우
    {
        normalWS = GetNormalFromMap(input);
    }

#if LIGHTING_MODEL_UNLIT
    output.color = float4(baseColor, 1);
#elif LIGHTING_MODEL_NORMAL
    output.color = float4(normalWS*0.5+0.5, 1);
#elif LIGHTING_MODEL_GOURAUD
    output.color = float4(input.GouraudColor.xyz * baseColor , 1);
#else  
    float3 lightRgb = Lighting(input.worldPos, normalWS).rgb;
    float3 litColor = baseColor * lightRgb;
    output.color = float4(litColor, 1);
#endif

    if (isSelected)
    {
        output.color += float4(0.02, 0.02, 0.02, 1);
    }

    return output;
}