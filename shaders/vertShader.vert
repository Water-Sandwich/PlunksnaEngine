#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

struct Object {
    mat4 model;
    uint textureIndex;
};

layout(binding = 0) readonly uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec3 pos;
} cameraUBO;

layout(std430, binding = 1) readonly buffer SSBO {
    Object objects[];
} modelSSBO;

layout(push_constant) uniform PushConstant{
    uint instanceIndex;
} constants;

layout(location = 0) out vec3 outLocalPos;
layout(location = 1) out vec3 outWorldPos;
layout(location = 2) out vec3 outWorldNormal;
layout(location = 3) out vec3 outLocalNormal;
layout(location = 4) out vec2 outTexCoord;
layout(location = 5) flat out uint outObjIndex;

void main() {
    uint index = constants.instanceIndex + gl_InstanceIndex;
    gl_Position = cameraUBO.proj * cameraUBO.view * modelSSBO.objects[index].model * vec4(inPosition, 1);

    mat4 model = modelSSBO.objects[index].model;

    mat4 normalMat = transpose(inverse(model));
    vec4 normal = vec4(inNormal, 0.0f);
    outWorldNormal = (normalMat * normal).xyz;
    outLocalNormal = inNormal;

    outWorldPos = vec4(model * vec4(inPosition, 1.0)).xyz;
    outLocalPos = inPosition;

    outTexCoord = vec2(inTexCoord.x, 1.0 - inTexCoord.y);
    outObjIndex = index;
}

