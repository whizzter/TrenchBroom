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

#ifndef __TrenchBroom__EntityPropertyGrid__
#define __TrenchBroom__EntityPropertyGrid__

#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <wx/grid.h>
#include <wx/panel.h>

class wxButton;
class wxCheckBox;
class wxWindow;

namespace TrenchBroom {
    namespace Model {
        class Object;
        class SelectionResult;
    }
    
    namespace View {
        class EntityPropertyGridTable;
        
        class EntityPropertyGrid : public wxPanel {
        private:
            MapDocumentWPtr m_document;
            
            EntityPropertyGridTable* m_table;
            wxGrid* m_grid;
            wxGridCellCoords m_lastHoveredCell;
            
            wxButton* m_addPropertyButton;
            wxButton* m_removePropertiesButton;
            wxCheckBox* m_showDefaultPropertiesCheckBox;
            
            bool m_ignoreSelection;
            Model::PropertyKey m_lastSelectedKey;
            int m_lastSelectedCol;
        public:
            EntityPropertyGrid(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller);
            ~EntityPropertyGrid();
            
            void OnPropertyGridSize(wxSizeEvent& event);
            void OnPropertyGridSelectCell(wxGridEvent& event);
            void OnPropertyGridTab(wxGridEvent& event);
            
            void OnPropertyGridKeyDown(wxKeyEvent& event);
            void OnPropertyGridKeyUp(wxKeyEvent& event);
        private:
            bool isInsertRowShortcut(const wxKeyEvent& event) const;
            bool isDeleteRowShortcut(const wxKeyEvent& event) const;
        public:
            void OnPropertyGridMouseMove(wxMouseEvent& event);

            void OnUpdatePropertyView(wxUpdateUIEvent& event);

            void OnAddPropertyButton(wxCommandEvent& event);
            void OnRemovePropertiesButton(wxCommandEvent& event);
            void OnShowDefaultPropertiesCheckBox(wxCommandEvent& event);
            void OnUpdateAddPropertyButton(wxUpdateUIEvent& event);
            void OnUpdateRemovePropertiesButton(wxUpdateUIEvent& event);
            void OnUpdateShowDefaultPropertiesCheckBox(wxUpdateUIEvent& event);
        private:
            void createGui(MapDocumentWPtr document, ControllerWPtr controller);
            void bindEvents();
            
            void bindObservers();
            void unbindObservers();
            
            void documentWasNewed();
            void documentWasLoaded();
            void objectDidChange(Model::Object* object);
            void selectionWillChange();
            void selectionDidChange(const Model::SelectionResult& result);
            
            void updateControls();
            Model::PropertyKey selectedRowKey() const;
        };
    }
}

#endif /* defined(__TrenchBroom__EntityPropertyGrid__) */
