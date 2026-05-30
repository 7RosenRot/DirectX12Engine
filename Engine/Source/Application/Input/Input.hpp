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
  // ↑ Mouse ↑
 private:
  // ↓ Keyboard ↓
  static HWND m_Hwnd;
  static bool m_Keys[256];
  // ↑ Keyboard ↑
  
  // ↓ Mouse ↓
  static bool  m_MouseLocker;
  static float m_MouseDeltaX;
  static float m_MouseDeltaY;
  // ↑ Mouse ↑
};