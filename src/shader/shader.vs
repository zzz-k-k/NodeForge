#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;


out vec3 ourColor;
out vec2 TexCoord;

out vec3 FragPos;
out vec3 Normal;

out vec4 FragPosLightSpace;

out mat3 TBN;

uniform float u_Tiling;
uniform mat4 transform;
uniform mat4 projection;
uniform mat4 view;

//阴影
uniform mat4 lightSpaceMatrix;

void main()
{
    //创建TBN矩阵
    vec3 T=normalize(vec3(transform*vec4(tangent,0.0)));
    vec3 B=normalize(vec3(transform*vec4(bitangent,0.0)));
    vec3 N=normalize(vec3(transform*vec4(aNormal,0.0)));
    //mat3 TBN=mat3(T,B,N);
    //求逆矩阵，世界空间到切线空间
    TBN=transpose(mat3(T,B,N));

    gl_Position = projection*view*transform*vec4(aPos, 1.0);
    TexCoord=aTexCoord;
    FragPos=vec3(transform*vec4(aPos,1.0));
    FragPosLightSpace=lightSpaceMatrix*vec4(FragPos,1.0);
    Normal = mat3(transpose(inverse(transform))) * aNormal;
};