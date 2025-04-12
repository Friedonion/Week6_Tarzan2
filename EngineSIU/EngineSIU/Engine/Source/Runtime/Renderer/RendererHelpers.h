#pragma once
#include "Launch/EngineLoop.h"

namespace RendererHelpers {
    
    inline FMatrix CalculateMVP(const FMatrix& Model, const FMatrix& View, const FMatrix& Projection) {
        return Model * View * Projection;
    }

    inline FMatrix CalculateNormalMatrix(const FMatrix& Model) {
        return FMatrix::Transpose(FMatrix::Inverse(Model));
    }
}

namespace MaterialUtils {
    inline void UpdateMaterial(FDXDBufferManager* BufferManager, FGraphicsDevice* Graphics, const FObjMaterialInfo& MaterialInfo) {
        FMaterialConstants data;
        data.DiffuseColor = MaterialInfo.Diffuse;
        data.TransparencyScalar = MaterialInfo.TransparencyScalar;
        data.AmbientColor = MaterialInfo.Ambient;
        data.DensityScalar = MaterialInfo.DensityScalar;
        data.SpecularColor = MaterialInfo.Specular;
        data.SpecularScalar = MaterialInfo.SpecularScalar;
        data.EmmisiveColor = MaterialInfo.Emissive;
        data.TextureInfo = MaterialInfo.TextureInfo;

        BufferManager->UpdateConstantBuffer(TEXT("FMaterialConstants"), data);

        if (MaterialInfo.bHasTexture) {
            std::shared_ptr<FTexture> texture = FEngineLoop::ResourceManager.GetTexture(MaterialInfo.DiffuseTexturePath);
            std::shared_ptr<FTexture> TextureNormal = FEngineLoop::ResourceManager.GetTexture(MaterialInfo.BumpTexturePath);

            ID3D11ShaderResourceView* srvs[2] = { nullptr, nullptr };
           

            // Shader Resource View 바인딩 (텍스처와 노말 맵)
            Graphics->DeviceContext->PSSetShaderResources(0, 1, &texture->TextureSRV);
            if (TextureNormal)
                Graphics->DeviceContext->PSSetShaderResources(1, 1, &TextureNormal->TextureSRV);
          

            // 샘플러 하나만 바인딩 (디퓨즈 기준)
            if (texture)
                Graphics->DeviceContext->PSSetSamplers(0, 1, &texture->SamplerState);
        }
        else {
            ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
            ID3D11SamplerState* nullSampler[1] = { nullptr };

            Graphics->DeviceContext->PSSetShaderResources(0, 2, nullSRVs);
            Graphics->DeviceContext->PSSetSamplers(0, 1, nullSampler);
        }

    }
}
