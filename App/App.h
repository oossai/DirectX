#ifndef __APP_H__
#define __APP_H__

#include "../Headers.h"
#include "../Window/Window.h"
#include "../Core/Graphics.h"
#include "../Utility/Timer.h"
#include "../Models/Mesh.h"
#include "Bunny.h"

class App
{
private:
	Window m_window;
	Bunny m_graphics;
	Timer m_timer;
public:
	App();
	~App() = default;
	int Run();
	void OnResize(unsigned width, unsigned height, bool minimized) 
	{
		if (m_graphics)
		{
			m_graphics.OnResize(width, height, minimized); 
		}
	}
private:
	NO_COPY_OR_MOVE(App);
	void OnInitialize();
	void FrameStats();
};

#endif