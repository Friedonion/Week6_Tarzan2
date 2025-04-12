#include "DirLightComponent.h"

UDirLightComponent::UDirLightComponent()
{
    Light.Type = ELightType::DIR_LIGHT;
}

UDirLightComponent::~UDirLightComponent()
{
}

void UDirLightComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}
