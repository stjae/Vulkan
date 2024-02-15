layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
} camera;

layout(set = 0, binding = 1) uniform Light {
    mat4 model;
    vec3 pos;
} light;

layout(set = 1, binding = 0) uniform Mesh {
    mat4 model;
    int id;
    int textureId;
    bool useTexture;
} mesh;

float Lambert(vec3 worldNormal, vec3 worldModel)
{
    vec4 lightPos = light.model * vec4(light.pos, 1.0);
    vec3 L = normalize(lightPos.xyz - worldModel);
    vec3 N = normalize(worldNormal);

    return max(0.0, dot(L, N));
}
