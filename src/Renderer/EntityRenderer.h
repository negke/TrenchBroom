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

#ifndef __TrenchBroom__EntityRenderer__
#define __TrenchBroom__EntityRenderer__

#include "Color.h"
#include "Model/ModelTypes.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/EntityModelRenderer.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/TextRenderer.h"

#include <map>

namespace TrenchBroom {
    namespace Model {
        class Filter;
    }
    
    namespace Renderer {
        class FontManager;
        class RenderContext;
        class Vbo;
        
        class EntityRenderer {
        private:
            typedef Model::Entity* Key;
            typedef TextRenderer<Key> ClassnameRenderer;
            
            class EntityClassnameAnchor : public TextAnchor {
            private:
                const Model::Entity* m_entity;
            protected:
                inline const Vec3f basePosition() const;
                inline const Alignment::Type alignment() const;
            public:
                EntityClassnameAnchor(const Model::Entity* entity);
            };
            
            class EntityClassnameFilter : public ClassnameRenderer::TextRendererFilter {
            private:
                const Model::Filter& m_filter;
            public:
                EntityClassnameFilter(const Model::Filter& filter);
                bool stringVisible(RenderContext& context, const Key& entity) const;
            };

            class EntityClassnameColorProvider : public ClassnameRenderer::TextColorProvider {
            private:
                const Color& m_textColor;
                const Color& m_backgroundColor;
            public:
                EntityClassnameColorProvider(const Color& textColor, const Color& backgroundColor);
                
                Color textColor(RenderContext& context, const Key& entity) const;
                Color backgroundColor(RenderContext& context, const Key& entity) const;
            };
            
            const Model::Filter& m_filter;
            Model::EntitySet m_entities;
            EdgeRenderer m_boundsRenderer;
            ClassnameRenderer m_classnameRenderer;
            EntityModelRenderer m_modelRenderer;
            bool m_boundsValid;
            
            Color m_overlayTextColor;
            Color m_overlayBackgroundColor;
            bool m_overrideBoundsColor;
            Color m_boundsColor;
            bool m_renderOccludedBounds;
            Color m_occludedBoundsColor;
        public:
            EntityRenderer(FontManager& m_fontManager, const Model::Filter& filter);
            ~EntityRenderer();

            void addEntity(Model::Entity* entity);
            void updateEntity(Model::Entity* entity);
            void removeEntity(Model::Entity* entity);
            void clear();

            template <typename Iter>
            void addEntities(Iter cur, const Iter end) {
                while (cur != end) {
                    addEntity(*cur);
                    ++cur;
                }
            }
            template <typename Iter>
            void updateEntities(Iter cur, const Iter end) {
                while (cur != end) {
                    updateEntity(*cur);
                    ++cur;
                }
            }
            
            template <typename Iter>
            void removeEntities(Iter cur, const Iter end) {
                while (cur != end) {
                    removeEntity(*cur);
                    ++cur;
                }
            }
            
            void render(RenderContext& context);
            
            const Color& overlayTextColor() const;
            void setOverlayTextColor(const Color& overlayTextColor);
            const Color& overlayBackgroundColor() const;
            void setOverlayBackgroundColor(const Color& overlayBackgroundColor);
            
            bool overrideBoundsColor() const;
            void setOverrideBoundsColor(const bool overrideBoundsColor);
            const Color& boundsColor() const;
            void setBoundsColor(const Color& boundsColor);
            
            bool renderOccludedBounds() const;
            void setRenderOccludedBounds(const bool renderOccludedBounds);
            const Color& occludedBoundsColor() const;
            void setOccludedBoundsColor(const Color& occludedBoundsColor);
        private:
            void renderBounds(RenderContext& context);
            void renderClassnames(RenderContext& context);
            void renderModels(RenderContext& context);
            static TextureFont& font(FontManager& fontManager);
            void invalidateBounds();
            void validateBounds();
            
            const Color& boundsColor(const Model::Entity& entity) const;
        };
    }
}

#endif /* defined(__TrenchBroom__EntityRenderer__) */