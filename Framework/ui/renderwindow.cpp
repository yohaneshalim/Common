
#include "../stdafx.h"
#include "renderwindow.h"
#include "dockwindow.h"
#include "dockmanager.h"
#include <mmsystem.h>


using namespace graphic;
using namespace framework;

int cRenderWindow::s_adapter = 0;

void RenderProc(cRenderWindow *wnd)
{
	using namespace std::chrono_literals;

	while (wnd->m_isThreadLoop && wnd->isOpen())
	{
		wnd->m_backBuffer.CopyFrom(wnd->m_sharedSurf.m_texture);
		std::this_thread::sleep_for(20ms);
	}
}


cRenderWindow::cRenderWindow()
	: m_sharedRenderer(NULL)
	, m_state(eState::NORMAL)
	, m_isVisible(true)
	, m_isThread(false)
	, m_isRequestResetDevice(false)
	, m_isThreadLoop(false)
	, m_dock(NULL)
	, m_sizingWindow(NULL)
	, m_isDrag(false)
	, m_isResize(false)
	, m_cursorType(eDockSizingType::NONE)
{
}

cRenderWindow::~cRenderWindow()
{
	Clear();
}


bool cRenderWindow::Create(const string &title, const int width, const int height
	, cRenderer *shared // = NULL
)
{
	__super::create(sf::VideoMode(width, height), title.c_str());

	if (!m_renderer.CreateDirectX(getSystemHandle(), width, height, s_adapter++))
	{
		return false;
	}

	m_title = title;
	m_camera.Init(&m_renderer);
	m_camera.SetCamera(Vector3(10, 10, -10), Vector3(0, 0, 0), Vector3(0, 1, 0));
	m_camera.SetProjection(D3DX_PI / 4.f, (float)width / (float)height, 1.f, 10000.0f);
	m_camera.SetViewPort(width, height);
	m_camera.Bind(m_renderer);

	m_light.Init(cLight::LIGHT_DIRECTIONAL,
		Vector4(0.2f, 0.2f, 0.2f, 1), 
		Vector4(0.9f, 0.9f, 0.9f, 1),
		Vector4(0.2f, 0.2f, 0.2f, 1));
	m_light.SetPosition(Vector3(-30000, 30000, -30000));
	m_light.SetDirection(Vector3(1, -1, 1).Normal());

	m_renderer.SetNormalizeNormals(true);
	m_renderer.SetLightEnable(0, true);
	m_light.Bind(m_renderer, 0);

	// ImGui Shared Device
	if (shared)
	{
		m_sharedRenderer = shared;
		m_gui.Init(getSystemHandle(), shared->GetDevice());
		m_backBuffer.Create(m_renderer, width, height, D3DFMT_A8R8G8B8);
		m_sharedSurf.Create(*shared, width, height, 1);
	}
	else
	{
		m_sharedRenderer = NULL;
		m_gui.Init(getSystemHandle(), m_renderer.GetDevice());
		m_backBuffer.Create(m_renderer, width, height, D3DFMT_A8R8G8B8);
		m_sharedSurf.Create(m_renderer, width, height, 1);
	}

	m_gui.SetContext();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("../Media/extra_fonts/Roboto-Medium.ttf", 18);

	if (m_isThread)
	{
		if (m_thread.joinable())
			m_thread.join();

		m_isThreadLoop = true;
		m_thread = std::thread(RenderProc, this);
	}

	return true;
}


// return value : return false if close window
bool cRenderWindow::TranslateEvent()
{
	m_gui.SetContext();

	sf::Event evt;
	while (pollEvent(evt))
	{
		if (evt.type == sf::Event::Closed)
		{
			close();
			return false;
		}
		else
		{
			DefaultEventProc(evt);
		}
	}

	return true;
}


