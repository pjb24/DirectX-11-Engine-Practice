#pragma once
#include "RenderWindow.h"

class WindowContainer
{
public:
	// WindowProc
	LRESULT WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	// Mouse
	// Keyboard

protected:
	RenderWindow render_window;
};
