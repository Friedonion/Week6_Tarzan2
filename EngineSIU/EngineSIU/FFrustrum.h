#pragma once
#include <Math/Matrix.h>

class FFrustrum
{
public:
    // 평면 정의 (법선 벡터와 원점으로부터의 거리)
    struct FPlane {
        FVector Normal;  // 바깥쪽을 향하는 법선 벡터
        float Distance;  // 원점으로부터의 거리

        // 점이 평면의 양쪽 중 어디에 있는지 확인
        float GetSignedDistance(const FVector& Point) const;
    };

    FPlane Planes[6]; // 좌, 우, 상, 하, 근, 원 평면

    // 뷰-투영 행렬로부터 Frustum 생성
    void MaekFrustrum(const FMatrix& ViewProj);

    // 점이 Frustum 내부에 있는지 확인
    bool ContainsPoint(const FVector& Point) const;

    // 구체가 Frustum과 교차하는지 확인
    bool IntersectsSphere(const FVector& Center, float Radius) const;

    // 축 정렬 경계 상자(AABB)가 Frustum과 교차하는지 확인
    bool IntersectsAABB(const FVector& Min, const FVector& Max) const;
};
