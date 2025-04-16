#include "SpotLightComponent.h"
USpotLightComponent::USpotLightComponent()
{
    Light.Type = ELightType::SPOT_LIGHT;
    Light.InnerDegree = 30.0f;
    Light.OuterDegree = 45.0f;
    Light.AttRadius = 50.0f;
}

USpotLightComponent::~USpotLightComponent()
{
}

FVector USpotLightComponent::GetDirection()
{
    return Light.Direction;
}

void USpotLightComponent::SetDirection(const FVector& dir)
{
    Light.Direction = dir;
}

void USpotLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("InnerAngle"), FString::Printf(TEXT("%f"),GetInnerAngle()));
    OutProperties.Add(TEXT("OuterAngle"), FString::Printf(TEXT("%f"), GetOuterAngle()));
}

void USpotLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);  

    const FString* TempStr = nullptr;

    TempStr = InProperties.Find(TEXT("InnerAngle"));
    if (TempStr)
    {
        float InnerAngle = FString::ToFloat(*TempStr);
        SetInnerAngle(InnerAngle);
    }

    TempStr = InProperties.Find(TEXT("OuterAngle"));
    if (TempStr)
    {
        float OuterAngle = FString::ToFloat(*TempStr);
        SetOuterAngle(OuterAngle);
    }
}


