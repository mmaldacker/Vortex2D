//
//  Shapes.cpp
//  Vortex2D
//

#include "Shapes.h"

#include <glm/gtx/transform.hpp>

#include <Vortex2D/Renderer/RenderTarget.h>

#include <limits>
#include <algorithm>

namespace Vortex2D { namespace Renderer {

Shape::Shape()
    : Shape(Shader::PositionVert, Shader::PositionFrag)
{

}

Shape::Shape(const char* vert, const char* frag)
    : mProgram(vert, frag)
    , mColourUniform(mProgram, "u_Colour")
{
}

Shape::~Shape()
{
}

Shape::Shape(Shape&& other)
    : Transformable(other)
    , Colour(other.Colour)
    , mProgram(std::move(other.mProgram))
    , mColourUniform(other.mColourUniform)
    , mType(other.mType)
{
}

void Shape::SetType(Type type, const Path& path)
{
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
}

void Shape::Render(RenderTarget& target, const glm::mat4& transform)
{
    vk::Viewport viewport;
    vk::Rect2D scissor;

    vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
    rasterizationInfo
            .setLineWidth(1.0f)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eClockwise);

    vk::PipelineMultisampleStateCreateInfo multisampleInfo;
    multisampleInfo
            .setRasterizationSamples(vk::SampleCountFlagBits::e1)
            .setMinSampleShading(1.0f);

    // TODO add blending
}

Rectangle::Rectangle(const glm::vec2& size)
{
    SetRectangle(size);
}

void Rectangle::SetRectangle(const glm::vec2& size)
{
    SetType(Type::TRIANGLES,
    {{0.0f, 0.0f},
     {size.x, 0.0f},
     {0.0f, size.y},
     {size.x, 0.0f,},
     {size.x, size.y},
     {0.0f, size.y}});
}

Ellipse::Ellipse(const glm::vec2& radius)
{
    SetEllipse(radius);
}

void Ellipse::SetEllipse(const glm::vec2& radius)
{
    SetType(Type::POINTS, {{0.0f,0.0f}});
    mRadius = radius;
}

void Ellipse::Render(RenderTarget& target, const glm::mat4& transform)
{
    glm::vec2 transformScale(glm::length(transform[0]), glm::length(transform[1]));
    glm::vec2 radius = mRadius * (glm::vec2)Scale * transformScale;
    glm::mat4 rotation4 = glm::rotate(glm::radians((float)Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat2 rotation(rotation4[0].xy, rotation4[1].xy);
    mProgram.Use()
            .Set("u_radius", radius)
            .Set("u_size", std::max(radius.x, radius.y))
            .Set("u_rotation", rotation);
    Shape::Render(target, transform);
}

}}
