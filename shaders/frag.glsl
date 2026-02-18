#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject
{
  mat4 model;
  mat4 view;
  mat4 proj;
  vec3 lightPos;
  vec3 viewPos;
  vec3 lightColor;
} ubo;

void main()
{
  // Ambient
  float ambientStrength = 0.1;
  vec3 ambient = ambientStrength * ubo.lightColor;

  // Diffuse
  vec3 norm = normalize(fragNormal);
  vec3 lightDir = normalize(ubo.lightPos - fragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * ubo.lightColor;

  // Specular
  float specularStrength = 0.5;
  vec3 viewDir = normalize(ubo.viewPos - fragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
  vec3 specular = specularStrength * spec * ubo.lightColor;

  vec3 result = (ambient + diffuse + specular) * fragColor;
  outColor = vec4(result, 1.0);
}
