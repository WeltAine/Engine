#Type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;

out vec2 v_UV;

uniform mat4 u_Transform;
uniform mat4 u_ProjectionViewMatrix;



void main(){
    gl_Position = u_ProjectionViewMatrix * u_Transform * vec4(a_Position, 1.0f);
    v_UV = a_UV;
}



#Type fragment
#version 460 core

in vec2 v_UV;

out vec4 color;

layout(location = 0) uniform sampler2D ourTexture;

void main(){
    color = texture(ourTexture, v_UV);
}
