/*
  yagears                  Yet Another Gears OpenGL / Vulkan demo
  Copyright (C) 2013-2020  Nicolas Caramelli

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <vulkan/vulkan.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vulkan_gears.h"

static void identity(float *a)
{
  float m[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
  };

  memcpy(a, m, sizeof(m));
}

static void multiply(float *a, const float *b)
{
  float m[16];
  int i, j;
  div_t d;

  for (i = 0; i < 16; i++) {
    m[i] = 0;
    d = div(i, 4);
    for (j = 0; j < 4; j++)
      m[i] += (a + d.rem)[j * 4] * (b + d.quot * 4)[j];
  }

  memcpy(a, m, sizeof(m));
}

static void translate(float *a, float tx, float ty, float tz)
{
  float m[16] = {
     1,  0,  0, 0,
     0,  1,  0, 0,
     0,  0,  1, 0,
    tx, ty, tz, 1
  };

  multiply(a, m);
}

static void rotate(float *a, float r, float ux, float uy, float uz)
{
  float s, c;

  sincosf(r * M_PI / 180, &s, &c);

  float m[16] = {
         ux * ux * (1 - c) + c, uy * ux * (1 - c) - uz * s, ux * uz * (1 - c) - uy * s, 0,
    ux * uy * (1 - c) + uz * s,      uy * uy * (1 - c) + c, uy * uz * (1 - c) - ux * s, 0,
    ux * uz * (1 - c) + uy * s, uy * uz * (1 - c) + ux * s,      uz * uz * (1 - c) + c, 0,
                             0,                          0,                          0, 1
  };

  multiply(a, m);
}

static void transpose(float *a)
{
  float m[16] = {
    a[0], a[4], a[8],  a[12],
    a[1], a[5], a[9],  a[13],
    a[2], a[6], a[10], a[14],
    a[3], a[7], a[11], a[15]
  };

  memcpy(a, m, sizeof(m));
}

static void invert(float *a)
{
  float m[16] = {
         1,      0,      0, 0,
         0,      1,      0, 0,
         0,      0,      1, 0,
    -a[12], -a[13], -a[14], 1,
  };

  a[12] = a[13] = a[14] = 0;
  transpose(a);

  multiply(a, m);
}

/******************************************************************************/

typedef float Vertex[6];

typedef struct {
  int begin;
  int count;
} Strip;

struct Uniform {
  float LightPos[4];
  float ModelViewProjection[16];
  float NormalMatrix[16];
  float Color[4];
};

struct gear {
  int nvertices;
  Vertex *vertices;
  int nstrips;
  Strip *strips;
  VkBuffer vbo;
  VkDeviceMemory vboMemory;
  void *vbo_data;
  VkBuffer ubo;
  VkDeviceMemory uboMemory;
  void *ubo_data;
  VkDescriptorSet descriptorSet;
};

struct gears {
  VkImage colorImage;
  VkImage depthImage;
  VkDeviceMemory depthMemory;
  VkImageView imageView[2];
  VkFramebuffer framebuffer;
  VkRenderPass renderPass;
  VkDescriptorSetLayout descriptorSetLayout;
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  VkCommandPool commandPool;
  VkCommandBuffer commandBuffer;
  VkDescriptorPool descriptorPool;
  struct gear *gear1;
  struct gear *gear2;
  struct gear *gear3;
  float Projection[16];
};

static struct gear *create_gear(float inner, float outer, float width, int teeth, float tooth_depth, VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkPipelineLayout pipelineLayout, VkCommandBuffer commandBuffer)
{
  struct gear *gear;
  float r0, r1, r2, da, a1, ai, s[5], c[5];
  int i, j;
  float n[3];
  int k = 0;
  VkBufferCreateInfo bufferCreateInfo;
  VkMemoryAllocateInfo memoryAllocateInfo;
  VkDeviceSize offset = 0;
  VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
  VkWriteDescriptorSet writeDescriptorSet;
  VkDescriptorBufferInfo descriptorBufferInfo;

  gear = calloc(1, sizeof(struct gear));
  if (!gear) {
    printf("calloc gear failed\n");
    return NULL;
  }

