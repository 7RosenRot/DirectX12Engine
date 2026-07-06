#pragma once
#include <windows.h>
#include <DirectXMath.h>

class Input {
 public:
  Input() = delete;
  ~Input() = delete;

  static void Initialize(HWND hwnd);
  
  // ↓ Keyboard ↓ 
  static void SetStatusKey(UINT8 key, bool isDown);
  static bool IsKeyDown(UINT8 key);
  // ↑ Keyboard ↑

  // ↓ Mouse ↓
  static void OnMouseMove(int x, int y);
  static void GetMouseDelta(float& outDx, float& outDy);
  static void SetMouseLock(bool lock);
  static bool IsMouseLocked() { return m_MouseLocker; }
  static void SetMmbMode(bool active) { m_MmbMode = active; }
  static bool IsMmbMode() { return m_MmbMode; }
  static void SetMouseWheelDelta(float delta) { m_MouseWheelDelta += delta; }
  static float GetMouseWheelDelta();
  // ↑ Mouse ↑
 private:
  // ↓ Keyboard ↓
  static HWND m_Hwnd;
  static bool m_Keys[256];
  // ↑ Keyboard ↑
  
  // ↓ Mouse ↓
  static bool  m_MouseLocker;
  static bool  m_MmbMode;
  static bool  m_IgnoreNextMouseMove;
  
  static float m_MouseDeltaX;
  static float m_MouseDeltaY;
  static float m_MouseWheelDelta;
  // ↑ Mouse ↑
};