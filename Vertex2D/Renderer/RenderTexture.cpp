//
//  RenderTexture.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 06/04/2014.
//
//

#include "RenderTexture.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace Renderer
{

RenderTexture::RenderTexture(int width, int height, Texture::PixelFormat pixelFormat, DepthFormat depthFormat)
    : Texture(width, height, pixelFormat, nullptr)
    , mDepthRenderBuffer(0)
{
    GLint oldRenderBuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mOldFrameBuffer);
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRenderBuffer);

    glGenFramebuffers(1, &mFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);

    //FIXME bind texture here?

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mId, 0);

    if(depthFormat != DepthFormat::NONE)
    {
        glGenRenderbuffers(1, &mDepthRenderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, (GLenum)depthFormat, Width(), Height());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRenderBuffer);

        // if depth format is the one with stencil part, bind same render buffer as stencil attachment
        if (depthFormat == DepthFormat::DEPTH24_STENCIL8)
        {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthRenderBuffer);
        }
    }

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindRenderbuffer(GL_RENDERBUFFER, oldRenderBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mOldFrameBuffer);

    Orth = glm::ortho(0.0f, (float)Width(), 0.0f, (float)Height());
}

RenderTexture::~RenderTexture()
{
    if(mFrameBuffer)
    {
        glDeleteFramebuffers(1, &mFrameBuffer);
    }
	if (mDepthRenderBuffer)
    {
		glDeleteRenderbuffers(1, &mDepthRenderBuffer);
    }
}

RenderTexture::RenderTexture(RenderTexture && other) : Texture(std::move(other))
{
    mFrameBuffer = other.mFrameBuffer;
    mDepthRenderBuffer = other.mDepthRenderBuffer;
    Orth = other.Orth;

    other.mFrameBuffer = 0;
    other.mDepthRenderBuffer = 0;
}

RenderTexture & RenderTexture::operator=(RenderTexture && other)
{
    mFrameBuffer = other.mFrameBuffer;
    mDepthRenderBuffer = other.mDepthRenderBuffer;
    Orth = other.Orth;

    other.mFrameBuffer = 0;
    other.mDepthRenderBuffer = 0;

    Texture::operator=(std::move(other));

    return *this;
}

void RenderTexture::Clear()
{
    begin({0,0,0,0});
    end();
}

void RenderTexture::begin()
{
    glGetIntegerv(GL_VIEWPORT, mOldViewPort);

    glViewport(0, 0, Width(), Height());

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mOldFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
}

void RenderTexture::begin(const glm::vec4 &colour)
{
    begin();

    GLfloat clearColour[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE,clearColour);
    glClearColor(colour.r, colour.g, colour.b, colour.a);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(clearColour[0], clearColour[1], clearColour[2], clearColour[3]);
}

void RenderTexture::end()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mOldFrameBuffer);
    glViewport(mOldViewPort[0], mOldViewPort[1], mOldViewPort[2], mOldViewPort[3]);
}

PingPong::PingPong(int width, int height, Texture::PixelFormat pixelFormat, RenderTexture::DepthFormat depthFormat)
: Front(width, height, pixelFormat, depthFormat)
, Back(width, height, pixelFormat, depthFormat)
{
    Orth = Front.Orth;
}

PingPong::PingPong(PingPong && other) : Front(std::move(other.Front)), Back(std::move(other.Back))
{
    Orth = Front.Orth;
}

void PingPong::Clear()
{
    Front.Clear();
    Back.Clear();
}

void PingPong::swap()
{
    std::swap(Front, Back);
}

void PingPong::begin()
{
    Front.begin();
}

void PingPong::begin(const glm::vec4 & colour)
{
    Front.begin(colour);
}

void PingPong::end()
{
    Front.end();
}

}