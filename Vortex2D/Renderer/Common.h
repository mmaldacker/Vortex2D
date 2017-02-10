//
//  Common.h
//  Vortex
//

#ifndef Vortex_Common_h
#define Vortex_Common_h

#define _USE_MATH_DEFINES
#include <cmath>

#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <glad/glad.h>

#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_SWIZZLE
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#define GLSL(src) "#version 150 core\n" #src

#endif
