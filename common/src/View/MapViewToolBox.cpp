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

#include "MapViewToolBox.h"
#include "View/CreateEntityTool.h"
#include "View/MoveObjectsTool.h"
#include "View/ResizeBrushesTool.h"
#include "View/RotateObjectsTool.h"
#include "View/SelectionTool.h"
#include "View/VertexTool.h"

namespace TrenchBroom {
    namespace View {
        MapViewToolBox::MapViewToolBox(MapDocumentWPtr document, wxBookCtrlBase* bookCtrl) :
        m_createEntityTool(NULL),
        m_moveObjectsTool(NULL),
        m_resizeBrushesTool(NULL),
        m_rotateObjectsTool(NULL),
        m_selectionTool(NULL),
        m_vertexTool(NULL) {
            createTools(document, bookCtrl);
            bindObservers();
        }
        
        MapViewToolBox::~MapViewToolBox() {
            unbindObservers();
            destroyTools();
        }

        CreateEntityTool* MapViewToolBox::createEntityTool() {
            return m_createEntityTool;
        }

        MoveObjectsTool* MapViewToolBox::moveObjectsTool() {
            return m_moveObjectsTool;
        }
        
        ResizeBrushesTool* MapViewToolBox::resizeBrushesTool() {
            return m_resizeBrushesTool;
        }
        
        RotateObjectsTool* MapViewToolBox::rotateObjectsTool() {
            return m_rotateObjectsTool;
        }
        
        SelectionTool* MapViewToolBox::selectionTool() {
            return m_selectionTool;
        }
        
        VertexTool* MapViewToolBox::vertexTool() {
            return m_vertexTool;
        }

        void MapViewToolBox::toggleRotateObjectsTool() {
            toggleTool(m_rotateObjectsTool);
        }
        
        bool MapViewToolBox::rotateObjectsToolActive() const {
            return toolActive(m_rotateObjectsTool);
        }

        double MapViewToolBox::rotateToolAngle() const {
            assert(rotateObjectsToolActive());
            return m_rotateObjectsTool->angle();
        }
        
        const Vec3 MapViewToolBox::rotateToolCenter() const {
            assert(rotateObjectsToolActive());
            return m_rotateObjectsTool->rotationCenter();
        }

        void MapViewToolBox::moveRotationCenter(const Vec3& delta) {
            assert(rotateObjectsToolActive());
            const Vec3 center = m_rotateObjectsTool->rotationCenter();
            m_rotateObjectsTool->setRotationCenter(center + delta);
        }

        void MapViewToolBox::toggleVertexTool() {
            toggleTool(m_vertexTool);
        }
        
        bool MapViewToolBox::vertexToolActive() const {
            return toolActive(m_vertexTool);
        }

        void MapViewToolBox::createTools(MapDocumentWPtr document, wxBookCtrlBase* bookCtrl) {
            m_createEntityTool = new CreateEntityTool(document);
            m_moveObjectsTool = new MoveObjectsTool(document);
            m_resizeBrushesTool = new ResizeBrushesTool(document);
            m_rotateObjectsTool = new RotateObjectsTool(document);
            m_selectionTool = new SelectionTool(document);
            m_vertexTool = new VertexTool(document);
            
            deactivateWhen(m_rotateObjectsTool, m_moveObjectsTool);
            deactivateWhen(m_rotateObjectsTool, m_resizeBrushesTool);
            deactivateWhen(m_vertexTool, m_moveObjectsTool);
            deactivateWhen(m_vertexTool, m_resizeBrushesTool);
            // deactivateWhen(m_clipTool, m_resizeBrushesTool);
            
            m_moveObjectsTool->createPage(bookCtrl);
            m_rotateObjectsTool->createPage(bookCtrl);
            
            m_createEntityTool->refreshViewsNotifier.addObserver(refreshViewsNotifier);
            m_moveObjectsTool->refreshViewsNotifier.addObserver(refreshViewsNotifier);
            m_resizeBrushesTool->refreshViewsNotifier.addObserver(refreshViewsNotifier);
            m_rotateObjectsTool->refreshViewsNotifier.addObserver(refreshViewsNotifier);
            m_selectionTool->refreshViewsNotifier.addObserver(refreshViewsNotifier);
            m_vertexTool->refreshViewsNotifier.addObserver(refreshViewsNotifier);
        }
        
        void MapViewToolBox::destroyTools() {
            delete m_vertexTool;
            delete m_selectionTool;
            delete m_rotateObjectsTool;
            delete m_resizeBrushesTool;
            delete m_moveObjectsTool;
            delete m_createEntityTool;
        }
        
        void MapViewToolBox::bindObservers() {
            toolActivatedNotifier.addObserver(this, &MapViewToolBox::toolActivated);
            toolDeactivatedNotifier.addObserver(this, &MapViewToolBox::toolDeactivated);
        }
        
        void MapViewToolBox::unbindObservers() {
            toolActivatedNotifier.removeObserver(this, &MapViewToolBox::toolActivated);
            toolDeactivatedNotifier.removeObserver(this, &MapViewToolBox::toolDeactivated);
        }
        
        void MapViewToolBox::toolActivated(Tool* tool) {
            if (tool == m_rotateObjectsTool)
                m_rotateObjectsTool->showPage();
        }
        
        void MapViewToolBox::toolDeactivated(Tool* tool) {
            m_moveObjectsTool->showPage();
        }
    }
}