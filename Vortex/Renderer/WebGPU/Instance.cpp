//
//  Instance.cpp
//  Vortex2D
//

#include "Instance.h"

#include <iostream>

namespace Vortex
{
namespace Renderer
{
void RequestAdapterCallback(WGPUAdapter received, void* userdata)
{
  auto promise = reinterpret_cast<std::promise<WGPUAdapter>*>(userdata);
  promise->set_value(received);
}

void LogCallback(WGPULogLevel level, const char* msg)
{
  std::cout << "[" << level << "]" << msg << std::endl;
}

Instance::Instance() : mAdapter(0)
{
  wgpuSetLogCallback(&LogCallback);
  wgpuSetLogLevel(WGPULogLevel_Debug);

  // TODO create instance
  // WGPUInstanceDescriptor descriptor{};
  // mInstance = wgpuCreateInstance(&descriptor);

  std::promise<WGPUAdapter> adapterPromise;

  WGPURequestAdapterOptions options{};
  wgpuInstanceRequestAdapter(nullptr, &options, &RequestAdapterCallback, (void*)&adapterPromise);

  mAdapter = adapterPromise.get_future().get();
}

Instance::~Instance()
{
  // TODO do we need to destroy mInstance?
  // TODO do we need to destroy mAdapter?
}

WGPUAdapter Instance::GetAdapter() const
{
  return mAdapter;
}

}  // namespace Renderer
}  // namespace Vortex
