/*
 Copyright (C) 2010-2012 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapGLCanvas.h"

#include "Controller/CameraEvent.h"
#include "Controller/Input.h"
#include "Controller/InputController.h"
#include "Renderer/Camera.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderContext.h"
#include "Model/Filter.h"
#include "Utility/Console.h"
#include "Utility/GLee.h"
#include "Utility/Preferences.h"
#include "Utility/VecMath.h"

#include <wx/wx.h>

#include <cassert>

using namespace TrenchBroom::Math;
using namespace TrenchBroom::Controller::ModifierKeys;
using namespace TrenchBroom::Controller::MouseButtons;

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(MapGLCanvas, wxGLCanvas)
        EVT_PAINT(MapGLCanvas::OnPaint)
        EVT_CAMERA_MOVE(MapGLCanvas::OnCameraMove)
        EVT_CAMERA_LOOK(MapGLCanvas::OnCameraLook)
        EVT_CAMERA_ORBIT(MapGLCanvas::OnCameraOrbit)
        EVT_LEFT_DOWN(MapGLCanvas::OnMouseLeftDown)
        EVT_LEFT_UP(MapGLCanvas::OnMouseLeftUp)
        EVT_RIGHT_DOWN(MapGLCanvas::OnMouseRightDown)
        EVT_RIGHT_UP(MapGLCanvas::OnMouseRightUp)
        EVT_MIDDLE_DOWN(MapGLCanvas::OnMouseMiddleDown)
        EVT_MIDDLE_UP(MapGLCanvas::OnMouseMiddleUp)
        EVT_MOTION(MapGLCanvas::OnMouseMove)
        EVT_MOUSEWHEEL(MapGLCanvas::OnMouseWheel)
        END_EVENT_TABLE()
        
        int* MapGLCanvas::Attribs() {
            // Todo: make multisample and depth size configurable through prefs
            
            m_attribs = new int[10];
            m_attribs[0] = WX_GL_RGBA;
            m_attribs[1] = WX_GL_DOUBLEBUFFER;
            m_attribs[2] = WX_GL_SAMPLE_BUFFERS;
            m_attribs[3] = GL_TRUE;
            m_attribs[4] = WX_GL_SAMPLES;
            m_attribs[5] = 4;
            m_attribs[6] = WX_GL_DEPTH_SIZE;
            m_attribs[7] = 32;
            m_attribs[8] = 0;
            m_attribs[9] = 0;
            
            return m_attribs;
        }
        
        MapGLCanvas::MapGLCanvas(wxWindow* parent, Utility::Console& console, Renderer::Camera& camera, Renderer::MapRenderer& renderer) :
        wxGLCanvas(parent, wxID_ANY, Attribs()),
        m_firstFrame(true),
		m_mouseCaptured(false),
        m_console(console),
        m_renderer(renderer),
        m_camera(camera),
        m_inputController(new Controller::InputController(*this)) {
            m_glContext = new wxGLContext(this);
            m_console.info("Created OpenGL context");
        }
        
        MapGLCanvas::~MapGLCanvas() {
            if (m_inputController != NULL) {
                delete m_inputController;
                m_inputController = NULL;
            }
        }
        
        void MapGLCanvas::OnPaint(wxPaintEvent& event) {
            wxPaintDC(this);
			if (SetCurrent(*m_glContext)) {
				if (m_firstFrame) {
					m_firstFrame = false;
					const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
					const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
					const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
					m_console.info("Renderer info: %s, version %s from %s", renderer, version, vendor);
				}
            
				Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
				const Color& backgroundColor = prefs.getColor(Preferences::BackgroundColor);
				glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
            
				glDisableClientState(GL_VERTEX_ARRAY);
				glDisableClientState(GL_COLOR_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDisable(GL_TEXTURE_2D);
            
				m_camera.update(0.0f, 0.0f, GetSize().x, GetSize().y);
            
				Model::Filter filter;
				Renderer::RenderContext renderContext(filter);
				m_renderer.render(renderContext);
            
				SwapBuffers();
			} else {
				m_console.error("Unable to set current OpenGL context");
			}
        }
        
        void MapGLCanvas::OnCameraMove(Controller::CameraMoveEvent& event) {
            m_camera.moveBy(event.forward(), event.right(), event.up());
            Refresh();
        }
        
        void MapGLCanvas::OnCameraLook(Controller::CameraLookEvent& event) {
            m_camera.rotate(event.hAngle(), event.vAngle());
            Refresh();
        }
        
        void MapGLCanvas::OnCameraOrbit(Controller::CameraOrbitEvent& event) {
            m_camera.orbit(event.center(), event.hAngle(), event.vAngle());
            Refresh();
        }
        
        void MapGLCanvas::OnMouseLeftDown(wxMouseEvent& event) {
			CaptureMouse();
			m_mouseCaptured = true;
            m_inputController->mouseDown(Left, static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
        }
        
        void MapGLCanvas::OnMouseLeftUp(wxMouseEvent& event) {
			if (m_mouseCaptured)
				ReleaseMouse();
            m_inputController->mouseUp(Left, static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
        }
        
        void MapGLCanvas::OnMouseRightDown(wxMouseEvent& event) {
			CaptureMouse();
			m_mouseCaptured = true;
            m_inputController->mouseDown(Right, static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
        }
        
        void MapGLCanvas::OnMouseRightUp(wxMouseEvent& event) {
			if (m_mouseCaptured)
				ReleaseMouse();
            m_inputController->mouseUp(Right, static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
        }
        
        void MapGLCanvas::OnMouseMiddleDown(wxMouseEvent& event) {
			CaptureMouse();
			m_mouseCaptured = true;
            m_inputController->mouseDown(Middle, static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
        }
        
        void MapGLCanvas::OnMouseMiddleUp(wxMouseEvent& event) {
			if (m_mouseCaptured)
				ReleaseMouse();
            m_inputController->mouseUp(Middle, static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
        }
        
        void MapGLCanvas::OnMouseMove(wxMouseEvent& event) {
            m_inputController->mouseMoved(static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
        }
        
        void MapGLCanvas::OnMouseWheel(wxMouseEvent& event) {
            int lines = event.GetLinesPerAction();
            float delta = static_cast<float>(event.GetWheelDelta()) / lines;
            delta *= event.GetWheelRotation();
            if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL)
                m_inputController->scrolled(delta, 0.0f);
            else if (event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL)
                m_inputController->scrolled(0.0f, delta);
        }
    }
}