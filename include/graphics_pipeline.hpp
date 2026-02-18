#ifndef GRAPHICS_PIPELINE_HPP
#define GRAPHICS_PIPELINE_HPP

#include "shader.hpp"
#include "vertex.hpp"
#include <vector>

class GraphicsPipeline
{
private:
    VkPipeline pipeline;
    VkDevice device;

public:
  GraphicsPipeline(
    VkDevice device, VkExtent2D extent, VkRenderPass renderPass, const Shader& shader, VkPipelineLayout layout
  );
  ~GraphicsPipeline();

  VkPipeline getPipeline() const { return pipeline; }
};

#endif
