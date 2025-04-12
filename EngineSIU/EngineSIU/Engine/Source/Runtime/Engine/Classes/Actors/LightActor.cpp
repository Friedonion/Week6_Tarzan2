#include "LightActor.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/Light/DirLightComponent.h"

APointLight::APointLight()
{
    PointLightComponent = AddComponent<UPointLightComponent>();
    BillboardComponent = AddComponent<UBillboardComponent>();

    RootComponent = BillboardComponent;

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/PointLight_64x.png");

    PointLightComponent->AttachToComponent(RootComponent);

}

APointLight::~APointLight()
{
}

ADirectionLight::ADirectionLight()
{
    DirectionLightComponent = AddComponent<UDirLightComponent>();
    BillboardComponent = AddComponent<UBillboardComponent>();
    RootComponent = BillboardComponent;
    BillboardComponent->SetTexture(L"Assets/Editor/Icon/PointLight_64x.png");
    DirectionLightComponent->AttachToComponent(RootComponent);
}

ADirectionLight::~ADirectionLight()
{
}

ASpotLight::ASpotLight()
{
    SpotLightComponent = AddComponent<USpotLightComponent>();
    BillboardComponent = AddComponent<UBillboardComponent>();
    RootComponent = BillboardComponent;
    BillboardComponent->SetTexture(L"Assets/Editor/Icon/PointLight_64x.png");
    SpotLightComponent->AttachToComponent(RootComponent);
}

ASpotLight::~ASpotLight()
{
}

