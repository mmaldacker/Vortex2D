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

#endif