  gear->nvertices = 0;
  gear->vertices = calloc(34 * teeth, sizeof(Vertex));
  if (!gear->vertices) {
    printf("calloc vertices failed\n");
    goto out;
  }

  gear->nstrips = 7 * teeth;
  gear->strips = calloc(gear->nstrips, sizeof(Strip));
  if (!gear->strips) {
    printf("calloc strips failed\n");
    goto out;
  }

  r0 = inner;
  r1 = outer - tooth_depth / 2;
  r2 = outer + tooth_depth / 2;
  a1 = 2 * M_PI / teeth;
  da = a1 / 4;

  #define normal(nx, ny, nz) \
    n[0] = nx; \
    n[1] = ny; \
    n[2] = nz;

  #define vertex(x, y, z) \
    gear->vertices[gear->nvertices][0] = x; \
    gear->vertices[gear->nvertices][1] = y; \
    gear->vertices[gear->nvertices][2] = z; \
    gear->vertices[gear->nvertices][3] = n[0]; \
    gear->vertices[gear->nvertices][4] = n[1]; \
    gear->vertices[gear->nvertices][5] = n[2]; \
    gear->nvertices++;

  for (i = 0; i < teeth; i++) {
    ai = i * a1;
    for (j = 0; j < 5; j++) {
      sincosf(ai + j * da, &s[j], &c[j]);
    }

    /* front face begin */
    gear->strips[k].begin = gear->nvertices;
    /* front face normal */
    normal(0, 0, 1);
    /* front face vertices */
    vertex(r2 * c[1], -r2 * s[1], width / 2);
    vertex(r2 * c[2], -r2 * s[2], width / 2);
    vertex(r1 * c[0], -r1 * s[0], width / 2);
    vertex(r1 * c[3], -r1 * s[3], width / 2);
    vertex(r0 * c[0], -r0 * s[0], width / 2);
    vertex(r1 * c[4], -r1 * s[4], width / 2);
    vertex(r0 * c[4], -r0 * s[4], width / 2);
    /* front face end */
    gear->strips[k].count = 7;
    k++;

    /* back face begin */
    gear->strips[k].begin = gear->nvertices;
    /* back face normal */
    normal(0, 0, -1);
    /* back face vertices */
    vertex(r2 * c[1], -r2 * s[1], -width / 2);
    vertex(r2 * c[2], -r2 * s[2], -width / 2);
    vertex(r1 * c[0], -r1 * s[0], -width / 2);
    vertex(r1 * c[3], -r1 * s[3], -width / 2);
    vertex(r0 * c[0], -r0 * s[0], -width / 2);
    vertex(r1 * c[4], -r1 * s[4], -width / 2);
    vertex(r0 * c[4], -r0 * s[4], -width / 2);
    /* back face end */
    gear->strips[k].count = 7;
    k++;

    /* first outward face begin */
    gear->strips[k].begin = gear->nvertices;
    /* first outward face normal */
    normal(r2 * s[1] - r1 * s[0], r2 * c[1] - r1 * c[0], 0);
    /* first outward face vertices */
    vertex(r1 * c[0], -r1 * s[0],  width / 2);
    vertex(r1 * c[0], -r1 * s[0], -width / 2);
    vertex(r2 * c[1], -r2 * s[1],  width / 2);
    vertex(r2 * c[1], -r2 * s[1], -width / 2);
    /* first outward face end */
    gear->strips[k].count = 4;
    k++;

    /* second outward face begin */
    gear->strips[k].begin = gear->nvertices;
    /* second outward face normal */
    normal(s[2] - s[1], c[2] - c[1], 0);
    /* second outward face vertices */
    vertex(r2 * c[1], -r2 * s[1],  width / 2);
    vertex(r2 * c[1], -r2 * s[1], -width / 2);
    vertex(r2 * c[2], -r2 * s[2],  width / 2);
    vertex(r2 * c[2], -r2 * s[2], -width / 2);
    /* second outward face end */
    gear->strips[k].count = 4;
    k++;

    /* third outward face begin */
    gear->strips[k].begin = gear->nvertices;
    /* third outward face normal */
    normal(r1 * s[3] - r2 * s[2], r1 * c[3] - r2 * c[2], 0);
    /* third outward face vertices */
    vertex(r2 * c[2], -r2 * s[2],  width / 2);
    vertex(r2 * c[2], -r2 * s[2], -width / 2);
    vertex(r1 * c[3], -r1 * s[3],  width / 2);
    vertex(r1 * c[3], -r1 * s[3], -width / 2);
    /* third outward face end */
    gear->strips[k].count = 4;
    k++;

    /* fourth outward face begin */
    gear->strips[k].begin = gear->nvertices;
    /* fourth outward face normal */
    normal(s[4] - s[3], c[4] - c[3], 0);
    /* fourth outward face vertices */
    vertex(r1 * c[3], -r1 * s[3],  width / 2);
    vertex(r1 * c[3], -r1 * s[3], -width / 2);
    vertex(r1 * c[4], -r1 * s[4],  width / 2);
    vertex(r1 * c[4], -r1 * s[4], -width / 2);
    /* fourth outward face end */
    gear->strips[k].count = 4;
    k++;

    /* inside face begin */
    gear->strips[k].begin = gear->nvertices;
    /* inside face normal */
    normal(s[0] - s[4], c[0] - c[4], 0);
    /* inside face vertices */
    vertex(r0 * c[0], -r0 * s[0],  width / 2);
    vertex(r0 * c[0], -r0 * s[0], -width / 2);
    vertex(r0 * c[4], -r0 * s[4],  width / 2);
    vertex(r0 * c[4], -r0 * s[4], -width / 2);
    /* inside face end */
    gear->strips[k].count = 4;
    k++;
  }

