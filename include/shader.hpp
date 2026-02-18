#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <vector>

class Shader
{
private:
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
  VkPipelineShaderStageCreateInfo vertShaderStageInfo;
  VkPipelineShaderStageCreateInfo fragShaderStageInfo;
  VkDevice device;

  VkShaderModule createShaderModule(const std::vector<char>& code);

public:
  Shader(VkDevice device, const std::string& vertPath, const std::string& fragPath);
  ~Shader();

  VkPipelineShaderStageCreateInfo getVertStageInfo() const { return vertShaderStageInfo; }
  VkPipelineShaderStageCreateInfo getFragStageInfo() const { return fragShaderStageInfo; }
};

#endif
