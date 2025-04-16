#include "PointLightComponent.h"

UPointLightComponent::UPointLightComponent()
{
    Light.Type = ELightType::POINT_LIGHT;
}

UPointLightComponent::~UPointLightComponent()
{
}

void UPointLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
}

void UPointLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
}
