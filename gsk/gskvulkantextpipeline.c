#include "config.h"

#include "gskvulkantextpipelineprivate.h"

struct _GskVulkanTextPipeline
{
  GObject parent_instance;
};

typedef struct _GskVulkanTextInstance GskVulkanTextInstance;

struct _GskVulkanTextInstance
{
  float rect[4];
  float tex_rect[4];
  float color[4];
};

G_DEFINE_TYPE (GskVulkanTextPipeline, gsk_vulkan_text_pipeline, GSK_TYPE_VULKAN_PIPELINE)

static const VkPipelineVertexInputStateCreateInfo *
gsk_vulkan_text_pipeline_get_input_state_create_info (GskVulkanPipeline *self)
{
  static const VkVertexInputBindingDescription vertexBindingDescriptions[] = {
      {
          .binding = 0,
          .stride = sizeof (GskVulkanTextInstance),
          .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE
      }
  };
  static const VkVertexInputAttributeDescription vertexInputAttributeDescription[] = {
      {
          .location = 0,
          .binding = 0,
          .format = VK_FORMAT_R32G32B32A32_SFLOAT,
          .offset = G_STRUCT_OFFSET (GskVulkanTextInstance, rect),
      },
      {
          .location = 1,
          .binding = 0,
          .format = VK_FORMAT_R32G32B32A32_SFLOAT,
          .offset = G_STRUCT_OFFSET (GskVulkanTextInstance, tex_rect),
      },
      {
          .location = 2,
          .binding = 0,
          .format = VK_FORMAT_R32G32B32A32_SFLOAT,
          .offset = G_STRUCT_OFFSET (GskVulkanTextInstance, color),
      }
  };
  static const VkPipelineVertexInputStateCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = G_N_ELEMENTS (vertexBindingDescriptions),
      .pVertexBindingDescriptions = vertexBindingDescriptions,
      .vertexAttributeDescriptionCount = G_N_ELEMENTS (vertexInputAttributeDescription),
      .pVertexAttributeDescriptions = vertexInputAttributeDescription
  };

  return &info;
}

static void
gsk_vulkan_text_pipeline_finalize (GObject *gobject)
{
  //GskVulkanTextPipeline *self = GSK_VULKAN_TEXT_PIPELINE (gobject);

  G_OBJECT_CLASS (gsk_vulkan_text_pipeline_parent_class)->finalize (gobject);
}

static void
gsk_vulkan_text_pipeline_class_init (GskVulkanTextPipelineClass *klass)
{
  GskVulkanPipelineClass *pipeline_class = GSK_VULKAN_PIPELINE_CLASS (klass);

  G_OBJECT_CLASS (klass)->finalize = gsk_vulkan_text_pipeline_finalize;

  pipeline_class->get_input_state_create_info = gsk_vulkan_text_pipeline_get_input_state_create_info;
}

static void
gsk_vulkan_text_pipeline_init (GskVulkanTextPipeline *self)
{
}

GskVulkanPipeline *
gsk_vulkan_text_pipeline_new (GskVulkanPipelineLayout *layout,
                               const char              *shader_name,
                               VkRenderPass             render_pass)
{
  return gsk_vulkan_pipeline_new_full (GSK_TYPE_VULKAN_TEXT_PIPELINE, layout, shader_name, render_pass,
                                       VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
}

gsize
gsk_vulkan_text_pipeline_count_vertex_data (GskVulkanTextPipeline *pipeline,
                                            int                    num_instances)
{
  return sizeof (GskVulkanTextInstance) * num_instances;
}

void
gsk_vulkan_text_pipeline_collect_vertex_data (GskVulkanTextPipeline  *pipeline,
                                              guchar                 *data,
                                              GskVulkanRenderer      *renderer,
                                              const graphene_rect_t  *rect,
                                              PangoFont              *font,
                                              PangoGlyphString       *glyphs,
                                              const GdkRGBA          *color,
                                              float                   x,
                                              float                   y)
{
  GskVulkanTextInstance *instances = (GskVulkanTextInstance *) data;
  int i, count;
  int x_position = 0;
  float dx, dy, dw, dh;

  count = 0;
  for (i = 0; i < glyphs->num_glyphs; i++)
    {
      PangoGlyphInfo *gi = &glyphs->glyphs[i];

      if (gi->glyph != PANGO_GLYPH_EMPTY)
        {
          double cx = (double)(x_position + gi->geometry.x_offset) / PANGO_SCALE;
          double cy = (double)(gi->geometry.y_offset) / PANGO_SCALE;

          if (!(gi->glyph & PANGO_GLYPH_UNKNOWN_FLAG))
            {
              GskVulkanTextInstance *instance = &instances[count];

              gsk_vulkan_renderer_get_glyph_coords (renderer, font, gi->glyph,
                                                    &instance->tex_rect[0],
                                                    &instance->tex_rect[1],
                                                    &instance->tex_rect[2],
                                                    &instance->tex_rect[3],
                                                    &dx, &dy, &dw, &dh);
              instance->rect[0] = x + cx + dx;
              instance->rect[1] = y + cy + dy;
              instance->rect[2] = dw;
              instance->rect[3] = dh;
              instance->color[0] = color->red;
              instance->color[1] = color->green;
              instance->color[2] = color->blue;
              instance->color[3] = color->alpha;

              count++;
            }
        }
      x_position += gi->geometry.width;
    }
}

gsize
gsk_vulkan_text_pipeline_draw (GskVulkanTextPipeline *pipeline,
                               VkCommandBuffer        command_buffer,
                               gsize                  offset,
                               gsize                  n_commands)
{
  vkCmdDraw (command_buffer,
             6, n_commands,
             0, offset);

  return n_commands;
}
