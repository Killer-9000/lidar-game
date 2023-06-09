#pragma once

#include "IWindow.h"

#include <backends/imgui_impl_sdl.h>

#include <memory>
#include <vector>

class WindowMgr
{
	typedef std::vector<std::shared_ptr<IWindow>> WindowVector;

	WindowMgr() { }
	~WindowMgr() { ClearWindows(); }

public:
	static WindowMgr* Instance()
	{
		static WindowMgr instance;
		return &instance;
	}

	WindowVector::reference AddWindow(IWindow* window)
	{
		WindowVector::reference wnd = m_windows.emplace_back(window);
		wnd->Init();
		return wnd;
	}

	WindowVector::const_iterator GetWindow(IWindow* window)
	{
		WindowVector::const_iterator itr = std::find_if(m_windows.begin(), m_windows.end(),
			[window](WindowVector::reference& _window) {
				return _window.get() == window;
			});
		return itr;
	}
	WindowVector::const_iterator GetWindowFromID(uint32_t windowID)
	{
		return GetWindow((IWindow*)SDL_GetWindowData(SDL_GetWindowFromID(windowID), "container"));
	}

	void RemoveWindow(IWindow* window)
	{
		WindowVector::const_iterator itr = std::find_if(m_windows.begin(), m_windows.end(),
			[window](WindowVector::reference& _window) {
				return _window.get() == window;
			});
		m_windows.erase(itr);
	}

	void ClearWindows()
	{
		for (auto& window : m_windows)
			window->Deinit();
		m_windows.clear();
	}

	size_t WindowCount() { return m_windows.size(); }

	void CleanupWindows()
	{
		for (int i = 0; i < m_windows.size(); i++)
		{
			if (m_windows[i]->ShouldClose())
			{
				ImGui::SetCurrentContext(m_windows[i]->GetImGuiContext());
				m_windows[i]->Deinit();
				m_windows.erase(m_windows.begin() + i);
			}
		}
	}

	void UpdateWindows()
	{
		for (int i = 0; i < m_windows.size(); i++)
		{
			std::shared_ptr<IWindow> window = m_windows[i];
			if (!window->ShouldClose())
			{
				ImGui::SetCurrentContext(window->GetImGuiContext());
				window->Update();
				window->Render();
				window->m_frameEnd = std::chrono::high_resolution_clock::now().time_since_epoch();
				window->m_frameDelta = (float)(window->m_frameEnd - window->m_frameStart).count() / (std::nano::den / std::milli::den);
				window->m_frameStart = std::chrono::high_resolution_clock::now().time_since_epoch();
			}
		}
	}

	void ProcessSDLEvents()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)// && IsCurrentSessionRemoteable())
					IWindow::s_keyboardState[event.key.keysym.scancode] = event.key.state;
			}
			default:
			{
				for (auto& window : m_windows)
				{
					auto window = GetWindowFromID(event.window.windowID);
					if (window == m_windows.end())
						continue;
					ImGui::SetCurrentContext((*window)->GetImGuiContext());
					ImGui_ImplSDL2_ProcessEvent(&event);
					(*window)->ProcessSDLEvent(event);
				}
			}
			}
		}
	}

private:
	WindowVector m_windows;
};

#define SWindowMgr WindowMgr::Instance()