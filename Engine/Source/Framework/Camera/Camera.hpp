#pragma once

#include <algorithm>
#include <DirectXMath.h>

#include <Application/Input/Input.hpp>
#include <Framework/Transform/Transform.hpp>

using namespace DirectX;

class Camera {
 public:
  Camera();
  ~Camera() = default;
  
  // ↓ Setters ↓
  void SetLensProperties(
    float FovAngleY, float AspectRatio, float NearDischarge, float FarDischarge
  );
  // ↑ Setters ↑
  
  // ↓ Input processing ↓
  void InputProcessing(const float MovementSpeed, const float mouseSensivity);
  // ↑ Input processing ↑

  // ↓ Getters ↓
  Transform& GetTransform() {
    return m_Transform;
  }
  
  XMMATRIX GetMatrixProjection() const {
    return m_MatrixProjection;
  }
  // ↑ Getters ↑
 private:
  Transform m_Transform;

  XMMATRIX m_MatrixProjection = XMMatrixIdentity();
};