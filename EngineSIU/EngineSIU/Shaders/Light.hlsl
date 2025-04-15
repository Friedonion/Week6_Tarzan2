// light.hlsl

#define MAX_LIGHTS 16 

#define DIR_LIGHT           1
#define POINT_LIGHT         2
#define SPOT_LIGHT          3

struct LIGHT
{
    float3 m_cDiffuse;
    float pad2;

    float3 m_cSpecular; // Specular Color
    float pad3;

    float3 m_vPosition; // position of light (used in point, spot)
    float m_fInnerDegree; // spotlight inner radius in degree

    float3 m_vDirection;    // direction (used in directional, spot)
    float m_fOuterDegree; // spotlight outer radius in degree

    float m_fAttenuation; // 거리 기반 감쇠 계수
    int m_bEnable;
    int m_nType;
    float m_fIntensity; // 광원 강도
    
    float m_fAttRadius; // 감쇠 반경 (Attenuation Radius)
    float3 LightPad;
};

cbuffer cbLights : register(b2)
{
    LIGHT gLights[MAX_LIGHTS];
    float4 gcGlobalAmbientLight;
    int gnLights;
    float3 padCB;
};

float GouraudLightingModel()
{
    return 0;
}

float LambertLightingModel(float3 vToLight, float3 vNormal)
{
    return saturate(dot(vNormal, vToLight));
}

float BlinnPhongLightingModel(float3 vToLight, float3 vPosition, float3 vNormal, float fSpecularScalar)
{
    float3 vView = normalize(CameraPosition-vPosition);
    float3 vHalf = normalize(vToLight + vView);

    float ndl = dot(vNormal, vToLight);
    float ndh = dot(vNormal, vHalf);

    if (ndl <= 0.0f || ndh <= 0.0f)
        return 0.0f;

    return pow(ndh, fSpecularScalar);
}


float4 CalculateDirLight(int nIndex, float3 vPosition, float3 vNormal)
{
    float3 vToLight = normalize(-gLights[nIndex].m_vDirection);
    float3 vView = normalize(CameraPosition - vPosition);
    
    float3 ambientLight = gcGlobalAmbientLight * Material.AmbientColor.rgb;
    float3 lit = ambientLight;
    
    if (dot(vNormal, vToLight) < 0.0f)
        return float4(lit, 1.0f);

    #if LIGHTING_MODEL_GOURAUD ||LIGHTING_MODEL_BLINNPHONG
    float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f)  ? float3(1,1,1)  : Material.DiffuseColor;
    lit = ambientLight +
        gLights[nIndex].m_cDiffuse.rgb * LambertLightingModel(vToLight, vNormal) * safeDiffuse +
        gLights[nIndex].m_cSpecular.rgb * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
#elif LIGHTING_MODEL_LAMBERT
    // float4 baseColor = hasTexture ? albedo : float4(1, 1, 1, 1);
    float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f)  ? float3(1,1,1)  : Material.DiffuseColor;
    lit = ambientLight + gLights[nIndex].m_cDiffuse.rgb * LambertLightingModel(vToLight, vNormal) * safeDiffuse;
#elif LIGHTING_MODEL_SPECULAR
    lit = ambientLight + gLights[nIndex].m_cSpecular.rgb * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;

#endif
    
    return float4(lit * gLights[nIndex].m_fIntensity, 1.0);
}

float4 CalculatePointLight(int nIndex, float3 vPosition, float3 vNormal)
{
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);

    // 감쇠 반경을 벗어나면 기여하지 않음
    if (fDistance > gLights[nIndex].m_fAttRadius)
        return float4(0.0f, 0.0f, 0.0f, 1.0f);

    if (dot(vNormal, vToLight) < 0.0f)
        return float4(0.0f, 0.0f, 0.0f, 1.0f);

    vToLight /= fDistance; // 정규화    
    float3 ambientLight = gcGlobalAmbientLight * Material.AmbientColor.rgb;
    float3 lit;

    #if LIGHTING_MODEL_GOURAUD || LIGHTING_MODEL_BLINNPHONG
        float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f)  ? float3(1,1,1)  : Material.DiffuseColor;
    lit = ambientLight +
        gLights[nIndex].m_cDiffuse.rgb * LambertLightingModel(vToLight, vNormal) * safeDiffuse +
        gLights[nIndex].m_cSpecular.rgb * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
