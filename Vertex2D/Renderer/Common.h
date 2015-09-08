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
#include <vector>
#include <iostream>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

//FIXME better error logging
#define CHECK_GL_ERROR_DEBUG() ({ GLenum __error = glGetError(); if(__error) std::cout << "OpenGL error " << __error << " " << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << "\n"; })

#define GLSL(src) "#version 150 core\n" #src

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

struct Rect
{
    Rect(glm::vec2 pos, glm::vec2 size) : Pos(pos), Size(size) {}
    Rect(glm::vec2 size) : Pos(glm::vec2{0.0}), Size(size) {}
    Rect() : Pos(glm::vec2{0.0}), Size(glm::vec2{0.0f}) {}

    glm::vec2 Pos;
    glm::vec2 Size;
};

struct TextureCoords
{
    Rect tex;
    Rect pos;
};

}

#endif
