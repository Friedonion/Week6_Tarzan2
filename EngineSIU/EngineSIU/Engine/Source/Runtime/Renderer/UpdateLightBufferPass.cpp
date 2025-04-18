#include "Define.h"
#include "UObject/Casts.h"
#include "UpdateLightBufferPass.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Components/Light/LightComponent.h"
#include "Components/Light/DirLightComponent.h"
#include "Engine/EditorEngine.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectIterator.h"
#include "Editor/UnrealEd/EditorViewportClient.h"
#include <FFrustrum.h>

#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"

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


void SortLightsByDistanceAndIntensity(TArray<ULightComponent*>& Lights, const FVector& CameraPosition)
{
    // 비교 함수: 카메라와의 거리와 빛의 강도를 모두 고려하여 정렬
    auto CompareByDistanceAndIntensity = [&CameraPosition](const ULightComponent* A, const ULightComponent* B) -> bool {
        float DistA = FVector::DistSquared(A->GetWorldLocation(), CameraPosition);
        float DistB = FVector::DistSquared(B->GetWorldLocation(), CameraPosition);

        // 거리에 따른 감쇠 적용 (거리의 제곱에 반비례)
        float ScoreA = DistA;
        float ScoreB = DistB;

        // 점수가 높은 라이트가 앞으로 오도록 정렬 (내림차순)
        return ScoreA < ScoreB;
        };

    // 정렬 수행
    std::sort(Lights.GetData(), Lights.GetData() + Lights.Num(), CompareByDistanceAndIntensity);
}


void FUpdateLightBufferPass::PrepareRender()
{
    for (const auto iter : TObjectRange<ULightComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (UDirLightComponent* dirLight = Cast<UDirLightComponent>(iter))
            {
                DirectionalLights.Add(dirLight);
            }
            else
            {
                Lights.Add(iter);
            }
          
        }
    }
}

void FUpdateLightBufferPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    FLightBuffer LightBufferData = {};
    int LightCount = 0;


    LightBufferData.GlobalAmbientLight = FVector4(0.1f, 0.1f, 0.1f, 1.f);
    for (auto Light : Lights)
    {
        FVector LightPos = Light->GetWorldLocation();

        // 광원의 유효 범위
        float LightRange = Light->GetAttenuationRadius();

       
        if (FFrustrum::Get().IntersectsSphere(LightPos, LightRange))
        {   
            VisiblePointLights.Add(Light);
        }
    }
    
    SortLightsByDistanceAndIntensity(VisiblePointLights, Viewport->ViewTransformPerspective.GetLocation());
    
    for (auto Light : VisiblePointLights)
    {
        if (LightCount == MAX_LIGHTS - DirectionalLights.Len())
        {
            break;
        }

        LightBufferData.gLights[LightCount] = Light->GetLightInfo();
        LightCount++;
    }

    for (int i = 0; i < DirectionalLights.Num(); i++)
    {
        LightBufferData.gLights[LightCount++] = DirectionalLights[i]->GetLightInfo();
    }

    LightBufferData.nLights = LightCount;
    BufferManager->UpdateConstantBuffer(TEXT("FLightBuffer"), LightBufferData);


    // draw wireframe of selected light component
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    AActor* Actor = nullptr;
    ULightComponent* Light = nullptr;
    if (Engine && (Actor = Engine->GetSelectedActor()))
    {
        for (auto comp: Actor->GetComponents())
        {
            if (Light = Cast<ULightComponent>(comp))
                break;
        }
    }
    
    if (Light)
    {
        if (UPointLightComponent* PointLight = Cast<UPointLightComponent>(Light))
        {
            FLinearColor lColor = PointLight->GetDiffuseColor();
            FVector4 color = FVector4(lColor.R, lColor.G, lColor.B, lColor.A);
            GEngineLoop.PrimitiveDrawBatch.AddSphereToBatch(
                PointLight->GetWorldLocation(),
                PointLight->GetAttenuationRadius(),
                48,
                color
                );
        }
        else if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(Light))
        {
            FLinearColor lColor = SpotLight->GetDiffuseColor();
            FVector4 color = FVector4(lColor.R, lColor.G, lColor.B, lColor.A);
            float InnerConeRadius = SpotLight->GetAttenuationRadius() * sin(SpotLight->GetInnerAngle() * PI / 180.f );
            GEngineLoop.PrimitiveDrawBatch.AddConeToBatch(
                SpotLight->GetWorldLocation(),
                InnerConeRadius,
                SpotLight->GetAttenuationRadius() * cos(SpotLight->GetInnerAngle() * PI / 180.f),
                18,
                color,
                SpotLight->GetRotationMatrix()
                );
            float OuterConeRadius = SpotLight->GetAttenuationRadius() * sin(SpotLight->GetOuterAngle() * PI / 180.f );
            GEngineLoop.PrimitiveDrawBatch.AddConeToBatch(
                SpotLight->GetWorldLocation(),
                OuterConeRadius,
                SpotLight->GetAttenuationRadius() * cos(SpotLight->GetOuterAngle() * PI / 180.f),
                18,
                color,
                SpotLight->GetRotationMatrix()
                );
        }
    }
}

void FUpdateLightBufferPass::ClearRenderArr()
{
    VisiblePointLights.Empty();
    DirectionalLights.Empty();
    Lights.Empty();
}

void FUpdateLightBufferPass::UpdateLightBuffer(FLight Light) const
{

}
