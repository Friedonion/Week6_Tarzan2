#include "Define.h"
#include "UObject/Casts.h"
#include "UpdateLightBufferPass.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Components/Light/LightComponent.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
#include "Engine/EditorEngine.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectIterator.h"
#include "Editor/UnrealEd/EditorViewportClient.h"
#include <FFrustrum.h>

//------------------------------------------------------------------------------
// 생성자/소멸자
//------------------------------------------------------------------------------
FUpdateLightBufferPass::FUpdateLightBufferPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FUpdateLightBufferPass::~FUpdateLightBufferPass()
{
}

void FUpdateLightBufferPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
}


void SortLightsByDistance(TArray<UPointLightComponent*>& Lights, const FVector& CameraPosition)
{
    // 비교 함수: 카메라에 가까운 라이트가 앞으로 오도록 정렬
    auto CompareByDistance = [&CameraPosition](const UPointLightComponent* A, const UPointLightComponent* B) -> bool {
        float DistA = FVector::Distance(A->GetWorldLocation(), CameraPosition);
        float DistB = FVector::Distance(B->GetWorldLocation(), CameraPosition);
        return DistA < DistB;
        };

    // 정렬 수행
    std::sort(Lights.GetData(), Lights.GetData() + Lights.Num(), CompareByDistance);
}


void FUpdateLightBufferPass::PrepareRender()
{
    for (const auto iter : TObjectRange<ULightComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (UPointLightComponent* PointLight = Cast<UPointLightComponent>(iter))
            {
                PointLights.Add(PointLight);
            }
            else if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(iter))
            {
                SpotLights.Add(SpotLight);
            }
        }
    }
}

void FUpdateLightBufferPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    FLightBuffer LightBufferData = {};
    int LightCount = 0;

    FFrustrum Frustrum;
    FMatrix ViewProjMatrix = Viewport->GetViewMatrix() * Viewport->GetProjectionMatrix();
    Frustrum.MaekFrustrum(ViewProjMatrix);
  

    LightBufferData.GlobalAmbientLight = FVector4(0.1f, 0.1f, 0.1f, 1.f);
    for (auto Light : PointLights)
    {
        //if (LightCount == MAX_LIGHTS) 
        //{
        //    break;
        //}

        FVector LightPos = Light->GetWorldLocation();

        // 광원의 유효 범위
        float LightRange = Light->GetAttenuationRadius();

        if (Frustrum.IntersectsSphere(LightPos, LightRange))
        {   
            VisiblePointLights.Add(Light);
        }

    }

    SortLightsByDistance(VisiblePointLights, Viewport->ViewTransformPerspective.GetLocation());

    for (auto Light : VisiblePointLights)
    {
        if (LightCount == MAX_LIGHTS)
        {
            break;
        }

        LightBufferData.gLights[LightCount] = Light->GetLightInfo();
        LightBufferData.gLights[LightCount].Position = Light->GetWorldLocation();
        LightCount++;
    }



    for (auto Light : SpotLights)
    {
        if (LightCount < MAX_LIGHTS)
        {
            //// 월드 변환 행렬 계산 (스케일 1로 가정)
            //FMatrix Model = JungleMath::CreateModelMatrix(Light->GetWorldLocation(), Light->GetWorldRotation(), { 1, 1, 1 });

            //FEngineLoop::PrimitiveDrawBatch.AddConeToBatch(Light->GetWorldLocation(), 100, Light->GetRange(), 140, {1,1,1,1}, Model);

            //FEngineLoop::PrimitiveDrawBatch.AddOBBToBatch(Light->GetBoundingBox(), Light->GetWorldLocation(), Model);
            LightBufferData.gLights[LightCount] = Light->GetLightInfo();
            LightBufferData.gLights[LightCount].Position = Light->GetWorldLocation();
            LightBufferData.gLights[LightCount].Direction = Light->GetForwardVector();
            LightBufferData.gLights[LightCount].Type = ELightType::SPOT_LIGHT;

            LightCount++;
        }
    }
    LightBufferData.nLights = LightCount;

    BufferManager->UpdateConstantBuffer(TEXT("FLightBuffer"), LightBufferData);
}

void FUpdateLightBufferPass::ClearRenderArr()
{
    VisiblePointLights.Empty();
    PointLights.Empty();
    SpotLights.Empty();
}

void FUpdateLightBufferPass::UpdateLightBuffer(FLight Light) const
{

}
