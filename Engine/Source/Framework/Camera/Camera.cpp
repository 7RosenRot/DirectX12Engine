#include <Framework/Camera/Camera.hpp>

Camera::Camera() {
  SetLensProperties(0.25f * XM_PI, 1.0f, 1.0f, 1000.0f);
}

void Camera::SetLensProperties(
  float FovAngleY, float AspectRatio, float NearDischarge, float FarDischarge
) {
  m_MatrixProjection = XMMatrixPerspectiveFovLH(
    FovAngleY, AspectRatio, NearDischarge, FarDischarge
  );
}

void Camera::InputProcessing(float Delta) {
  const float speed = 5.0f * Delta;

  if (Input::IsKeyDown('W')) { m_Transform.Forward( speed ); }
  if (Input::IsKeyDown('S')) { m_Transform.Forward(-speed ); }
  if (Input::IsKeyDown('A')) { m_Transform.Sideway(-speed ); }
  if (Input::IsKeyDown('D')) { m_Transform.Sideway( speed ); }

  float mouseDx = 0.0f, mouseDy = 0.0f;
  Input::GetMouseDelta(mouseDx, mouseDy);

  if (mouseDx != 0.0f || mouseDy != 0.0f) {
    float mouseSensivity = 0.05F;
    
    m_Transform.RotateX(DirectX::XMConvertToRadians(mouseDy * mouseSensivity));
    m_Transform.RotateY(DirectX::XMConvertToRadians(mouseDx * mouseSensivity));
  }
}