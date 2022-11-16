#include "Engine.h"

bool Engine::Initialize( HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height )
{
	this->timer.Start();

	if ( !this->render_window.Initialize( this, hInstance, window_title, window_class, width, height ) )
	{
		return false;
	}

	if ( !gfx.Initialize( this->render_window.GetHWND(), width, height ) )
	{
		return false;
	}

	return true;
}

bool Engine::ProcessMessage()
{
	return this->render_window.ProcessMessage();
}

void Engine::Update()
{
	float dt = this->timer.GetMilisecondsElapsed();
	this->timer.Restart();

	while ( !mouse.EventBuffersIsEmpty() )
	{
		MouseEvent me = mouse.ReadEvent();
		if ( mouse.IsRightDown() )
		{
		}
	}

	KeyboardEvent ke = keyboard.ReadKey();
	if (ke.GetKeyCode() == 'A' && ke.IsPress())
	{
		this->gfx.CreateSprite("rtsp://admin:1q2w3e4r5t!@192.168.1.211/Streaming/Channels/201");
	}

	if (ke.GetKeyCode() == 'B' && ke.IsPress())
	{
		this->gfx.CreateSprite("rtsp://admin:1q2w3e4r5t!@192.168.1.211/Streaming/Channels/301");
	}

	if (ke.GetKeyCode() == 'C' && ke.IsPress())
	{
		this->gfx.DeleteSprite();
	}
}

void Engine::Update(DX::StepTimer const& timer)
{
	float delta = float(timer.GetElapsedSeconds());

	while (!mouse.EventBuffersIsEmpty())
	{
		MouseEvent me = mouse.ReadEvent();
		if (mouse.IsRightDown())
		{
		}
	}

	KeyboardEvent ke = keyboard.ReadKey();
	if (ke.GetKeyCode() == 'A' && ke.IsPress())
	{
		//this->gfx.CreateSprite("rtsp://admin:1q2w3e4r5t!@192.168.1.211/Streaming/Channels/201");
		this->gfx.CreateSprite("C:/D/SampleMedia/Media1.mp4");
	}

	if (ke.GetKeyCode() == 'B' && ke.IsPress())
	{
		//this->gfx.CreateSprite("rtsp://admin:1q2w3e4r5t!@192.168.1.211/Streaming/Channels/301");
		this->gfx.CreateSprite("C:/D/SampleMedia/Media2.mp4");
	}

	if (ke.GetKeyCode() == 'C' && ke.IsPress())
	{
		this->gfx.DeleteSprite();
	}

	//for (std::vector<Sprite*>::iterator it = gfx.sprites.begin(); it != gfx.sprites.end(); it++)
	//{
	//	(*it)->AdjustRotation(0, 0, -2 * delta);
	//}
}

void Engine::RenderFrame()
{
	gfx.RenderFrame();
}

void Engine::Tick()
{
	static int engineTickCount = 0;
	static bool needSomeUpdate = false;
	s_timer.Tick([&]()
		{
			Update(s_timer);

			//engineTickCount++;
			//if (timer.GetMilisecondsElapsed() > 1000)
			//{
			//	static std::wstring _engineString;
			//	_engineString += L"Update Count: " + std::to_wstring(engineTickCount) + L"\n";
			//	OutputDebugString(_engineString.c_str());
			//	engineTickCount = 0;
			//	timer.Restart();
			//}

			needSomeUpdate = true;
		});

	if (needSomeUpdate)
	{
		RenderFrame();
		needSomeUpdate = false;
	}
}

void Engine::SetFixedTimeStep(bool isFixedTimestep)
{
	s_timer.SetFixedTimeStep(isFixedTimestep);
}

void Engine::SetTargetElapsedSeconds(double targetElapsed)
{
	s_timer.SetTargetElapsedSeconds(targetElapsed);
}
