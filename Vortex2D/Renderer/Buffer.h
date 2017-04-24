//
//  Buffer.h
//  Vortex2D
//

#ifndef Buffer_h
#define Buffer_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>

namespace Vortex2D { namespace Renderer {

template<typename T>
class VertexBuffer
{
public:
    VertexBuffer(const Device& device, const std::vector<T>& vertexBuffer);

private:
    vk::UniqueBuffer mBuffer;
    vk::UniqueDeviceMemory mBufferMemory;
};


}}

#endif
