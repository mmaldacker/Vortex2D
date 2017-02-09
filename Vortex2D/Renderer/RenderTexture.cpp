//
//  RenderTexture.cpp
//  Vortex
//

#include "RenderTexture.h"

#include <algorithm>
#include <string>
#include <stdexcept>

namespace Vortex2D { namespace Renderer {

RenderTexture::RenderTexture(int width, int height, Texture::PixelFormat pixelFormat, DepthFormat depthFormat)
    : Texture(width, height, pixelFormat)
    , RenderTarget(width, height)
    , mDepthRenderBuffer(0)
{
    GLint oldRenderBuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mOldFrameBuffer);
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRenderBuffer);

    glGenFramebuffers(1, &mFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mId, 0);

    if (depthFormat != DepthFormat::NONE)
    {
        glGenRenderbuffers(1, &mDepthRenderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, (GLenum)depthFormat, Width(), Height());

        if (depthFormat == DepthFormat::DEPTH24_STENCIL8)
        {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthRenderBuffer);
        }
        else
        {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRenderBuffer);
        }
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("RenderTexture creation failed with error " + std::to_string(status));
    }

    glBindRenderbuffer(GL_RENDERBUFFER, oldRenderBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mOldFrameBuffer);
}

RenderTexture::~RenderTexture()
{
    if (mFrameBuffer)
    {
        glDeleteFramebuffers(1, &mFrameBuffer);
    }
    if (mDepthRenderBuffer)
    {
        glDeleteRenderbuffers(1, &mDepthRenderBuffer);
    }
}

RenderTexture::RenderTexture(RenderTexture&& other)
    : Texture(std::move(other))
    , RenderTarget(std::move(other))
{
    mFrameBuffer = other.mFrameBuffer;
    mDepthRenderBuffer = other.mDepthRenderBuffer;
    Orth = other.Orth;

    other.mFrameBuffer = 0;
    other.mDepthRenderBuffer = 0;
}

RenderTexture& RenderTexture::operator=(RenderTexture&& other)
{
    mFrameBuffer = other.mFrameBuffer;
    mDepthRenderBuffer = other.mDepthRenderBuffer;
    Orth = other.Orth;

    other.mFrameBuffer = 0;
    other.mDepthRenderBuffer = 0;

    Texture::operator=(std::move(other));
    RenderTarget::operator=(std::move(other));

    return *this;
}

void RenderTexture::Clear(const glm::vec4& colour)
{
    Begin();
    GLfloat clearColour[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE,clearColour);
    glClearColor(colour.r, colour.g, colour.b, colour.a);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(clearColour[0], clearColour[1], clearColour[2], clearColour[3]);
    End();
}

void RenderTexture::ClearStencil()
{
    Begin();
    glClearStencil(0);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilMask(0);
    End();
}

void RenderTexture::Render(Drawable& object, const glm::mat4& transform)
{
    Begin();
    object.Render(*this, transform);
    End();
}

void RenderTexture::Begin()
{
    glGetIntegerv(GL_VIEWPORT, mOldViewPort);

    glViewport(0, 0, Width(), Height());

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mOldFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
}

void RenderTexture::End()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mOldFrameBuffer);
    glViewport(mOldViewPort[0], mOldViewPort[1], mOldViewPort[2], mOldViewPort[3]);
}

}}
