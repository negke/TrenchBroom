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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__Path__
#define __TrenchBroom__Path__

#include "StringUtils.h"

#include <vector>

namespace TrenchBroom {
    namespace IO {
        class Path {
        public:
            typedef std::vector<Path> List;
            static const char Separator;
        private:
            static const String Separators;
            
            StringList m_components;
            bool m_absolute;
            
            Path(bool absolute, const StringList& components);
        public:
            Path(const String& path);
            
            Path operator+ (const Path& rhs) const;
            bool operator== (const Path& rhs) const;
            
            String asString() const;
            operator String() const;
            
            bool isEmpty() const;
            String lastComponent() const;
            Path deleteLastComponent() const;
            const String extension() const;
            Path deleteExtension() const;
            Path addExtension(const String& extension) const;
            
            bool isAbsolute() const;
            Path makeAbsolute(const Path& relativePath) const;
            Path makeRelative(const Path& absolutePath) const;
            Path makeCanonical() const;
        private:
            StringList resolvePath(const bool absolute, const StringList& components) const;
        };
    }
}

#endif /* defined(__TrenchBroom__Path__) */