#elif LIGHTING_MODEL_LAMBERT
    // float4 baseColor = hasTexture ? albedo : float4(1, 1, 1, 1);
    float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f)  ? float3(1,1,1)  : Material.DiffuseColor;
    lit = ambientLight + gLights[nIndex].m_cDiffuse.rgb * LambertLightingModel(vToLight, vNormal) * safeDiffuse;
#elif LIGHTING_MODEL_SPECULAR
    lit = ambientLight + gLights[nIndex].m_cSpecular.rgb * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
#endif

    float normalizedRadius = fDistance / gLights[nIndex].m_fAttRadius;
    float distanceAttenuation = 1.0f / (1.0f + gLights[nIndex].m_fAttenuation * fDistance * fDistance);
    float radiusAttenuation = 1 - normalizedRadius * normalizedRadius;
    float fAttenuationFactor = distanceAttenuation * radiusAttenuation;

    return float4(lit * fAttenuationFactor * gLights[nIndex].m_fIntensity, 1.0f);
}

float4 CalculateSpotLight(int nIndex, float3 vPosition, float3 vNormal)
{
    float3 vToLight = normalize(gLights[nIndex].m_vPosition - vPosition);
    float3 vLightDir = normalize(gLights[nIndex].m_vDirection);
    float fDistance = length(gLights[nIndex].m_vPosition - vPosition);
    
    // 감쇠 반경을 벗어나면 기여하지 않음
    if (fDistance > gLights[nIndex].m_fAttRadius)
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (dot(vNormal, vToLight) < 0.0f)
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    float fCos = dot(vToLight, -vLightDir);
    float fCosInner = cos(radians(gLights[nIndex].m_fInnerDegree/2));
    float fCosOuter = cos(radians(gLights[nIndex].m_fOuterDegree/2));
    float3 lit;
    float3 ambientLight = gcGlobalAmbientLight * Material.AmbientColor.rgb;

#if LIGHTING_MODEL_GOURAUD || LIGHTING_MODEL_BLINNPHONG 
        float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f)  ? float3(1,1,1)  : Material.DiffuseColor;
    lit = ambientLight +
        gLights[nIndex].m_cDiffuse.rgb * LambertLightingModel(vToLight, vNormal) * safeDiffuse +
        gLights[nIndex].m_cSpecular.rgb * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
#elif LIGHTING_MODEL_LAMBERT
    // float4 baseColor = hasTexture ? albedo : float4(1, 1, 1, 1);
    float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f)  ? float3(1,1,1)  : Material.DiffuseColor;
    lit = ambientLight + gLights[nIndex].m_cDiffuse.rgb * LambertLightingModel(vToLight, vNormal) * safeDiffuse;
#elif LIGHTING_MODEL_SPECULAR
    lit = ambientLight + gLights[nIndex].m_cSpecular.rgb * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
#endif
    
    float normalizedRadius = fDistance / gLights[nIndex].m_fAttRadius;
    float distanceAttenuation = 1.0f / (1.0f + gLights[nIndex].m_fAttenuation * fDistance * fDistance);
    float radiusAttenuation = 1 - normalizedRadius * normalizedRadius;
    float fSpotAttenuation = 1 - saturate((fCos - fCosOuter) / (fCosInner - fCosOuter));
    float fAttenuationFactor = fSpotAttenuation * radiusAttenuation * distanceAttenuation;
    return float4(lit * fAttenuationFactor * gLights[nIndex].m_fIntensity, 1.0f);
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    [unroll(MAX_LIGHTS)]
    for (int i = 0; i < gnLights; i++)
    {
        if (gLights[i].m_bEnable)
        {
            if (gLights[i].m_nType == DIR_LIGHT)
            {
                cColor += CalculateDirLight(i, vPosition, vNormal);
            }
            else if (gLights[i].m_nType == POINT_LIGHT)
            {
                cColor += CalculatePointLight(i, vPosition, vNormal);
            }
            else if (gLights[i].m_nType == SPOT_LIGHT)
            {
                cColor += CalculateSpotLight(i, vPosition, vNormal);
            }
        }
    }
    
    // 전역 환경광 추가
    cColor += gcGlobalAmbientLight;
    cColor.a = 1;
    
    return cColor;
}
