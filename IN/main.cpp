#include "in_scene.h"
#include "mini_exceptions.h"
#include "mini_collision.h"

using namespace std;
using namespace mini;
using namespace DirectX;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
	UNREFERENCED_PARAMETER(prevInstance);
	UNREFERENCED_PARAMETER(cmdLine);
	unique_ptr<INScene> app;
	int exitCode = 0;
	try
	{
		app.reset(new INScene(hInstance, 1280, 720));
		exitCode = app->Run(cmdShow);
	}
	catch (Exception& e)
	{
		MessageBoxW(NULL, e.getMessage().c_str(), L"B³¹d", MB_OK);
		exitCode = e.getExitCode();
	}
	catch (std::exception& e)
	{
		string s(e.what());
		MessageBoxW(NULL, wstring(s.begin(), s.end()).c_str(), L"B³¹d!", MB_OK);
	}
	catch(const char* str)
	{
		string s(str);
		MessageBoxW(NULL, wstring(s.begin(), s.end()).c_str(), L"B³¹d!", MB_OK);
	}
	catch(const wchar_t* str)
	{
		MessageBoxW(NULL, str, L"B³¹d!", MB_OK);
	}
	catch(...)
	{
		MessageBoxW(NULL, L"Nieznany B³¹d", L"B³¹d", MB_OK);
		exitCode = -1;
	}
	app.reset();
	return exitCode;
}