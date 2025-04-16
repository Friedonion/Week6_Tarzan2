#include "LightComponent.h"
#include "Components/BillboardComponent.h"
#include "UObject/Casts.h"
#include "Classes/GameFramework/Actor.h"
#include "Classes/Components/BillboardComponent.h"

ULightComponent::ULightComponent()
{
    // FString name = "SpotLight";
    // SetName(name);
    InitializeLight();
}

ULightComponent::~ULightComponent()
{
  
}

UObject* ULightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->Light = Light;

    return NewComponent;
}

void ULightComponent::SetDiffuseColor(FLinearColor NewColor)
{
    Light.DiffuseColor = FVector(NewColor.R, NewColor.G, NewColor.B);
}

void ULightComponent::SetSpecularColor(FLinearColor NewColor)
{
   Light.SpecularColor = FVector(NewColor.R, NewColor.G, NewColor.B);
}

void ULightComponent::SetAttenuation(float Attenuation)
{
    Light.Attenuation = Attenuation;
}

void ULightComponent::SetAttenuationRadius(float AttenuationRadius)
{
    Light.AttRadius = AttenuationRadius;
}

void ULightComponent::SetIntensity(float Intensity)
{
    Light.Intensity = Intensity;
}

FLinearColor ULightComponent::GetDiffuseColor() const
{
    return FLinearColor(Light.DiffuseColor.X, Light.DiffuseColor.Y, Light.DiffuseColor.Z, 1);
}

FLinearColor ULightComponent::GetSpecularColor()const
{
    return FLinearColor(Light.SpecularColor.X, Light.SpecularColor.Y, Light.SpecularColor.Z, 1);
}

float ULightComponent::GetAttenuation() const
{
    return Light.Attenuation;
}

float ULightComponent::GetAttenuationRadius()const
{
    return Light.AttRadius;
}

FLight ULightComponent::GetLightInfo()
{
    Light.Position = GetWorldLocation();
    Light.Direction = GetWorldForwardVector();
    return Light;
}

void ULightComponent::InitializeLight()
{  
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };
    
    Light = FLight();
    Light.Enabled = 1;
}

int ULightComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    bool res = AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance);
    return res;
}

void ULightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);

    OutProperties.Add(TEXT("BaseColor"), GetDiffuseColor().ToString());
    OutProperties.Add(TEXT("SpecularColor"), GetSpecularColor().ToString());
    OutProperties.Add(TEXT("Intensity"), FString::Printf(TEXT("%f"), GetIntensity()));
    OutProperties.Add(TEXT("Attenuation"), FString::Printf(TEXT("%f"), GetAttenuation()));
    OutProperties.Add(TEXT("Radius"), FString::Printf(TEXT("%f"), GetAttenuationRadius()));
}

void ULightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;

    TempStr = InProperties.Find(TEXT("BaseColor"));
    if (TempStr)
    {
        FLinearColor DiffuseColor = GetDiffuseColor().FromString(*TempStr);
        SetDiffuseColor(DiffuseColor);

        if (UBillboardComponent* Comp = Cast<UBillboardComponent>(GetOwner()->GetRootComponent()))
        {
            Comp->SetColor(DiffuseColor);
        }
    }

    TempStr = InProperties.Find(TEXT("SpecularColor"));
    if (TempStr)
    {
        FLinearColor DiffuseColor = GetSpecularColor().FromString(*TempStr);
        SetSpecularColor(DiffuseColor);
    }

    TempStr = InProperties.Find(TEXT("Intensity"));
    if (TempStr)
    {
        float Intensity = FString::ToFloat(*TempStr);
        SetIntensity(Intensity);
    }

    TempStr = InProperties.Find(TEXT("Attenuation"));
    if (TempStr)
    {
        float Attenuation = FString::ToFloat(*TempStr);
        SetAttenuation(Attenuation);
    }

    TempStr = InProperties.Find(TEXT("Radius"));
    if (TempStr)
    {
        float Radius = FString::ToFloat(*TempStr);
        SetAttenuationRadius(Radius);
    }
}

