#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 inLocalPos;
layout(location = 1) in vec3 inWorldPos;
layout(location = 2) in vec3 inWorldNormal;
layout(location = 3) in vec3 inLocalNormal;
layout(location = 4) in vec2 inTexCoord;
layout(location = 5) flat in uint inObjIndex;

struct Object {
    mat4 model;
    uint textureIndex;
};

layout(binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec3 pos;
} cameraUBO;

layout(std430, binding = 1) readonly buffer SSBO {
    Object objects[];
} modelSSBO;

layout(binding = 2) uniform sampler2D textures[];

layout(location = 0) out vec4 outColor;

void main() {
    vec3 lightDir = normalize(vec3(0,0,1));
    vec3 lightCol = vec3(1,1,1);

    uint texIndex = modelSSBO.objects[inObjIndex].textureIndex;
    vec4 texColor = texture(textures[nonuniformEXT(texIndex)], inTexCoord);

    vec3 worldNormal = normalize(inWorldNormal);

    float ambient = 0.01;
    float diffuse = max(dot( worldNormal, lightDir), 0.0);

    vec3 viewDir = normalize(cameraUBO.pos - inWorldPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float shinyness = 128;
    float specular = pow(max(dot(worldNormal, halfDir), 0.0), shinyness);

    vec3 ambientCol = texColor.rgb * ambient;
    vec3 diffuseCol = texColor.rgb * diffuse * lightCol;
    vec3 specularCol = texColor.rgb * specular;

    vec3 result = ambientCol + diffuseCol + specularCol;

    outColor = vec4(result, texColor.a);
}