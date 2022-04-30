#include "Engine.h"

int APIENTRY wWinMain( _In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPWSTR lpCmdLine,
					   _In_ int nCmdShow )
{
	Engine engine;

	if ( engine.Initialize( hInstance, "Title", "MyWindowClas", 800, 600 ) )
	{
		while ( engine.ProcessMessage() == true )
		{
			engine.Update();
			engine.RenderFrame();
		}
	}

	return 0;
}

// INPUT ASSEMBLER - COMPLETED
// VERTEX SHADER - COMPLETED
// RASTERIZER - COMPLETED
// PIXEL SHADER - COMPLETED
// OUTPUT MERGER - COMPLETED