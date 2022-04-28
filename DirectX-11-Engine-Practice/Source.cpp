#include "Engine.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DirectXTK.lib")

int APIENTRY wWinMain( _In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPWSTR lpCmdLine,
					   _In_ int nCmdShow )
{
	Engine engine;
	engine.Initialize( hInstance, "Title", "MyWindowClas", 800, 600 );

	while ( engine.ProcessMessage() == true )
	{

	}

	return 0;
}
