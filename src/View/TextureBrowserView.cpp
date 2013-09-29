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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "TextureBrowserView.h"

#include "GL/GL.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/FaceTexture.h"
#include "Renderer/RenderResources.h"
#include "Renderer/TextureFont.h"
#include "Renderer/VertexArray.h"
#include "View/TextureSelectedCommand.h"

namespace TrenchBroom {
    namespace View {
        TextureCellData::TextureCellData(Assets::FaceTexture* i_texture, const Renderer::FontDescriptor& i_fontDescriptor) :
        texture(i_texture),
        fontDescriptor(i_fontDescriptor) {}

        TextureBrowserView::TextureBrowserView(wxWindow* parent, wxWindowID windowId,
                                               wxScrollBar* scrollBar,
                                               Renderer::RenderResources& resources,
                                               Assets::TextureManager& textureManager) :
        CellView(parent, windowId, &resources.glAttribs().front(), resources.sharedContext(), scrollBar),
        m_resources(resources),
        m_textureManager(textureManager),
        m_group(false),
        m_hideUnused(false),
        m_sortOrder(Assets::TextureManager::Name),
        m_vbo(0xFFF),
        m_selectedTexture(NULL) {}
        
        TextureBrowserView::~TextureBrowserView() {
            clear();
        }

        void TextureBrowserView::setSortOrder(const Assets::TextureManager::SortOrder sortOrder) {
            if (sortOrder == m_sortOrder)
                return;
            m_sortOrder = sortOrder;
            reload();
            Refresh();
        }
        
        void TextureBrowserView::setGroup(const bool group) {
            if (group == m_group)
                return;
            m_group = group;
            reload();
            Refresh();
        }
        
        void TextureBrowserView::setHideUnused(const bool hideUnused) {
            if (hideUnused == m_hideUnused)
                return;
            m_hideUnused = hideUnused;
            reload();
            Refresh();
        }
        
        void TextureBrowserView::setFilterText(const String& filterText) {
            if (filterText == m_filterText)
                return;
            m_filterText = filterText;
            reload();
            Refresh();
        }

        Assets::FaceTexture* TextureBrowserView::selectedTexture() const {
            return m_selectedTexture;
        }
        
        void TextureBrowserView::setSelectedTexture(Assets::FaceTexture* selectedTexture) {
            if (m_selectedTexture == selectedTexture)
                return;
            m_selectedTexture = selectedTexture;
            Refresh();
        }

        void TextureBrowserView::doInitLayout(Layout& layout) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const float scaleFactor = prefs.getFloat(Preferences::TextureBrowserIconSize);
            
            layout.setOuterMargin(5.0f);
            layout.setGroupMargin(5.0f);
            layout.setRowMargin(5.0f);
            layout.setCellMargin(5.0f);
            layout.setTitleMargin(2.0f);
            layout.setCellWidth(scaleFactor * 64.0f, scaleFactor * 64.0f);
            layout.setCellHeight(scaleFactor * 64.0f, scaleFactor * 128.0f);
        }
        
        void TextureBrowserView::doReloadLayout(Layout& layout) {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            const String fontName = prefs.getString(Preferences::RendererFontName);
            int fontSize = prefs.getInt(Preferences::BrowserFontSize);
            assert(fontSize > 0);
            
            const Renderer::FontDescriptor font(fontName, static_cast<size_t>(fontSize));
            
            if (m_group) {
                const Assets::TextureManager::GroupList& groups = m_textureManager.groups(m_sortOrder);
                Assets::TextureManager::GroupList::const_iterator gIt, gEnd;
                for (gIt = groups.begin(), gEnd = groups.end(); gIt != gEnd; ++gIt) {
                    const IO::Path& path = gIt->first;
                    const String groupName = path.lastComponent().asString();
                    const Assets::FaceTextureList& textures = gIt->second;
                    
                    layout.addGroup(groupName, fontSize + 2.0f);
                    
                    Assets::FaceTextureList::const_iterator tIt, tEnd;
                    for (tIt = textures.begin(), tEnd = textures.end(); tIt != tEnd; ++tIt) {
                        Assets::FaceTexture* texture = *tIt;
                        addTextureToLayout(layout, texture, font);
                    }
                }
            } else {
                const Assets::FaceTextureList& textures = m_textureManager.textures(m_sortOrder);
                Assets::FaceTextureList::const_iterator it, end;
                for (it = textures.begin(), end = textures.end(); it != end; ++it) {
                    Assets::FaceTexture* texture = *it;
                    addTextureToLayout(layout, texture, font);
                }
            }
        }
        
