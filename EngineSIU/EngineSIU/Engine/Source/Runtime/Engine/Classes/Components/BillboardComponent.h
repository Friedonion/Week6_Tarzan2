#pragma once

#define _TCHAR_DEFINED
#include <wrl.h>
#include "PrimitiveComponent.h"

enum class EBillboardComponentType : uint8
{
    Default,
    Light,
};

class UBillboardComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UBillboardComponent, UPrimitiveComponent)

public:
    UBillboardComponent();
    void SetColor(FLinearColor Color);
    FLinearColor GetColor();
    virtual ~UBillboardComponent();
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(
        FVector& rayOrigin,
        FVector& rayDirection,
        float& pfNearHitDistance
    ) override;

    virtual void SetTexture(const FWString& _fileName);
    void SetUUIDParent(USceneComponent* _parent);
    FMatrix CreateBillboardMatrix() const;
    FString GetBufferKey();
    void SetBillboardType(EBillboardComponentType Type) { BillboardType = Type; }
    EBillboardComponentType GetBillboardType() const { return BillboardType; }
 
    FLinearColor Color;
    float finalIndexU = 0.0f;
    float finalIndexV = 0.0f;
    std::shared_ptr<FTexture> Texture;
    FString BufferKey = TEXT("default");

protected:
    USceneComponent* m_parent = nullptr;

    // NDC 픽킹을 위한 내부 함수 : quadVertices는 월드 공간 정점 배열
    bool CheckPickingOnNDC(const TArray<FVector>& quadVertices, float& hitDistance) const;

private:
    void CreateQuadTextureVertexBuffer();

    EBillboardComponentType BillboardType = EBillboardComponentType::Default;
};
