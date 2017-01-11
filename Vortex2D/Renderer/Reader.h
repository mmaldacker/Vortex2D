//
//  Reader.h
//  Vortex2D
//

#ifndef __Vortex2D__Reader__
#define __Vortex2D__Reader__

#include "Common.h"
#include "RenderTexture.h"

namespace Vortex2D { namespace Renderer {

/**
 * @brief Helper class to read & print (to stdout) the content of a RenderTexture
 */
class Reader
{
public:
    Reader(Renderer::RenderTexture & texture);
    Reader(Reader &&);
    ~Reader();

    /**
     * @brief Reads the content of the render texture (necessary before calling the print/get methods)
     * @return returns *this
     */
    Reader & Read();

    /**
     * @brief Prints the colour part of the render texture to stdout
     * @return returns *this
     */
    Reader & Print();

    /**
     * @brief Prints the stencil part of the render texture to stdout
     * @return returns *this
     */
    Reader & PrintStencil();

    /**
     * @brief Returns the r value of the render texture
     * @param x must be between 0 and texture width
     * @param y must be between 0 and texture height
     */
    float GetFloat(int x, int y) const;

    /**
     * @brief Returns the rg value of the render texture
     * @param x must be between 0 and texture width
     * @param y must be between 0 and texture height
     */
    glm::vec2 GetVec2(int x, int y) const;

    /**
     * @brief Returns the rgba value of the render texture
     * @param x must be between 0 and texture width
     * @param y must be between 0 and texture height
     */
    glm::vec4 GetVec4(int x, int y) const;

private:
    int GetSize() const;
    float Get(int x, int y, int size, int offset) const;

    Renderer::RenderTexture & mTexture;
    float * mPixels;
    uint8_t * mStencil;
};

}}

#endif /* defined(__Vortex2D__Reader__) */
