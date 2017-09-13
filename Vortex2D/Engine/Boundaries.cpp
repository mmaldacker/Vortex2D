//
//  Boundaries.cpp
//  Vortex2D
//

#include "Boundaries.h"

#include <Vortex2D/Engine/LevelSet.h>

namespace Vortex2D { namespace Fluid {

namespace
{

    bool IsClockwise(const std::vector<glm::vec2>& points)
    {
        float total = 0.0f;
        for (int i = points.size() - 1, j = 0; j < points.size(); i = j++)
        {
            total += (points[j].x - points[i].x) * (points[j].y + points[j].y);
        }

        return total > 0.0;
    }

}

Polygon::Polygon(const Renderer::Device& device, std::vector<glm::vec2> points, bool inverse)
    : mSize(points.size())
    , mLocalMVPBuffer(device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(glm::mat4))
    , mMVPBuffer(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(glm::mat4))
    , mVertexBuffer(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(glm::vec2) * points.size())
    , mTransformedVertices(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(glm::vec2) * points.size())
    , mUpdateCmd(device, false)
    , mRender(device, Renderer::ComputeSize::Default2D(), inverse ? "../Vortex2D/PolygonMax.comp.spv" : "../Vortex2D/PolygonMin.comp.spv",
             {vk::DescriptorType::eStorageImage,
              vk::DescriptorType::eStorageBuffer}, 4)
    , mUpdate(device, {(int)points.size()}, "../Vortex2D/UpdateVertices.comp.spv",
              {vk::DescriptorType::eStorageBuffer,
               vk::DescriptorType::eStorageBuffer,
               vk::DescriptorType::eStorageBuffer}, 4)
    , mUpdateBound(mUpdate.Bind({mMVPBuffer, mVertexBuffer, mTransformedVertices}))
{
    Renderer::Buffer localVertexBuffer(device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(glm::vec2) * points.size());

    assert(IsClockwise(points));

    if (inverse)
    {
        std::reverse(points.begin(), points.end());
    }
    localVertexBuffer.CopyFrom(points);

    Renderer::ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        mVertexBuffer.CopyFrom(commandBuffer, localVertexBuffer);
    });

    mUpdateCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
       mMVPBuffer.CopyFrom(commandBuffer, mLocalMVPBuffer);
       mUpdateBound.PushConstant(commandBuffer, 8, mSize);
       mUpdateBound.Record(commandBuffer);
       mTransformedVertices.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    });
}

void Polygon::Initialize(LevelSet& levelSet)
{
    glm::ivec2 size(levelSet.Width, levelSet.Height);
    mRenderBounds.emplace_back(levelSet, mRender.Bind(size, {levelSet, mTransformedVertices}));
}

void Polygon::Update(const glm::mat4& view)
{
    mLocalMVPBuffer.CopyFrom(view * GetTransform());
    mUpdateCmd.Submit();
}

void Polygon::Draw(vk::CommandBuffer commandBuffer, LevelSet& levelSet)
{
    auto renderBoundIt = std::find_if(mRenderBounds.begin(), mRenderBounds.end(), [&](const auto& other)
    {
        return other.first == levelSet;
    });

    if (renderBoundIt != mRenderBounds.end())
    {
        renderBoundIt->second.PushConstant(commandBuffer, 8, mSize);
        renderBoundIt->second.Record(commandBuffer);
    }
}

Rectangle::Rectangle(const Renderer::Device& device, const glm::vec2& size, bool inverse)
    : Polygon(device, {{0.0f, size.y}, {size.x, size.y}, {size.x, 0.0f}, {0.0f, 0.0f}}, inverse)
{
}

DistanceField::DistanceField(const Renderer::Device& device, LevelSet& levelSet)
    : Renderer::AbstractSprite(device, "../Vortex2D/DistanceField.frag.spv", levelSet)
{
}

}}
