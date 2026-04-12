#Type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;

out vec3 v_Position;

uniform mat4 u_ProjectionViewMatrix;


layout(std140, binding = 1) uniform TransformBlock{
    mat4 t_Transform;
};

void main(){
    v_Position = a_Position;
    gl_Position = u_ProjectionViewMatrix * t_Transform * vec4(a_Position, 1.0f);
}




#Type fragment
#version 460 core
in vec3 v_Position;

out vec4 color;

uniform vec3 colorOffset;

void main(){
    color = vec4(v_Position * 0.5f + 0.5f, 1.0f) + vec4(colorOffset, 1.0f);
}
