#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} cameraUBO;

struct Object {
    mat4 model;
};

layout(std430, binding = 1) readonly buffer SSBO {
    Object objects[];
} modelSSBO;

layout(push_constant) uniform PushConstant{
    uint instanceIndex;
} constants;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    uint index = constants.instanceIndex + gl_InstanceIndex;
    gl_Position = cameraUBO.proj * cameraUBO.view * modelSSBO.objects[index].model * vec4(inPosition, 1);

    fragColor = inColor;
    fragTexCoord = vec2(inTexCoord.x, 1.0 - inTexCoord.y);
}


//vec4 roundNearest(in vec4 num, in float rnd){
//    vec4 modNum = mod(num, rnd);
//    return num - modNum;
//}
//
//float roundNearest(in float num, in float rnd){
//    float modNum = mod(num, rnd);
//    return num - modNum;
//}
