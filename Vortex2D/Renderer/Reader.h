//
//  Reader.h
//  Vortex2D
//

#ifndef Vortex2D_Reader_h
#define Vortex2D_Reader_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Renderer/Data.h>

namespace Vortex2D { namespace Renderer {

/**
 * @brief Helper class to read & print (to stdout) the content of a RenderTexture
 */
class Reader
{
public:
    Reader(RenderTexture& texture);
    Reader(Buffer& buffer);
    Reader(Reader&&);
    ~Reader();

    /**
     * @brief Reads the content of the render texture (necessary before calling the print/get methods)
     * @return returns *this
     */
    Reader& Read();

    /**
     * @brief Prints the colour part of the render texture to stdout
     * @return returns *this
     */
    Reader& Print();

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


    int Width() const;
    int Height() const;

private:
    float Get(int x, int y, int size, int offset) const;

    Renderer::RenderTexture& mTexture;
};

}}

#endif
