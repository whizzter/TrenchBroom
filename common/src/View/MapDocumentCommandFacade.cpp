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

#include "MapDocumentCommandFacade.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/CollectNodesWithDescendantSelectionCountVisitor.h"
#include "Model/CollectParentsVisitor.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Node.h"
#include "Model/Snapshot.h"
#include "Model/TransformObjectVisitor.h"
#include "Model/World.h"
#include "View/Selection.h"

namespace TrenchBroom {
    namespace View {
        class NodeChangeNotifier {
        public:
            typedef Notifier1<const Model::NodeList&> Notifier;
        private:
            Notifier& m_didChange;
            const Model::NodeList& m_nodes;
        public:
            NodeChangeNotifier(Notifier& willChange, Notifier& didChange, const Model::NodeList& nodes) :
            m_didChange(didChange),
            m_nodes(nodes) {
                willChange(nodes);
            }
            
            ~NodeChangeNotifier() {
                m_didChange(m_nodes);
            }
        };
        
        MapDocumentSPtr MapDocumentCommandFacade::newMapDocument() {
            return MapDocumentSPtr(new MapDocumentCommandFacade());
        }

        MapDocumentCommandFacade::MapDocumentCommandFacade() :
        m_commandProcessor(this) {}

        void MapDocumentCommandFacade::performSelect(const Model::NodeList& nodes) {
            selectionWillChangeNotifier();

            Model::NodeList selected;
            selected.reserve(nodes.size());
            
            Model::CollectNodesWithDescendantSelectionCountVisitor visitor(0);

            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                if (!node->selected() && m_editorContext->selectable(node)) {
                    node->escalate(visitor);
                    node->select();
                    selected.push_back(node);
                }
            }

            const Model::NodeList& partiallySelected = visitor.result();
            
            VectorUtils::append(m_selectedNodes, selected);
            VectorUtils::append(m_partiallySelectedNodes, partiallySelected);
            
            Selection selection;
            selection.addSelectedNodes(selected);
            selection.addPartiallySelectedNodes(partiallySelected);
            
            selectionDidChangeNotifier(selection);
        }
        
        void MapDocumentCommandFacade::performSelect(const Model::BrushFaceList& faces) {
            selectionWillChangeNotifier();
            
            Model::BrushFaceList selected;
            selected.reserve(faces.size());
            
            Model::CollectNodesWithDescendantSelectionCountVisitor visitor(0);
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                if (!face->selected() && m_editorContext->selectable(face)) {
                    face->brush()->acceptAndEscalate(visitor);
                    face->select();
                    selected.push_back(face);
                }
            }
            
            const Model::NodeList& partiallySelected = visitor.result();
            
            VectorUtils::append(m_selectedBrushFaces, selected);
            VectorUtils::append(m_partiallySelectedNodes, partiallySelected);
            
            Selection selection;
            selection.addSelectedBrushFaces(selected);
            selection.addPartiallySelectedNodes(partiallySelected);
            
