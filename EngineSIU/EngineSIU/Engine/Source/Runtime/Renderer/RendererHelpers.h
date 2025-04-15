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
            std::shared_ptr<FTexture> TextureMetallic = FEngineLoop::ResourceManager.GetTexture(MaterialInfo.MetallicTexturePath);
            std::shared_ptr<FTexture> TextureSpecular = FEngineLoop::ResourceManager.GetTexture(MaterialInfo.SpecularTexturePath);
            std::shared_ptr<FTexture> TextureEmissive = FEngineLoop::ResourceManager.GetTexture(MaterialInfo.EmissiveTexturePath);

            ID3D11ShaderResourceView* srvs[5] = { nullptr, nullptr,nullptr,nullptr,nullptr };
           

            // Shader Resource View 바인딩 (텍스처와 노말 맵)
            Graphics->DeviceContext->PSSetShaderResources(0, 1, &texture->TextureSRV);
            if (TextureNormal)
                Graphics->DeviceContext->PSSetShaderResources(1, 1, &TextureNormal->TextureSRV);
            if (TextureMetallic)
                Graphics->DeviceContext->PSSetShaderResources(2, 1, &TextureMetallic->TextureSRV);
            if (TextureSpecular)
                Graphics->DeviceContext->PSSetShaderResources(3, 1, &TextureSpecular->TextureSRV);
            if (TextureEmissive)
                Graphics->DeviceContext->PSSetShaderResources(4, 1, &TextureEmissive->TextureSRV);
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