void cRenderWindow::Update(const float deltaSeconds)
{
	RET(!isOpen());
	RET(!m_isVisible);

	if (m_isRequestResetDevice)
	{
		ChangeDevice();
		m_isRequestResetDevice = false;
	}

	// Check Resize End
	if (m_isResize)
	{
		if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) // state change bug fix
		{
			m_isResize = false;
			if (m_renderer.CheckResetDevice())
			{
				sf::Vector2u size = getSize();
				ChangeDevice(size.x, size.y);

				if (m_dock)
					m_dock->ResizeEnd(eDockResize::RENDER_WINDOW, m_dock->m_rect);
			}
		}
	}

	if (m_dock)
		m_dock->Update(deltaSeconds);

	MouseProc(deltaSeconds);

	m_renderer.Update(deltaSeconds);

	OnUpdate(deltaSeconds);
}


void cRenderWindow::MouseProc(const float deltaSeconds)
{
	const Vector2 pos(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
	switch (m_state)
	{
	case eState::NORMAL:
	{
		cDockWindow *sizerWnd = UpdateCursor();
		if (ImGui::IsMouseDown(0))
		{
			if (sizerWnd)
			{
				m_state = eState::SIZE;
				m_mousePos = pos;
				m_sizingWindow = sizerWnd;
			}
			else
			{
				m_state = eState::NORMAL_DOWN;
			}
		}
	}
	break;

	case eState::NORMAL_DOWN:
	{
		if (ImGui::IsMouseReleased(0))
		{
			m_state = eState::NORMAL;
			m_sizingWindow = NULL;
		}
	}
	break;

	case eState::SIZE:
	{
		if (m_sizingWindow)
		{
			Vector2 delta = pos - m_mousePos;
			m_sizingWindow->CalcResizeWindow(eDockResize::DOCK_WINDOW, (m_sizingWindow->GetDockSizingType() == eDockSizingType::VERTICAL) ? (int)delta.y : (int)delta.x);
			m_mousePos = pos;
		}

		if (ImGui::IsMouseReleased(0))
		{
			m_state = eState::NORMAL;
			if (m_sizingWindow)
				m_sizingWindow->ResizeEnd(eDockResize::DOCK_WINDOW, m_sizingWindow->m_rect);
			m_sizingWindow = NULL;
		}
	}
	break;

	case eState::DRAG:
	{
		POINT mousePos;
		GetCursorPos(&mousePos);
		setPosition(sf::Vector2i((int)mousePos.x - 30, (int)mousePos.y - 60));

		if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) // state change bug fix
		{
			m_state = eState::NORMAL;
			setAlpha(1.f);
			cDockManager::Get()->SetDragState(this, false);
		}
	}
	break;

	case eState::DRAG_BIND:
		break;

	default: assert(0);
	}
}


cDockWindow* cRenderWindow::UpdateCursor()
{
	const Vector2 pos(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
	eDockSizingType::Enum cursorType = eDockSizingType::NONE;

	cDockWindow *sizerWnd = GetSizerTargetWindow(pos);
	if (sizerWnd)
		cursorType = sizerWnd->GetDockSizingType();

	if (m_cursorType != cursorType)
	{
		m_cursorType = cursorType;

		switch (cursorType)
		{
		case eDockSizingType::HORIZONTAL: setMouseCursor(sf::Window::SizeHorizontal); break;
		case eDockSizingType::VERTICAL: setMouseCursor(sf::Window::SizeVertical); break;
		default: setMouseCursor(sf::Window::Arrow); break;
		}
	}

	return sizerWnd;
}


void cRenderWindow::Render(const float deltaSeconds)
{
	RET(!isOpen());
	RET(!m_isVisible);

	if (m_dock)
		m_dock->PreRender();	

	m_gui.SetContext();
	m_gui.NewFrame();

	if (m_dock)
	{
		const sf::Vector2u size = getSize();
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(size.x), static_cast<float>(size.y)));

		RenderTitleBar();

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoFocusOnAppearing
			;
		ImGui::Begin("", NULL, flags);
		m_dock->RenderDock();
		ImGui::End();
	}

	if (m_sharedRenderer)
	{
		m_camera.Bind(*m_sharedRenderer);
		m_sharedSurf.Begin(m_renderer);
		if (m_sharedRenderer->ClearScene())
		{
			m_sharedRenderer->BeginScene();
			m_sharedRenderer->GetDevice()->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&Matrix44::Identity);
			m_gui.Render();
			m_sharedRenderer->EndScene();
			//m_sharedRenderer->Present();
		}
		m_sharedSurf.End(m_renderer);

		if (!m_isThread)
			m_backBuffer.CopyFrom(m_sharedSurf.m_texture);

		m_camera.Bind(m_renderer);
		m_light.Bind(m_renderer, 0);

		OnPreRender(deltaSeconds);
		if (m_renderer.ClearScene())
		{
			m_renderer.BeginScene();
			m_renderer.GetDevice()->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&Matrix44::Identity);

			OnRender(deltaSeconds);

			m_backBuffer.Render2D(m_renderer);

			m_renderer.EndScene();
			m_renderer.Present();
		}
		OnPostRender(deltaSeconds);
	}
	else
	{
		m_camera.Bind(m_renderer);
		m_light.Bind(m_renderer, 0);

		PreRender(deltaSeconds);

		if (m_renderer.ClearScene())
		{
			m_renderer.BeginScene();
			m_renderer.GetDevice()->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&Matrix44::Identity);
		
			OnRender(deltaSeconds);
		
			m_gui.Render();

			OnPostRender(deltaSeconds);

			m_renderer.EndScene();
			m_renderer.Present();
		}
		else
		{
			// Device Lost
		}
	}
}