            selectionDidChangeNotifier(selection);
        }
        
        class CollectSelectableNodes : public Model::NodeVisitor {
        private:
            const Model::EditorContext& m_editorContext;
            Model::NodeList m_nodes;
        public:
            CollectSelectableNodes(const Model::EditorContext& editorContext) :
            m_editorContext(editorContext) {}
            
            const Model::NodeList& result() const {
                return m_nodes;
            }
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   { if (m_editorContext.selectable(group))  { m_nodes.push_back(group);  stopRecursion(); } }
            void doVisit(Model::Entity* entity) { if (m_editorContext.selectable(entity)) { m_nodes.push_back(entity); stopRecursion(); } }
            void doVisit(Model::Brush* brush)   { if (m_editorContext.selectable(brush))  { m_nodes.push_back(brush);  stopRecursion(); } }
        };

        void MapDocumentCommandFacade::performSelectAllNodes() {
            performDeselectAll();
            
            CollectSelectableNodes visitor(*m_editorContext);
            m_world->acceptAndRecurse(visitor);
            performSelect(visitor.result());
        }
        
        class CollectSelectableFaces : public Model::NodeVisitor {
        private:
            const Model::EditorContext& m_editorContext;
            Model::BrushFaceList m_faces;
        public:
            CollectSelectableFaces(const Model::EditorContext& editorContext) :
            m_editorContext(editorContext) {}
            
            const Model::BrushFaceList& result() const {
                return m_faces;
            }
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Entity* entity) {}
            void doVisit(Model::Brush* brush)   {
                const Model::BrushFaceList& faces = brush->faces();
                Model::BrushFaceList::const_iterator it, end;
                for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                    Model::BrushFace* face = *it;
                    if (m_editorContext.selectable(face))
                        m_faces.push_back(face);
                }
            }
        };
        
        void MapDocumentCommandFacade::performSelectAllBrushFaces() {
            performDeselectAll();
            
            CollectSelectableFaces visitor(*m_editorContext);
            m_world->acceptAndRecurse(visitor);
            performSelect(visitor.result());
        }

        void MapDocumentCommandFacade::performConvertToBrushFaceSelection() {
            CollectSelectableFaces visitor(*m_editorContext);
            Model::Node::acceptAndRecurse(m_selectedNodes.begin(), m_selectedNodes.end(), visitor);
            
            performDeselectAll();
            performSelect(visitor.result());
        }

        void MapDocumentCommandFacade::performDeselect(const Model::NodeList& nodes) {
            selectionWillChangeNotifier();
            
            Model::NodeList deselected;
            deselected.reserve(nodes.size());
            
            Model::CollectNodesWithDescendantSelectionCountVisitor visitor(0);
            
            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                if (node->selected()) {
                    node->deselect();
                    deselected.push_back(node);
                    node->escalate(visitor);
                }
            }
            
            const Model::NodeList& partiallyDeselected = visitor.result();
            
            VectorUtils::eraseAll(m_selectedNodes, deselected);
            VectorUtils::eraseAll(m_partiallySelectedNodes, partiallyDeselected);
            
            Selection selection;
            selection.addDeselectedNodes(deselected);
            selection.addPartiallyDeselectedNodes(partiallyDeselected);
            
            selectionDidChangeNotifier(selection);
        }
        
        void MapDocumentCommandFacade::performDeselect(const Model::BrushFaceList& faces) {
            selectionWillChangeNotifier();
            
            Model::BrushFaceList deselected;
            deselected.reserve(faces.size());
            
            Model::CollectNodesWithDescendantSelectionCountVisitor visitor(0);
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                if (face->selected()) {
                    face->deselect();
                    deselected.push_back(face);
                    face->brush()->acceptAndEscalate(visitor);
                }
            }
            
            const Model::NodeList& partiallyDeselected = visitor.result();

            VectorUtils::eraseAll(m_selectedBrushFaces, deselected);
            VectorUtils::eraseAll(m_partiallySelectedNodes, partiallyDeselected);
            
            Selection selection;
            selection.addDeselectedBrushFaces(deselected);
            selection.addPartiallyDeselectedNodes(partiallyDeselected);
            
            selectionDidChangeNotifier(selection);
        }

        void MapDocumentCommandFacade::performDeselectAll() {
            if (hasSelectedNodes())
                deselectAllNodes();
            if (hasSelectedBrushFaces())
                deselectAllBrushFaces();
        }

        void MapDocumentCommandFacade::deselectAllNodes() {
            selectionWillChangeNotifier();

            Model::NodeList::const_iterator it, end;
            for (it = m_selectedNodes.begin(), end = m_selectedNodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                node->deselect();
            }
            
            Selection selection;
            selection.addDeselectedNodes(m_selectedNodes);
            selection.addPartiallyDeselectedNodes(m_partiallySelectedNodes);

            m_selectedNodes.clear();
            m_partiallySelectedNodes.clear();
            
            selectionDidChangeNotifier(selection);
        }
        
        void MapDocumentCommandFacade::deselectAllBrushFaces() {
            selectionWillChangeNotifier();
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = m_selectedBrushFaces.begin(), end = m_selectedBrushFaces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                face->deselect();
            }
            
            Selection selection;
            selection.addDeselectedBrushFaces(m_selectedBrushFaces);
            selection.addPartiallyDeselectedNodes(m_partiallySelectedNodes);
            
            m_selectedBrushFaces.clear();
            m_partiallySelectedNodes.clear();
            
            selectionDidChangeNotifier(selection);
        }

        void MapDocumentCommandFacade::performTransform(const Mat4x4& transform, const bool lockTextures) {
            const Model::NodeList& nodes = m_selectedNodes;
            const Model::NodeList parents = collectParents(nodes);
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            Model::TransformObjectVisitor visitor(transform, lockTextures, m_worldBounds);
            Model::Node::accept(nodes.begin(), nodes.end(), visitor);
        }

        void MapDocumentCommandFacade::restoreSnapshot(Model::Snapshot* snapshot) {
            const Model::NodeList& nodes = m_selectedNodes;
            const Model::NodeList parents = collectParents(nodes);
            
            NodeChangeNotifier notifyParents(nodesWillChangeNotifier, nodesDidChangeNotifier, parents);
            NodeChangeNotifier notifyNodes(nodesWillChangeNotifier, nodesDidChangeNotifier, nodes);
            
            snapshot->restore(m_worldBounds);
        }

        Model::NodeList MapDocumentCommandFacade::collectParents(const Model::NodeList& nodes) const {
            Model::CollectParentsVisitor visitor;
            Model::Node::acceptAndEscalate(nodes.begin(), nodes.end(), visitor);
            return visitor.parentList();
        }
        
        void MapDocumentCommandFacade::incModificationCount(const size_t delta) {
            m_modificationCount += delta;
            documentModificationStateDidChangeNotifier();
        }
        
        void MapDocumentCommandFacade::decModificationCount(const size_t delta) {
            assert(m_modificationCount >= delta);
            m_modificationCount -= delta;
            documentModificationStateDidChangeNotifier();
        }

        bool MapDocumentCommandFacade::doCanUndoLastCommand() const {
            return m_commandProcessor.hasLastCommand();
        }
        
        bool MapDocumentCommandFacade::doCanRedoNextCommand() const {
            return m_commandProcessor.hasNextCommand();
        }
        
        const String& MapDocumentCommandFacade::doGetLastCommandName() const {
            return m_commandProcessor.lastCommandName();
        }
        
        const String& MapDocumentCommandFacade::doGetNextCommandName() const {
            return m_commandProcessor.nextCommandName();
        }
        
        void MapDocumentCommandFacade::doUndoLastCommand() {
            m_commandProcessor.undoLastCommand();
        }
        
        void MapDocumentCommandFacade::doRedoNextCommand() {
            m_commandProcessor.redoNextCommand();
        }
        
        bool MapDocumentCommandFacade::doRepeatLastCommands() {
            return m_commandProcessor.repeatLastCommands();
        }
        
        void MapDocumentCommandFacade::doClearRepeatableCommands() {
            m_commandProcessor.clearRepeatableCommands();
        }

        void MapDocumentCommandFacade::doBeginTransaction(const String& name) {
            m_commandProcessor.beginGroup(name);
        }
        
        void MapDocumentCommandFacade::doEndTransaction() {
            m_commandProcessor.endGroup();
        }
        
        void MapDocumentCommandFacade::doRollbackTransaction() {
            m_commandProcessor.rollbackGroup();
        }

        bool MapDocumentCommandFacade::doSubmit(UndoableCommand* command) {
            return m_commandProcessor.submitAndStoreCommand(command);
        }
    }
}