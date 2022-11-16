#include "Engine.h"

int APIENTRY wWinMain( _In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow )
{
	HRESULT hr = CoInitialize( NULL );
	if ( FAILED( hr ) )
	{
		ErrorLogger::Log( hr, "Failed to call CoInitialize." );
		return -1;
	}

	Engine engine;

	if ( engine.Initialize( hInstance, "Title", "MyWindowClas", 800, 600 ) )
	{
		engine.SetFixedTimeStep(true);
		engine.SetTargetElapsedSeconds(1.f / 30.f);
		while ( engine.ProcessMessage() == true )
		{
			//engine.Update();
			//engine.RenderFrame();
			engine.Tick();
		}
	}

	return 0;
}
