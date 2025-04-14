// MatrixBuffer: 변환 행렬 관리
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

struct VS_INPUT
{
    float3 position : POSITION; // 버텍스 위치
    float3 normal : NORMAL; // 버텍스 노멀
    float3 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR; // 버텍스 색상
    int materialIndex : MATERIAL_INDEX;
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // 클립 공간으로 변환된 화면 좌표
    float3 worldPos : TEXCOORD0; // 월드 공간 위치 (조명용)
    float4 color : COLOR; // 버텍스 컬러 또는 머티리얼 베이스 컬러
    float normalFlag : TEXCOORD1; // 노멀 유효 플래그 (1.0 또는 0.0)
    float2 texcoord : TEXCOORD2; // UV 좌표
    int materialIndex : MATERIAL_INDEX; // 머티리얼 인덱스
    float3x3 TBN : TANGENT; // 탄젠트 공간 (tangent, bitangent, normal)
    float3 GouraudColor : COLOR2;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    
    output.materialIndex = input.materialIndex;
    
    float4 worldPosition = mul(float4(input.position, 1), Model);
    
    output.worldPos = worldPosition.xyz;
    
    float4 viewPosition = mul(worldPosition, View);
    
    output.position = mul(viewPosition, Projection);
    
    output.color = input.color;
  
    float3 WorldNormal = normalize(mul(input.normal, (float3x3) MInverseTranspose));
    
    output.texcoord = input.texcoord;
    
    float3 WorldTangent = normalize(mul(input.tangent, (float3x3) MInverseTranspose));
    
    float3 WorldBitangent = cross(WorldTangent, WorldNormal);
    
    float3x3 WorldTBN =
    {
    WorldTangent.x, WorldTangent.y, WorldTangent.z, // column 0
    WorldBitangent.x, WorldBitangent.y, WorldBitangent.z, // column 1
    WorldNormal.x, WorldNormal.y, WorldNormal.z            // column 2
    };
    
    output.TBN = WorldTBN;
    
    output.normalFlag = 1.0f;
    
    return output;
}
