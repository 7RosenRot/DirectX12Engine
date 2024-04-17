#pragma once
#include <Window.h>

class Application : public Window {
public:
	virtual void OnCreate() override;
	virtual void OnUpdate() override;
	virtual void OnDestroy() override;
};