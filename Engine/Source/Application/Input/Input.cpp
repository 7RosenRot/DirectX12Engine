#include <Application/Input/Input.hpp>

// ↓ Initializing ↓
HWND Input::m_Hwnd = nullptr;
bool Input::m_Keys[256] = {false};

bool  Input::m_MouseLocker         = false;
bool  Input::m_IgnoreNextMouseMove = false;
float Input::m_MouseDeltaX         = 0.0F;
float Input::m_MouseDeltaY         = 0.0F;
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

    // ↓ Set cursor in center of client's ViewPort ↓
    RECT ClientRect;
    GetClientRect(m_Hwnd, &ClientRect);
    POINT centerPoint{
      (ClientRect.right - ClientRect.left) / 2,
      (ClientRect.bottom - ClientRect.top) / 2
    };
    
    ClientToScreen(m_Hwnd, &centerPoint);
    SetCursorPos(centerPoint.x, centerPoint.y);
    // ↑ Set cursor in center of client's ViewPort ↑

    // ↓ Skip first WM_MOUSEMOVE ↓
    m_IgnoreNextMouseMove = true;
    // ↑ Skip first WM_MOUSEMOVE ↑
  } else {
    ShowCursor(TRUE);
  }
}

void Input::OnMouseMove(int x, int y) {
  if (!m_MouseLocker) {
    return;
  }

  RECT ClientRect;
  GetClientRect(m_Hwnd, &ClientRect);
  POINT centerPoint{
    (ClientRect.right - ClientRect.left) / 2,
    (ClientRect.bottom - ClientRect.top) / 2 
  };

  // ↓ Skip first Frame ↓
  if (m_IgnoreNextMouseMove) {
    m_IgnoreNextMouseMove = false;
    
    ClientToScreen(m_Hwnd, &centerPoint);
    SetCursorPos(centerPoint.x, centerPoint.y);
    
    return;
  }
  // ↑ Skip first Frame ↑
  
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