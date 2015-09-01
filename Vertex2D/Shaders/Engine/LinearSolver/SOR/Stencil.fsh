#version 150

precision highp float;

void main() 
{
    if(mod(gl_FragCoord.x + gl_FragCoord.y,2.0) == 1.0)
    {
        discard;
    }
}