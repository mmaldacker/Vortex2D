//
//  Common.h
//  Vortex
//
//  Created by Maximilian Maldacker on 07/04/2014.
//
//

#ifndef Vortex_Common_h
#define Vortex_Common_h

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>

#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#define CHECK_GL_ERROR_DEBUG() ({ GLenum __error = glGetError(); if(__error) SDL_Log("OpenGL error 0x%04X in %s %d\n", __error, __FUNCTION__, __LINE__); })

namespace Renderer
{

template<typename T>
T next_power_of_two(T value)
{
    if ((value & (value - 1)) == 0)
        return value;
    value -= 1;
    for (size_t i = 1; i < sizeof(T) * 8; i <<= 1)
        value = value | value >> i;
    return value + 1;
}

inline bool supports_npot_textures()
{
    return true;
}

typedef std::vector<glm::vec2> Path;

inline glm::vec4 GetBoundingBox(const Path & path)
{
    glm::vec2 min{std::numeric_limits<float>::max()};
    glm::vec2 max{std::numeric_limits<float>::lowest()};
    for(auto p : path)
    {
        if(p.x < min.x) min.x = p.x;
        if(p.y < min.y) min.y = p.y;
        if(p.x > max.x) max.x = p.x;
        if(p.y > max.y) max.y = p.y;
    }

    return glm::vec4{min, glm::vec2{max - min}};
}

inline float Area(const Path &poly)
{
    int size = (int)poly.size();
    if (size < 3) return 0;

    float a = 0;
    for (int i = 0, j = size - 1; i < size; j = i++)
    {
        a += (poly[j].x + poly[i].x) * (poly[j].y - poly[i].y);
    }

    return -a * 0.5;
}

inline bool Orientation(const Path &poly)
{
    return Area(poly) >= 0;
}

}

#endif
