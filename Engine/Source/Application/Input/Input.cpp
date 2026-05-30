#include <Application/Input/Input.hpp>

// ↓ Initializing ↓
HWND Input::m_Hwnd = nullptr;
bool Input::m_Keys[256] = {false};

bool  Input::m_MouseLocker = false;
float Input::m_MouseDeltaX = 0.0F;
float Input::m_MouseDeltaY = 0.0F;
// ↑ Initializing ↑

void Input::Initialize(HWND hwnd) {
  m_Hwnd = hwnd;
}

void Input::SetStatusKey(UINT8 key, bool isDown) {
  m_Keys[key] = isDown;
}

bool Input::IsKeyDown(UINT8 key) {
  return m_Keys[key];
}

void Input::SetMouseLock(bool lock) {
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

void Input::OnMouseMove(int x, int y) {
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

void Input::GetMouseDelta(float& dx, float& dy) {
  dx = m_MouseDeltaX;
  dy = m_MouseDeltaY;

  m_MouseDeltaX = 0.0F;
  m_MouseDeltaY = 0.0F;
}