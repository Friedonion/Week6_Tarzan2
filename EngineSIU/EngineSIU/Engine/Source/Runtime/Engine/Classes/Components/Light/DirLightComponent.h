#pragma once
#include "LightComponent.h"

class UDirLightComponent: public ULightComponent
{
    DECLARE_CLASS(UDirLightComponent, ULightComponent);
public:
    UDirLightComponent();
    ~UDirLightComponent();
    virtual void TickComponent(float DeltaTime) override;
private:
    FVector Direction;
};
