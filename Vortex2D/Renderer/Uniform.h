//
//  Uniform.h
//  Vortex2D
//

#ifndef Vortex_Uniform_h
#define Vortex_Uniform_h

#include "Common.h"

namespace Vortex2D { namespace Renderer {

inline void glUniform(GLint loc, const glm::mat4 & value)
{
    glUniformMatrix4fv(loc, 1, GL_FALSE, &value[0][0]);
}

inline void glUniform(GLint loc, const GLint & value)
{
    glUniform1i(loc, value);
}

inline void glUniform(GLint loc, const float & value)
{
    glUniform1f(loc, value);
}

inline void glUniform(GLint loc, const glm::vec4 & value)
{
    glUniform4fv(loc, 1, &value[0]);
}

inline void glUniform(GLint loc, const glm::vec2 & value)
{
    glUniform2fv(loc, 1, &value[0]);
}

}}

#endif
