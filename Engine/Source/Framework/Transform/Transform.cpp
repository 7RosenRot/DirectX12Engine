#include <algorithm>

#include <Framework/Transform/Transform.hpp>

void Transform::SetLookTarget(FXMVECTOR Position, FXMVECTOR Target, FXMVECTOR TrueUp) {
  XMVECTOR Look = XMVector3Normalize(XMVectorSubtract(Target, Position));
  XMVECTOR Right = XMVector3Normalize(XMVector3Cross(TrueUp, Look));
  XMVECTOR Up = XMVector3Cross(Look, Right);

  XMStoreFloat3(&m_Position, Position);
  XMStoreFloat3(&m_Look, Look);
  XMStoreFloat3(&m_Right, Right);
  XMStoreFloat3(&m_Up, Up);
}

void Transform::Forward(float Delta) {
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

void Transform::Sideway(float Delta) {
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

void Transform::RotateX(float angle) {
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

void Transform::RotateY(float angle) {
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

XMMATRIX Transform::GetMatrixView() {
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

  XMMATRIX view;
  view.r[0] = XMVectorSet(m_Right.x, m_Up.x, m_Look.x, 0.0f);
  view.r[1] = XMVectorSet(m_Right.y, m_Up.y, m_Look.y, 0.0f);
  view.r[2] = XMVectorSet(m_Right.z, m_Up.z, m_Look.z, 0.0f);
  view.r[3] = XMVectorSet(x, y, z, 1.0f);

  return view;
}

XMMATRIX Transform::GetMatrixModel() {
  XMMATRIX S = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
  XMMATRIX R = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
  XMMATRIX T = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

  return S * R * T;
}