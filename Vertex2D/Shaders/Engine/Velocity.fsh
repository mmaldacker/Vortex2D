precision highp float;

uniform float ang;
uniform vec2 center;
uniform vec2 vel;

void main()
{
    vec2 r = gl_FragCoord.xy - center;
    vec2 v = vel + ang * vec2(r.y, -r.x);
    
    gl_FragColor = vec4(v,0.0,0.0);
}