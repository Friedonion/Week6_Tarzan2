#pragma once
#include "LightComponent.h"

class UDirLightComponent: public ULightComponent
{
    DECLARE_CLASS(UDirLightComponent, ULightComponent);
public:
    UDirLightComponent();
    ~UDirLightComponent();
private:
    FVector Direction;
};
