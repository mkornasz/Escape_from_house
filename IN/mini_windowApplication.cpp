#include "mini_windowApplication.h"

using namespace std;
using namespace mini;

WindowApplication::WindowApplication(HINSTANCE hInstance, int wndWidht, int wndHeight,
									 wstring wndTitle)
									 : m_window(hInstance, wndWidht, wndHeight, wndTitle, this), m_hInstance(hInstance)
{ }

WindowApplication::~WindowApplication()
{
	Shutdown();
}

int WindowApplication::MainLoop()
{
	MSG msg = { 0 };
	while ( GetMessageW(&msg, 0, 0, 0) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return static_cast<int>(msg.wParam);
}

int WindowApplication::Run(int cmdShow)
{
	if (!Initialize())
		return -1;
	m_window.Show(cmdShow);
	int r = MainLoop();
	Shutdown();
	return r;
}