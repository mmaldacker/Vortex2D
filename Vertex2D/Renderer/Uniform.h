//
//  Uniform.h
//  Vortex
//
//  Created by Maximilian Maldacker on 07/04/2014.
//
//

#ifndef Vortex_Uniform_h
#define Vortex_Uniform_h

#include "Common.h"

namespace Renderer
{

template<class T>
inline void glUniform(GLint, const T &)
{
    assert(false);
}

template<>
inline void glUniform<glm::mat4>(GLint loc, const glm::mat4 & value)
{
    glUniformMatrix4fv(loc, 1, GL_FALSE, &value[0][0]);
}

template<>
inline void glUniform<GLint>(GLint loc, const GLint & value)
{
    glUniform1i(loc, value);
}

template<>
inline void glUniform<float>(GLint loc, const float & value)
{
    glUniform1f(loc, value);
}

template<>
inline void glUniform<glm::vec4>(GLint loc, const glm::vec4 & value)
{
    glUniform4fv(loc, 1, &value[0]);
}

template<>
inline void glUniform<glm::vec2>(GLint loc, const glm::vec2 & value)
{
    glUniform2fv(loc, 1, &value[0]);
}

}

#endif
