#pragma once
#include "ErrorLogger.h"

class RenderWindow
{
public:
	bool Initialize( HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height );
	bool ProcessMessage();
	~RenderWindow();

private:
	void RegisterWindowClass();
	HWND handle = NULL;	// Handle to this window
	HINSTANCE hInstance = NULL;	// Handle to application instance
	std::string window_title = "";
	std::wstring window_title_wide = L"";	// Wide string representation of window title
	std::string window_class = "";
	std::wstring window_class_wide = L"";
	int width = 0;
	int height = 0;
};
