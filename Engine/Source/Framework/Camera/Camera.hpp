#pragma once

#include <algorithm>
#include <DirectXMath.h>

using namespace DirectX;

namespace D3D12Engine {
  class Camera {
   public:
    Camera();
    ~Camera() = default;

    // ↓ Setters ↓
    void SetPosition(float x, float y, float z);
    void SetLookTarget(FXMVECTOR Position, FXMVECTOR Target, FXMVECTOR TrueUp);
    void SetLensProperties(
      float FovAngleY, float AspectRatio, float NearDischarge, float FarDischarge
    );
    // ↑ Setters ↑
    
    // ↓ Movement, Rotating ↓
    void Forward(float Delta);
    void Sideway(float Delta);

    void RotateX(float Angle);
    void RotateY(float Angle);
    // ↑ Movement, Rotating ↑

    // ↓ Matrices ↓
    void UpdateMatrixView();
    // ↑ Matrices ↑

    // ↓ Getters ↓
    XMMATRIX GetMatrixView() const { return m_MatrixView; }
    XMMATRIX GetMatrixProjection() const { return m_MatrixProjection; }
    XMFLOAT3 GetPosition() const { return m_Position; }
    // ↑ Getters ↑

   private:
    // ↓ Lens Properties ↓
    XMFLOAT3 m_Position{0.0F, 0.0F, 0.0F};
    XMFLOAT3 m_Look{0.0F, 0.0F, 1.0F};
    
    XMFLOAT3 m_Right{1.0F, 0.0F, 0.0F};
    XMFLOAT3 m_Up{0.0F, 1.0F, 0.0F};
    // ↑ Lens Properties ↑

    // ↓ Matrices ↓
    XMMATRIX m_MatrixView = XMMatrixIdentity();
    XMMATRIX m_MatrixProjection = XMMatrixIdentity();
    // ↑ Matrices ↑

    float m_PitchAngle{0.0F};
  };
}