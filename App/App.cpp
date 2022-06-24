#include "App.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

App::App()
{
	if (!DirectX::XMVerifyCPUSupport())
	{
		throw Error{L"DirectXMath library not supported", L"App.cpp", 10, E_ABORT};
	}

	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
}

void App::OnInitialize()
{
	m_window.OnInitialize(this);
	m_graphics.OnInitialize(m_window.Get());
	m_graphics.CreateRootSignature();
	m_graphics.CreatePSO();
	m_graphics.LoadModel();
	m_timer.Reset();
}

int App::Run()
{
	OnInitialize();
	MSG msg{};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		m_timer.Tick();
		m_graphics.OnUpdate(m_timer.GameTime());
		m_graphics.OnRender(false);
		FrameStats();
	}
	return (int)msg.wParam;
}

void App::FrameStats()
{
	static uint32_t frameCount{};
	static float startTime{};
	std::wstringstream ss;
	if (m_timer.GameTime() - startTime > 1.0f)
	{
		ss << L"FPS : ";
		ss << std::to_wstring(frameCount);
		SetWindowText(m_window.Get(), ss.str().data());
		startTime++;
		frameCount = 0;
	}
	frameCount++;
}