void cRenderWindow::RenderTitleBar()
{
	const ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoFocusOnAppearing
		;
	ImGui::Begin("", NULL, flags);
	
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14.0f, 3.0f));
	ImVec4 childBg = ImGui::GetStyle().Colors[ImGuiCol_ChildWindowBg];

	ImGui::PushStyleColor(ImGuiCol_Button, childBg);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, childBg);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, childBg);

	ImVec2 pos = ImGui::GetCursorPos();
	ImGui::SetCursorPos(ImVec2(0, 0));
	ImVec2 pos2 = ImGui::GetCursorPos();

	ImGui::Button(m_title.c_str(), ImVec2((float)getSize().x-150, 37));
	if (ImGui::IsItemActive())
	{
		int a = 0;
	}
	ImGui::PopStyleColor(3);

	ImGui::SameLine(); ImGui::Button("-", ImVec2(37, 37)); 
	ImGui::SameLine(); ImGui::Button("+", ImVec2(37, 37));
	ImGui::SameLine(); ImGui::Button("X", ImVec2(37, 37));

	ImGui::PopStyleVar(1);
	ImGui::End();
}


void cRenderWindow::PreRender(const float deltaSeconds)
{
	
	OnPreRender(deltaSeconds);
}


void cRenderWindow::LostDevice()
{
	m_isThreadLoop = false;
	if (m_thread.joinable())
		m_thread.join();

	m_gui.InvalidateDeviceObjects();
	m_sharedSurf.LostDevice();
	m_backBuffer.LostDevice();

	if (m_dock)
		m_dock->LostDevice();

	OnLostDevice();
}


void cRenderWindow::ResetDevice(cRenderer *shared)//=NULL
{
	const int width = m_renderer.m_viewPort.m_vp.Width;
	const int height = m_renderer.m_viewPort.m_vp.Height;

	m_camera.SetViewPort(width, height);
	m_gui.CreateDeviceObjects();
	m_sharedSurf.m_vp.Width = width;
	m_sharedSurf.m_vp.Height = height;
	m_sharedSurf.ResetDevice((shared)? *shared : m_renderer);
	
	m_backBuffer.m_imageInfo.Width = width;
	m_backBuffer.m_imageInfo.Height = height;
	m_backBuffer.ResetDevice(m_renderer);

	if (m_dock)
		m_dock->ResetDevice(shared);

	if (m_isThread)
	{
		m_isThreadLoop = true;
		m_thread = std::thread(RenderProc, this);
	}

	OnResetDevice(shared);
}