        void TextureBrowserView::addTextureToLayout(Layout& layout, Assets::FaceTexture* texture, const Renderer::FontDescriptor& font) {
            if ((!m_hideUnused || texture->usageCount() > 0) &&
                (m_filterText.empty() || StringUtils::containsCaseInsensitive(texture->name(), m_filterText))) {
                Renderer::FontManager& fontManager = m_resources.fontManager();
                const float maxCellWidth = layout.maxCellWidth();
                const Renderer::FontDescriptor actualFont = fontManager.selectFontSize(font, texture->name(), maxCellWidth, 5);
                const Vec2f actualSize = fontManager.font(actualFont).measure(texture->name());
                
                PreferenceManager& prefs = PreferenceManager::instance();
                const float scaleFactor = prefs.getFloat(Preferences::TextureBrowserIconSize);
                const size_t scaledTextureWidth = static_cast<size_t>(Math::round(scaleFactor * static_cast<float>(texture->width())));
                const size_t scaledTextureHeight = static_cast<size_t>(Math::round(scaleFactor * static_cast<float>(texture->height())));

                layout.addItem(TextureCellData(texture, actualFont),
                               scaledTextureWidth,
                               scaledTextureHeight,
                               actualSize.x(),
                               font.size() + 2.0f);
            }
        }

        void TextureBrowserView::doClear() {}
        
        void TextureBrowserView::doRender(Layout& layout, const float y, const float height) {

            const float viewLeft      = static_cast<float>(GetClientRect().GetLeft());
            const float viewTop       = static_cast<float>(GetClientRect().GetBottom());
            const float viewRight     = static_cast<float>(GetClientRect().GetRight());
            const float viewBottom    = static_cast<float>(GetClientRect().GetTop());
            
            const Mat4x4f projection = orthoMatrix(-1.0f, 1.0f, viewLeft, viewTop, viewRight, viewBottom);
            const Mat4x4f view = viewMatrix(Vec3f::NegZ, Vec3f::PosY) * translationMatrix(Vec3f(0.0f, 0.0f, 0.1f));
            const Renderer::Transformation transformation(projection, view);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.active();
            
            glDisable(GL_DEPTH_TEST);
            glFrontFace(GL_CCW);
            
            renderBounds(layout, y, height);
            renderTextures(layout, y, height);
            renderNames(layout, y, height);
        }

