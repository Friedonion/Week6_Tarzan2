#pragma once
#include "LightComponentBase.h"
#include "Define.h"
class UBillboardComponent;

class ULightComponent : public ULightComponentBase
{
    DECLARE_CLASS(ULightComponent, ULightComponentBase)

public:
    ULightComponent();
    virtual ~ULightComponent() override;
    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    void InitializeLight();
    
    void SetDiffuseColor(FLinearColor NewColor);
    void SetSpecularColor(FLinearColor NewColor);
    void SetAttenuation(float Attenuation);
    void SetAttenuationRadius(float AttenuationRadius);
    void SetIntensity(float Intensity);

    FLinearColor GetDiffuseColor();
    FLinearColor GetSpecularColor();
    float GetAttenuation();
    float GetAttenuationRadius();
    
    float GetInnerAngle();
    void SetInnerAngle(float angle);
    float GetOuterAngle();
    void SetOuterAngle(float angle);
    FLight GetLightInfo();
protected:

    FBoundingBox AABB;
    FLight Light;

public:
    FBoundingBox GetBoundingBox() const {return AABB;}
    
    float GetIntensity() const {return Light.Intensity;}
    
};

inline float ULightComponent::GetInnerAngle()
{
    return Light.InnerDegree;
}

inline void ULightComponent::SetInnerAngle(float angle)
{
    Light.InnerDegree = angle;
}

inline float ULightComponent::GetOuterAngle()
{
    return Light.OuterDegree;
}

inline void ULightComponent::SetOuterAngle(float angle)
{
    Light.OuterDegree = angle;
}
