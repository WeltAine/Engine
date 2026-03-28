#Type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;

out vec3 v_Position;
out vec2 v_UV;

uniform mat4 u_ProjectionViewMatrix;


layout(std140, binding = 1) uniform TransformBlock{
    mat4 t_Position;
};

void main(){
    v_Position = a_Position;
    gl_Position = u_ProjectionViewMatrix * t_Position * vec4(a_Position, 1.0f);

    v_UV = a_UV;
}



#Type fragment
#version 460 core
in vec3 v_Position;
in vec2 v_UV;

out vec4 color;

uniform vec3 colorOffset;
uniform sampler2D ourTexture;

void main(){
    color = texture(ourTexture, v_UV);
}
