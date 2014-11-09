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

#ifndef __TrenchBroom__ChangeBrushFaceAttributesCommand__
#define __TrenchBroom__ChangeBrushFaceAttributesCommand__

#include "View/DocumentCommand.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }
    
    namespace View {
        class ChangeBrushFaceAttributesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
        private:
            
            Model::ChangeBrushFaceAttributesRequest m_request;
            Model::Snapshot* m_snapshot;
        public:
            static ChangeBrushFaceAttributesCommand* command(const Model::ChangeBrushFaceAttributesRequest& request);
        private:
            ChangeBrushFaceAttributesCommand(const Model::ChangeBrushFaceAttributesRequest& request);
        public:
            ~ChangeBrushFaceAttributesCommand();
        private:
            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            UndoableCommand* doRepeat(MapDocumentCommandFacade* document) const;
            
            bool doCollateWith(UndoableCommand* command);
        };
    }
}

#endif /* defined(__TrenchBroom__ChangeBrushFaceAttributesCommand__) */