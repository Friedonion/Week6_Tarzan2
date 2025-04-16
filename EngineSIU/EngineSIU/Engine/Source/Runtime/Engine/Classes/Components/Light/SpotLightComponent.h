#pragma once
#include "LightComponent.h"

class USpotLightComponent :public ULightComponent
{

    DECLARE_CLASS(USpotLightComponent, ULightComponent)
public:
    USpotLightComponent();
    ~USpotLightComponent();
    FVector GetDirection();
    void SetDirection(const FVector& dir);

    void GetProperties(TMap<FString, FString>& OutProperties) const;
    void SetProperties(const TMap<FString, FString>& InProperties);



};

