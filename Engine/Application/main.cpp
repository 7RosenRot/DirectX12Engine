#include <Window.h>

int main() {
	DirectX12Game MyApplication(1280, 720, L"RosenRot | Geometry2D");

	int nCmdShow = SW_SHOW;
	HINSTANCE hInstance = nullptr;
	return Window::Run(&MyApplication, hInstance, nCmdShow);
}