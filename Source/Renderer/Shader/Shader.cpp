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

#include "Shader.h"

#include "Model/Texture.h"
#include "Utility/Console.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        Shader::Shader(const String& name, GLenum type, const String& source, Utility::Console& console) :
        m_name(name),
        m_type(type),
        m_source(source),
        m_console(console),
        m_shaderId(0) {
            assert(m_type == GL_VERTEX_SHADER || m_type == GL_FRAGMENT_SHADER);
        }
        
        Shader::~Shader() {
            if (m_shaderId != 0) {
                glDeleteShader(m_shaderId);
                m_shaderId = 0;
            }
        }

        bool Shader::createShader() {
            assert(m_shaderId == 0);
            m_shaderId = glCreateShader(m_type);
            if (m_shaderId == 0) {
                m_console.error("Unable to create %s", m_name.c_str());
                return false;
            }
            
            const char* cSource = m_source.c_str();
            glShaderSource(m_shaderId, 1, &cSource, NULL);
            glCompileShader(m_shaderId);
            
            GLint compileStatus;
            glGetShaderiv(m_shaderId, GL_COMPILE_STATUS, &compileStatus);

            GLint infoLogLength;
            glGetShaderiv(m_shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
            char infoLog[infoLogLength];
            glGetShaderInfoLog(m_shaderId, infoLogLength, &infoLogLength, infoLog);
            
            if (compileStatus == 0) {
                m_console.error("Unable to compile %s, compilation output was:", m_name.c_str());
                m_console.error(infoLog);
                return false;
            } else {
                m_console.debug(infoLog);
            }

            m_console.debug("Created %s", m_name.c_str());

            return true;
        }

        void Shader::attachTo(GLuint programId) {
            glAttachShader(programId, m_shaderId);
        }

        void Shader::detachFrom(GLuint programId) {
            glDetachShader(programId, m_shaderId);
        }
        
        GLint ShaderProgram::uniformLocation(const String& name) {
            UniformVariableMap::iterator it = m_uniformVariables.find(name);
            if (it == m_uniformVariables.end()) {
                m_console.warn("Cannot set unknown uniform variable '%s' in %s", name.c_str(), m_name.c_str());
                return -1;
            }
            
            GLint location = it->second;
            if (location == -1) {
                m_console.warn("Cannot set uniform variable '%s' with unknown location in %s", name.c_str(), m_name.c_str());
                return -1;
            }
            
            return location;
        }

        ShaderProgram::ShaderProgram(const String& name, Utility::Console& console) :
        m_name(name),
        m_console(console),
        m_programId(0),
        m_needsLinking(true) {}
        
        ShaderProgram::ShaderProgram(const String& name, Utility::Console& console, const String& uniformVariable1) :
        m_name(name),
        m_console(console),
        m_programId(0),
        m_needsLinking(true) {
            m_uniformVariables[uniformVariable1] = -1;
        }
        
        ShaderProgram::ShaderProgram(const String& name, Utility::Console& console, const String& uniformVariable1, const String& uniformVariable2) :
        m_name(name),
        m_console(console),
        m_programId(0),
        m_needsLinking(true) {
            m_uniformVariables[uniformVariable1] = -1;
            m_uniformVariables[uniformVariable2] = -1;
        }
        
        ShaderProgram::ShaderProgram(const String& name, Utility::Console& console, const String& uniformVariable1, const String& uniformVariable2, const String& uniformVariable3) :
        m_name(name),
        m_console(console),
        m_programId(0),
        m_needsLinking(true) {
            m_uniformVariables[uniformVariable1] = -1;
            m_uniformVariables[uniformVariable2] = -1;
            m_uniformVariables[uniformVariable3] = -1;
        }
        
        ShaderProgram::ShaderProgram(const String& name, Utility::Console& console, const String& uniformVariable1, const String& uniformVariable2, const String& uniformVariable3, const String& uniformVariable4) :
        m_name(name),
        m_console(console),
        m_programId(0),
        m_needsLinking(true) {
            m_uniformVariables[uniformVariable1] = -1;
            m_uniformVariables[uniformVariable2] = -1;
            m_uniformVariables[uniformVariable3] = -1;
            m_uniformVariables[uniformVariable4] = -1;
        }
        
        ShaderProgram::ShaderProgram(const String& name, Utility::Console& console, const String& uniformVariable1, const String& uniformVariable2, const String& uniformVariable3, const String& uniformVariable4, const String& uniformVariable5) :
        m_name(name),
        m_console(console),
        m_programId(0),
        m_needsLinking(true) {
            m_uniformVariables[uniformVariable1] = -1;
            m_uniformVariables[uniformVariable2] = -1;
            m_uniformVariables[uniformVariable3] = -1;
            m_uniformVariables[uniformVariable4] = -1;
            m_uniformVariables[uniformVariable5] = -1;
        }
        
        ShaderProgram::ShaderProgram(const String& name, Utility::Console& console, const StringList& uniformVariables) :
        m_name(name),
        m_console(console),
        m_programId(0),
        m_needsLinking(true) {
            for (unsigned int i = 0; i < uniformVariables.size(); i++)
                m_uniformVariables[uniformVariables[i]] = -1;
        }

        ShaderProgram::~ShaderProgram() {
            if (m_programId != 0) {
                glDeleteProgram(m_programId);
                m_programId = NULL;
            }
        }
        
        bool ShaderProgram::createProgram() {
            assert(m_programId == 0);
            m_programId = glCreateProgram();
            if (m_programId == 0) {
                m_console.error("Unable to create %s", m_name.c_str());
                return false;
            } else {
                m_console.debug("Created %s", m_name.c_str());
            }

            return true;
        }

        void ShaderProgram::attachShader(Shader& shader) {
            assert(m_programId != 0);
            shader.attachTo(m_programId);
            m_needsLinking = true;
        }
        
        void ShaderProgram::detachShader(Shader& shader) {
            assert(m_programId != 0);
            shader.detachFrom(m_programId);
            m_needsLinking = true;
        }
        
        bool ShaderProgram::activate() {
            if (m_programId == 0)
                return false;
            
            if (m_needsLinking) {
                glLinkProgram(m_programId);

                GLint linkStatus;
                glGetProgramiv(m_programId, GL_LINK_STATUS, &linkStatus);
                
                GLint infoLogLength;
                glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &infoLogLength);
                char infoLog[infoLogLength];
                glGetProgramInfoLog(m_programId, infoLogLength, &infoLogLength, infoLog);
                
                if (linkStatus == 0) {
                    m_console.error("Unable to link %s, linker output was:", m_name.c_str());
                    m_console.error(infoLog);
                    return false;
                }
                
                m_console.debug(infoLog);
                m_needsLinking = false;
                
                UniformVariableMap::iterator it, end;
                for (it = m_uniformVariables.begin(), end = m_uniformVariables.end(); it != end; ++it) {
                    const String& uniformVariable = it->first;
                    GLint index = glGetUniformLocation(m_programId, uniformVariable.c_str());
                    if (index == -1)
                        m_console.warn("Location of uniform variable '%s' could not be found in %s", uniformVariable.c_str(), m_name.c_str());
                    it->second = index;
                }
            }
            
            glUseProgram(m_programId);
            return true;
        }
        
        void ShaderProgram::deactivate() {
            glUseProgram(0);
        }

        bool ShaderProgram::setUniformVariable(const String& name, int value) {
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniform1i(location, value);
            return true;
        }

        bool ShaderProgram::setUniformVariable(const String& name, float value) {
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniform1f(location, value);
            return true;
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, const Vec2f& value) {
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniform2f(location, value.x, value.y);
            return true;
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, const Vec3f& value) {
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniform3f(location, value.x, value.y, value.z);
            return true;
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, const Vec4f& value) {
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniform4f(location, value.x, value.y, value.z, value.w);
            return true;
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, const Mat2f& value) {
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniformMatrix2fv(location, 1, false, value.v);
            return true;
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, const Mat3f& value) {
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniformMatrix3fv(location, 1, false, value.v);
            return true;
        }
        
        bool ShaderProgram::setUniformVariable(const String& name, const Mat4f& value) {
            GLint location = uniformLocation(name);
            if (location == -1)
                return false;
            glUniformMatrix4fv(location, 1, false, value.v);
            return true;
        }

        bool ShaderProgram::setUniformVariable(const String& name, Model::Texture* texture) {
            return setUniformVariable(name, static_cast<int>(texture->textureId()));
        }
        
    }
}