#pragma once
#include "WindowContainer.h"
#include "Timer.h"

#include "StepTimer.h"

class Engine : WindowContainer
{
public:
	bool Initialize( HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height );
	bool ProcessMessage();
	void Update();
	void Update(DX::StepTimer const& timer);
	void RenderFrame();

	void Tick();

	void SetFixedTimeStep(bool isFixedTimestep);
	void SetTargetElapsedSeconds(double targetElapsed);

private:
	Timer timer;

	DX::StepTimer s_timer;
};
