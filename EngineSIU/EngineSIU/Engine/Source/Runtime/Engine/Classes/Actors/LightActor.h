#pragma once
#include "GameFramework/Actor.h"

class UPointLightComponent;
class UDirLightComponent;
class USpotLightComponent;
class UBillboardComponent;

class APointLight :public AActor
{
    DECLARE_CLASS(APointLight, AActor)
public:
    APointLight();
    virtual ~APointLight();
protected:
  
    UPROPERTY
    (UPointLightComponent*, PointLightComponent, = nullptr);

   UPROPERTY
   (UBillboardComponent*, BillboardComponent, = nullptr);
};

class ADirectionLight: public AActor
{
    DECLARE_CLASS(ADirectionLight, AActor)
public:
    ADirectionLight();
    virtual ~ADirectionLight();
protected:
    UPROPERTY(UDirLightComponent*, DirectionLightComponent, = nullptr);
    UPROPERTY(UBillboardComponent*, BillboardComponent, = nullptr);
};

class ASpotLight: public AActor
{
    DECLARE_CLASS(ASpotLight, AActor)
public:
    ASpotLight();
    virtual ~ASpotLight();
protected:
    UPROPERTY(USpotLightComponent*, SpotLightComponent, = nullptr);
    UPROPERTY(UBillboardComponent*, BillboardComponent, = nullptr);
};
