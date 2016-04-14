//
//  Reduce.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 14/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#include "Reduce.h"

namespace Fluid
{

const char * ReduceFrag = GLSL(
   out vec4 colour_out;
   
   uniform sampler2D u_texture;
   
   void main()
   {
       ivec2 pos = 2 * ivec2(gl_FragCoord.xy - 0.5);

       float x = 0.0;
       x += texelFetch(u_texture, pos + ivec2(0,0), 0).x;
       x += texelFetch(u_texture, pos + ivec2(1,0), 0).x;
       x += texelFetch(u_texture, pos + ivec2(0,1), 0).x;
       x += texelFetch(u_texture, pos + ivec2(1,1), 0).x;

       colour_out = vec4(x, 0.0, 0.0, 0.0);
   }
);

const char * MultiplyFrag = GLSL(
     in vec2 v_texCoord;
     out vec4 colour_out;

     uniform sampler2D u_texture;
     uniform sampler2D u_other;

     void main()
     {
         float x = texture(u_texture, v_texCoord).x;
         float y = texture(u_other, v_texCoord).x;
         
         colour_out = vec4(x*y, 0.0, 0.0, 0.0);
     }
);

Reduce::Reduce(glm::vec2 size)
    : reduce(Renderer::Shader::PositionVert, ReduceFrag)
    , multiply(Renderer::Shader::TexturePositionVert, MultiplyFrag)
{
    while(size.x > 1.0f && size.y > 1.0f)
    {
        s.emplace_back(size, 1);
        size = glm::ceil(size/glm::vec2(2.0f));
    }

    reduce.Use().Set("u_texture", 0).Unuse();
    multiply.Use().Set("u_texture", 0).Set("u_other", 1).Unuse();
}

}