#include <Include/Graphics/Scene/Camera.hpp>

D3D12Engine::Camera::Camera() {
  SetLensProperties(0.25f * XM_PI, 1.0f, 1.0f, 1000.0f);
}

void D3D12Engine::Camera::SetLensProperties(
  float FovAngleY, float AspectRatio, float NearDischarge, float FarDischarge
) {
  m_MatrixProjection = XMMatrixPerspectiveFovLH(
    FovAngleY, AspectRatio, NearDischarge, FarDischarge
  );
}

void D3D12Engine::Camera::SetPosition(float x, float y, float z) {
  m_Position.x = x;
  m_Position.y = y;
  m_Position.z = z;
}

void D3D12Engine::Camera::SetLookTarget(FXMVECTOR Position, FXMVECTOR Target, FXMVECTOR TrueUp) {
  XMVECTOR Look = XMVector3Normalize(XMVectorSubtract(Target, Position));
  XMVECTOR Right = XMVector3Normalize(XMVector3Cross(TrueUp, Look));
  XMVECTOR Up = XMVector3Cross(Look, Right);

  XMStoreFloat3(&m_Position, Position);
  XMStoreFloat3(&m_Look, Look);
  XMStoreFloat3(&m_Right, Right);
  XMStoreFloat3(&m_Up, Up);
}

void D3D12Engine::Camera::UpdateMatrixView() {
  XMVECTOR Right = XMLoadFloat3(&m_Right);
  XMVECTOR Up = XMLoadFloat3(&m_Up);
  XMVECTOR Look = XMLoadFloat3(&m_Look);
  XMVECTOR Position = XMLoadFloat3(&m_Position);

  Look = XMVector3Normalize(Look);
  Up = XMVector3Normalize(XMVector3Cross(Look, Right));
  Right = XMVector3Cross(Up, Look);

  float x = -XMVectorGetX(XMVector3Dot(Position, Right));
  float y = -XMVectorGetX(XMVector3Dot(Position, Up));
  float z = -XMVectorGetX(XMVector3Dot(Position, Look));

  XMStoreFloat3(&m_Right, Right);
  XMStoreFloat3(&m_Up, Up);
  XMStoreFloat3(&m_Look, Look);

  m_MatrixView.r[0] = XMVectorSet(m_Right.x, m_Up.x, m_Look.x, 0.0f);
  m_MatrixView.r[1] = XMVectorSet(m_Right.y, m_Up.y, m_Look.y, 0.0f);
  m_MatrixView.r[2] = XMVectorSet(m_Right.z, m_Up.z, m_Look.z, 0.0f);
  m_MatrixView.r[3] = XMVectorSet(x, y, z, 1.0f);
}

void D3D12Engine::Camera::Forward(float Delta) {
  XMVECTOR ScaledVector = XMVectorReplicate(Delta);
  XMVECTOR Look = XMLoadFloat3(&m_Look);
  XMVECTOR Position = XMLoadFloat3(&m_Position);
  
  XMStoreFloat3(
    &m_Position,

    XMVectorMultiplyAdd(
      ScaledVector, Look, Position
    )
  );
}

void D3D12Engine::Camera::Sideway(float Delta) {
  XMVECTOR ScaledVector = XMVectorReplicate(Delta);
  XMVECTOR Right = XMLoadFloat3(&m_Right);
  XMVECTOR Position = XMLoadFloat3(&m_Position);
  
  XMStoreFloat3(
    &m_Position,
    
    XMVectorMultiplyAdd(
      ScaledVector, Right, Position
    )
  );
}

void D3D12Engine::Camera::RotateX(float angle) {
  const float maxPitch = 0.99f * XM_PIDIV2;

  float newPitch = m_PitchAngle + angle;
  newPitch = std::clamp(newPitch, -maxPitch, maxPitch);

  float allowedAngle = newPitch - m_PitchAngle;
  m_PitchAngle = newPitch;

  if (allowedAngle == 0.0f) {
    return;
  }

  XMMATRIX Right = XMMatrixRotationAxis(XMLoadFloat3(&m_Right), allowedAngle);
  
  XMStoreFloat3(
    &m_Up,
    
    XMVector3TransformNormal(
      XMLoadFloat3(&m_Up), Right
    )
  );
  
  XMStoreFloat3(
    &m_Look,
  
    XMVector3TransformNormal(
      XMLoadFloat3(&m_Look), Right
    )
  );
}

void D3D12Engine::Camera::RotateY(float angle) {
  XMMATRIX Right = XMMatrixRotationY(angle);
  
  XMStoreFloat3(
    &m_Right,
    
    XMVector3TransformNormal(
      XMLoadFloat3(&m_Right), Right
    )
  );
  
  XMStoreFloat3(
    &m_Up,
    
    XMVector3TransformNormal(
      XMLoadFloat3(&m_Up), Right
    )
  );
  
  XMStoreFloat3(
    &m_Look,
    
    XMVector3TransformNormal(
      XMLoadFloat3(&m_Look), Right
    )
  );
}