//
//  Device.cpp
//  Vortex
//

#include <fstream>
#include <iostream>

#include "Device.h"
#include "Instance.h"
#include "Vulkan.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace Vortex
{
namespace Renderer
{
namespace
{
int ComputeFamilyIndex(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface = nullptr)
{
  int index = -1;
  const auto& familyProperties = physicalDevice.getQueueFamilyProperties();
  for (std::size_t i = 0; i < familyProperties.size(); i++)
  {
    const auto& property = familyProperties[i];
    if ((property.queueFlags & vk::QueueFlagBits::eCompute) &&
        (property.queueFlags & vk::QueueFlagBits::eGraphics) &&
        (!surface || physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface)))
    {
      index = static_cast<int32_t>(i);
      break;
    }
  }

  if (index == -1)
  {
    throw std::runtime_error("Suitable physical device not found");
  }

  return index;
}

vk::DescriptorType GetDescriptorType(uint32_t bind, const SPIRV::ShaderLayouts& layout)
{
  for (auto& shaderLayout : layout)
  {
    auto it = shaderLayout.bindings.find(bind);
    if (it != shaderLayout.bindings.end())
    {
      return ConvertDescriptorType(it->second);
    }
  }

  throw std::runtime_error("no bindings defined");
}

void Bind(vk::Device device,
          BindGroup& dstSet,
          const SPIRV::ShaderLayouts& layout,
          const std::vector<BindingInput>& bindingInputs)
{
  std::vector<vk::DescriptorBufferInfo> bufferInfo(20);
  std::vector<vk::DescriptorImageInfo> imageInfo(20);
  std::vector<vk::WriteDescriptorSet> descriptorWrites;
  std::size_t numBuffers = 0;
  std::size_t numImages = 0;

  for (std::size_t i = 0; i < bindingInputs.size(); i++)
  {
    bindingInputs[i].Input.match(
        [&](GenericBuffer* buffer)
        {
          uint32_t bind = bindingInputs[i].Bind == BindingInput::DefaultBind
                              ? static_cast<uint32_t>(i)
                              : bindingInputs[i].Bind;

          auto descriptorType = GetDescriptorType(bind, layout);
          if (descriptorType != vk::DescriptorType::eStorageBuffer &&
              descriptorType != vk::DescriptorType::eUniformBuffer)
            throw std::runtime_error("Binding not a storage buffer");

          vk::DescriptorSet descriptorSet = reinterpret_cast<VkDescriptorSet>(dstSet.Handle());
          auto writeDescription = vk::WriteDescriptorSet()
                                      .setDstSet(descriptorSet)
                                      .setDstBinding(bind)
                                      .setDstArrayElement(0)
                                      .setDescriptorType(descriptorType)
                                      .setPBufferInfo(bufferInfo.data() + numBuffers);
          descriptorWrites.push_back(writeDescription);

          if (!descriptorWrites.empty() && numBuffers != bufferInfo.size() &&
              descriptorWrites.back().pBufferInfo)
          {
            descriptorWrites.back().descriptorCount++;
            bufferInfo[numBuffers++] = vk::DescriptorBufferInfo(
                Handle::ConvertBuffer(buffer->Handle()), 0, buffer->Size());
          }
          else
          {
            assert(false);
          }
        },
        [&](Image image)
        {
          uint32_t bind = bindingInputs[i].Bind == BindingInput::DefaultBind
                              ? static_cast<uint32_t>(i)
                              : bindingInputs[i].Bind;

          auto descriptorType = GetDescriptorType(bind, layout);
          if (descriptorType != vk::DescriptorType::eStorageImage &&
              descriptorType != vk::DescriptorType::eCombinedImageSampler)
            throw std::runtime_error("Binding not an image");

          vk::DescriptorSet descriptorSet = reinterpret_cast<VkDescriptorSet>(dstSet.Handle());
          auto writeDescription = vk::WriteDescriptorSet()
                                      .setDstSet(descriptorSet)
                                      .setDstBinding(bind)
                                      .setDstArrayElement(0)
                                      .setDescriptorType(descriptorType)
                                      .setPImageInfo(imageInfo.data() + numImages);
          descriptorWrites.push_back(writeDescription);

          if (!descriptorWrites.empty() && numImages != imageInfo.size() &&
              descriptorWrites.back().pImageInfo)
          {
            descriptorWrites.back().descriptorCount++;

            vk::Sampler sampler = image.Sampler != nullptr
                                      ? Handle::ConvertSampler(image.Sampler->Handle())
                                      : vk::Sampler{};

            imageInfo[numImages++] =
                vk::DescriptorImageInfo(sampler,
                                        Handle::ConvertImageView(image.Texture->GetView()),
                                        vk::ImageLayout::eGeneral);
          }
          else
          {
            assert(false);
          }
        });
  }

  device.updateDescriptorSets(descriptorWrites, {});
}
}  // namespace

void DynamicDispatcher::vkCmdDebugMarkerBeginEXT(
    VkCommandBuffer commandBuffer,
    const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const
{
  if (mVkCmdDebugMarkerBeginEXT != nullptr)
  {
    mVkCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
  }
}

void DynamicDispatcher::vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const
{
  if (mVkCmdDebugMarkerEndEXT != nullptr)
  {
    mVkCmdDebugMarkerEndEXT(commandBuffer);
  }
}

VulkanDevice::VulkanDevice(const Instance& instance, bool validation)
    : VulkanDevice(instance, ComputeFamilyIndex(instance.GetPhysicalDevice()), false, validation)
{
}

VulkanDevice::VulkanDevice(const Instance& instance, vk::SurfaceKHR surface, bool validation)
    : VulkanDevice(instance,
                   ComputeFamilyIndex(instance.GetPhysicalDevice(), surface),
                   true,
                   validation)
{
}

VulkanDevice::VulkanDevice(const Instance& instance, int familyIndex, bool surface, bool validation)
    : mPhysicalDevice(instance.GetPhysicalDevice()), mFamilyIndex(familyIndex)
{
  float queuePriority = 1.0f;
  auto deviceQueueInfo = vk::DeviceQueueCreateInfo()
                             .setQueueFamilyIndex(familyIndex)
                             .setQueueCount(1)
                             .setPQueuePriorities(&queuePriority);

  std::vector<const char*> deviceExtensions;
  std::vector<const char*> validationLayers;

  if (surface)
  {
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  }

  // make sure we request valid layers only
  auto availableLayers = mPhysicalDevice.enumerateDeviceLayerProperties();
  auto availableExtensions = mPhysicalDevice.enumerateDeviceExtensionProperties();

  // add validation extensions and layers
  if (validation)
  {
    if (HasLayer(VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME, availableLayers))
    {
      validationLayers.push_back(VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME);
    }
    if (HasExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, availableExtensions))
    {
      deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
  }

  // create queue
  auto deviceFeatures = vk::PhysicalDeviceFeatures().setShaderStorageImageExtendedFormats(true);
  auto deviceInfo = vk::DeviceCreateInfo()
                        .setQueueCreateInfoCount(1)
                        .setPQueueCreateInfos(&deviceQueueInfo)
                        .setPEnabledFeatures(&deviceFeatures)
                        .setEnabledExtensionCount((uint32_t)deviceExtensions.size())
                        .setPpEnabledExtensionNames(deviceExtensions.data())
                        .setEnabledLayerCount((uint32_t)validationLayers.size())
                        .setPpEnabledLayerNames(validationLayers.data());

  mDevice = mPhysicalDevice.createDeviceUnique(deviceInfo);
  mQueue = mDevice->getQueue(familyIndex, 0);

  // load marker ext
  if (HasExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, availableExtensions))
  {
    mLoader.mVkCmdDebugMarkerBeginEXT =
        (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(*mDevice, "vkCmdDebugMarkerBeginEXT");
    mLoader.mVkCmdDebugMarkerEndEXT =
        (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(*mDevice, "vkCmdDebugMarkerEndEXT");
  }

  // create command pool
  auto commandPoolInfo = vk::CommandPoolCreateInfo()
                             .setQueueFamilyIndex(familyIndex)
                             .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
  mCommandPool = mDevice->createCommandPoolUnique(commandPoolInfo);

  // create alllocator
  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.physicalDevice = mPhysicalDevice;
  allocatorInfo.device = *mDevice;

  if (vmaCreateAllocator(&allocatorInfo, &mAllocator) != VK_SUCCESS)
  {
    throw std::runtime_error("Error creating allocator");
  }

  // create objects depending on device
  CreateDescriptorPool();
  mPipelineCache = mDevice->createPipelineCacheUnique({});
  mCommandBuffer = std::make_unique<CommandBuffer>(*this, true);
}

VulkanDevice::~VulkanDevice()
{
  vmaDestroyAllocator(mAllocator);
}

bool VulkanDevice::HasTimer() const
{
  auto properties = mPhysicalDevice.getProperties();
  return properties.limits.timestampComputeAndGraphics;
}

void VulkanDevice::WaitIdle()
{
  mDevice->waitIdle();
}

void VulkanDevice::CreateDescriptorPool(int size)
{
  // create descriptor pool
  std::vector<vk::DescriptorPoolSize> poolSizes;
  poolSizes.emplace_back(vk::DescriptorType::eUniformBuffer, size);
  poolSizes.emplace_back(vk::DescriptorType::eCombinedImageSampler, size);
  poolSizes.emplace_back(vk::DescriptorType::eStorageImage, size);
  poolSizes.emplace_back(vk::DescriptorType::eStorageBuffer, size);

  vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
  descriptorPoolInfo.maxSets = 512;
  descriptorPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  mDescriptorPool = mDevice->createDescriptorPoolUnique(descriptorPoolInfo);
}

vk::Device VulkanDevice::Handle() const
{
  return *mDevice;
}

vk::Queue VulkanDevice::Queue() const
{
  return mQueue;
}

const DynamicDispatcher& VulkanDevice::Loader() const
{
  return mLoader;
}

vk::PhysicalDevice VulkanDevice::GetPhysicalDevice() const
{
  return mPhysicalDevice;
}

int VulkanDevice::GetFamilyIndex() const
{
  return mFamilyIndex;
}

void VulkanDevice::Execute(CommandBuffer::CommandFn commandFn) const
{
  (*mCommandBuffer).Record(commandFn).Submit().Wait();
}

VmaAllocator VulkanDevice::Allocator() const
{
  return mAllocator;
}

Handle::ShaderModule VulkanDevice::CreateShaderModule(const SpirvBinary& spirv)
{
  auto it = mShaders.find(spirv.data());
  if (it != mShaders.end())
  {
    VkShaderModule handle = *it->second;
    return reinterpret_cast<Handle::ShaderModule>(handle);
  }

  if (spirv.size() == 0)
    throw std::runtime_error("Invalid SPIRV");

  auto shaderInfo = vk::ShaderModuleCreateInfo().setCodeSize(spirv.size()).setPCode(spirv.data());

  auto shaderModule = mDevice->createShaderModuleUnique(shaderInfo);
  auto& shader = mShaders[spirv.data()] = std::move(shaderModule);

  VkShaderModule handle = *shader;
  return reinterpret_cast<Handle::ShaderModule>(handle);
}

Handle::BindGroupLayout VulkanDevice::CreateBindGroupLayout(const SPIRV::ShaderLayouts& layout)
{
  auto it = std::find_if(mDescriptorSetLayouts.begin(),
                         mDescriptorSetLayouts.end(),
                         [&](const auto& descriptorSetLayout)
                         { return std::get<0>(descriptorSetLayout) == layout; });

  if (it == mDescriptorSetLayouts.end())
  {
    std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings;
    for (auto& shaderLayout : layout)
    {
      for (auto& desciptorType : shaderLayout.bindings)
      {
        descriptorSetLayoutBindings.push_back({desciptorType.first,
                                               ConvertDescriptorType(desciptorType.second),
                                               1,
                                               ConvertShaderStage(shaderLayout.shaderStage),
                                               nullptr});
      }
    }

    auto descriptorSetLayoutInfo =
        vk::DescriptorSetLayoutCreateInfo()
            .setBindingCount((uint32_t)descriptorSetLayoutBindings.size())
            .setPBindings(descriptorSetLayoutBindings.data());

    auto descriptorSetLayout = mDevice->createDescriptorSetLayoutUnique(descriptorSetLayoutInfo);
    mDescriptorSetLayouts.emplace_back(layout, std::move(descriptorSetLayout));
    VkDescriptorSetLayout handle = *std::get<1>(mDescriptorSetLayouts.back());
    return reinterpret_cast<Handle::BindGroupLayout>(handle);
  }

  VkDescriptorSetLayout handle = *std::get<1>(*it);
  return reinterpret_cast<Handle::BindGroupLayout>(handle);
}

Handle::PipelineLayout VulkanDevice::CreatePipelineLayout(const SPIRV::ShaderLayouts& layout)
{
  auto it = std::find_if(mPipelineLayouts.begin(),
                         mPipelineLayouts.end(),
                         [&](const auto& pipelineLayout)
                         { return std::get<0>(pipelineLayout) == layout; });

  if (it == mPipelineLayouts.end())
  {
    auto bindGroupLayout = CreateBindGroupLayout(layout);
    vk::DescriptorSetLayout descriptorSetlayouts[] = {
        reinterpret_cast<VkDescriptorSetLayout>(bindGroupLayout)};
    std::vector<vk::PushConstantRange> pushConstantRanges;
    uint32_t totalPushConstantSize = 0;
    for (auto& shaderLayout : layout)
    {
      if (shaderLayout.pushConstantSize > 0)
      {
        pushConstantRanges.push_back({ConvertShaderStage(shaderLayout.shaderStage),
                                      totalPushConstantSize,
                                      shaderLayout.pushConstantSize});
        totalPushConstantSize += shaderLayout.pushConstantSize;
      }
    }

    auto pipelineLayoutInfo =
        vk::PipelineLayoutCreateInfo().setSetLayoutCount(1).setPSetLayouts(descriptorSetlayouts);

    if (totalPushConstantSize > 0)
    {
      pipelineLayoutInfo.setPPushConstantRanges(pushConstantRanges.data())
          .setPushConstantRangeCount((uint32_t)pushConstantRanges.size());
    }

    mPipelineLayouts.emplace_back(layout, mDevice->createPipelineLayoutUnique(pipelineLayoutInfo));
    VkPipelineLayout handle = *std::get<1>(mPipelineLayouts.back());
    return reinterpret_cast<Handle::PipelineLayout>(handle);
  }

  VkPipelineLayout handle = *std::get<1>(*it);
  return reinterpret_cast<Handle::PipelineLayout>(handle);
}

BindGroup VulkanDevice::CreateBindGroup(const Handle::BindGroupLayout& bindGroupLayout,
                                        const SPIRV::ShaderLayouts& layout,
                                        const std::vector<BindingInput>& bindingInputs)
{
  BindGroup bindGroup(*this, bindGroupLayout);
  Bind(*mDevice, bindGroup, layout, bindingInputs);

  return bindGroup;
}

Handle::Pipeline VulkanDevice::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& graphics,
                                                      const RenderState& renderState)
{
  auto it = std::find_if(mGraphicsPipelines.begin(),
                         mGraphicsPipelines.end(),
                         [&](const GraphicsPipelineCache& pipeline) {
                           return pipeline.Graphics == graphics && pipeline.State == renderState;
                         });

  if (it != mGraphicsPipelines.end())
  {
    VkPipeline handle = *it->Pipeline;
    return reinterpret_cast<Handle::Pipeline>(handle);
  }

  std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
  for (auto& shaderStage : graphics.shaders)
  {
    auto shaderStageInfo =
        vk::PipelineShaderStageCreateInfo()
            .setModule(reinterpret_cast<VkShaderModule>(shaderStage.shaderModule))
            .setPName("main")
            .setStage(ConvertShaderStage(shaderStage.shaderStage));

    shaderStages.push_back(shaderStageInfo);
  }

  std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
  for (auto& vertexBinding : graphics.vertexBindings)
  {
    vertexBindingDescriptions.push_back(
        {vertexBinding.binding, vertexBinding.stride, vk::VertexInputRate::eVertex});
  }

  std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;
  for (auto& vertexAttribute : graphics.vertexAttributes)
  {
    vertexAttributeDescriptions.push_back({vertexAttribute.location,
                                           vertexAttribute.binding,
                                           ConvertFormat(vertexAttribute.format),
                                           vertexAttribute.offset});
  }

  auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo().setTopology(
      ConvertTopology(graphics.primitiveTopology));

  auto rasterizationInfo = vk::PipelineRasterizationStateCreateInfo()
                               .setLineWidth(1.0f)
                               .setCullMode(vk::CullModeFlagBits::eNone)
                               .setFrontFace(vk::FrontFace::eCounterClockwise)
                               .setPolygonMode(vk::PolygonMode::eFill);

  auto multisampleInfo = vk::PipelineMultisampleStateCreateInfo()
                             .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                             .setMinSampleShading(1.0f);

  auto vertexInputInfo =
      vk::PipelineVertexInputStateCreateInfo()
          .setVertexBindingDescriptionCount((uint32_t)vertexBindingDescriptions.size())
          .setPVertexBindingDescriptions(vertexBindingDescriptions.data())
          .setVertexAttributeDescriptionCount((uint32_t)vertexAttributeDescriptions.size())
          .setPVertexAttributeDescriptions(vertexAttributeDescriptions.data());

  auto viewPort = vk::Viewport(0,
                               0,
                               static_cast<float>(renderState.Width),
                               static_cast<float>(renderState.Height),
                               0.0f,
                               1.0f);
  auto scissor = vk::Rect2D({0, 0}, {renderState.Width, renderState.Height});

  auto viewPortState = vk::PipelineViewportStateCreateInfo()
                           .setScissorCount(1)
                           .setPScissors(&scissor)
                           .setViewportCount(1)
                           .setPViewports(&viewPort);

  auto colorBlendAttachement =
      vk::PipelineColorBlendAttachmentState()
          .setBlendEnable(renderState.BlendState.Enabled)
          .setSrcColorBlendFactor(ConvertBlendFactor(renderState.BlendState.Src))
          .setDstColorBlendFactor(ConvertBlendFactor(renderState.BlendState.Dst))
          .setColorBlendOp(ConvertBlendOp(renderState.BlendState.ColorBlend))
          .setSrcAlphaBlendFactor(ConvertBlendFactor(renderState.BlendState.SrcAlpha))
          .setDstAlphaBlendFactor(ConvertBlendFactor(renderState.BlendState.DstAlpha))
          .setAlphaBlendOp(ConvertBlendOp(renderState.BlendState.AlphaBlend))
          .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                             vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

  auto blendInfo = vk::PipelineColorBlendStateCreateInfo()
                       .setAttachmentCount(1)
                       .setPAttachments(&colorBlendAttachement)
                       .setBlendConstants(renderState.BlendState.BlendConstants);

  auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
                          .setStageCount((uint32_t)shaderStages.size())
                          .setPStages(shaderStages.data())
                          .setPVertexInputState(&vertexInputInfo)
                          .setPInputAssemblyState(&inputAssembly)
                          .setPRasterizationState(&rasterizationInfo)
                          .setPMultisampleState(&multisampleInfo)
                          .setPColorBlendState(&blendInfo)
                          .setLayout(reinterpret_cast<VkPipelineLayout>(graphics.pipelineLayout))
                          .setRenderPass(reinterpret_cast<VkRenderPass>(renderState.RenderPass))
                          .setPViewportState(&viewPortState);

  GraphicsPipelineCache pipeline = {
      renderState,
      graphics,
      {mDevice->createGraphicsPipelineUnique(*mPipelineCache, pipelineInfo)}};
  mGraphicsPipelines.push_back(std::move(pipeline));

  VkPipeline handle = *mGraphicsPipelines.back().Pipeline;
  return reinterpret_cast<Handle::Pipeline>(handle);
}

Handle::Pipeline VulkanDevice::CreateComputePipeline(Handle::ShaderModule shader,
                                                     Handle::PipelineLayout layout,
                                                     SpecConstInfo specConstInfo)
{
  vk::ShaderModule shaderModule = reinterpret_cast<VkShaderModule>(shader);
  vk::PipelineLayout pipelineLayout = reinterpret_cast<VkPipelineLayout>(layout);

  auto it = std::find_if(mComputePipelines.begin(),
                         mComputePipelines.end(),
                         [&](const ComputePipelineCache& pipeline)
                         {
                           return pipeline.Shader == shaderModule &&
                                  pipeline.Layout == pipelineLayout &&
                                  pipeline.SpecConst == specConstInfo;
                         });

  if (it != mComputePipelines.end())
  {
    VkPipeline handle = *it->Pipeline;
    return reinterpret_cast<Handle::Pipeline>(handle);
  }

  static_assert(sizeof(vk::SpecializationMapEntry) == sizeof(SpecConstInfo::Entry),
                "Incorrect sized spec const info");

  auto specInfo = vk::SpecializationInfo()
                      .setMapEntryCount(static_cast<uint32_t>(specConstInfo.mapEntries.size()))
                      .setPMapEntries(reinterpret_cast<vk::SpecializationMapEntry*>(
                          specConstInfo.mapEntries.data()))
                      .setDataSize(specConstInfo.data.size())
                      .setPData(specConstInfo.data.data());

  auto stageInfo = vk::PipelineShaderStageCreateInfo()
                       .setModule(shaderModule)
                       .setPName("main")
                       .setStage(vk::ShaderStageFlagBits::eCompute)
                       .setPSpecializationInfo(&specInfo);

  auto pipelineInfo = vk::ComputePipelineCreateInfo().setStage(stageInfo).setLayout(
      reinterpret_cast<VkPipelineLayout>(layout));

  mComputePipelines.push_back(
      {shaderModule,
       pipelineLayout,
       specConstInfo,
       mDevice->createComputePipelineUnique(*mPipelineCache, pipelineInfo)});

  VkPipeline handle = *mComputePipelines.back().Pipeline;
  return reinterpret_cast<Handle::Pipeline>(handle);
}

vk::UniqueCommandBuffer VulkanDevice::CreateCommandBuffer() const
{
  auto commandBufferInfo = vk::CommandBufferAllocateInfo()
                               .setCommandBufferCount(1)
                               .setCommandPool(*mCommandPool)
                               .setLevel(vk::CommandBufferLevel::ePrimary);

  return std::move(mDevice->allocateCommandBuffersUnique(commandBufferInfo).at(0));
}

vk::UniqueDescriptorSet VulkanDevice::CreateDescriptorSet(vk::DescriptorSetLayout layout) const
{
  vk::DescriptorSetLayout descriptorSetlayouts[] = {layout};

  auto descriptorSetInfo = vk::DescriptorSetAllocateInfo()
                               .setDescriptorPool(*mDescriptorPool)
                               .setDescriptorSetCount(1)
                               .setPSetLayouts(descriptorSetlayouts);

  return std::move(mDevice->allocateDescriptorSetsUnique(descriptorSetInfo).at(0));
}

}  // namespace Renderer
}  // namespace Vortex
