#include "App/App.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	try
	{
		App app;
		return app.Run();
	}
	catch(Error& e)
	{
		MessageBox(nullptr, e.ErrorString(), L"Error", MB_OK | MB_ICONEXCLAMATION);
	
		return EXIT_FAILURE;
	}
	catch(std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Standard Expect", MB_OK | MB_ICONEXCLAMATION);
	
		return EXIT_FAILURE;
	}
}