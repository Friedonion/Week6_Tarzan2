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
    float m_fInnerDegree; // spotlight inner radius in degree

    float3 m_vPosition; // position of light (used in point, spot)
    float m_fOuterDegree; // spotlight outer radius in degree
    
    float3 m_vDirection;    // direction (used in directional, spot)
    float pad3;
    
    float m_fAttenuation; // 거리 기반 감쇠 계수
    int m_bEnable;
    int m_nType;
    float m_fIntensity; // 광원 강도
    
    float m_fAttRadius; // 감쇠 반경 (Attenuation Radius)
    float3 LightPad;
};

struct LightingResult
{
    float3 Diffuse;
    float3 Specular;
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


LightingResult CalculateDirLight(int nIndex, float3 vPosition, float3 vNormal)
{
    LightingResult result = (LightingResult) 0;
    float3 vToLight = normalize(-gLights[nIndex].m_vDirection);

    if (dot(vNormal, vToLight) < 0.0f)
        return result;

    float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f) ? float3(1, 1, 1) : Material.DiffuseColor;

#if LIGHTING_MODEL_GOURAUD || LIGHTING_MODEL_BLINNPHONG
        result.Diffuse = gLights[nIndex].m_cDiffuse.rgb * LambertLightingModel(vToLight, vNormal) * safeDiffuse;
        result.Specular = gLights[nIndex].m_cSpecular.rgb * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
#elif LIGHTING_MODEL_LAMBERT
        result.Diffuse = gLights[nIndex].m_cDiffuse.rgb * LambertLightingModel(vToLight, vNormal) * safeDiffuse;
#elif LIGHTING_MODEL_SPECULAR
        result.Specular = gLights[nIndex].m_cSpecular.rgb * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
#endif

    float intensity = gLights[nIndex].m_fIntensity;
    result.Diffuse *= intensity;
    result.Specular *= intensity;

    return result;
}


LightingResult CalculatePointLight(int nIndex, float3 vPosition, float3 vNormal)
{
    LightingResult result = (LightingResult) 0;
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    if (fDistance > gLights[nIndex].m_fAttRadius)
        return result;

    vToLight /= fDistance;
    if (dot(vNormal, vToLight) < 0.0f)
        return result;

    float normalizedRadius = fDistance / gLights[nIndex].m_fAttRadius;
    float distanceAttenuation = 1.0f / (1.0f + gLights[nIndex].m_fAttenuation * fDistance * fDistance);
    float radiusAttenuation = 1.0f - normalizedRadius * normalizedRadius;
    float fAttenuationFactor = distanceAttenuation * radiusAttenuation;

    float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f) ? float3(1, 1, 1) : Material.DiffuseColor;

#if LIGHTING_MODEL_GOURAUD || LIGHTING_MODEL_BLINNPHONG
        result.Diffuse = gLights[nIndex].m_cDiffuse * LambertLightingModel(vToLight, vNormal) * safeDiffuse;
        result.Specular = gLights[nIndex].m_cSpecular * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
#elif LIGHTING_MODEL_LAMBERT
        result.Diffuse = gLights[nIndex].m_cDiffuse * LambertLightingModel(vToLight, vNormal) * safeDiffuse;
#elif LIGHTING_MODEL_SPECULAR
        result.Specular = gLights[nIndex].m_cSpecular * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
#endif

    result.Diffuse *= fAttenuationFactor * gLights[nIndex].m_fIntensity;
    result.Specular *= fAttenuationFactor * gLights[nIndex].m_fIntensity;

    return result;
}

LightingResult CalculateSpotLight(int nIndex, float3 vPosition, float3 vNormal)
{
    float3 vToLight = normalize(gLights[nIndex].m_vPosition - vPosition);
    float3 vLightDir = -normalize(gLights[nIndex].m_vDirection);
    float fDistance = length(gLights[nIndex].m_vPosition - vPosition);
    
    // 감쇠 반경을 벗어나면 기여하지 않음
    if (fDistance > gLights[nIndex].m_fAttRadius)
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (dot(vNormal, vToLight) < 0.0f)
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    float fCos = dot(vToLight, vLightDir);
    float fCosInner = cos(radians(gLights[nIndex].m_fInnerDegree));
    float fCosOuter = cos(radians(gLights[nIndex].m_fOuterDegree));
    float3 lit;
    float3 ambientLight = gcGlobalAmbientLight * Material.AmbientColor.rgb;

    float normalizedRadius = fDistance / gLights[nIndex].m_fAttRadius;
    float distanceAttenuation = 1.0f / (1.0f + gLights[nIndex].m_fAttenuation * fDistance * fDistance);
    float radiusAttenuation = 1 - normalizedRadius * normalizedRadius;
    float fSpotAttenuation = saturate((fCos - fCosOuter) / (fCosInner - fCosOuter));
    float fAttenuationFactor = fSpotAttenuation * radiusAttenuation * distanceAttenuation;

    float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f) ? float3(1, 1, 1) : Material.DiffuseColor;

#if LIGHTING_MODEL_GOURAUD || LIGHTING_MODEL_BLINNPHONG
        result.Diffuse = gLights[nIndex].m_cDiffuse * LambertLightingModel(vToLight, vNormal) * safeDiffuse;
        result.Specular = gLights[nIndex].m_cSpecular * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
#elif LIGHTING_MODEL_LAMBERT
        result.Diffuse = gLights[nIndex].m_cDiffuse * LambertLightingModel(vToLight, vNormal) * safeDiffuse;
#elif LIGHTING_MODEL_SPECULAR
        result.Specular = gLights[nIndex].m_cSpecular * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
#endif

    result.Diffuse *= fAttenuationFactor * gLights[nIndex].m_fIntensity;
    result.Specular *= fAttenuationFactor * gLights[nIndex].m_fIntensity;

    return result;
}

LightingResult Lighting(float3 vPosition, float3 vNormal)
{
    LightingResult result = (LightingResult) 0;

    [unroll(MAX_LIGHTS)]
    for (int i = 0; i < gnLights; i++)
    {
        if (!gLights[i].m_bEnable)
            continue;

        LightingResult tmp;
        if (gLights[i].m_nType == DIR_LIGHT)
            tmp = CalculateDirLight(i, vPosition, vNormal);
        else if (gLights[i].m_nType == POINT_LIGHT)
            tmp = CalculatePointLight(i, vPosition, vNormal);
        else if (gLights[i].m_nType == SPOT_LIGHT)
            tmp = CalculateSpotLight(i, vPosition, vNormal);

        result.Diffuse += tmp.Diffuse;
        result.Specular += tmp.Specular;
    }

    // 환경광 추가
    result.Diffuse += gcGlobalAmbientLight.rgb * Material.AmbientColor;
    return result;
}

