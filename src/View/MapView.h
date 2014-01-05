/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__MapView__
#define __TrenchBroom__MapView__


#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "Assets/AssetTypes.h"
#include "Controller/Command.h"
#include "Renderer/GL.h"
#include "Renderer/Camera.h"
#include "Renderer/Compass.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderResources.h"
#include "Renderer/Vbo.h"
#include "View/InputState.h"
#include "View/MovementRestriction.h"
#include "View/ViewTypes.h"

#include <vector>
#include <wx/datetime.h>
#include <wx/event.h>
#include <wx/glcanvas.h>

namespace TrenchBroom {
    class Logger;
    
    namespace Model {
        class BrushFace;
        class Object;
        class SelectionResult;
    }
    
    namespace Renderer {
        class RenderContext;
    }
    
    namespace View {
        class BaseTool;
        class CameraTool;
        class ClipTool;
        class CreateBrushTool;
        class MoveObjectsTool;
        class ResizeBrushesTool;
        class RotateObjectsTool;
        class SelectionTool;
        class VertexTool;
        
        class MapView : public wxGLCanvas {
        private:
            Logger* m_logger;
            bool m_initialized;
            wxGLContext* m_glContext;
            
            Renderer::Vbo m_auxVbo;
            Color m_focusColor;
            
            View::MapDocumentWPtr m_document;
            ControllerWPtr m_controller;
            Renderer::Camera m_camera;
            Renderer::RenderResources m_renderResources;
            Renderer::MapRenderer m_renderer;
            Renderer::Compass m_compass;
            
            InputState m_inputState;
            MovementRestriction m_movementRestriction;
            wxPoint m_clickPos;
            CameraTool* m_cameraTool;
            ClipTool* m_clipTool;
            CreateBrushTool* m_createBrushTool;
            MoveObjectsTool* m_moveObjectsTool;
            VertexTool* m_vertexTool;
            ResizeBrushesTool* m_resizeBrushesTool;
            RotateObjectsTool* m_rotateObjectsTool;
            SelectionTool* m_selectionTool;
            BaseTool* m_toolChain;
            BaseTool* m_dragReceiver;
            BaseTool* m_modalReceiver;
            
            bool m_cancelNextDrag;
            bool m_ignoreNextClick;
            wxDateTime m_lastFrameActivation;
        public:
            MapView(wxWindow* parent, Logger* logger, View::MapDocumentWPtr document, ControllerWPtr controller);
            ~MapView();
            
            Renderer::RenderResources& renderResources();
            
            bool anyToolActive() const;
            void toggleClipTool();
            bool clipToolActive() const;
            bool canToggleClipSide() const;
            void toggleClipSide();
            bool canPerformClip() const;
            void performClip();
            bool canDeleteLastClipPoint() const;
            void deleteLastClipPoint();
            
            void toggleRotateObjectsTool();
            bool rotateObjectsToolActive() const;
            
            void toggleVertexTool();
            bool vertexToolActive() const;
            
            void toggleMovementRestriction();
            
            Vec3 pasteObjectsDelta(const BBox3& bounds) const;
            
            void OnKey(wxKeyEvent& event);
            void OnMouseButton(wxMouseEvent& event);
            void OnMouseMotion(wxMouseEvent& event);
            void OnMouseWheel(wxMouseEvent& event);
            void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
            void OnActivateFrame(wxActivateEvent& event);
            
            void OnPaint(wxPaintEvent& event);
            void OnSize(wxSizeEvent& event);
            
            void OnPopupReparentBrushes(wxCommandEvent& event);
            void OnPopupMoveBrushesToWorld(wxCommandEvent& event);
            void OnPopupCreatePointEntity(wxCommandEvent& event);
            void OnPopupCreateBrushEntity(wxCommandEvent& event);
            void OnUpdatePopupMenuItem(wxUpdateUIEvent& event);
        private:
            void updateReparentBrushesMenuItem(wxUpdateUIEvent& event) const;
            void updateMoveBrushesToWorldMenuItem(wxUpdateUIEvent& event) const;
            Model::Entity* findNewBrushParent(const Model::BrushList& brushes) const;
            bool canReparentBrushes(const Model::BrushList& brushes, const Model::Entity* newParent) const;
            void reparentBrushes(const Model::BrushList brushes, Model::Entity* newParent);
            Assets::EntityDefinition* findEntityDefinition(const Assets::EntityDefinitionGroups& groups, const size_t index) const;
            void createPointEntity(const Assets::PointEntityDefinition& definition);
            void createBrushEntity(const Assets::BrushEntityDefinition& definition);
            
            void bindObservers();
            void unbindObservers();
            
            void documentWasNewed();
            void documentWasLoaded();
            void objectWasAdded(Model::Object* object);
            void objectDidChange(Model::Object* object);
            void faceDidChange(Model::BrushFace* face);
            void selectionDidChange(const Model::SelectionResult& result);
            void commandDoneOrUndone(Controller::Command::Ptr command);
            void modsDidChange();
            void preferenceDidChange(const IO::Path& path);
            
            void updatePickResults(const int x, const int y);
            
            void createTools();
            void deleteTools();
            void toggleTool(BaseTool* tool);
            void cancelCurrentDrag();
            ModifierKeyState modifierKeys();
            bool updateModifierKeys();
            bool clearModifierKeys();
            MouseButtonState mouseButton(wxMouseEvent& event);
            void showPopupMenu();
            wxMenu* makeEntityGroupsMenu(const Assets::EntityDefinitionGroups& groups, int id);
            void bindEvents();
            
            void setupGL(Renderer::RenderContext& context);
            void setRenderOptions(Renderer::RenderContext& context);
            void clearBackground(Renderer::RenderContext& context);
            void renderCoordinateSystem(Renderer::RenderContext& context);
            void renderCoordinateSystem(const Color& xColor, const Color& yColor, const Color& zColor);
            void renderMap(Renderer::RenderContext& context);
            void renderTools(Renderer::RenderContext& context);
            void renderCompass(Renderer::RenderContext& context);
            void renderFocusRect(Renderer::RenderContext& context);
            
            void initializeGL();
            
            static const Renderer::RenderResources::GLAttribs& attribs();
            static int depthBits();
            static bool multisample();
        };
    }
}

#endif /* defined(__TrenchBroom__MapView__) */
