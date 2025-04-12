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

    // 싱글톤 인스턴스 얻기
    static FFrustrum& Get() {
        static FFrustrum Instance;
        return Instance;
    }

    // 뷰-투영 행렬로부터 Frustum 생성
    void UpdateFrustrum(const FMatrix& ViewProj);

    // 점이 Frustum 내부에 있는지 확인
    bool ContainsPoint(const FVector& Point) const;

    // 구체가 Frustum과 교차하는지 확인
    bool IntersectsSphere(const FVector& Center, float Radius) const;

    // 축 정렬 경계 상자(AABB)가 Frustum과 교차하는지 확인
    bool IntersectsAABB(const FVector& Min, const FVector& Max) const;

    // Frustum까지의 거리 계산
    float GetDistanceToFrustum(const FVector& Point);

    // 평면 데이터 직접 접근
    const FPlane* GetPlanes() const { return Planes; }

private:
    // 싱글톤을 위한 private 생성자
    FFrustrum() = default;

    // 복사 방지
    FFrustrum(const FFrustrum&) = delete;
    FFrustrum& operator=(const FFrustrum&) = delete;

    FPlane Planes[6]; // 좌, 우, 상, 하, 근, 원 평면
};
