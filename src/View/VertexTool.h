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

#ifndef __TrenchBroom__VertexTool__
#define __TrenchBroom__VertexTool__

#include "StringUtils.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/MoveTool.h"
#include "View/Tool.h"
#include "View/VertexHandleManager.h"

namespace TrenchBroom {
    namespace Model {
        class SelectionResult;
    }
    
    namespace View {
        class InputState;
        class MovementRestriction;
        
        class VertexTool : public MoveTool<ActivationPolicy, PickingPolicy, NoMousePolicy, RenderPolicy> {
        private:
            static const FloatType MaxVertexDistance;
            
            typedef enum {
                VMMove,
                VMSplit,
                VMSnap
            } VertexToolMode;

            VertexHandleManager m_handleManager;
            VertexToolMode m_mode;
            size_t m_changeCount;
        public:
            VertexTool(BaseTool* next, MapDocumentWPtr document, ControllerWPtr controller, MovementRestriction& movementRestriction);
        private:
            bool doHandleMove(const InputState& inputState) const;
            Vec3 doGetMoveOrigin(const InputState& inputState) const;
            String doGetActionName(const InputState& inputState) const;
            bool doStartMove(const InputState& inputState);
            Vec3 doSnapDelta(const InputState& inputState, const Vec3& delta) const;
            MoveResult doMove(const Vec3& delta);
            void doEndMove(const InputState& inputState);
            
            bool initiallyActive() const;
            bool doActivate(const InputState& inputState);
            bool doDeactivate(const InputState& inputState);
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult);

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);
            
            void selectionDidChange(const Model::SelectionResult& selection);
        };
    }
}

#endif /* defined(__TrenchBroom__VertexTool__) */
