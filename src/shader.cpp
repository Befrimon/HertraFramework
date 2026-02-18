#include "shader.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>

static std::vector<char> readFile(const std::string& filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
    throw std::runtime_error("Failed to open file: " + filename);
  size_t fileSize = (size_t)file.tellg();
  if (fileSize == 0)
    throw std::runtime_error("Shader file is empty: " + filename);

  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  std::cout << "Loaded shader: " << filename << " (" << fileSize << " bytes)" << std::endl;

  // Проверка magic number SPIR-V
  if (fileSize < 4)
    throw std::runtime_error("Shader file too small: " + filename);

  uint32_t magic = *reinterpret_cast<uint32_t*>(buffer.data());
  if (magic != 0x07230203) {
    std::cerr << "WARNING: Invalid SPIR-V magic number in " << filename << std::endl;
    std::cerr << "Expected: 0x07230203, Got: 0x" << std::hex << magic << std::dec << std::endl;
  }

  return buffer;
}

Shader::Shader(VkDevice dev, const std::string& vertPath, const std::string& fragPath)
  : device(dev)
{
  auto vertCode = readFile(vertPath);
  auto fragCode = readFile(fragPath);

  std::cout << "Creating vertex shader module..." << std::endl;
  vertShaderModule = createShaderModule(vertCode);
  std::cout << "Vertex shader module created: " << vertShaderModule << std::endl;

  std::cout << "Creating fragment shader module..." << std::endl;
  fragShaderModule = createShaderModule(fragCode);
  std::cout << "Fragment shader module created: " << fragShaderModule << std::endl;

  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";
  vertShaderStageInfo.flags = 0;
  vertShaderStageInfo.pNext = nullptr;
  vertShaderStageInfo.pSpecializationInfo = nullptr;

  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";
  fragShaderStageInfo.flags = 0;
  fragShaderStageInfo.pNext = nullptr;
  fragShaderStageInfo.pSpecializationInfo = nullptr;

  std::cout << "Shader stages configured" << std::endl;
}

Shader::~Shader()
{
  if (fragShaderModule != VK_NULL_HANDLE)
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
  if (vertShaderModule != VK_NULL_HANDLE)
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

VkShaderModule Shader::createShaderModule(const std::vector<char>& code)
{
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    throw std::runtime_error("Failed to create shader module!");

  return shaderModule;
}
