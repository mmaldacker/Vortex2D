//
//  Common.h
//  Vortex
//
//  Created by Maximilian Maldacker on 07/04/2014.
//
//

#ifndef Vortex_Common_h
#define Vortex_Common_h

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#endif

#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#define GLSL(src) "#version 150 core\n" #src

#endif
