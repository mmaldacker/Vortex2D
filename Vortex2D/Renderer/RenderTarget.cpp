//
//  RenderTarget.cpp
//  Vortex2D
//

#include "RenderTarget.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Vortex
{
namespace Renderer
{
RenderTarget::RenderTarget(uint32_t width, uint32_t height)
    : Width(width)
    , Height(height)
    , Orth(glm::ortho(0.0f, (float)width, 0.0f, (float)height))
    , View(1.0f)
{
}

RenderTarget::RenderTarget(RenderTarget&& other)
    : Width(other.Width)
    , Height(other.Height)
    , Orth(other.Orth)
    , View(other.View)
    , RenderPass(std::move(other.RenderPass))
{
}

RenderTarget::~RenderTarget() {}

RenderpassBuilder& RenderpassBuilder::Attachement(vk::Format format)
{
  mAttachementDescriptions.push_back({{}, format});
  return *this;
}

RenderpassBuilder& RenderpassBuilder::AttachementLoadOp(vk::AttachmentLoadOp value)
{
  mAttachementDescriptions.back().setLoadOp(value);
  return *this;
}

RenderpassBuilder& RenderpassBuilder::AttachementStoreOp(vk::AttachmentStoreOp value)
{
  mAttachementDescriptions.back().setStoreOp(value);
  return *this;
}

RenderpassBuilder& RenderpassBuilder::AttachementInitialLayout(vk::ImageLayout layout)
{
  mAttachementDescriptions.back().setInitialLayout(layout);
  return *this;
}

RenderpassBuilder& RenderpassBuilder::AttachementFinalLayout(vk::ImageLayout layout)
{
  mAttachementDescriptions.back().setFinalLayout(layout);
  return *this;
}

RenderpassBuilder& RenderpassBuilder::Subpass(vk::PipelineBindPoint bindPoint)
{
  auto subpassDescription = vk::SubpassDescription().setPipelineBindPoint(bindPoint);

  mSubpassDescriptions.push_back(subpassDescription);
  return *this;
}

RenderpassBuilder& RenderpassBuilder::SubpassColorAttachment(vk::ImageLayout layout,
                                                             uint32_t attachment)
{
  auto& subpass = mSubpassDescriptions.back();
  auto* attachmentRef = GetAttachmentReference();
  attachmentRef->setLayout(layout);
  attachmentRef->setAttachment(attachment);

  if (subpass.colorAttachmentCount == 0)
  {
    subpass.setPColorAttachments(attachmentRef);
  }

  subpass.colorAttachmentCount++;
  return *this;
}

RenderpassBuilder& RenderpassBuilder::Dependency(uint32_t srcSubpass, uint32_t dstSubpass)
{
  auto dependency = vk::SubpassDependency().setSrcSubpass(srcSubpass).setDstSubpass(dstSubpass);

  mSubpassDependencies.push_back(dependency);
  return *this;
}

RenderpassBuilder& RenderpassBuilder::DependencySrcStageMask(vk::PipelineStageFlags value)
{
  mSubpassDependencies.back().setSrcStageMask(value);
  return *this;
}

RenderpassBuilder& RenderpassBuilder::DependencyDstStageMask(vk::PipelineStageFlags value)
{
  mSubpassDependencies.back().setDstStageMask(value);
  return *this;
}

RenderpassBuilder& RenderpassBuilder::DependencySrcAccessMask(vk::AccessFlags value)
{
  mSubpassDependencies.back().setSrcAccessMask(value);
  return *this;
}

RenderpassBuilder& RenderpassBuilder::DependencyDstAccessMask(vk::AccessFlags value)
{
  mSubpassDependencies.back().setDstAccessMask(value);
  return *this;
}

vk::UniqueRenderPass RenderpassBuilder::Create(vk::Device device)
{
  auto renderPassInfo = vk::RenderPassCreateInfo()
                            .setAttachmentCount((uint32_t)mAttachementDescriptions.size())
                            .setPAttachments(mAttachementDescriptions.data())
                            .setSubpassCount((uint32_t)mSubpassDescriptions.size())
                            .setPSubpasses(mSubpassDescriptions.data())
                            .setDependencyCount((uint32_t)mSubpassDependencies.size())
                            .setPDependencies(mSubpassDependencies.data());

  return device.createRenderPassUnique(renderPassInfo);
}

vk::AttachmentReference* RenderpassBuilder::GetAttachmentReference()
{
  if (mNumRefs < mMaxRefs)
  {
    return &mAttachmentReferences[mNumRefs++];
  }
  else
  {
    throw std::runtime_error("Too many attachements");
  }
}

}  // namespace Renderer
}  // namespace Vortex2D
