/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "MapView3D.h"
#include "Logger.h"
#include "Renderer/Compass.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Vbo.h"
#include "View/ActionManager.h"
#include "View/CameraTool.h"
#include "View/CommandIds.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MovementRestriction.h"
#include "View/SelectionTool.h"
#include "View/wxUtils.h"

namespace TrenchBroom {
    namespace View {
        MapView3D::MapView3D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, Renderer::MapRenderer& renderer) :
        RenderView(parent, attribs()),
        m_logger(logger),
        m_document(document),
        m_movementRestriction(new MovementRestriction()),
        m_vbo(new Renderer::Vbo(0xFFFFFFF)),
        m_renderer(renderer),
        m_camera(),
        m_compass(new Renderer::Compass(*m_movementRestriction)),
        m_toolBox(this, this),
        m_cameraTool(NULL) {
            createTools();
            bindObservers();
            bindEvents();
            updateAcceleratorTable(HasFocus());
        }

        MapView3D::~MapView3D() {
            unbindObservers();
            destroyTools();
            delete m_compass;
            delete m_vbo;
            delete m_movementRestriction;
        }
        
        void MapView3D::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->selectionDidChangeNotifier.addObserver(this, &MapView3D::selectionDidChange);
        }
        
        void MapView3D::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->selectionDidChangeNotifier.removeObserver(this, &MapView3D::selectionDidChange);
            }
        }
        
        void MapView3D::selectionDidChange(const Selection& selection) {
            updateAcceleratorTable(HasFocus());
            Refresh();
        }

        void MapView3D::bindEvents() {
            /*
            Bind(wxEVT_KEY_DOWN, &MapView3D::OnKey, this);
            Bind(wxEVT_KEY_UP, &MapView3D::OnKey, this);
            */
            
            Bind(wxEVT_SET_FOCUS, &MapView3D::OnSetFocus, this);
            Bind(wxEVT_KILL_FOCUS, &MapView3D::OnKillFocus, this);
            
            /*
            Bind(wxEVT_MENU, &MapView3D::OnToggleClipTool,               this, CommandIds::Actions::ToggleClipTool);
            Bind(wxEVT_MENU, &MapView3D::OnToggleClipSide,               this, CommandIds::Actions::ToggleClipSide);
            Bind(wxEVT_MENU, &MapView3D::OnPerformClip,                  this, CommandIds::Actions::PerformClip);
            Bind(wxEVT_MENU, &MapView3D::OnDeleteLastClipPoint,          this, CommandIds::Actions::DeleteLastClipPoint);
            
            Bind(wxEVT_MENU, &MapView3D::OnToggleVertexTool,             this, CommandIds::Actions::ToggleVertexTool);
            Bind(wxEVT_MENU, &MapView3D::OnMoveVerticesForward,          this, CommandIds::Actions::MoveVerticesForward);
            Bind(wxEVT_MENU, &MapView3D::OnMoveVerticesBackward,         this, CommandIds::Actions::MoveVerticesBackward);
            Bind(wxEVT_MENU, &MapView3D::OnMoveVerticesLeft,             this, CommandIds::Actions::MoveVerticesLeft);
            Bind(wxEVT_MENU, &MapView3D::OnMoveVerticesRight,            this, CommandIds::Actions::MoveVerticesRight);
            Bind(wxEVT_MENU, &MapView3D::OnMoveVerticesUp,               this, CommandIds::Actions::MoveVerticesUp);
            Bind(wxEVT_MENU, &MapView3D::OnMoveVerticesDown,             this, CommandIds::Actions::MoveVerticesDown);
            
            Bind(wxEVT_MENU, &MapView3D::OnToggleRotateObjectsTool,      this, CommandIds::Actions::ToggleRotateObjectsTool);
            Bind(wxEVT_MENU, &MapView3D::OnToggleFlyMode,                this, CommandIds::Actions::ToggleFlyMode);
            
            Bind(wxEVT_MENU, &MapView3D::OnToggleMovementRestriction,    this, CommandIds::Actions::ToggleMovementRestriction);
            
            Bind(wxEVT_MENU, &MapView3D::OnDeleteObjects,                this, CommandIds::Actions::DeleteObjects);
            */
            
            Bind(wxEVT_MENU, &MapView3D::OnMoveObjectsForward,           this, CommandIds::Actions::MoveObjectsForward);
            Bind(wxEVT_MENU, &MapView3D::OnMoveObjectsBackward,          this, CommandIds::Actions::MoveObjectsBackward);
            Bind(wxEVT_MENU, &MapView3D::OnMoveObjectsLeft,              this, CommandIds::Actions::MoveObjectsLeft);
            Bind(wxEVT_MENU, &MapView3D::OnMoveObjectsRight,             this, CommandIds::Actions::MoveObjectsRight);
            Bind(wxEVT_MENU, &MapView3D::OnMoveObjectsUp,                this, CommandIds::Actions::MoveObjectsUp);
            Bind(wxEVT_MENU, &MapView3D::OnMoveObjectsDown,              this, CommandIds::Actions::MoveObjectsDown);
            
            /*
            Bind(wxEVT_MENU, &MapView3D::OnRollObjectsCW,                this, CommandIds::Actions::RollObjectsCW);
            Bind(wxEVT_MENU, &MapView3D::OnRollObjectsCCW,               this, CommandIds::Actions::RollObjectsCCW);
            Bind(wxEVT_MENU, &MapView3D::OnPitchObjectsCW,               this, CommandIds::Actions::PitchObjectsCW);
            Bind(wxEVT_MENU, &MapView3D::OnPitchObjectsCCW,              this, CommandIds::Actions::PitchObjectsCCW);
            Bind(wxEVT_MENU, &MapView3D::OnYawObjectsCW,                 this, CommandIds::Actions::YawObjectsCW);
            Bind(wxEVT_MENU, &MapView3D::OnYawObjectsCCW,                this, CommandIds::Actions::YawObjectsCCW);
            
            Bind(wxEVT_MENU, &MapView3D::OnFlipObjectsH,                 this, CommandIds::Actions::FlipObjectsHorizontally);
            Bind(wxEVT_MENU, &MapView3D::OnFlipObjectsV,                 this, CommandIds::Actions::FlipObjectsVertically);
            
            Bind(wxEVT_MENU, &MapView3D::OnDuplicateObjectsForward,      this, CommandIds::Actions::DuplicateObjectsForward);
            Bind(wxEVT_MENU, &MapView3D::OnDuplicateObjectsBackward,     this, CommandIds::Actions::DuplicateObjectsBackward);
            Bind(wxEVT_MENU, &MapView3D::OnDuplicateObjectsLeft,         this, CommandIds::Actions::DuplicateObjectsLeft);
            Bind(wxEVT_MENU, &MapView3D::OnDuplicateObjectsRight,        this, CommandIds::Actions::DuplicateObjectsRight);
            Bind(wxEVT_MENU, &MapView3D::OnDuplicateObjectsUp,           this, CommandIds::Actions::DuplicateObjectsUp);
            Bind(wxEVT_MENU, &MapView3D::OnDuplicateObjectsDown,         this, CommandIds::Actions::DuplicateObjectsDown);
            Bind(wxEVT_MENU, &MapView3D::OnDuplicateObjects,             this, CommandIds::Actions::DuplicateObjects);
            
            Bind(wxEVT_MENU, &MapView3D::OnCancel,                       this, CommandIds::Actions::Cancel);
            
            Bind(wxEVT_MENU, &MapView3D::OnMoveRotationCenterForward,    this, CommandIds::Actions::MoveRotationCenterForward);
            Bind(wxEVT_MENU, &MapView3D::OnMoveRotationCenterBackward,   this, CommandIds::Actions::MoveRotationCenterBackward);
            Bind(wxEVT_MENU, &MapView3D::OnMoveRotationCenterLeft,       this, CommandIds::Actions::MoveRotationCenterLeft);
            Bind(wxEVT_MENU, &MapView3D::OnMoveRotationCenterRight,      this, CommandIds::Actions::MoveRotationCenterRight);
            Bind(wxEVT_MENU, &MapView3D::OnMoveRotationCenterUp,         this, CommandIds::Actions::MoveRotationCenterUp);
            Bind(wxEVT_MENU, &MapView3D::OnMoveRotationCenterDown,       this, CommandIds::Actions::MoveRotationCenterDown);
            
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesUp,               this, CommandIds::Actions::MoveTexturesUp);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesDown,             this, CommandIds::Actions::MoveTexturesDown);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesLeft,             this, CommandIds::Actions::MoveTexturesLeft);
            Bind(wxEVT_MENU, &MapView3D::OnMoveTexturesRight,            this, CommandIds::Actions::MoveTexturesRight);
            
            Bind(wxEVT_MENU, &MapView3D::OnRotateTexturesCW,             this, CommandIds::Actions::RotateTexturesCW);
            Bind(wxEVT_MENU, &MapView3D::OnRotateTexturesCCW,            this, CommandIds::Actions::RotateTexturesCCW);
            
            Bind(wxEVT_MENU, &MapView3D::OnPopupReparentBrushes,         this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
            Bind(wxEVT_MENU, &MapView3D::OnPopupMoveBrushesToWorld,      this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_MENU, &MapView3D::OnPopupCreatePointEntity,       this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_MENU, &MapView3D::OnPopupCreateBrushEntity,       this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
            
            Bind(wxEVT_UPDATE_UI, &MapView3D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
            Bind(wxEVT_UPDATE_UI, &MapView3D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_UPDATE_UI, &MapView3D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_UPDATE_UI, &MapView3D::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
            */
            
            wxFrame* frame = findFrame(this);
            frame->Bind(wxEVT_ACTIVATE, &MapView3D::OnActivateFrame, this);
        }
        
        void MapView3D::OnMoveObjectsForward(wxCommandEvent& event) {
            moveObjects(Math::Direction_Forward);
        }
        
        void MapView3D::OnMoveObjectsBackward(wxCommandEvent& event) {
            moveObjects(Math::Direction_Backward);
        }
        
        void MapView3D::OnMoveObjectsLeft(wxCommandEvent& event) {
            moveObjects(Math::Direction_Left);
        }
        
        void MapView3D::OnMoveObjectsRight(wxCommandEvent& event) {
            moveObjects(Math::Direction_Right);
        }
        
        void MapView3D::OnMoveObjectsUp(wxCommandEvent& event) {
            moveObjects(Math::Direction_Up);
        }
        
        void MapView3D::OnMoveObjectsDown(wxCommandEvent& event) {
            moveObjects(Math::Direction_Down);
        }

        void MapView3D::moveObjects(const Math::Direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList& nodes = document->selectedNodes();
            if (nodes.empty())
                return;
            
            const Grid& grid = document->grid();
            const Vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            document->translateObjects(delta);
        }

        Vec3 MapView3D::moveDirection(const Math::Direction direction) const {
            switch (direction) {
                case Math::Direction_Forward: {
                    Vec3 dir = m_camera.direction().firstAxis();
                    if (dir.z() < 0.0)
                        dir = m_camera.up().firstAxis();
                    else if (dir.z() > 0.0)
                        dir = -m_camera.up().firstAxis();
                    return dir;
                }
                case Math::Direction_Backward:
                    return -moveDirection(Math::Direction_Forward);
                case Math::Direction_Left:
                    return -moveDirection(Math::Direction_Right);
                case Math::Direction_Right: {
                    Vec3 dir = m_camera.right().firstAxis();
                    if (dir == moveDirection(Math::Direction_Forward))
                        dir = crossed(dir, Vec3::PosZ);
                    return dir;
                }
                case Math::Direction_Up:
                    return Vec3::PosZ;
                case Math::Direction_Down:
                    return Vec3::NegZ;
                    DEFAULT_SWITCH()
            }
        }

        void MapView3D::OnSetFocus(wxFocusEvent& event) {
            updateAcceleratorTable(true);
            event.Skip();
        }
        
        void MapView3D::OnKillFocus(wxFocusEvent& event) {
            /*
            if (cameraFlyModeActive())
                toggleCameraFlyMode();
             */
            updateAcceleratorTable(false);
            event.Skip();
        }

        void MapView3D::OnActivateFrame(wxActivateEvent& event) {
            if (event.GetActive())
                m_toolBox.updateLastActivation();
            /*
            if (cameraFlyModeActive())
                toggleCameraFlyMode();
             */
            event.Skip();
        }

        void MapView3D::updateAcceleratorTable(const bool hasFocus) {
            if (hasFocus) {
                const ActionManager& actionManager = ActionManager::instance();
                const Action::Context context = actionContext();
                const wxAcceleratorTable acceleratorTable = actionManager.createMapViewAcceleratorTable(context);
                SetAcceleratorTable(acceleratorTable);
            } else {
                SetAcceleratorTable(wxNullAcceleratorTable);
            }
        }
        
        Action::Context MapView3D::actionContext() const {
            /*
            if (clipToolActive())
                return Action::Context_ClipTool;
            if (vertexToolActive())
                return Action::Context_VertexTool;
            if (rotateObjectsToolActive())
                return Action::Context_RotateTool;
            if (cameraFlyModeActive())
                return Action::Context_FlyMode;
            */
            
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedNodes())
                return Action::Context_NodeSelection;
            if (document->hasSelectedBrushFaces())
                return Action::Context_FaceSelection;
            return Action::Context_Default;
        }

        void MapView3D::doInitializeGL() {
            const wxString vendor   = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
            const wxString renderer = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
            const wxString version  = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
            
            m_logger->info(wxString::Format(L"Renderer info: %s version %s from %s", renderer, version, vendor));
            m_logger->info("Depth buffer bits: %d", depthBits());
            
            if (multisample())
                m_logger->info("Multisampling enabled");
            else
                m_logger->info("Multisampling disabled");
        }
        
        void MapView3D::doUpdateViewport(const int x, const int y, const int width, const int height) {
            const Renderer::Camera::Viewport viewport(x, y, width, height);
            m_camera.setViewport(viewport);
        }
        
        bool MapView3D::doShouldRenderFocusIndicator() const {
            return true;
        }
        
        void MapView3D::doRender() {
            MapDocumentSPtr document = lock(m_document);
            Renderer::RenderContext renderContext(m_camera, contextHolder()->fontManager(), contextHolder()->shaderManager());
            setupGL(renderContext);
            
            Renderer::RenderBatch renderBatch(*m_vbo);
            
            renderMap(renderContext, renderBatch);
            renderCompass(renderBatch);
            
            renderBatch.render(renderContext);
        }

        void MapView3D::setupGL(Renderer::RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().viewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glShadeModel(GL_SMOOTH);
        }

        void MapView3D::renderMap(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_renderer.render(renderContext, renderBatch);
        }

        void MapView3D::renderCompass(Renderer::RenderBatch& renderBatch) {
            m_compass->render(renderBatch);
        }

        Ray3 MapView3D::doGetPickRay(const int x, const int y) const {
            return m_camera.pickRay(x, y);
        }
        
        Hits MapView3D::doPick(const Ray3& pickRay) const {
            MapDocumentSPtr document = lock(m_document);
            return document->pick(pickRay);
        }
        
        void MapView3D::doShowPopupMenu() {
        }

        void MapView3D::createTools() {
            m_cameraTool = new CameraTool(m_document, m_camera);
            m_selectionTool = new SelectionTool(m_document);
            m_toolBox.addTool(m_selectionTool);
            m_toolBox.addTool(m_cameraTool);
        }
        
        void MapView3D::destroyTools() {
            delete m_cameraTool;
            delete m_selectionTool;
        }

        const GLContextHolder::GLAttribs& MapView3D::attribs() {
            static bool initialized = false;
            static GLContextHolder::GLAttribs attribs;
            if (initialized)
                return attribs;
            
            int testAttribs[] =
            {
                // 32 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 24 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 32 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 24 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 16 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 16 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 32 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                0,
                // 24 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                0,
                // 16 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                0,
                0,
            };
            
            size_t index = 0;
            while (!initialized && testAttribs[index] != 0) {
                size_t count = 0;
                for (; testAttribs[index + count] != 0; ++count);
                if (wxGLCanvas::IsDisplaySupported(&testAttribs[index])) {
                    for (size_t i = 0; i < count; ++i)
                        attribs.push_back(testAttribs[index + i]);
                    attribs.push_back(0);
                    initialized = true;
                }
                index += count + 1;
            }
            
            assert(initialized);
            assert(!attribs.empty());
            return attribs;
        }
        
        int MapView3D::depthBits() {
            return attribs()[3];
        }
        
        bool MapView3D::multisample() {
            return attribs()[4] != 0;
        }
    }
}