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

float4 DirLight(int nIndex, float3 vPosition, float3 vNormal)
{
    float3 vToLight = normalize(-gLights[nIndex].m_vDirection);
    float3 vView = normalize(CameraPosition - vPosition);
    float3 vHalf = normalize(vView + vToLight);
    float fDiffuseFactor = saturate(dot(vNormal, vToLight));
    float fSpecularFactor = pow(saturate(dot(vHalf, vNormal)), Material.SpecularScalar);

    float3 lit = (gcGlobalAmbientLight * Material.AmbientColor.rgb) +
                (gLights[nIndex].m_cDiffuse.rgb * fDiffuseFactor * Material.DiffuseColor) +
                (gLights[nIndex].m_cSpecular.rgb * fSpecularFactor * Material.SpecularColor);

    return float4(lit, 1.0);
}

float4 SpotLight(int nIndex, float3 vPosition, float3 vNormal)
{
    // 광원과 픽셀 위치 간 벡터 계산
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);

    // 감쇠 반경을 벗어나면 기여하지 않음
    if (fDistance < gLights[nIndex].m_fAttRadius)
    {
        
    }

    float fSpecularFactor = 0.0f;
    vToLight /= fDistance; // 정규화
    
    float fDiffuseFactor = saturate(dot(vNormal, vToLight));

    if (fDiffuseFactor > 0.0f)
    {
        float3 vView = normalize(CameraPosition - vPosition);
        float3 vHalf = normalize(vToLight + vView);
        fSpecularFactor = pow(max(dot(normalize(vNormal), vHalf), 0.0f), Material.SpecularScalar);
    }
    
    float fSpotFactor = pow(max(dot(-vToLight, gLights[nIndex].m_vDirection), 0.0f), 1.0f); //gLights[nIndex].m_fFalloff);
    float fAttenuationFactor = 1.0f / (1.0f + gLights[nIndex].m_fAttenuation * fDistance * fDistance);
    
    float3 lit = (gcGlobalAmbientLight * Material.AmbientColor.rgb) +
                 (gLights[nIndex].m_cDiffuse.rgb * fDiffuseFactor * Material.DiffuseColor) +
                 (gLights[nIndex].m_cSpecular.rgb * fSpecularFactor * Material.SpecularColor);

    // intensity와 attenuation factor, spot factor를 곱하여 최종 색상 계산
    return float4(lit * fAttenuationFactor * fSpotFactor * gLights[nIndex].m_fIntensity, 1.0);
}

float4 PointLight(int nIndex, float3 vPosition, float3 vNormal)
{
    // 광원과 픽셀 위치 간 벡터 계산
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);

    // 감쇠 반경을 벗어나면 기여하지 않음
    if (fDistance > gLights[nIndex].m_fAttRadius)
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    
    float fSpecularFactor = 0.0f;
    vToLight /= fDistance; // 정규화
    float fDiffuseFactor = saturate(dot(vNormal, vToLight));

    if (fDiffuseFactor > 0.0f)
    {
        float3 vView = normalize(CameraPosition - vPosition);
        float3 vHalf = normalize(vToLight + vView);
        fSpecularFactor = pow(max(dot(normalize(vNormal), vHalf), 0.0f), Material.SpecularScalar);
    }
    else
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    
        

    float fAttenuationFactor = 1.0f / (1.0f + gLights[nIndex].m_fAttenuation * fDistance * fDistance);
   
    float3 lit = (gcGlobalAmbientLight * Material.AmbientColor.rgb) +
                 (gLights[nIndex].m_cDiffuse.rgb * fDiffuseFactor * Material.DiffuseColor) +
                 (gLights[nIndex].m_cSpecular.rgb * fSpecularFactor * Material.SpecularColor);

    return float4(lit * fAttenuationFactor * gLights[nIndex].m_fIntensity, 1.0);
}

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
    float3 vView = normalize(CameraPosition - vPosition);
    float3 vHalf = normalize(vToLight + vView);
    return pow(dot(normalize(vNormal), vHalf), fSpecularScalar);   
    // return pow(max(dot(normalize(vNormal), vHalf), 0.0f), 1);    // max(dot(normalize(vNormal), vHalf), 0.0f) 는 적절하지 않다.
}