  /* vertex buffer object */

  memset(&bufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
  vkCreateBuffer(device, &bufferCreateInfo, NULL, &gear->vbo);
  memset(&memoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
  memoryAllocateInfo.allocationSize = gear->nvertices * sizeof(Vertex);
  vkAllocateMemory(device, &memoryAllocateInfo, NULL, &gear->vboMemory);
  vkMapMemory(device, gear->vboMemory, 0, gear->nvertices * sizeof(Vertex), 0, &gear->vbo_data);
  vkBindBufferMemory(device, gear->vbo, gear->vboMemory, 0);

  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &gear->vbo, &offset);

  memcpy(gear->vbo_data, gear->vertices, gear->nvertices * sizeof(Vertex));

  /* uniform buffer object */

  memset(&bufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
  vkCreateBuffer(device, &bufferCreateInfo, NULL, &gear->ubo);
  memset(&memoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
  memoryAllocateInfo.allocationSize = sizeof(struct Uniform);
  vkAllocateMemory(device, &memoryAllocateInfo, NULL, &gear->uboMemory);
  vkMapMemory(device, gear->uboMemory, 0, sizeof(struct Uniform), 0, &gear->ubo_data);
  vkBindBufferMemory(device, gear->ubo, gear->uboMemory, 0);

  memset(&descriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
  descriptorSetAllocateInfo.descriptorPool = descriptorPool;
  descriptorSetAllocateInfo.descriptorSetCount = 1;
  descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
  vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &gear->descriptorSet);

  memset(&writeDescriptorSet, 0, sizeof(VkWriteDescriptorSet));
  writeDescriptorSet.dstSet = gear->descriptorSet;
  writeDescriptorSet.descriptorCount = 1;
  writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  memset(&descriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
  descriptorBufferInfo.buffer = gear->ubo;
  writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
  vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &gear->descriptorSet, 0, NULL);

  for (k = 0; k < gear->nstrips; k++)
    vkCmdDraw(commandBuffer, gear->strips[k].count, 1, gear->strips[k].begin, 0);

  return gear;

out:
  if (gear->strips) {
    free(gear->strips);
  }
  if (gear->vertices) {
    free(gear->vertices);
  }
  free(gear);
  return NULL;
}

static void draw_gear(struct gear *gear, float model_tx, float model_ty, float model_rz, const float *color, float *View, float *Projection)
{
  const float pos[4] = { 5.0, -5.0, 10.0, 0.0 };
  float ModelView[16], ModelViewProjection[16];
  struct Uniform u;

  memcpy(u.LightPos, pos, sizeof(pos));

  memcpy(ModelView, View, sizeof(ModelView));

  translate(ModelView, model_tx, model_ty, 0);
  rotate(ModelView, model_rz, 0, 0, 1);

  memcpy(ModelViewProjection, Projection, sizeof(ModelViewProjection));
  multiply(ModelViewProjection, ModelView);
  memcpy(u.ModelViewProjection, ModelViewProjection, sizeof(ModelViewProjection));

  invert(ModelView);
  transpose(ModelView);
  memcpy(u.NormalMatrix, ModelView, sizeof(ModelView));

  memcpy(u.Color, color, sizeof(u.Color));

  memcpy(gear->ubo_data, &u, sizeof(struct Uniform));
}

static void delete_gear(struct gear *gear, VkDevice device, VkDescriptorPool descriptorPool)
{
  vkFreeDescriptorSets(device, descriptorPool, 1, &gear->descriptorSet);
  vkUnmapMemory(device, gear->uboMemory);
  vkFreeMemory(device, gear->uboMemory, NULL);
  vkDestroyBuffer(device, gear->ubo, NULL);
  vkUnmapMemory(device, gear->vboMemory);
  vkFreeMemory(device, gear->vboMemory, NULL);
  vkDestroyBuffer(device, gear->vbo, NULL);
  free(gear->strips);
  free(gear->vertices);

  free(gear);
}

/******************************************************************************/

gears_t *vk_gears_init(int win_width, int win_height, void *device, void *swapchain)
{
  gears_t *gears = NULL;
  const uint32_t vertShaderSource[] = {
    #include "vert.spv"
  };
  const uint32_t fragShaderSource[] = {
    #include "frag.spv"
  };
  uint32_t count = 1;
  VkImageCreateInfo imageCreateInfo;
  VkMemoryRequirements memoryRequirements;
  VkMemoryAllocateInfo memoryAllocateInfo;
  VkImageViewCreateInfo imageViewCreateInfo;
  VkFramebufferCreateInfo framebufferCreateInfo;
  VkRenderPassCreateInfo renderPassCreateInfo;
  VkAttachmentDescription attachmentDescription[2];
  VkSubpassDescription subpassDescription;
  VkAttachmentReference attachmentReference[2];
  VkShaderModuleCreateInfo shaderModuleCreateInfo;
  VkShaderModule vertShaderModule = VK_NULL_HANDLE;
  VkShaderModule fragShaderModule = VK_NULL_HANDLE;
  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
  VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
  VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
  VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo[2];
  VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo;
  VkVertexInputBindingDescription vertexInputBindingDescription;
  VkVertexInputAttributeDescription vertexInputAttributeDescription[2];
  VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo;
  VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo;
  VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo;
  VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo;
  VkDynamicState dynamicState[2];
  VkCommandPoolCreateInfo commandPoolCreateInfo;
  VkCommandBufferAllocateInfo commandBufferAllocateInfo;
  VkCommandBufferBeginInfo commandBufferBeginInfo;
  VkRenderPassBeginInfo renderPassBeginInfo;
  VkClearValue clearValue[2];
  VkRect2D scissor;
  VkViewport viewport;
  VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
  VkDescriptorPoolSize descriptorPoolSize;
  const float zNear = 5, zFar = 60;

  gears = calloc(1, sizeof(gears_t));
  if (!gears) {
    printf("calloc gears failed\n");
    return NULL;
  }

  /* color attachment */

  vkGetSwapchainImagesKHR(device, swapchain, &count, &gears->colorImage);

  /* depth attachment */

  memset(&imageCreateInfo, 0, sizeof(VkImageCreateInfo));
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
  imageCreateInfo.extent.width = win_width;
  imageCreateInfo.extent.height = win_height;
  imageCreateInfo.mipLevels = 1;
  imageCreateInfo.arrayLayers = 1;
  imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  vkCreateImage(device, &imageCreateInfo, 0, &gears->depthImage);

  memset(&memoryRequirements, 0, sizeof(VkMemoryRequirements));
  vkGetImageMemoryRequirements(device, gears->depthImage, &memoryRequirements);

  memset(&memoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
  memoryAllocateInfo.allocationSize = memoryRequirements.size;
  vkAllocateMemory(device, &memoryAllocateInfo, NULL, &gears->depthMemory);
  vkBindImageMemory(device, gears->depthImage, gears->depthMemory, 0);

  /* create framebuffer */

  memset(&imageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
  imageViewCreateInfo.image = gears->colorImage;
  imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
  imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageViewCreateInfo.subresourceRange.levelCount = 1;
  imageViewCreateInfo.subresourceRange.layerCount = 1;
  vkCreateImageView(device, &imageViewCreateInfo, NULL, &gears->imageView[0]);

  memset(&imageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
  imageViewCreateInfo.image = gears->depthImage;
  imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
  imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  imageViewCreateInfo.subresourceRange.levelCount = 1;
  imageViewCreateInfo.subresourceRange.layerCount = 1;
  vkCreateImageView(device, &imageViewCreateInfo, 0, &gears->imageView[1]);

  memset(&framebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));
  framebufferCreateInfo.attachmentCount = 2;
  framebufferCreateInfo.pAttachments = gears->imageView;
  framebufferCreateInfo.width = win_width;
  framebufferCreateInfo.height = win_height;
  vkCreateFramebuffer(device, &framebufferCreateInfo, NULL, &gears->framebuffer);

  /* create render pass */

  memset(&renderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));
  renderPassCreateInfo.attachmentCount = 2;
  memset(&attachmentDescription[0], 0, sizeof(VkAttachmentDescription));
  attachmentDescription[0].format = VK_FORMAT_B8G8R8A8_UNORM;
  attachmentDescription[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  memset(&attachmentDescription[1], 0, sizeof(VkAttachmentDescription));
  attachmentDescription[1].format = VK_FORMAT_D32_SFLOAT_S8_UINT;
  attachmentDescription[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  renderPassCreateInfo.pAttachments = attachmentDescription;
  renderPassCreateInfo.subpassCount = 1;
  memset(&subpassDescription, 0, sizeof(VkSubpassDescription));
  subpassDescription.colorAttachmentCount = 1;
  memset(&attachmentReference[0], 0, sizeof(VkAttachmentReference));
  attachmentReference[0].attachment = 0;
  subpassDescription.pColorAttachments = &attachmentReference[0];
  memset(&attachmentReference[1], 0, sizeof(VkAttachmentReference));
  attachmentReference[1].attachment = 1;
  subpassDescription.pDepthStencilAttachment = &attachmentReference[1];
  renderPassCreateInfo.pSubpasses = &subpassDescription;
  vkCreateRenderPass(device, &renderPassCreateInfo, NULL, &gears->renderPass);

  /* vertex shader */

  memset(&shaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
  shaderModuleCreateInfo.codeSize = sizeof(vertShaderSource);
  shaderModuleCreateInfo.pCode = vertShaderSource;
  vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, &vertShaderModule);

  /* fragment shader */

  memset(&shaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
  shaderModuleCreateInfo.codeSize = sizeof(fragShaderSource);
  shaderModuleCreateInfo.pCode = fragShaderSource;
  vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, &fragShaderModule);

  /* create pipeline */

  memset(&descriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
  descriptorSetLayoutCreateInfo.bindingCount = 1;
  memset(&descriptorSetLayoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
  descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorSetLayoutBinding.descriptorCount = 1;
  descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;
  vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &gears->descriptorSetLayout);

  memset(&pipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
  pipelineLayoutCreateInfo.setLayoutCount = 1;
  pipelineLayoutCreateInfo.pSetLayouts = &gears->descriptorSetLayout;
  vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &gears->pipelineLayout);

  memset(&graphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
  graphicsPipelineCreateInfo.stageCount = 2;
  memset(&pipelineShaderStageCreateInfo[0], 0, sizeof(VkPipelineShaderStageCreateInfo));
  pipelineShaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  pipelineShaderStageCreateInfo[0].module = vertShaderModule;
  pipelineShaderStageCreateInfo[0].pName = "main";
  memset(&pipelineShaderStageCreateInfo[1], 0, sizeof(VkPipelineShaderStageCreateInfo));
  pipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  pipelineShaderStageCreateInfo[1].module = fragShaderModule;
  pipelineShaderStageCreateInfo[1].pName = "main";
  graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfo;
  memset(&pipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
  pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
  memset(&vertexInputBindingDescription, 0, sizeof(VkVertexInputBindingDescription));
  vertexInputBindingDescription.stride = sizeof(Vertex);
  pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
  pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 2;
  memset(&vertexInputAttributeDescription[0], 0, sizeof(VkVertexInputAttributeDescription));
  vertexInputAttributeDescription[0].location = 0;
  vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  vertexInputAttributeDescription[0].offset = 0;
  memset(&vertexInputAttributeDescription[1], 0, sizeof(VkVertexInputAttributeDescription));
  vertexInputAttributeDescription[1].location = 1;
  vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  vertexInputAttributeDescription[1].offset = sizeof(float) * 3;
  pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription;
  graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
  memset(&pipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
  pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
  memset(&pipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
  graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
  memset(&pipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
  pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
  pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
  pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
  memset(&pipelineDynamicStateCreateInfo, 0, sizeof(VkPipelineDynamicStateCreateInfo));
  pipelineDynamicStateCreateInfo.dynamicStateCount = 2;
  dynamicState[0] = VK_DYNAMIC_STATE_VIEWPORT;
  dynamicState[1] = VK_DYNAMIC_STATE_SCISSOR;
  pipelineDynamicStateCreateInfo.pDynamicStates = dynamicState;
  graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
  graphicsPipelineCreateInfo.layout = gears->pipelineLayout;
  vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, NULL, &gears->pipeline);

  /* destory shaders */

  vkDestroyShaderModule(device, fragShaderModule, NULL);
  vkDestroyShaderModule(device, vertShaderModule, NULL);
  vertShaderModule = fragShaderModule = VK_NULL_HANDLE;

  /* command buffer submitted to queue, set clear values, set viewport */

  memset(&commandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));
  vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &gears->commandPool);

  memset(&commandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
  commandBufferAllocateInfo.commandPool = gears->commandPool;
  commandBufferAllocateInfo.commandBufferCount = 1;
  vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &gears->commandBuffer);

  memset(&commandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
  vkBeginCommandBuffer(gears->commandBuffer, &commandBufferBeginInfo);

  memset(&renderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));
  renderPassBeginInfo.renderPass = gears->renderPass;
  renderPassBeginInfo.framebuffer = gears->framebuffer;
  renderPassBeginInfo.renderArea.extent.width = win_width;
  renderPassBeginInfo.renderArea.extent.height = win_height;
  renderPassBeginInfo.clearValueCount = 2;
  memset(&clearValue[0], 0, sizeof(VkClearValue));
  memset(&clearValue[1], 0, sizeof(VkClearValue));
  clearValue[1].depthStencil.depth = 1;
  renderPassBeginInfo.pClearValues = clearValue;
  vkCmdBeginRenderPass(gears->commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(gears->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gears->pipeline);

  memset(&scissor, 0, sizeof(VkRect2D));
  scissor.extent.width = win_width;
  scissor.extent.height = win_height;
  vkCmdSetScissor(gears->commandBuffer, 0, 1, &scissor);

  memset(&viewport, 0, sizeof(VkViewport));
  viewport.width = win_width;
  viewport.height = win_height;
  viewport.maxDepth = 1;
  vkCmdSetViewport(gears->commandBuffer, 0, 1, &viewport);

  memset(&descriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
  descriptorPoolCreateInfo.maxSets = 3;
  descriptorPoolCreateInfo.poolSizeCount = 1;
  memset(&descriptorPoolSize, 0, sizeof(VkDescriptorPoolSize));
  descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorPoolSize.descriptorCount = 3;
  descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
  vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL, &gears->descriptorPool);

  /* create gears */

  gears->gear1 = create_gear(1.0, 4.0, 1.0, 20, 0.7, device, gears->descriptorPool, gears->descriptorSetLayout, gears->pipelineLayout, gears->commandBuffer);
  if (!gears->gear1) {
    goto out;
  }

  gears->gear2 = create_gear(0.5, 2.0, 2.0, 10, 0.7, device, gears->descriptorPool, gears->descriptorSetLayout, gears->pipelineLayout, gears->commandBuffer);
  if (!gears->gear2) {
    goto out;
  }

  gears->gear3 = create_gear(1.3, 2.0, 0.5, 10, 0.7, device, gears->descriptorPool, gears->descriptorSetLayout, gears->pipelineLayout, gears->commandBuffer);
  if (!gears->gear3) {
    goto out;
  }

  vkCmdEndRenderPass(gears->commandBuffer);

  vkEndCommandBuffer(gears->commandBuffer);

  memset(gears->Projection, 0, sizeof(gears->Projection));
  gears->Projection[0] = zNear;
  gears->Projection[5] = (float)win_width/win_height * zNear;
  gears->Projection[10] = -(zFar + zNear) / (zFar - zNear);
  gears->Projection[11] = -1;
  gears->Projection[14] = -2 * zFar * zNear / (zFar - zNear);

  return gears;

out:
  if (gears->gear3) {
    delete_gear(gears->gear3, device, gears->descriptorPool);
  }
  if (gears->gear2) {
    delete_gear(gears->gear2, device, gears->descriptorPool);
  }
  if (gears->gear1) {
    delete_gear(gears->gear1, device, gears->descriptorPool);
  }
  free(gears);
  return NULL;
}

void vk_gears_draw(gears_t *gears, float view_tz, float view_rx, float view_ry, float model_rz, void *queue)
{
  const float red[4] = { 0.8, 0.1, 0.0, 1.0 };
  const float green[4] = { 0.0, 0.8, 0.2, 1.0 };
  const float blue[4] = { 0.2, 0.2, 1.0, 1.0 };
  float View[16];
  VkSubmitInfo submitInfo;

  if (!gears) {
    return;
  }

  identity(View);
  translate(View, 0, 0, view_tz);
  rotate(View, view_rx, 1, 0, 0);
  rotate(View, view_ry, 0, 1, 0);

  draw_gear(gears->gear1, -3.0,  2.0,      model_rz     , red  , View, gears->Projection);
  draw_gear(gears->gear2,  3.1,  2.0, -2 * model_rz - 9 , green, View, gears->Projection);
  draw_gear(gears->gear3, -3.1, -4.2, -2 * model_rz - 25, blue , View, gears->Projection);

  memset(&submitInfo, 0, sizeof(VkSubmitInfo));
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &gears->commandBuffer;
  vkQueueSubmit(queue, 1, &submitInfo, NULL);
}

void vk_gears_term(gears_t *gears, void *device)
{
  if (!gears) {
    return;
  }

  delete_gear(gears->gear1, device, gears->descriptorPool);
  delete_gear(gears->gear2, device, gears->descriptorPool);
  delete_gear(gears->gear3, device, gears->descriptorPool);
  vkDestroyDescriptorPool(device, gears->descriptorPool, NULL);
  vkFreeCommandBuffers(device, gears->commandPool, 1, &gears->commandBuffer);
  vkDestroyCommandPool(device, gears->commandPool, NULL);
  vkDestroyPipeline(device, gears->pipeline, NULL);
  vkDestroyPipelineLayout(device, gears->pipelineLayout, NULL);
  vkDestroyDescriptorSetLayout(device, gears->descriptorSetLayout, NULL);
  vkDestroyRenderPass(device, gears->renderPass, NULL);
  vkDestroyFramebuffer(device, gears->framebuffer, NULL);
  vkDestroyImageView(device, gears->imageView[1], NULL);
  vkDestroyImageView(device, gears->imageView[0], NULL);
  vkFreeMemory(device, gears->depthMemory, NULL);
  vkDestroyImage(device, gears->depthImage, NULL);

  free(gears);
}
