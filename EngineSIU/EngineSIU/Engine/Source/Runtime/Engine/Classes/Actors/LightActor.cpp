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

    BillboardComponent->SetBillboardType(EBillboardComponentType::Light);
    BillboardComponent->SetTexture(L"Assets/Editor/Icon/PointLight_64x.png");
    PointLightComponent->AttachToComponent(RootComponent);
 
    PointLightComponent->SetDiffuseColor(FLinearColor(1.f, 1.f, 1.f, 1.f));
    BillboardComponent->SetColor(FLinearColor(1.f, 1.f, 1.f, 1.f));
}

APointLight::~APointLight()
{
}

ADirectionLight::ADirectionLight()
{
    DirectionLightComponent = AddComponent<UDirLightComponent>();
    BillboardComponent = AddComponent<UBillboardComponent>();

    RootComponent = BillboardComponent;

    BillboardComponent->SetBillboardType(EBillboardComponentType::Light);
    BillboardComponent->SetTexture(L"Assets/Editor/Icon/DirectionalLight_64x.png");
    DirectionLightComponent->AttachToComponent(RootComponent);

    DirectionLightComponent->SetDiffuseColor(FLinearColor(1.f, 1.f, 1.f, 1.f));
    BillboardComponent->SetColor(FLinearColor(1.f, 1.f, 1.f, 1.f));
}

ADirectionLight::~ADirectionLight()
{
}

ASpotLight::ASpotLight()
{
    SpotLightComponent = AddComponent<USpotLightComponent>();
    BillboardComponent = AddComponent<UBillboardComponent>();

    RootComponent = BillboardComponent;

    BillboardComponent->SetBillboardType(EBillboardComponentType::Light);
    BillboardComponent->SetTexture(L"Assets/Editor/Icon/SpotLight_64x.png");
    SpotLightComponent->AttachToComponent(RootComponent);

    SpotLightComponent->SetDiffuseColor(FLinearColor(1.f, 1.f, 1.f, 1.f));
    BillboardComponent->SetColor(FLinearColor(1.f, 1.f, 1.f, 1.f));
}

ASpotLight::~ASpotLight()
{
}

