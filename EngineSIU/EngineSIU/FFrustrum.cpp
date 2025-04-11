#include "FFrustrum.h"

float FFrustrum::FPlane::GetSignedDistance(const FVector& Point) const
{
    return FVector::DotProduct(Normal, Point) + Distance;
}

void FFrustrum::MaekFrustrum(const FMatrix& ViewProj)
{
    ViewProj.Transpose(ViewProj);

    // 좌측 평면
    Planes[0].Normal.X = ViewProj.M[0][3] + ViewProj.M[0][0];
    Planes[0].Normal.Y = ViewProj.M[1][3] + ViewProj.M[1][0];
    Planes[0].Normal.Z = ViewProj.M[2][3] + ViewProj.M[2][0];
    Planes[0].Distance = ViewProj.M[3][3] + ViewProj.M[3][0];

    // 우측 평면
    Planes[1].Normal.X = ViewProj.M[0][3] - ViewProj.M[0][0];
    Planes[1].Normal.Y = ViewProj.M[1][3] - ViewProj.M[1][0];
    Planes[1].Normal.Z = ViewProj.M[2][3] - ViewProj.M[2][0];
    Planes[1].Distance = ViewProj.M[3][3] - ViewProj.M[3][0];

    // 상단 평면
    Planes[2].Normal.X = ViewProj.M[0][3] - ViewProj.M[0][1];
    Planes[2].Normal.Y = ViewProj.M[1][3] - ViewProj.M[1][1];
    Planes[2].Normal.Z = ViewProj.M[2][3] - ViewProj.M[2][1];
    Planes[2].Distance = ViewProj.M[3][3] - ViewProj.M[3][1];

    // 하단 평면
    Planes[3].Normal.X = ViewProj.M[0][3] + ViewProj.M[0][1];
    Planes[3].Normal.Y = ViewProj.M[1][3] + ViewProj.M[1][1];
    Planes[3].Normal.Z = ViewProj.M[2][3] + ViewProj.M[2][1];
    Planes[3].Distance = ViewProj.M[3][3] + ViewProj.M[3][1];

    // 근평면
    Planes[4].Normal.X = ViewProj.M[0][2];
    Planes[4].Normal.Y = ViewProj.M[1][2];
    Planes[4].Normal.Z = ViewProj.M[2][2];
    Planes[4].Distance = ViewProj.M[3][2];

    // 원평면
    Planes[5].Normal.X = ViewProj.M[0][3] - ViewProj.M[0][2];
    Planes[5].Normal.Y = ViewProj.M[1][3] - ViewProj.M[1][2];
    Planes[5].Normal.Z = ViewProj.M[2][3] - ViewProj.M[2][2];
    Planes[5].Distance = ViewProj.M[3][3] - ViewProj.M[3][2];

    // 평면 정규화
    for (int32 i = 0; i < 6; i++) {
        
        float Length = Planes[i].Normal.Length();
        Planes[i].Normal = Planes[i].Normal / Length;
        Planes[i].Distance = Planes[i].Distance / Length;
        
    }
}

bool FFrustrum::ContainsPoint(const FVector& Point) const
{
    for (int32 i = 0; i < 6; i++) {
        if (Planes[i].GetSignedDistance(Point) < 0) {
            return false; // 하나라도 평면의 뒷면에 있으면 Frustum 밖에 있음
        }
    }
    return true;
}

bool FFrustrum::IntersectsSphere(const FVector& Center, float Radius) const
{
    for (int32 i = 0; i < 6; i++) {
        float Distance = Planes[i].GetSignedDistance(Center);
        if (Distance < -Radius) {
            return false; // 구체가 평면의 뒷면에 완전히 있음
        }
    }
    return true; // 모든 평면과 교차하거나 앞에 있음
}

bool FFrustrum::IntersectsAABB(const FVector& Min, const FVector& Max) const
{
    for (int32 i = 0; i < 6; i++) {
        const FVector& Normal = Planes[i].Normal;

        // AABB의 가장 앞쪽 점 찾기(평면 법선 방향으로)
        FVector P;
        P.X = (Normal.X >= 0) ? Max.X : Min.X;
        P.Y = (Normal.Y >= 0) ? Max.Y : Min.Y;
        P.Z = (Normal.Z >= 0) ? Max.Z : Min.Z;

        // 가장 앞쪽 점이 평면 뒤에 있다면, AABB는 완전히 평면 뒤에 있음
        if (Planes[i].GetSignedDistance(P) < 0) {
            return false;
        }
    }
    return true; // 모든 평면과 교차하거나 앞에 있음
}
