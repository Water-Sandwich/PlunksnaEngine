#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) flat in uint objectIndex;

struct Object {
    mat4 model;
    uint textureIndex;
};

layout(std430, binding = 1) readonly buffer SSBO {
    Object objects[];
} modelSSBO;

layout(binding = 2) uniform sampler2D textures[];

layout(location = 0) out vec4 outColor;

void main() {
    uint texIndex = modelSSBO.objects[objectIndex].textureIndex;
    outColor = texture(textures[nonuniformEXT(texIndex)], fragTexCoord);
}