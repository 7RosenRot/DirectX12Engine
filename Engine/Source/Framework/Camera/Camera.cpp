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

void Camera::InputProcessing(const float MovementSpeed, const float MouseSensivity, const DirectX::XMFLOAT3& TargetPoint) {
  if (!Input::IsMmbMode()) {
    m_OrbitTarget = TargetPoint;
  }

  float wheelDelta = Input::GetMouseWheelDelta();
  if (wheelDelta != 0.0f) {
    XMVECTOR Pos = XMLoadFloat3(&m_Transform.GetPosition());
    XMVECTOR Target = XMLoadFloat3(&m_OrbitTarget);
    XMVECTOR V = XMVectorSubtract(Pos, Target);
    float dist = XMVectorGetX(XMVector3Length(V));

    float zoomSpeed = 20.0f * MovementSpeed;
    float newDist = dist - wheelDelta * zoomSpeed;
    if (newDist < 0.5f) {
      newDist = 0.5f;
    }

    if (dist > 0.001f) {
      XMVECTOR dir = XMVector3Normalize(V);
      XMVECTOR newPos = XMVectorAdd(Target, XMVectorScale(dir, newDist));
      XMFLOAT3 newPosF;
      XMStoreFloat3(&newPosF, newPos);
      m_Transform.SetPosition(newPosF.x, newPosF.y, newPosF.z);
    }
  }

  if (!Input::IsMouseLocked()) {
    return;
  }

  if (Input::IsMmbMode()) {
    float mouseDx = 0.0F, mouseDy = 0.0F;
    Input::GetMouseDelta(mouseDx, mouseDy);

    if (mouseDx != 0.0F || mouseDy != 0.0F) {
      bool isShiftPressed = (Input::IsKeyDown(VK_SHIFT) || (GetKeyState(VK_SHIFT) & 0x8000) != 0);
      if (isShiftPressed) {
        XMVECTOR Pos = XMLoadFloat3(&m_Transform.GetPosition());
        XMVECTOR Target = XMLoadFloat3(&m_OrbitTarget);
        XMVECTOR V = XMVectorSubtract(Pos, Target);
        float dist = XMVectorGetX(XMVector3Length(V));

        float panFactor = max(dist, 0.5f) * MouseSensivity * 0.01f;

        XMVECTOR Right = XMLoadFloat3(&m_Transform.GetRight());
        XMVECTOR Up = XMLoadFloat3(&m_Transform.GetUp());

        XMVECTOR translation = XMVectorAdd(
          XMVectorScale(Right, -mouseDx * panFactor),
          XMVectorScale(Up, mouseDy * panFactor)
        );

        XMVECTOR newPos = XMVectorAdd(Pos, translation);
        XMVECTOR newTarget = XMVectorAdd(Target, translation);

        XMFLOAT3 newPosF, newTargetF;
        XMStoreFloat3(&newPosF, newPos);
        XMStoreFloat3(&newTargetF, newTarget);

        m_Transform.SetPosition(newPosF.x, newPosF.y, newPosF.z);
        m_OrbitTarget = newTargetF;
      } else {
        float pitch = DirectX::XMConvertToRadians(mouseDy * MouseSensivity);
        float yaw = DirectX::XMConvertToRadians(mouseDx * MouseSensivity);
        m_Transform.Orbit(XMLoadFloat3(&m_OrbitTarget), pitch, yaw);
      }
    }
  } else {
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
}