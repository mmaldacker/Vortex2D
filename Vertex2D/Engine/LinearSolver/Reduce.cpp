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

const char * ReduceVert = GLSL(
   in vec2 a_Position;
   in vec2 a_TexCoords;
   out vec2 v_texCoord;

   uniform mat4 u_Projection;
   uniform sampler2D u_texture;

   const vec2 off = vec2(0.5);

   void main()
   {
       gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);

       vec2 h = textureSize(u_texture, 0);
       vec2 k = ceil(h/vec2(2.0));

       v_texCoord = (vec2(2.0) * (a_TexCoords * k - off) + off) / h;
   }
);

const char * ReduceFrag = GLSL(
   in vec2 v_texCoord;
   out vec4 colour_out;
   
   uniform sampler2D u_texture;
   
   const vec4 q = vec4(1.0);
   
   void main()
   {
       vec4 p;
       p.x = texture(u_texture, v_texCoord).x;
       p.y = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
       p.z = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
       p.w = textureOffset(u_texture, v_texCoord, ivec2(1,1)).x;
       
       colour_out = vec4(dot(p,q), 0.0, 0.0, 0.0);
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
    : reduce(ReduceVert, ReduceFrag)
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