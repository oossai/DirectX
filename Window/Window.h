/**
 * Implementation of a Win32 windowing system
 */

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "../Headers.h"
#include "../Macros.h"
#include "../Error.h"
#include "../Core/Graphics.h"

class App;

class Window
{
private:
	HWND m_window = nullptr; // handle to window
	const wchar_t* CLASSNAME = L"Main Window";
	RECT m_windowRect; // window dimensions
	bool m_fullscreen = false;
	App* m_app = nullptr;
public:
	Window() = default;
	~Window() = default;
	
	HWND Get() { return m_window; }
	/**
	 * Initializes window resources.
	 *
	 * @throws IoError Thrown if `fileName` does not exist;
	 */
	void OnInitialize(App* app);
private:
	NO_COPY_OR_MOVE(Window);
	
	/**
	 * Toggles fullScreen on Alt Enter key sequence
	 *
	 */
	void ToggleFullScreen();

	/**
	 * Recieves messages from the OS for the current thread.
	 * Forwards the message to either ProcessMessage or DefWindowProc
	 * 
	 * @param hWnd, handle to the window.
	 * @param msg, the message.
	 * @param wParam, additonal info about the message.
	 * @param lParam, additonal info about the message.
	 * 
	 * @return the result of processing a message, dependent on msg 
	 */
	static LRESULT CALLBACK WindowProc(HWND hWnd, unsigned msg, WPARAM wParam, LPARAM lParam);

	/**
	 * Processes messages recieved from WindowProc.
	 * 
	 * @param hWnd, handle to the window.
	 * @param msg, the message.
	 * @param wParam, additonal info about the message.
	 * @param lParam, additonal info about the message.
	 * 
	 * @return the result of processing a message, dependent on msg 
	 */
	LRESULT ProcessMessage(HWND hWnd, unsigned Msg, WPARAM wParam, LPARAM lParam);
};

#endif