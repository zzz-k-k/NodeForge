#version 330 core
layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec2 aTexCoord;
//layout(location=2)in vec3 aNormal;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;


out vec3 ourColor;
out vec2 TexCoord;

out vec3 FragPos;
out vec3 Normal;

uniform float u_Tiling;
uniform mat4 transform;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    gl_Position = projection*view*transform*vec4(aPos, 1.0);
    TexCoord=aTexCoord;
    FragPos=vec3(transform*vec4(aPos,1.0));
    Normal = mat3(transpose(inverse(transform))) * aNormal;
};