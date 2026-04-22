#Type vertex
#version 460 core

layout(location = 0) in vec3 a_VertexPosition;
layout(location = 1) in vec2 a_VertexUV;

layout(location = 2) in vec3 a_Position;
layout(location = 3) in vec3 a_Rotation;
layout(location = 4) in vec3 a_Size;
layout(location = 5) in vec4 a_Color;
layout(location = 6) in vec2 a_TilingFactor;
layout(location = 7) in int a_TextureIndex;

out vec2 v_UV;
out vec4 v_Color;
out vec2 v_TilingFactor;
flat out int v_TextureIndex;

uniform mat4 u_ProjectionViewMatrix;



float Radians(float degrees){

    return degrees * 0.017453292519943295;

}

mat4 RotateZ(float angle){

    float s = sin(angle);
    float c = cos(angle);

    return mat4(
         c ,  s , 0.0, 0.0,
        -s ,  c , 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

}

mat4 RotateY(float angle){

    float s = sin(angle);
    float c = cos(angle);

    return mat4(
         c , 0.0,  s , 0.0,
        0.0, 1.0, 0.0, 0.0,
        -s , 0.0,  c , 0.0,
        0.0, 0.0, 0.0, 1.0
    );

}


mat4 RotateX(float angle){

    float s = sin(angle);
    float c = cos(angle);

    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0,  c ,  s , 0.0,
        0.0,  -s,  c , 0.0,
        0.0, 0.0, 0.0, 1.0
    );

}


mat4 Translate(vec3 v){

    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        v.x, v.y, v.z, 1.0
    );

}


mat4 Scale(vec3 v){

    return mat4(
        v.x, 0.0, 0.0, 0.0,
        0.0, v.y, 0.0, 0.0,
        0.0, 0.0, v.z, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

}


void main(){

    mat4 transform = mat4(1.0f);

    mat4 pitch = RotateX(Radians(a_Rotation.x));
    mat4 yaw = RotateY(Radians(a_Rotation.y));
    mat4 roll = RotateZ(Radians(a_Rotation.z));


    transform = Translate(a_Position) * pitch * yaw * roll * Scale(a_Size);


    gl_Position = u_ProjectionViewMatrix * transform * vec4(a_VertexPosition, 1.0f);

    v_UV = a_VertexUV;
    v_Color = a_Color;

    v_TilingFactor = a_TilingFactor;
    v_TextureIndex = a_TextureIndex;
}



#Type fragment
#version 460 core

in vec2 v_UV;
in vec4 v_Color;
in vec2 v_TilingFactor;
flat in int v_TextureIndex;

out vec4 color;

layout(location = 0) uniform sampler2D u_Textures[32];

void main(){
    color = texture(u_Textures[v_TextureIndex], v_UV * v_TilingFactor) * v_Color;
}
