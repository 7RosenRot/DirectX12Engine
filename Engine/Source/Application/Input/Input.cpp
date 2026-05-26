#include <Include/Graphics/Scene/Input.hpp>

// ↓ Initializing ↓
HWND D3D12Engine::Input::m_Hwnd = nullptr;
bool D3D12Engine::Input::m_Keys[256] = {false};

bool  D3D12Engine::Input::m_MouseLocker = false;
float D3D12Engine::Input::m_MouseDeltaX = 0.0F;
float D3D12Engine::Input::m_MouseDeltaY = 0.0F;
// ↑ Initializing ↑

void D3D12Engine::Input::Initialize(HWND hwnd) {
  m_Hwnd = hwnd;
}

void D3D12Engine::Input::SetStatusKey(UINT8 key, bool isDown) {
  m_Keys[key] = isDown;
}

bool D3D12Engine::Input::IsKeyDown(UINT8 key) {
  return m_Keys[key];
}

void D3D12Engine::Input::SetMouseLock(bool lock) {
  if (m_MouseLocker == lock) {
    return;
  }
  
  m_MouseLocker = lock;
  
  if (m_MouseLocker) {
    ShowCursor(FALSE);
    
    int centerX = GetSystemMetrics(SM_CXSCREEN) / 2;
    int centerY = GetSystemMetrics(SM_CYSCREEN) / 2;
    
    SetCursorPos(centerX, centerY);
  } else {
    ShowCursor(TRUE);
  }
}

void D3D12Engine::Input::OnMouseMove(int x, int y) {
  if (!m_MouseLocker) {
    return;
  }

  RECT rect;
  GetClientRect(m_Hwnd, &rect);
  POINT centerPoint{
    (rect.right - rect.left) / 2,
    (rect.bottom - rect.top) / 2 
  };
  
  float dx = static_cast<float>(x - centerPoint.x);
  float dy = static_cast<float>(y - centerPoint.y);

  if (dx == 0.0f && dy == 0.0f) {
    return;
  }

  m_MouseDeltaX += dx;
  m_MouseDeltaY += dy;

  ClientToScreen(m_Hwnd, &centerPoint);
  SetCursorPos(centerPoint.x, centerPoint.y);
}

void D3D12Engine::Input::GetMouseDelta(float& dx, float& dy) {
  dx = m_MouseDeltaX;
  dy = m_MouseDeltaY;

  m_MouseDeltaX = 0.0F;
  m_MouseDeltaY = 0.0F;
}