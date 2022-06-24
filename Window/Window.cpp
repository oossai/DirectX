#include "Window.h"
#include "../App/App.h"

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

void Window::OnInitialize(App* app)
{
	assert(app);

	m_app = app;

	WNDCLASSEX wndClass{};
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WindowProc;
	wndClass.hInstance = GetModuleHandle(nullptr);
	wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = CLASSNAME;
	wndClass.hIconSm = nullptr;	

	if (RegisterClassEx(&wndClass) == 0)
	{
		THROW_IF_FAILED(__HRESULT_FROM_WIN32(GetLastError()));
	}

	int displayWidth = GetSystemMetrics(SM_CXSCREEN);
	int displayHeight = GetSystemMetrics(SM_CYSCREEN);

	m_windowRect = { 0, 0, 1280, 720 };

	AdjustWindowRect(&m_windowRect, WS_OVERLAPPEDWINDOW, false);

	int windowWidth = m_windowRect.right - m_windowRect.left;
	int windowHeight = m_windowRect.bottom - m_windowRect.top;

	// Make window initially load at the center of display
	int initialXPositon = std::max(0, (displayWidth - windowWidth) / 2);
	int initialYPositon = std::max(0, (displayHeight - windowHeight) / 2);

	m_window = CreateWindow(CLASSNAME, L"Direct3D", WS_OVERLAPPEDWINDOW, 
		initialXPositon, initialYPositon, windowWidth, windowHeight, 
		nullptr, nullptr, GetModuleHandle(nullptr), this);

	if (m_window == nullptr)
	{
		THROW_IF_FAILED(__HRESULT_FROM_WIN32(GetLastError()));
	}

	// dark mode title bar
	BOOL value = TRUE;
	DwmSetWindowAttribute(m_window, DWMWA_USE_IMMERSIVE_DARK_MODE, 
		&value, sizeof(value));

	// m_graphics.OnInitialize(m_window);

	ShowWindow(m_window, SW_NORMAL);
}

void Window::ToggleFullScreen()
{
	if (!m_fullscreen)
	{
		// Save window dimensions to m_windowRect
		GetWindowRect(m_window, &m_windowRect);

		// Remove decorations
		LONG windowStyle = GetWindowLong(m_window, GWL_STYLE) &  
			~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
		
		SetWindowLong(m_window, GWL_STYLE, windowStyle);

		// Go fullscreen on neareast monitor
		HMONITOR hMonitor = MonitorFromWindow(m_window, MONITOR_DEFAULTTONEAREST);
			
		MONITORINFO monitorInfo{};
		monitorInfo.cbSize = sizeof(monitorInfo);
			
		GetMonitorInfo(hMonitor, &monitorInfo);

		int x = monitorInfo.rcMonitor.left;
		int y = monitorInfo.rcMonitor.top;
		int cx = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
		int cy = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

		SetWindowPos(m_window, HWND_TOP, 0,0,0,0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
		ShowWindow(m_window, SW_MAXIMIZE);
	}
	else
	{
		// Restore windowed mode
		SetWindowLong(m_window, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		int x = m_windowRect.left;
		int y = m_windowRect.top;
		int cx = m_windowRect.right - m_windowRect.left;
		int cy = m_windowRect.bottom - m_windowRect.top;
		SetWindowPos(m_window, HWND_NOTOPMOST, x, y, cx, cy, SWP_FRAMECHANGED | SWP_NOACTIVATE);
		ShowWindow(m_window, SW_SHOW);
	}
	m_fullscreen = !m_fullscreen;
}

LRESULT CALLBACK Window::WindowProc(HWND hWnd, unsigned Msg, WPARAM wParam, LPARAM lParam)
{
	Window* pWindow = reinterpret_cast<Window*>(GetWindowLongPtr(
			hWnd, GWLP_USERDATA));

	if (Msg == WM_CREATE)
	{
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pWindow = reinterpret_cast<Window*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
	}
	if (pWindow)
	{
		return pWindow->ProcessMessage(hWnd, Msg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
}

LRESULT CALLBACK Window::ProcessMessage(HWND hWnd, unsigned Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	
	case WM_ENTERSIZEMOVE:
		break;

	case WM_EXITSIZEMOVE:
		{
			RECT clientRect{};
        	GetClientRect(hWnd, &clientRect);
			uint32_t width = clientRect.right - clientRect.left;
			uint32_t height = clientRect.bottom - clientRect.top;
			break;
		}

	case WM_SIZE:
		m_app->OnResize(LOWORD(lParam), HIWORD(lParam), wParam == SIZE_MINIMIZED);
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* defaultWindowBound = reinterpret_cast<MINMAXINFO*>(lParam);
			defaultWindowBound->ptMinTrackSize.x = 640;
			defaultWindowBound->ptMinTrackSize.y = 480;
			break;
		}
	case WM_ACTIVATE:
	case WM_ACTIVATEAPP:
		break;
	
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
    break;

	case WM_MOUSEACTIVATE:
    	// When you click activate the window, we want Mouse to ignore it.
    	return MA_ACTIVATEANDEAT;
	
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
    break;

	case WM_SYSKEYDOWN:
		// Process alt-enter due to swapchain
    	if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
    	{
			ToggleFullScreen();
    	}
    break;
	
	case WM_MENUCHAR:
        // A menu is active and the user presses a key that does not correspond
        // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
        return MAKELRESULT(0, MNC_CLOSE);
	
	default:
		break;
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}