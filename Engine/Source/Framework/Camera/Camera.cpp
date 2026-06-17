#include <Framework/Camera/Camera.hpp>

Camera::Camera() {
  SetLensProperties(0.25f * XM_PI, 1.0F, 1.0F, 1000.0F);
}

void Camera::SetLensProperties(
  float FovAngleY, float AspectRatio, float NearDischarge, float FarDischarge
) {
  m_MatrixProjection = XMMatrixPerspectiveFovLH(
    FovAngleY, AspectRatio, NearDischarge, FarDischarge
  );
}

void Camera::InputProcessing(const float MovementSpeed, const float MouseSensivity) {
  if (!Input::IsMouseLocked()) {
    return;
  }

  const float speed = 1.0F * MovementSpeed;

  if (Input::IsKeyDown('W')) { m_Transform.Forward( speed ); }
  if (Input::IsKeyDown('S')) { m_Transform.Forward(-speed ); }
  if (Input::IsKeyDown('A')) { m_Transform.Sideway(-speed ); }
  if (Input::IsKeyDown('D')) { m_Transform.Sideway( speed ); }

  float mouseDx = 0.0F, mouseDy = 0.0F;
  Input::GetMouseDelta(mouseDx, mouseDy);

  if (mouseDx != 0.0F || mouseDy != 0.0F) {
    m_Transform.RotateX(DirectX::XMConvertToRadians(mouseDy * MouseSensivity));
    
    m_Transform.RotateY(DirectX::XMConvertToRadians(mouseDx * MouseSensivity));
  }
}