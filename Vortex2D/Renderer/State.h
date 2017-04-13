//
//  State.h
//  Vortex2D
//

#ifndef State_h
#define State_h

#include <Vortex2D/Renderer/Common.h>

namespace Vortex2D { namespace Renderer { namespace State {

// glUseProgram
void UseProgram(GLuint shader);
void ClearProgram(GLuint shader);

// glActiveTexture/glBindTexture
void BindTexture(GLuint texture, int textureUnit = 0);
void ClearTexture(GLuint texture);

// glViewPort/glBindFrameBuffer
void SetViewPort(GLint x, GLint y, GLsizei width, GLsizei height);

// glClearColor/glClear

}}}

#endif
