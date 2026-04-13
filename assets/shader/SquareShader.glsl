#Type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
out vec3 v_Position;
out vec4 v_Color;

uniform mat4 u_Transform;
uniform mat4 u_ProjectionViewMatrix;

void main(){

    gl_Position = u_ProjectionViewMatrix * u_Transform * vec4(a_Position, 1.0f);
    v_Position = gl_Position.xyz;
    v_Color = vec4(a_Position + vec3(0.5f, 0.5f, 0.0f), 1.0f);

}


#Type fragment
#version 460 core

in vec3 v_Position;
in vec4 v_Color;
out vec4 color;

void main(){

    color = v_Color;

}