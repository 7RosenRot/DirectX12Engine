#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class Transform {
 public:
  Transform() = default;
  ~Transform() = default;

  // ↓ Setters ↓
  void SetLookTarget(
    FXMVECTOR Position, FXMVECTOR Target, FXMVECTOR TrueUp
  );

  void SetPosition(float Ox, float Oy, float Oz) {
    m_Position = { Ox, Oy, Oz };
  }
  // ↑ Setters ↑

  // ↓ Movement, Rotating ↓
  void Forward(float Delta);
  void Sideway(float Delta);
  
  void RotateX(float Angle);
  void RotateY(float Angle);
  // ↑ Movement, Rotating ↑

  // ↓ Getters ↓
  XMMATRIX GetMatrixView();
  
  XMFLOAT3 GetPosition() const {
    return m_Position;
  }
  // ↑ Getters ↑
 private:
  // ↓ Lens Properties ↓
  XMFLOAT3 m_Position{0.0F, 0.0F, 0.0F};
  XMFLOAT3 m_Look{0.0F, 0.0F, 1.0F};
  
  XMFLOAT3 m_Right{1.0F, 0.0F, 0.0F};
  XMFLOAT3 m_Up{0.0F, 1.0F, 0.0F};
  // ↑ Lens Properties ↑

  float m_PitchAngle{0.0F};
};