void cRenderWindow::DefaultEventProc(const sf::Event &evt)
{
	OnEventProc(evt);

	if (m_dock)
		m_dock->DefaultEventProc(evt);

	ImGuiIO& io = ImGui::GetIO();
	switch (evt.type)
	{
	case sf::Event::KeyPressed:
		if (evt.key.code < 256)
			io.KeysDown[evt.key.code] = 1;
		break;

	case sf::Event::MouseMoved:
		io.MousePos.x = (float)evt.mouseMove.x;
		io.MousePos.y = (float)evt.mouseMove.y;
		break;

	case sf::Event::MouseButtonPressed:
		switch (evt.mouseButton.button)
		{
		case sf::Mouse::Left: {
			m_clickPos = ImVec2((float)evt.mouseButton.x, (float)evt.mouseButton.y);
			io.MouseDown[0] = true;
		}
			break;
		case sf::Mouse::Right: io.MouseDown[1] = true; break;
		case sf::Mouse::Middle: io.MouseDown[2] = true; break;
		}
		break;

	case sf::Event::MouseButtonReleased:
		switch (evt.mouseButton.button)
		{
		case sf::Mouse::Left: io.MouseDown[0] = false; break;
		case sf::Mouse::Right: io.MouseDown[1] = false; break;
		case sf::Mouse::Middle: io.MouseDown[2] = false; break;
		}
		break;

	case sf::Event::MouseWheelMoved:
		io.MouseWheel += evt.mouseWheel.delta > 0 ? +1.0f : -1.0f;
		break;

	case sf::Event::Resized:
	{
		m_isResize = true;
		//if (m_renderer.CheckResetDevice())
		//{
		//	ChangeDevice(evt.size.width, evt.size.height);
		//}
	}
	break;
	}
}


cDockWindow* cRenderWindow::GetSizerTargetWindow(const Vector2 &mousePt)
{
	RETV(!m_dock, false);

	queue<cDockWindow*> q;
	q.push(m_dock);

	while (!q.empty())
	{
		cDockWindow *wnd = q.front();
		q.pop();

		if (wnd->m_state != eDockState::VIRTUAL)
			continue;

		if (wnd->IsInSizerSpace(mousePt))
			return wnd;

		if (wnd->m_lower)
			q.push(wnd->m_lower);
		if (wnd->m_upper)
			q.push(wnd->m_upper);
	}

	return NULL;
}


void cRenderWindow::SetDragState()
{
	m_state = eState::DRAG;
	setAlpha(0.3f);
	cDockManager::Get()->SetDragState(this);
}


void cRenderWindow::SetDragBindState()
{
	m_state = eState::DRAG_BIND;
	if (m_dock)
		m_dock->SetBindState(true);
}


void cRenderWindow::SetFinishDragBindState()
{
	m_state = eState::NORMAL;
	if (m_dock)
		m_dock->SetBindState(false);
}


bool cRenderWindow::IsDragState()
{
	return m_state == eState::DRAG;
}


void cRenderWindow::Sleep()
{
	m_isThreadLoop = false;
	if (m_thread.joinable())
		m_thread.join();

	setVisible(false);
}


void cRenderWindow::WakeUp(const string &title, const int width, const int height)
{
	setTitle(title);
	setSize(sf::Vector2u((u_int)width, (u_int)height));
	m_isThreadLoop = true;
	m_thread = std::thread(RenderProc, this);

	setVisible(true);
}


void cRenderWindow::ChangeDevice(
	const int width //=0
	, const int height //=0)
)
{
	LostDevice();

	const bool restResource = (cDockManager::Get()->m_mainWindow == this);
	m_renderer.ResetDevice(0, 0, false, restResource);

	sRectf rect(0, 0, (float)((width==0)? getSize().x : width), (float)((height==0)? getSize().y : height));
	if (m_dock)
		m_dock->CalcResizeWindow(eDockResize::RENDER_WINDOW, rect);

	ResetDevice(m_sharedRenderer);
}


void cRenderWindow::RequestResetDeviceNextFrame()
{
	m_isRequestResetDevice = true;
}


void cRenderWindow::Clear()
{
	m_gui.Shutdown();

	m_isThreadLoop = false;
	if (m_thread.joinable())
		m_thread.join();

	// Delete All Docking Window
	if (m_dock)
		m_dock->Clear();
	m_dock = NULL;
}