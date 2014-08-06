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

#include "IssueBrowserView.h"

#include "Model/Issue.h"
#include "Model/IssueManager.h"
#include "View/ControllerFacade.h"
#include "View/MapDocument.h"

#include <wx/menu.h>
#include <wx/settings.h>

namespace TrenchBroom {
    namespace View {
        IssueBrowserView::IssueBrowserView(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES | wxLC_VRULES | wxBORDER_NONE),
        m_document(document),
        m_controller(controller) {
            AppendColumn("Line");
            AppendColumn("Description");
            
            Model::IssueManager& issueManager = lock(m_document)->issueManager();
            issueCountDidChange(issueManager.issueCount());
            
            bindObservers();
            bindEvents();
        }
        
        IssueBrowserView::~IssueBrowserView() {
            unbindObservers();
        }
        
        void IssueBrowserView::OnSize(wxSizeEvent& event) {
            const int newWidth = std::max(1, GetClientSize().x - GetColumnWidth(0));
            SetColumnWidth(1, newWidth);
            event.Skip();
        }
        
        void IssueBrowserView::OnItemRightClick(wxListEvent& event) {
            if (GetSelectedItemCount() == 0 || event.GetIndex() < 0)
                return;
            
            wxMenu popupMenu;
            popupMenu.Append(SelectObjectsCommandId, "Select");
            popupMenu.AppendSeparator();
            popupMenu.Append(ShowIssuesCommandId, "Show");
            popupMenu.Append(HideIssuesCommandId, "Hide");
            popupMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowserView::OnSelectIssues, this, SelectObjectsCommandId);
            popupMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowserView::OnShowIssues, this, ShowIssuesCommandId);
            popupMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowserView::OnHideIssues, this, HideIssuesCommandId);
            
            const Model::QuickFix::List quickFixes = collectQuickFixes(getSelection());
            if (!quickFixes.empty()) {
                wxMenu* quickFixMenu = new wxMenu();
                for (size_t i = 0; i < quickFixes.size(); ++i) {
                    const Model::QuickFix* quickFix = quickFixes[i];
                    const int quickFixId = FixObjectsBaseId + static_cast<int>(i);
                    quickFixMenu->Append(quickFixId, quickFix->description());
                }

                popupMenu.AppendSeparator();
                popupMenu.AppendSubMenu(quickFixMenu, "Fix");
                
                const int firstId = FixObjectsBaseId;
                const int lastId = firstId + static_cast<int>(quickFixes.size());
                quickFixMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowserView::OnApplyQuickFix, this, firstId, lastId);
            }
            
            PopupMenu(&popupMenu);
        }
        
        void IssueBrowserView::OnSelectIssues(wxCommandEvent& event) {
            View::ControllerSPtr controller = lock(m_controller);
            const UndoableCommandGroup commandGroup(controller);
            
            const IndexList selection = getSelection();
            selectIssueObjects(selection, controller);
        }
        
        void IssueBrowserView::OnShowIssues(wxCommandEvent& event) {
            setIssueVisibility(true);
        }
        
        void IssueBrowserView::OnHideIssues(wxCommandEvent& event) {
            setIssueVisibility(false);
        }
        
        void IssueBrowserView::OnApplyQuickFix(wxCommandEvent& event) {
            const IndexList selection = getSelection();
            deselectAll();

            assert(!selection.empty());
            
            Model::QuickFix::List quickFixes = collectQuickFixes(selection);
            const size_t index = static_cast<size_t>(event.GetId()) - FixObjectsBaseId;
            assert(index < quickFixes.size());
            const Model::QuickFix* quickFix = quickFixes[index];
            
            View::ControllerSPtr controller = lock(m_controller);
            Model::IssueManager& issueManager = lock(m_document)->issueManager();
            const UndoableCommandGroup commandGroup(controller);
            
            selectIssueObjects(selection, controller);
            
            for (size_t i = 0; i < selection.size(); ++i) {
                Model::Issue* issue = issueManager.issues()[selection[i]];
                issue->applyQuickFix(quickFix, controller);
            }
            
        }
        
        Model::QuickFix::List IssueBrowserView::collectQuickFixes(const IndexList& selection) const {
            if (selection.empty())
                return Model::QuickFix::List(0);
            
            Model::IssueManager& issueManager = lock(m_document)->issueManager();
            const Model::Issue* issue = issueManager.issues()[selection[0]];
            
            const Model::IssueType type = issue->type();
            Model::QuickFix::List result = issue->quickFixes();
            
            for (size_t i = 1; i < selection.size(); ++i) {
                issue = issueManager.issues()[selection[i]];
                if (issue->type() != type)
                    return Model::QuickFix::List(0);
            }
            return result;
        }
        
        void IssueBrowserView::setIssueVisibility(const bool show) {
            const IndexList selection = getSelection();
            
            MapDocumentSPtr document = lock(m_document);
            Model::IssueManager& issueManager = document->issueManager();
            
            for (size_t i = 0; i < selection.size(); ++i) {
                Model::Issue* issue = issueManager.issues()[selection[i]];
                issueManager.setIssueHidden(issue, !show);
            }
            
            document->incModificationCount();
            deselectAll();
        }
        
        void IssueBrowserView::selectIssueObjects(const IndexList& selection, View::ControllerSPtr controller) {
            MapDocumentSPtr document = lock(m_document);
            Model::IssueManager& issueManager = document->issueManager();

            controller->deselectAll();
            for (size_t i = 0; i < selection.size(); ++i) {
                Model::Issue* issue = issueManager.issues()[selection[i]];
                issue->select(controller);
            }
        }

        IssueBrowserView::IndexList IssueBrowserView::getSelection() const {
            IndexList result(static_cast<size_t>(GetSelectedItemCount()));
            
            size_t i = 0;
            long itemIndex = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            while (itemIndex >= 0) {
                result[i++] = static_cast<size_t>(itemIndex);
                itemIndex = GetNextItem(itemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
            return result;
        }
        
        void IssueBrowserView::select(const IndexList& selection) {
            for (size_t i = 0; i < selection.size(); ++i) {
                const long index = static_cast<long>(selection[i]);
                SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            }
        }

        void IssueBrowserView::deselectAll() {
            const IndexList selection = getSelection();
            for (size_t i = 0; i < selection.size(); ++i) {
                const long index = static_cast<long>(selection[i]);
                SetItemState(index, 0, wxLIST_STATE_SELECTED);
            }
        }
        
        wxString IssueBrowserView::OnGetItemText(const long item, const long column) const {
            Model::IssueManager& issueManager = lock(m_document)->issueManager();
            assert(item >= 0 && static_cast<size_t>(item) < issueManager.issueCount());
            assert(column >= 0 && column < 2);
            
            Model::Issue* issue = issueManager.issues()[static_cast<size_t>(item)];
            if (column == 0) {
                wxString result;
                result << issue->filePosition();
                return result;
            } else {
                return issue->description();
            }
        }
        
        void IssueBrowserView::bindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                Model::IssueManager& issueManager = document->issueManager();
                issueManager.issueCountDidChangeNotifier.addObserver(this, &IssueBrowserView::issueCountDidChange);
            }
        }
        
        void IssueBrowserView::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                Model::IssueManager& issueManager = document->issueManager();
                issueManager.issueCountDidChangeNotifier.removeObserver(this, &IssueBrowserView::issueCountDidChange);
            }
        }
        
        void IssueBrowserView::issueCountDidChange(const size_t count) {
            SetItemCount(static_cast<long>(count));
        }
        
        void IssueBrowserView::bindEvents() {
            Bind(wxEVT_SIZE, &IssueBrowserView::OnSize, this);
            Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &IssueBrowserView::OnItemRightClick, this);
        }
    }
}