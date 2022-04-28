#include "Engine.h"

bool Engine::Initialize( HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height )
{
	return this->render_window.Initialize( this, hInstance, window_title, window_class, width, height );
}

bool Engine::ProcessMessage()
{
	return this->render_window.ProcessMessage();
}