        void TextureBrowserView::renderBounds(Layout& layout, const float y, const float height) {
            typedef Renderer::VertexSpecs::P2C4::Vertex BoundsVertex;
            BoundsVertex::List vertices;
            
            for (size_t i = 0; i < layout.size(); ++i) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (size_t j = 0; j < group.size(); ++j) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (size_t k = 0; k < row.size(); ++k) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                const LayoutBounds& bounds = cell.itemBounds();
                                const Assets::FaceTexture* texture = cell.item().texture;
                                const Color& color = textureColor(*texture);
                                vertices.push_back(BoundsVertex(Vec2f(bounds.left() - 2.0f, height - (bounds.top() - 2.0f - y)), color));
                                vertices.push_back(BoundsVertex(Vec2f(bounds.left() - 2.0f, height - (bounds.bottom() + 2.0f - y)), color));
                                vertices.push_back(BoundsVertex(Vec2f(bounds.right() + 2.0f, height - (bounds.bottom() + 2.0f - y)), color));
                                vertices.push_back(BoundsVertex(Vec2f(bounds.right() + 2.0f, height - (bounds.top() - 2.0f - y)), color));
                            }
                        }
                    }
                }
            }

            Renderer::VertexArray vertexArray(m_vbo, GL_QUADS, vertices);
            Renderer::ActiveShader shader(m_resources.shaderManager(), Renderer::Shaders::TextureBrowserBorderShader);
            vertexArray.render();
        }
        
        const Color& TextureBrowserView::textureColor(const Assets::FaceTexture& texture) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            if (&texture == m_selectedTexture)
                return prefs.getColor(Preferences::TextureBrowserSelectedColor);
            if (texture.usageCount() > 0)
                return prefs.getColor(Preferences::TextureBrowserUsedColor);
            return prefs.getColor(Preferences::TextureBrowserDefaultColor);
        }

        void TextureBrowserView::renderTextures(Layout& layout, const float y, const float height) {
            PreferenceManager& prefs = PreferenceManager::instance();

            typedef Renderer::VertexSpecs::P2T2::Vertex TextureVertex;
            TextureVertex::List vertices(4);

            Renderer::ActiveShader shader(m_resources.shaderManager(), Renderer::Shaders::TextureBrowserShader);
            shader.set("ApplyTinting", false);
            shader.set("Texture", 0);
            shader.set("Brightness", prefs.getFloat(Preferences::Brightness));
            
            for (size_t i = 0; i < layout.size(); ++i) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (size_t j = 0; j < group.size(); ++j) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (size_t k = 0; k < row.size(); ++k) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                const LayoutBounds& bounds = cell.itemBounds();
                                const Assets::FaceTexture* texture = cell.item().texture;
                                
                                vertices[0] = TextureVertex(Vec2f(bounds.left(),  height - (bounds.top() - y)),    Vec2f(0.0f, 0.0f));
                                vertices[1] = TextureVertex(Vec2f(bounds.left(),  height - (bounds.bottom() - y)), Vec2f(0.0f, 1.0f));
                                vertices[2] = TextureVertex(Vec2f(bounds.right(), height - (bounds.bottom() - y)), Vec2f(1.0f, 1.0f));
                                vertices[3] = TextureVertex(Vec2f(bounds.right(), height - (bounds.top() - y)),    Vec2f(1.0f, 0.0f));

                                Renderer::VertexArray vertexArray(m_vbo, GL_QUADS, vertices);

                                shader.set("GrayScale", texture->overridden());
                                texture->activate();
                                vertexArray.render();
                            }
                        }
                    }
                }
            }
            
        }
        
        void TextureBrowserView::renderNames(Layout& layout, const float y, const float height) {
            renderGroupTitleBackgrounds(layout, y, height);
            renderStrings(layout, y, height);
        }

        void TextureBrowserView::renderGroupTitleBackgrounds(Layout& layout, const float y, const float height) {
            typedef Renderer::VertexSpecs::P2::Vertex Vertex;
            Vertex::List vertices;
            
            for (size_t i = 0; i < layout.size(); ++i) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    const LayoutBounds titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                    vertices.push_back(Vertex(Vec2f(titleBounds.left(), height - (titleBounds.top() - y))));
                    vertices.push_back(Vertex(Vec2f(titleBounds.left(), height - (titleBounds.bottom() - y))));
                    vertices.push_back(Vertex(Vec2f(titleBounds.right(), height - (titleBounds.bottom() - y))));
                    vertices.push_back(Vertex(Vec2f(titleBounds.right(), height - (titleBounds.top() - y))));
                }
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            Renderer::ActiveShader shader(m_resources.shaderManager(), Renderer::Shaders::BrowserGroupShader);
            shader.set("Color", prefs.getColor(Preferences::BrowserGroupBackgroundColor));
            
            Renderer::VertexArray vertexArray(m_vbo, GL_QUADS, vertices);
            vertexArray.render();
        }
        
        void TextureBrowserView::renderStrings(Layout& layout, const float y, const float height) {
            typedef std::map<Renderer::FontDescriptor, Renderer::VertexArray> StringRendererMap;
            StringRendererMap stringRenderers;
            
            { // create and upload all vertex arrays
                Renderer::SetVboState mapVbo(m_vbo);
                mapVbo.mapped();
                
                const StringMap stringVertices = collectStringVertices(layout, y, height);
                StringMap::const_iterator it, end;
                for (it = stringVertices.begin(), end = stringVertices.end(); it != end; ++it) {
                    const Renderer::FontDescriptor& descriptor = it->first;
                    const StringVertex::List& vertices = it->second;
                    stringRenderers[descriptor] = Renderer::VertexArray(m_vbo, GL_QUADS, vertices);
                    stringRenderers[descriptor].prepare();
                }
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            Renderer::ActiveShader shader(m_resources.shaderManager(), Renderer::Shaders::TextShader);
            shader.set("Color", prefs.getColor(Preferences::BrowserTextColor));
            shader.set("FaceTexture", 0);
            
            StringRendererMap::iterator it, end;
            for (it = stringRenderers.begin(), end = stringRenderers.end(); it != end; ++it) {
                const Renderer::FontDescriptor& descriptor = it->first;
                Renderer::VertexArray& vertexArray = it->second;
                
                Renderer::TextureFont& font = m_resources.fontManager().font(descriptor);
                font.activate();
                vertexArray.render();
                font.deactivate();
            }
        }
        
        TextureBrowserView::StringMap TextureBrowserView::collectStringVertices(Layout& layout, const float y, const float height) {
            PreferenceManager& prefs = PreferenceManager::instance();
            Renderer::FontDescriptor defaultDescriptor(prefs.getString(Preferences::RendererFontName),
                                                       static_cast<size_t>(prefs.getInt(Preferences::BrowserFontSize)));
            
            StringMap stringVertices;
            for (size_t i = 0; i < layout.size(); ++i) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    const String& title = group.item();
                    if (!title.empty()) {
                        const LayoutBounds titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                        const Vec2f offset(titleBounds.left() + 2.0f, height - (titleBounds.top() - y) - titleBounds.height());
                        
                        Renderer::TextureFont& font = m_resources.fontManager().font(defaultDescriptor);
                        const Vec2f::List quads = font.quads(title, false, offset);
                        const StringVertex::List titleVertices = StringVertex::fromLists(quads, quads, quads.size() / 2, 0, 2, 1, 2);
                        StringVertex::List& vertices = stringVertices[defaultDescriptor];
                        vertices.insert(vertices.end(), titleVertices.begin(), titleVertices.end());
                    }
                    
                    for (size_t j = 0; j < group.size(); ++j) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                const LayoutBounds titleBounds = cell.titleBounds();
                                const Vec2f offset(titleBounds.left(), height - (titleBounds.top() - y) - titleBounds.height());
                                
                                Renderer::TextureFont& font = m_resources.fontManager().font(cell.item().fontDescriptor);
                                const Vec2f::List quads = font.quads(cell.item().texture->name(), false, offset);
                                const StringVertex::List titleVertices = StringVertex::fromLists(quads, quads, quads.size() / 2, 0, 2, 1, 2);
                                StringVertex::List& vertices = stringVertices[cell.item().fontDescriptor];
                                vertices.insert(vertices.end(), titleVertices.begin(), titleVertices.end());
                            }
                        }
                    }
                }
            }
            
            return stringVertices;
        }

        void TextureBrowserView::doLeftClick(Layout& layout, const float x, const float y) {
            const Layout::Group::Row::Cell* result = NULL;
            if (layout.cellAt(x, y, &result)) {
                if (!result->item().texture->overridden()) {
                    Assets::FaceTexture* texture = result->item().texture;
                    
                    TextureSelectedCommand command;
                    command.setTexture(texture);
                    command.SetEventObject(this);
                    command.SetId(GetId());
                    ProcessEvent(command);
                    
                    if (command.IsAllowed())
                        m_selectedTexture = texture;

                    Refresh();
                }
            }
        }

        wxString TextureBrowserView::tooltip(const Layout::Group::Row::Cell& cell) {
            wxString tooltip;
            tooltip << cell.item().texture->name() << "\n";
            tooltip << cell.item().texture->width() << "x" << cell.item().texture->height();
            return tooltip;
        }
    }
}