float4 CalculateDirLight(int nIndex, float3 vPosition, float3 vNormal)
{
    float3 vToLight = normalize(-gLights[nIndex].m_vDirection);
    float3 vView = normalize(CameraPosition - vPosition);
    
    float3 ambientLight = gcGlobalAmbientLight * Material.AmbientColor.rgb;
    float3 lit = ambientLight;
    
    if (dot(vNormal, vToLight) < 0.0f)
        return float4(lit, 1.0f);

#if LIGHTING_MODEL_GOURAUD
    lit = 0.0f;
#elif LIGHTING_MODEL_LAMBERT
    // float4 baseColor = hasTexture ? albedo : float4(1, 1, 1, 1);
    float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f)  ? float3(1,1,1)  : Material.DiffuseColor;
    lit = ambientLight + gLights[nIndex].m_cDiffuse.rgb * LambertLightingModel(vToLight, vNormal) * safeDiffuse;
#elif LIGHTING_MODEL_SPECULAR
    lit = ambientLight + gLights[nIndex].m_cSpecular.rgb * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
#elif LIGHTING_MODEL_BLINNPHONG
    float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f)  ? float3(1,1,1)  : Material.DiffuseColor;
    lit = ambientLight +
        gLights[nIndex].m_cDiffuse.rgb * LambertLightingModel(vToLight, vNormal) * safeDiffuse +
        gLights[nIndex].m_cSpecular.rgb * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
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

#if LIGHTING_MODEL_GOURAUD
    lit = 0.0f;
#elif LIGHTING_MODEL_LAMBERT
    // float4 baseColor = hasTexture ? albedo : float4(1, 1, 1, 1);
    float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f)  ? float3(1,1,1)  : Material.DiffuseColor;
    lit = ambientLight + gLights[nIndex].m_cDiffuse.rgb * LambertLightingModel(vToLight, vNormal) * safeDiffuse;
#elif LIGHTING_MODEL_SPECULAR
    lit = ambientLight + gLights[nIndex].m_cSpecular.rgb * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
#elif LIGHTING_MODEL_BLINNPHONG
    float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f)  ? float3(1,1,1)  : Material.DiffuseColor;
    lit = ambientLight +
        gLights[nIndex].m_cDiffuse.rgb * LambertLightingModel(vToLight, vNormal) * safeDiffuse +
        gLights[nIndex].m_cSpecular.rgb * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
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
    float fCosInner = cos(radians(gLights[nIndex].m_fInnerDegree));
    float fCosOuter = cos(radians(gLights[nIndex].m_fOuterDegree));
    float3 lit;
    float3 ambientLight = gcGlobalAmbientLight * Material.AmbientColor.rgb;

#if LIGHTING_MODEL_GOURAUD
    lit = 0.0f;
#elif LIGHTING_MODEL_LAMBERT
    // float4 baseColor = hasTexture ? albedo : float4(1, 1, 1, 1);
    float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f)  ? float3(1,1,1)  : Material.DiffuseColor;
    lit = ambientLight + gLights[nIndex].m_cDiffuse.rgb * LambertLightingModel(vToLight, vNormal) * safeDiffuse;
#elif LIGHTING_MODEL_SPECULAR
    lit = ambientLight + gLights[nIndex].m_cSpecular.rgb * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
#elif LIGHTING_MODEL_BLINNPHONG
    float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f)  ? float3(1,1,1)  : Material.DiffuseColor;
    lit = ambientLight +
        gLights[nIndex].m_cDiffuse.rgb * LambertLightingModel(vToLight, vNormal) * safeDiffuse +
        gLights[nIndex].m_cSpecular.rgb * BlinnPhongLightingModel(vToLight, vPosition, vNormal, Material.SpecularScalar) * Material.SpecularColor;
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
