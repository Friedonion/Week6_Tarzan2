#include "PointLightComponent.h"
#include "Classes/GameFramework/Actor.h"
#include "Classes/Components/BillboardComponent.h"

UPointLightComponent::UPointLightComponent()
{
    Light.Type = ELightType::POINT_LIGHT;
}

UPointLightComponent::~UPointLightComponent()
{
}

void UPointLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);

}

void UPointLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    
}
