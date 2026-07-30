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

  void SetRotation(float Pitch, float Yaw, float Roll) {
    m_Rotation = { XMConvertToRadians(Pitch), XMConvertToRadians(Yaw), XMConvertToRadians(Roll) };
  }

  void SetScale(float Sx, float Sy, float Sz) {
    m_Scale = { Sx, Sy, Sz };
  }
  // ↑ Setters ↑

  // ↓ Movement, Rotating ↓
  void Forward(float Delta);
  void Sideway(float Delta);
  
  void RotateX(float Angle);
  void RotateY(float Angle);
  void Orbit(FXMVECTOR Target, float PitchAngle, float YawAngle);
  // ↑ Movement, Rotating ↑

  // ↓ Getters ↓
  XMMATRIX GetMatrixView();
  XMMATRIX GetMatrixModel();
  
  XMFLOAT3 GetPosition() const {
    return m_Position;
  }

  XMFLOAT3 GetRotation() const {
    return m_Rotation;
  }

  XMFLOAT3 GetScale() const {
    return m_Scale;
  }

  XMFLOAT3 GetRight() const {
    return m_Right;
  }

  XMFLOAT3 GetUp() const {
    return m_Up;
  }

  XMFLOAT3 GetLook() const {
    return m_Look;
  }
  // ↑ Getters ↑
 private:
  // ↓ Lens Properties ↓
  XMFLOAT3 m_Position{0.0F, 0.0F, 0.0F};
  XMFLOAT3 m_Rotation{0.0F, 0.0F, 0.0F};
  XMFLOAT3 m_Scale{1.0F, 1.0F, 1.0F};

  XMFLOAT3 m_Look{0.0F, 0.0F, 1.0F};
  
  XMFLOAT3 m_Right{1.0F, 0.0F, 0.0F};
  XMFLOAT3 m_Up{0.0F, 1.0F, 0.0F};
  // ↑ Lens Properties ↑

  float m_PitchAngle{0.0F};
  XMFLOAT3 m_LastRotation{-999.0f, -999.0f, -999.0f};
};