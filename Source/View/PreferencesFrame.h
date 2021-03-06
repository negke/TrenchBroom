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

#ifndef __TrenchBroom__PreferencesFrame__
#define __TrenchBroom__PreferencesFrame__

#include <wx/frame.h>

class wxPanel;
class wxToolBar;
class wxToolBarToolBase;

namespace TrenchBroom {
    namespace View {
        class PreferencePane;

        class PreferencesFrame : public wxFrame {
        private:
            DECLARE_DYNAMIC_CLASS(PreferencesFrame)
        protected:
            typedef enum {
                PPGeneral,
                PPKeyboard
                // Update the tool event macro when adding new tools!
            } PrefPane;

            wxToolBar* m_toolBar;
            wxPanel* m_panel;
            PrefPane m_currentPane;
            PreferencePane* m_pane;

            void switchToPane(PrefPane pane);
        public:
            PreferencesFrame();

            void OnToolClicked(wxCommandEvent& event);
            void OnOkClicked(wxCommandEvent& event);
			void OnCancelClicked(wxCommandEvent& event);
			void OnClose(wxCloseEvent& event);
            void OnFileExit(wxCommandEvent& event);

            DECLARE_EVENT_TABLE();
        };
    }
}

#endif /* defined(__TrenchBroom__PreferencesFrame__) */
