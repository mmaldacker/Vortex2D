//
//  Instance.cpp
//  Vortex2D
//

#include "Instance.h"

#include <iostream>

namespace Vortex2D
{
namespace Renderer
{
void RequestAdapterCallback(WGPUAdapterId received, void* userdata)
{
  auto promise = reinterpret_cast<std::promise<WGPUAdapterId>*>(userdata);
  promise->set_value(received);
}

void WGPULogCallback(int level, const char* msg)
{
  std::cout << "[" << level << "]" << msg << std::endl;
}

Instance::Instance() : mAdapter(0)
{
  wgpu_set_log_callback(&WGPULogCallback);
  wgpu_set_log_level(WGPULogLevel_Debug);

  std::promise<WGPUAdapterId> adapterPromise;

  wgpu_request_adapter_async(
      nullptr, 2 | 4 | 8, false, &RequestAdapterCallback, (void*)&adapterPromise);

  mAdapter = adapterPromise.get_future().get();
}

Instance::~Instance()
{
  if (mAdapter != 0)
  {
    wgpu_adapter_destroy(mAdapter);
  }
}

WGPUAdapterId Instance::GetAdapter() const
{
  return mAdapter;
}

}  // namespace Renderer
}  // namespace Vortex2D
