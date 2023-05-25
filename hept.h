#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include "hephaestus.h"

#include "c7h16.h"

//

/// OBJECT template

/*

//

/// name

OBJECT(
        name
        , // elements
        s32 x;
        s32 y;
        ptr(u8) data;
        , // parameters
        (in s32 in_x)
        , // initiate
        {
                name->x = in_x;
        }
)

//

*/

//

#define OBJECT(OBJ, STRUCT_VARIABLES, NEW_PARAMS, NEW_SET)                     \
                                                                               \
  fn pure $new_##OBJ();                                                        \
                                                                               \
  make_struct {                                                                \
    volatile spinlock lock;                                                    \
    volatile u32 pool_id;                                                      \
    volatile str object_name;                                                  \
    STRUCT_VARIABLES                                                           \
  }                                                                            \
  struct_##OBJ;                                                                \
  make_ptr(struct_##OBJ) OBJ;                                                  \
                                                                               \
  global OBJ current_##OBJ = null;                                             \
  global pool global_pool_##OBJ = null;                                        \
                                                                               \
  fn pure OBJ##_lock(in OBJ OBJ##_to_lock) {                                   \
    spinlock_engage(OBJ##_to_lock->lock);                                      \
  }                                                                            \
  fn pure OBJ##_unlock(in OBJ OBJ##_to_unlock) {                               \
    spinlock_vacate(OBJ##_to_unlock->lock);                                    \
  }                                                                            \
                                                                               \
  fn pure OBJ##_set_current(in OBJ set_this_##OBJ##_current) {                 \
    safe_ptr_set(current_##OBJ, set_this_##OBJ##_current);                     \
  }                                                                            \
  fn OBJ OBJ##_get_current() { out safe_ptr_get(current_##OBJ); }              \
                                                                               \
  fn OBJ new_##OBJ NEW_PARAMS {                                                \
    OBJ OBJECT_##OBJ = new_mem(struct_##OBJ, 1);                               \
    OBJECT_##OBJ->object_name = #OBJ;                                          \
    NEW_SET                                                                    \
    OBJ##_set_current(OBJECT_##OBJ);                                           \
    pool_lock(global_pool_##OBJ);                                              \
    pool_add(global_pool_##OBJ, OBJ, OBJECT_##OBJ);                            \
    safe_u32_set(OBJECT_##OBJ->pool_id, global_pool_##OBJ->pos_prev);          \
    pool_unlock(global_pool_##OBJ);                                            \
    out OBJECT_##OBJ;                                                          \
  }                                                                            \
                                                                               \
  fn pure delete_##OBJ(in OBJ delete_this_##OBJ) {                             \
    if (delete_this_##OBJ == OBJ##_get_current())                              \
      OBJ##_set_current(null);                                                 \
    pool_lock(global_pool_##OBJ);                                              \
    pool_free(global_pool_##OBJ, OBJ,                                          \
              safe_u32_get(delete_this_##OBJ->pool_id));                       \
    pool_unlock(global_pool_##OBJ);                                            \
    free(delete_this_##OBJ);                                                   \
  }

#define new(OBJ) new_##OBJ()

//

fn h_command_buffer begin_single_time_commands(in h_device device,
                                               in h_command_pool command_pool) {
  VkCommandBufferAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandPool = command_pool,
      .commandBufferCount = 1};

  h_command_buffer command_buffer;
  vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);

  VkCommandBufferBeginInfo begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

  vkBeginCommandBuffer(command_buffer, &begin_info);

  out command_buffer;
}

fn pure end_single_time_commands(in h_device device,
                                 in h_command_pool command_pool,
                                 in h_queue graphics_queue,
                                 in h_command_buffer command_buffer) {
  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                              .commandBufferCount = 1,
                              .pCommandBuffers = &command_buffer};

  vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphics_queue);

  vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

//

/// timer

fn u64 ns_now() {
  LARGE_INTEGER counter;
  LARGE_INTEGER frequency;

  QueryPerformanceCounter(&counter);
  QueryPerformanceFrequency(&frequency);

  out u64_(((double)counter.QuadPart / frequency.QuadPart) * 1000000000);
}

make_type(s64) timer;

#define new_timer() ns_now()

#define timer_get_sec(t) (f64_(ns_now() - t) / 1000000000.L)
#define timer_get_milli(t) (f64_(ns_now() - t) / 1000000.L)
#define timer_get_micro(t) (f64_(ns_now() - t) / 1000.L)
#define timer_get_nano(t) (f64_(ns_now() - t))

//

/// file

make_struct {
  str name;
  str data;
  u32 size;
}
os_file;

os_file file_load(in str file_name) {
#ifdef OS_WINDOWS
  HANDLE file = CreateFile(file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Failed to open file: %s\n", file_name);
  }

  DWORD file_size_low, file_size_high;
  file_size_low = GetFileSize(file, &file_size_high);

  HANDLE file_mapping =
      CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
  pure_ptr file_data = MapViewOfFile(file_mapping, FILE_MAP_READ, 0, 0, 0);
  CloseHandle(file_mapping);
  CloseHandle(file);

  out(os_file){file_name, file_data,
               file_size_low | ((size_t)file_size_high << 32)};
#elif OS_LINUX
  //
#elif OS_MACOS
  //
#endif
}

/*
make_struct {
  f32 x, y;
  f32 u, v;
} sprite_struct;

fn u32 find_memory_type(in VkPhysicalDevice physical_device, in uint32_t
type_filter, in VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

  iter(memory_properties.memoryTypeCount, i)
  {
        if((type_filter & (1 << i)) and
(memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
          out i;
        }
  }

  fprintf(stderr, "Failed to find suitable memory type\n");
  out null;
}

make_struct{
        h_buffer buff;
        h_device_mem mem;
        u32 type_size;
        u32 size;
} storage_struct;
make_ptr(storage_struct) storage;

fn storage new_storage( in h_device device, in h_physical_device
physical_device, in u32 data_size)
{
        storage temp_storage = new_mem(storage_struct, 1);

        // Create SSBO
        h_info_buffer bufferInfo = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = data_size,
                .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        if (vkCreateBuffer(device, &bufferInfo, NULL, adr(temp_storage->buff))
!= VK_SUCCESS) { printf("Failed to create SSBO\n"); exit(1);
        }

        // Allocate memory for SSBO
        h_mem_requirements memRequirements;
        vkGetBufferMemoryRequirements(device, temp_storage->buff,
&memRequirements);

        h_physical_device_memory_properties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

        h_info_mem_allocate allocInfo = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = memRequirements.size,
                .memoryTypeIndex = find_memory_type(physical_device,
memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
        };

        if (vkAllocateMemory(device, &allocInfo, NULL, &temp_storage->mem) !=
VK_SUCCESS) { printf("Failed to allocate SSBO memory\n"); exit(1);
        }

        vkBindBufferMemory(device, temp_storage->buff, temp_storage->mem, 0);

        out temp_storage;
}

make_struct{
        h_image img;
        h_device_mem mem;
        h_image_view view;
        h_sampler sampler;
        u32 width;
        u32 height;
} image_struct;
make_ptr(image_struct) image;

image new_image(h_device device, h_physical_device physical_device, u32 width,
u32 height) { image temp_image = (image)malloc(sizeof(image_struct));

        h_extent_3d temp_extent = {.width = width, .height = height, .depth = 1
}; h_info_image image_info = h_make_info_image( VK_IMAGE_TYPE_2D, temp_extent,
                1,
                1,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_SHARING_MODE_EXCLUSIVE,
                VK_SAMPLE_COUNT_1_BIT
        );

        if (vkCreateImage(device, &image_info, NULL, &(temp_image->img)) !=
VK_SUCCESS) { printf("Failed to create Image\n"); exit(1);
        }

        h_mem_requirements mem_requirements;
        vkGetImageMemoryRequirements(device, temp_image->img,
&mem_requirements);

        h_info_mem_allocate alloc_info = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = mem_requirements.size,
                .memoryTypeIndex = find_memory_type(physical_device,
mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
        };

        if (vkAllocateMemory(device, &alloc_info, NULL, &(temp_image->mem)) !=
VK_SUCCESS) { printf("Failed to allocate Image memory\n"); exit(1);
        }

        vkBindImageMemory(device, temp_image->img, temp_image->mem, 0);

        VkImageViewCreateInfo view_info = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = temp_image->img,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                }
        };

        if (vkCreateImageView(device, &view_info, NULL, &(temp_image->view)) !=
VK_SUCCESS) { printf("Failed to create Image View\n"); exit(1);
        }

        VkSamplerCreateInfo sampler_info = {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_NEAREST,
                .minFilter = VK_FILTER_NEAREST,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .mipLodBias = 0.0f,
                .anisotropyEnable = VK_FALSE,
                .maxAnisotropy = 1,
                .compareEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_ALWAYS,
                .minLod = 0.0f,
                .maxLod = 0.0f,
                .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                .unnormalizedCoordinates = VK_FALSE
        };

        if (vkCreateSampler(device, &sampler_info, NULL, &(temp_image->sampler))
!= VK_SUCCESS) { printf("Failed to create Image Sampler\n"); exit(1);
        }

        temp_image->width = width;
        temp_image->height = height;

        return temp_image;
}

void transition_image_layout(VkDevice device, VkCommandPool cmd_pool, h_queue
queue, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout) {
        h_command_buffer temp_cmd = begin_single_time_commands(device,cmd_pool);
        VkImageMemoryBarrier barrier = {0};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;

        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags src_stage;
        VkPipelineStageFlags dst_stage;

        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout ==
VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) { barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
                // Handle other layout transitions as needed
        }

        vkCmdPipelineBarrier(
                temp_cmd,
                src_stage, dst_stage,
                0,
                0, NULL,
                0, NULL,
                1, &barrier
        );
        end_single_time_commands(device,cmd_pool,queue,temp_cmd);
}

void scale_image_nearest_neighbor(
        VkDevice device,
        VkCommandPool commandPool,
        VkQueue graphicsQueue,
        VkImage sourceImage,
        VkImage dstImage,
        uint32_t srcWidth,
        uint32_t srcHeight,
        uint32_t dstWidth,
        uint32_t dstHeight
) {
        VkCommandBuffer commandBuffer = begin_single_time_commands(device,
commandPool);

        VkImageBlit blit = {0};
        blit.srcOffsets[0] = (VkOffset3D){0, 0, 0};
        blit.srcOffsets[1] = (VkOffset3D){srcWidth, srcHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = 0;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = (VkOffset3D){0, 0, 0};
        blit.dstOffsets[1] = (VkOffset3D){dstWidth, dstHeight, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = 0;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(
                commandBuffer,
                sourceImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_NEAREST
        );

        end_single_time_commands(device, commandPool, graphicsQueue,
commandBuffer);
}

void* load_file(const char* filename, size_t* size) {
        FILE* file = fopen(filename, "rb");
        if (!file) {
                return NULL;
        }

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        void* buffer = malloc(file_size);
        if (!buffer) {
                fclose(file);
                return NULL;
        }

        size_t read_size = fread(buffer, 1, file_size, file);
        fclose(file);

        if (read_size != file_size) {
                free(buffer);
                return NULL;
        }

        if (size) {
                *size = file_size;
        }

        return buffer;
}

VkShaderModule new_shader(VkDevice device, const char *path, shaderc_shader_kind
kind) { size_t s; const char *source = load_file(path, &s); VkShaderModule
shaderModule = null;

  // Initialize shaderc
  shaderc_compiler_t compiler = shaderc_compiler_initialize();
  shaderc_compile_options_t options = shaderc_compile_options_initialize();

  // Compile GLSL to SPIR-V
  shaderc_compilation_result_t result = shaderc_compile_into_spv( compiler,
source, s, kind, path, "main", options);

  if (shaderc_result_get_compilation_status(result) !=
          shaderc_compilation_status_success) {
        fprintf(stderr, "GLSL compilation error: %s\n",
                        shaderc_result_get_error_message(result));
  } else {
        const uint32_t *spirv = (const uint32_t
*)shaderc_result_get_bytes(result); const size_t spirv_len =
shaderc_result_get_length(result);

        // Use the SPIR-V bytecode to create the shader module
        VkShaderModuleCreateInfo createInfo = {
                VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        createInfo.codeSize = spirv_len;
        createInfo.pCode = spirv;

        if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) !=
VK_SUCCESS) { print("Failed to create shader module\n");
        }
  }

  // Release shaderc resources
  shaderc_result_release(result);
  shaderc_compile_options_release(options);
  shaderc_compiler_release(compiler);

  return shaderModule;
}

u32 find_debug_layers( in ptr(str) desired_layers, in u32 desired_count,
ptr(ptr(str)) out_layers )
{
        u32 debug_layer_count = 0;
        vkEnumerateInstanceLayerProperties(adr(debug_layer_count), null);
        ptr(h_layer_properties) available_layers = new_mem(h_layer_properties,
debug_layer_count); vkEnumerateInstanceLayerProperties(adr(debug_layer_count),
available_layers); val(out_layers) = new_mem(str, desired_count);
        //
        u32 enabled_count = 0;
        iter(desired_count, i)
        {
                iter(debug_layer_count, j)
                {
                        if(strcmp(desired_layers[i],
available_layers[j].layerName) == 0)
                        {
                                (val(out_layers))[enabled_count++] =
desired_layers[i]; break;
                        }
                }
        }
        free(available_layers);
        //
        out enabled_count;
}

//

h_physical_device_features get_physical_device_features(h_physical_device
in_physical_device)
{
        h_physical_device_features temp_physical_device_features;
        vkGetPhysicalDeviceFeatures(in_physical_device,
adr(temp_physical_device_features)); out temp_physical_device_features;
}

u32 find_physical_device(in h_instance in_instance, in h_surface in_surface,
ptr(h_physical_device) out_physical_device )
{
        val(out_physical_device) = null;
        u32 queue_family_index = u32_max;

        u32 physical_devices_count = 0;
        vkEnumeratePhysicalDevices(in_instance, adr(physical_devices_count),
null); ptr(h_physical_device) physical_devices = new_mem(h_physical_device,
physical_devices_count); vkEnumeratePhysicalDevices(in_instance,
adr(physical_devices_count), physical_devices);

        h_physical_device integrated = null;
        iter(physical_devices_count, i)
        {

                h_physical_device_properties dev_prop;
                vkGetPhysicalDeviceProperties(physical_devices[i],
adr(dev_prop));

                if (dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
or dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) { u32
queue_family_n; vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i],
adr(queue_family_n), null);

                        ptr(VkQueueFamilyProperties) queue_family_prop =
new_mem(VkQueueFamilyProperties, queue_family_n);

                        vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i],
adr(queue_family_n), queue_family_prop);

                        iter(queue_family_n, j)
                        {
                                queue_family_index = j;
                                VkBool32 support_present;
                                vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices[i],
j, in_surface, adr(support_present));

                                if ((queue_family_prop[j].queueFlags &
VK_QUEUE_GRAPHICS_BIT) and support_present) { if (dev_prop.deviceType ==
VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { val(out_physical_device) =
physical_devices[i];

                                                break;
                                        } else if (integrated == null) {
                                                integrated =
physical_devices[i];
                                        }
                                }
                        }
                        free(queue_family_prop);
                }

                if (val(out_physical_device) != null) {
                        break;
                }
        }
        free(physical_devices);

        if (val(out_physical_device) == null) {
                if (integrated != null) {
                        val(out_physical_device) = integrated;
                } else {
                        printf("Failed to find a suitable GPU\n");
                }
        }

        out queue_family_index;
}*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//

make_struct {
#if OS_WINDOWS
  HWND hwnd;
  HINSTANCE inst;
#elif OS_LINUX
  //
#elif OS_MACOS
  //
#endif
  str name;
  u32 w, h;
}
struct_h_window;
make_ptr(struct_h_window) h_window;

fn LRESULT CALLBACK window_proc(in HWND hwnd, in UINT u_msg, in WPARAM w_param,
                                in LPARAM l_param) {
  if (u_msg == WM_DESTROY) {
    PostQuitMessage(0);
    out 0;
  }
  out DefWindowProc(hwnd, u_msg, w_param, l_param);
}

fn h_window new_window(in str in_name, in u32 in_width, in u32 in_height) {
  h_window temp_window = new_mem(struct_h_window, 1);
  //
#if OS_WINDOWS
  HINSTANCE h_inst = GetModuleHandle(NULL);
  WNDCLASSEX wc = {sizeof(WNDCLASSEX),
                   CS_HREDRAW | CS_VREDRAW,
                   window_proc,
                   0,
                   0,
                   h_inst,
                   NULL,
                   LoadCursor(NULL, IDC_ARROW),
                   CreateSolidBrush(RGB(0, 0, 0)),
                   NULL,
                   in_name,
                   NULL};
  RegisterClassEx(adr(wc));
  temp_window->w = in_width;
  temp_window->h = in_height;
  HWND hwnd = CreateWindowEx(
      0, wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
      CW_USEDEFAULT, temp_window->w, temp_window->h, NULL, NULL, h_inst, NULL);
  temp_window->hwnd = hwnd;
  temp_window->inst = h_inst;
#elif OS_LINUX
  //
#elif OS_MACOS
  //
#endif
  //
  out temp_window;
}

//

fn u32 find_debug_layers(in ptr(str) desired_layers, in u32 desired_count,
                         ptr(ptr(str)) out_layers) {
  u32 debug_layer_count = 0;
  vkEnumerateInstanceLayerProperties(adr(debug_layer_count), null);
  ptr(h_layer_properties) available_layers =
      new_mem(h_layer_properties, debug_layer_count);
  vkEnumerateInstanceLayerProperties(adr(debug_layer_count), available_layers);
  val(out_layers) = new_mem(str, desired_count);
  //
  u32 enabled_count = 0;
  iter(desired_count, i) {
    iter(debug_layer_count, j) {
      if (strcmp(desired_layers[i], available_layers[j].layerName) == 0) {
        (val(out_layers))[enabled_count++] = desired_layers[i];
        break;
      }
    }
  }
  free(available_layers);
  //
  out enabled_count;
}

fn u32 find_physical_device(in h_instance in_instance, in h_surface in_surface,
                            ptr(h_physical_device) out_physical_device) {
  val(out_physical_device) = null;
  u32 queue_family_index = u32_max;

  u32 physical_devices_count = 0;
  vkEnumeratePhysicalDevices(in_instance, adr(physical_devices_count), null);
  ptr(h_physical_device) physical_devices =
      new_mem(h_physical_device, physical_devices_count);
  vkEnumeratePhysicalDevices(in_instance, adr(physical_devices_count),
                             physical_devices);

  h_physical_device integrated = null;
  iter(physical_devices_count, i) {

    h_physical_device_properties dev_prop;
    vkGetPhysicalDeviceProperties(physical_devices[i], adr(dev_prop));

    if (dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU or
        dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
      u32 queue_family_n = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i],
                                               adr(queue_family_n), null);

      ptr(VkQueueFamilyProperties) queue_family_prop =
          new_mem(VkQueueFamilyProperties, queue_family_n);

      vkGetPhysicalDeviceQueueFamilyProperties(
          physical_devices[i], adr(queue_family_n), queue_family_prop);

      iter(queue_family_n, j) {
        queue_family_index = j;
        VkBool32 support_present;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices[i], j, in_surface,
                                             adr(support_present));

        if ((queue_family_prop[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) and
            support_present) {
          if (dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            val(out_physical_device) = physical_devices[i];

            break;
          }
          elif (integrated == null) { integrated = physical_devices[i]; }
        }
      }
      free(queue_family_prop);
    }

    if (val(out_physical_device) != null) {
      break;
    }
  }
  free(physical_devices);

  if (val(out_physical_device) == null) {
    if (integrated != null) {
      val(out_physical_device) = integrated;
    } else {
      print("Failed to find a suitable GPU\n");
    }
  }

  out queue_family_index;
}

fn h_physical_device_features
get_physical_device_features(h_physical_device in_physical_device) {
  h_physical_device_features temp_physical_device_features;
  vkGetPhysicalDeviceFeatures(in_physical_device,
                              adr(temp_physical_device_features));
  out temp_physical_device_features;
}

//

OBJECT(machine, // elements
       h_window window;
       h_instance instance; h_surface surface; u32 queue_family_index;
       h_physical_device physical_device; h_device device;
       ,   // parameters
       (), // initiate
       { $new_machine(OBJECT_machine); })

fn pure $new_machine(in machine in_machine) {
  in_machine->window = new_window("hept", 512, 256);
  ShowWindow(in_machine->window->hwnd, SW_SHOWDEFAULT);

  volkInitialize();

  //

  h_info_app info_app =
      h_make_info_app("NYKRA", h_make_version(1, 0, 0), "hept",
                      h_make_version(1, 0, 0), h_make_version(1, 3, 0));

  str instance_extensions[] = {"VK_KHR_surface", "VK_EXT_debug_utils"};

  str desired_debug_layers[] = {
      "VK_LAYER_KHRONOS_validation",
      //"VK_LAYER_LUNARG_api_dump",
      //"VK_LAYER_LUNARG_device_simulation",
      //"VK_LAYER_LUNARG_monitor",
      //"VK_LAYER_LUNARG_screenshot"
  };

  ptr(str) debug_layers = null;
  u32 debug_layer_count =
      find_debug_layers(desired_debug_layers, array_size(desired_debug_layers),
                        adr(debug_layers));

  u32 extension_count = 2;
  str extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME,
                      VK_KHR_WIN32_SURFACE_EXTENSION_NAME};

  h_info_instance instance_info =
      h_make_info_instance(adr(info_app), debug_layer_count, debug_layers,
                           extension_count, extensions);

  in_machine->instance = h_new_instance(instance_info);

  //

  VkWin32SurfaceCreateInfoKHR create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  create_info.hwnd = in_machine->window->hwnd;
  create_info.hinstance = in_machine->window->inst;

  vkCreateWin32SurfaceKHR(in_machine->instance, &create_info, NULL,
                          adr(in_machine->surface));

  //

  in_machine->queue_family_index =
      find_physical_device(in_machine->instance, in_machine->surface,
                           adr(in_machine->physical_device));

  f32 queue_priority = 1.0f;
  h_info_device_queue device_queue = h_make_info_device_queue(
      in_machine->queue_family_index, 1, adr(queue_priority));

  h_physical_device_features features =
      get_physical_device_features(in_machine->physical_device);

  str device_ext[] = {"VK_KHR_swapchain"};
  h_info_device info_device = h_make_info_device(
      1, adr(device_queue), null, null, 1, adr(device_ext), adr(features));

  in_machine->device = h_new_device(in_machine->physical_device, info_device);
}

//

fn h_surface_capabilities get_surface_capabilities(
    in h_physical_device in_physical_device, in h_surface in_surface) {
  h_surface_capabilities temp_capabilities;
  if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(in_physical_device, in_surface,
                                                adr(temp_capabilities)) !=
      VK_SUCCESS) {
    print("Failed to find surface capabilities\n");
  }
  out temp_capabilities;
}

fn h_surface_format get_surface_format(in h_physical_device in_physical_device,
                                       in h_surface in_surface) {
  h_surface_format temp_surface_format;
  uint32_t format_n = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(in_physical_device, in_surface,
                                       adr(format_n), null);
  ptr(h_surface_format) formats = new_mem(h_surface_format, format_n);
  vkGetPhysicalDeviceSurfaceFormatsKHR(in_physical_device, in_surface,
                                       adr(format_n), formats);
  temp_surface_format = formats[0];
  free(formats);
  out temp_surface_format;
}

fn h_present_mode get_present_mode(in h_physical_device in_physical_device,
                                   in h_surface in_surface) {
  h_present_mode temp_present_mode = h_present_mode_vsync_off;
  u32 present_mode_n = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(in_physical_device, in_surface,
                                            adr(present_mode_n), null);
  ptr(h_present_mode) present_modes = new_mem(h_present_mode, present_mode_n);
  vkGetPhysicalDeviceSurfacePresentModesKHR(in_physical_device, in_surface,
                                            adr(present_mode_n), present_modes);
  iter(present_mode_n, i) {
    if (present_modes[i] == h_present_mode_vsync_optimal) {
      temp_present_mode = h_present_mode_vsync_optimal;
      break;
    }
  }
  free(present_modes);
  out temp_present_mode;
}

OBJECT(renderer, // elements
       h_queue queue;
       h_viewport viewport; h_swapchain swapchain;
       h_format swapchain_image_format; h_extent swapchain_extent;
       ptr(h_image_view) image_views; ptr(h_image) images; u32 image_count;
       u32 current_frame; h_render_pass render_pass;
       ptr(h_framebuffer) framebuffers; h_command_pool command_pool;
       ptr(h_command_buffer) command_buffers; ptr(h_semaphore) image_ready;
       ptr(h_semaphore) image_done; ptr(h_fence) flight_fences;
       ,                        // parameters
       (in machine in_machine), // initiate
       { $new_renderer(in_machine, OBJECT_renderer); })

fn pure $new_renderer(in machine in_machine, in renderer in_renderer) {
  in_renderer->queue =
      h_new_queue(in_machine->device, in_machine->queue_family_index, 0);
  //

  h_surface_capabilities capabilities = get_surface_capabilities(
      in_machine->physical_device, in_machine->surface);
  h_surface_format surface_format =
      get_surface_format(in_machine->physical_device, in_machine->surface);
  h_present_mode present_mode =
      get_present_mode(in_machine->physical_device, in_machine->surface);

  //

  h_info_swapchain swapchain_info = h_make_info_swapchain(
      in_machine->surface, capabilities.minImageCount + 1,
      surface_format.format, surface_format.colorSpace,
      capabilities.currentExtent, 1,
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      VK_SHARING_MODE_EXCLUSIVE, null, null, capabilities.currentTransform,
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, present_mode, VK_TRUE, null);

  in_renderer->swapchain = h_new_swapchain(in_machine->device, swapchain_info);
  in_renderer->swapchain_image_format = surface_format.format;
  in_renderer->swapchain_extent = capabilities.currentExtent;
  in_renderer->viewport =
      h_make_viewport(0.0, 0.0, f32_(in_renderer->swapchain_extent.width),
                      f32_(in_renderer->swapchain_extent.height), 0.0, 1.0);

  vkGetSwapchainImagesKHR(in_machine->device, in_renderer->swapchain,
                          adr(in_renderer->image_count), null);
  in_renderer->images = new_mem(h_image, in_renderer->image_count);
  vkGetSwapchainImagesKHR(in_machine->device, in_renderer->swapchain,
                          adr(in_renderer->image_count), in_renderer->images);

  in_renderer->image_views = new_mem(h_image_view, in_renderer->image_count);
  iter(in_renderer->image_count, i) {
    h_info_image_view image_view_info = h_make_info_image_view(
        in_renderer->images[i], VK_IMAGE_VIEW_TYPE_2D,
        in_renderer->swapchain_image_format,
        ((h_component_mapping){
            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY}),
        ((h_image_subresource_range){VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}));

    in_renderer->image_views[i] =
        h_new_image_view(in_machine->device, image_view_info);
  }

  h_subpass_description subpass = {0};
  h_attachment_reference attach_ref = {0};
  attach_ref.attachment = 0;
  attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL; // COLOR_ATTACHMENT_OPTIMAL;
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = adr(attach_ref);

  h_attachment_description attachment = {0};
  attachment.format = in_renderer->swapchain_image_format;
  attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  h_info_render_pass render_pass_info =
      h_make_info_render_pass(1, adr(attachment), 1, adr(subpass), 0, null);
  in_renderer->render_pass =
      h_new_render_pass(in_machine->device, render_pass_info);

  //

  h_info_command_pool command_pool_info =
      h_make_info_command_pool(in_machine->queue_family_index);
  command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  in_renderer->command_pool =
      h_new_command_pool(in_machine->device, command_pool_info);
  in_renderer->command_buffers =
      new_mem(h_command_buffer, in_renderer->image_count);

  h_info_command_buffer command_buffer_info = h_make_info_command_buffer(
      in_renderer->command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

  //

  in_renderer->framebuffers = new_mem(h_framebuffer, in_renderer->image_count);
  in_renderer->image_ready = new_mem(h_semaphore, in_renderer->image_count);
  in_renderer->image_done = new_mem(h_semaphore, in_renderer->image_count);
  in_renderer->flight_fences = new_mem(h_fence, in_renderer->image_count);

  h_info_framebuffer framebuffer_info = h_make_info_framebuffer(
      in_renderer->render_pass, 1, null, in_renderer->swapchain_extent.width,
      in_renderer->swapchain_extent.height, 1);

  h_info_semaphore semaphore_info = h_make_info_semaphore();
  h_info_fence fence_info = h_make_info_fence();

  iter(in_renderer->image_count, i) {
    framebuffer_info.pAttachments = adr(in_renderer->image_views[i]);
    in_renderer->framebuffers[i] =
        h_new_framebuffer(in_machine->device, framebuffer_info);

    in_renderer->image_ready[i] =
        h_new_semaphore(in_machine->device, semaphore_info);
    in_renderer->image_done[i] =
        h_new_semaphore(in_machine->device, semaphore_info);
    in_renderer->flight_fences[i] = h_new_fence(in_machine->device, fence_info);
    in_renderer->command_buffers[i] =
        h_new_command_buffer(in_machine->device, command_buffer_info);
  }
  //
  in_renderer->current_frame = 0;
}

//

///  vert_attrib

//

#define $format_type_u8_size_1 VK_FORMAT_R8_UINT
global str $format_type_u8_size_1_str = "uint";
#define $format_type_u8_size_2 VK_FORMAT_R8G8_UINT
global str $format_type_u8_size_2_str = "uvec2";
#define $format_type_u8_size_3 VK_FORMAT_R8G8B8_UINT
global str $format_type_u8_size_3_str = "uvec3";
#define $format_type_u8_size_4 VK_FORMAT_R8G8B8A8_UINT
global str $format_type_u8_size_4_str = "uvec4";

#define $format_type_u16_size_1 VK_FORMAT_R16_UINT
global str $format_type_u16_size_1_str = "uint";
#define $format_type_u16_size_2 VK_FORMAT_R16G16_UINT
global str $format_type_u16_size_2_str = "uvec2";
#define $format_type_u16_size_3 VK_FORMAT_R16G16B16_UINT
global str $format_type_u16_size_3_str = "uvec3";
#define $format_type_u16_size_4 VK_FORMAT_R16G16B16A16_UINT
global str $format_type_u16_size_4_str = "uvec4";

#define $format_type_u32_size_1 VK_FORMAT_R32_UINT
global str $format_type_u32_size_1_str = "uint";
#define $format_type_u32_size_2 VK_FORMAT_R32G32_UINT
global str $format_type_u32_size_2_str = "uvec2";
#define $format_type_u32_size_3 VK_FORMAT_R32G32B32_UINT
global str $format_type_u32_size_3_str = "uvec3";
#define $format_type_u32_size_4 VK_FORMAT_R32G32B32A32_UINT
global str $format_type_u32_size_4_str = "uvec4";

#define $format_type_u64_size_1 VK_FORMAT_R64_UINT
global str $format_type_u64_size_1_str = "uint";
#define $format_type_u64_size_2 VK_FORMAT_R64G64_UINT
global str $format_type_u64_size_2_str = "uvec2";
#define $format_type_u64_size_3 VK_FORMAT_R64G64B64_UINT
global str $format_type_u64_size_3_str = "uvec3";
#define $format_type_u64_size_4 VK_FORMAT_R64G64B64A64_UINT
global str $format_type_u64_size_4_str = "uvec4";

//

#define $format_type_s8_size_1 VK_FORMAT_R8_SINT
global str $format_type_s8_size_1_str = "int";
#define $format_type_s8_size_2 VK_FORMAT_R8G8_SINT
global str $format_type_s8_size_2_str = "ivec2";
#define $format_type_s8_size_3 VK_FORMAT_R8G8B8_SINT
global str $format_type_s8_size_3_str = "ivec3";
#define $format_type_s8_size_4 VK_FORMAT_R8G8B8A8_SINT
global str $format_type_s8_size_4_str = "ivec4";

#define $format_type_s16_size_1 VK_FORMAT_R16_SINT
global str $format_type_s16_size_1_str = "int";
#define $format_type_s16_size_2 VK_FORMAT_R16G16_SINT
global str $format_type_s16_size_2_str = "ivec2";
#define $format_type_s16_size_3 VK_FORMAT_R16G16B16_SINT
global str $format_type_s16_size_3_str = "ivec3";
#define $format_type_s16_size_4 VK_FORMAT_R16G16B16A16_SINT
global str $format_type_s16_size_4_str = "ivec4";

#define $format_type_s32_size_1 VK_FORMAT_R32_SINT
global str $format_type_s32_size_1_str = "int";
#define $format_type_s32_size_2 VK_FORMAT_R32G32_SINT
global str $format_type_s32_size_2_str = "ivec2";
#define $format_type_s32_size_3 VK_FORMAT_R32G32B32_SINT
global str $format_type_s32_size_3_str = "ivec3";
#define $format_type_s32_size_4 VK_FORMAT_R32G32B32A32_SINT
global str $format_type_s32_size_4_str = "ivec4";

#define $format_type_s64_size_1 VK_FORMAT_R64_SINT
global str $format_type_s64_size_1_str = "int";
#define $format_type_s64_size_2 VK_FORMAT_R64G64_SINT
global str $format_type_s64_size_2_str = "ivec2";
#define $format_type_s64_size_3 VK_FORMAT_R64G64B64_SINT
global str $format_type_s64_size_3_str = "ivec3";
#define $format_type_s64_size_4 VK_FORMAT_R64G64B64A64_SINT
global str $format_type_s64_size_4_str = "ivec4";

//

#define $format_type_f16_size_1 VK_FORMAT_R16_SFLOAT
global str $format_type_f16_size_1_str = "float";
#define $format_type_f16_size_2 VK_FORMAT_R16G16_SFLOAT
global str $format_type_f16_size_2_str = "vec2";
#define $format_type_f16_size_3 VK_FORMAT_R16G16B16_SFLOAT
global str $format_type_f16_size_3_str = "vec3";
#define $format_type_f16_size_4 VK_FORMAT_R16G16B16A16_SFLOAT
global str $format_type_f16_size_4_str = "vec4";

#define $format_type_f32_size_1 VK_FORMAT_R32_SFLOAT
global str $format_type_f32_size_1_str = "float";
#define $format_type_f32_size_2 VK_FORMAT_R32G32_SFLOAT
global str $format_type_f32_size_2_str = "vec2";
#define $format_type_f32_size_3 VK_FORMAT_R32G32B32_SFLOAT
global str $format_type_f32_size_3_str = "vec3";
#define $format_type_f32_size_4 VK_FORMAT_R32G32B32A32_SFLOAT
global str $format_type_f32_size_4_str = "vec4";

#define $format_type_f64_size_1 VK_FORMAT_R64_SFLOAT
global str $format_type_f64_size_1_str = "double";
#define $format_type_f64_size_2 VK_FORMAT_R64G64_SFLOAT
global str $format_type_f64_size_2_str = "dvec2";
#define $format_type_f64_size_3 VK_FORMAT_R64G64B64_SFLOAT
global str $format_type_f64_size_3_str = "dvec3";
#define $format_type_f64_size_4 VK_FORMAT_R64G64B64A64_SFLOAT
global str $format_type_f64_size_4_str = "dvec4";

//

make_struct {
  h_format format;
  u32 type_size;
  u32 size;
  str type_glsl;
}
struct_vert_attrib;
make_ptr(struct_vert_attrib) vert_attrib;

fn vert_attrib $new_vert_attrib(in h_format in_format, in u32 in_type_size,
                                in u32 in_size, in str in_type_glsl) {
  vert_attrib temp_vert_attrib = new_mem(vert_attrib, 1);
  //
  temp_vert_attrib->format = in_format;
  temp_vert_attrib->type_size = in_type_size;
  temp_vert_attrib->size = in_size;
  temp_vert_attrib->type_glsl = in_type_glsl;
  //
  out temp_vert_attrib;
}
#define new_vert_attrib(type, size)                                            \
  $new_vert_attrib($format_type_##type##_size_##size, sizeof(type), size,      \
                   $format_type_##type##_size_##size##_str)

//

make_struct {
  u32 type_size;
  list attrib_list;
  str layout_glsl;
}
struct_vert_form;
make_ptr(struct_vert_form) vert_form;

fn vert_form new_vert_form() {
  vert_form temp_vert_form = new_mem(vert_form, 1);
  //
  temp_vert_form->type_size = 0;
  temp_vert_form->attrib_list = new_list(vert_attrib);
  temp_vert_form->layout_glsl = new_str("", 0);
  //
  out temp_vert_form;
}

fn vert_form vert_form_add_attrib(in vert_form in_vert_form,
                                  in vert_attrib in_attrib,
                                  in str in_attrib_var) {
  str layout_str =
      new_str("layout(location = ", str_size(in_attrib->type_glsl) + 6);
  str_add(layout_str, dec_to_str[in_vert_form->attrib_list->size]);
  str_add(layout_str, ") in ");
  str_add(layout_str, in_attrib->type_glsl);
  str_end(layout_str);

  str temp_str = new_str(in_vert_form->layout_glsl,
                         str_size(layout_str) + str_size(in_attrib_var) + 3);
  str_add(temp_str, layout_str);
  str_add(temp_str, " ");
  str_add(temp_str, in_attrib_var);
  str_add(temp_str, ";\n");
  str_end(temp_str);

  delete_str(layout_str);
  delete_str(in_vert_form->layout_glsl);

  list_add(in_vert_form->attrib_list, vert_attrib, in_attrib);
  in_vert_form->type_size += in_attrib->type_size * in_attrib->size;
  in_vert_form->layout_glsl = temp_str;
}

//

/// default glsl

str $glsl_vert_global =
    "#version 460 core\n"
    "\n"
    "struct quat {\n"
    "	float a;\n"
    "	vec3 v;\n"
    "};\n"
    "\n"
    "quat neg(quat q) { return quat(-q.a,-q.v); }\n"
    "quat conj(quat q) { return quat(q.a,-q.v); }\n"
    "quat quat_add(quat a, quat b) { return quat(a.a + b.a, a.v + b.v); }\n"
    "quat quat_mul(quat a, quat b) { return quat(a.a * b.a - dot(a.v, b.v), "
    "a.a * b.v + b.a * a.v + cross(a.v, b.v)); }\n"
    "\n"
    "struct dual_quat {\n"
    "	quat r;\n"
    "	quat d;\n"
    "};\n"
    "\n"
    "dual_quat dual_quat_mul(dual_quat a, dual_quat b) { return "
    "dual_quat(quat_mul(a.r, b.r), quat_add(quat_mul(a.r, b.d), quat_mul(a.d, "
    "b.r))); }\n"
    "vec3 dual_quat_trans( dual_quat d, vec3 p ) { return dual_quat_mul( "
    "dual_quat_mul( d, dual_quat( quat(1,vec3(0)), quat( 0, p ))), dual_quat( "
    "conj( d.r ), neg( conj( d.d )))).d.v; }\n"
    "\n"
    "struct camera {\n"
    "	dual_quat dq;\n"
    "	vec4 proj;\n"
    "	vec3 pos;\n"
    "	float scl;\n"
    "};\n"
    "\n"
    "camera make_camera(mat4 cam) {\n"
    "	return camera(\n"
    "		dual_quat(quat(cam[0].x, vec3(cam[0].y, cam[0].z, cam[0].w)), "
    "quat(cam[1].x, vec3(cam[1].y, cam[1].z, cam[1].w))), \n"
    "		vec4(cam[2].x,cam[2].y,cam[2].z,cam[2].w),\n"
    "		vec3(cam[3].x,cam[3].y,cam[3].z),\n"
    "		1.\n"
    "	);\n"
    "}\n"
    "\n"
    "vec4 camera_trans(vec3 pos, camera cam) {\n"
    "	vec3 vert = dual_quat_trans(cam.dq, pos - cam.pos);\n"
    "	return vec4(vert.x * cam.proj.x, vert.y * cam.proj.y, (vert.z * "
    "cam.proj.z) + cam.proj.w, vert.z * -1.);\n"
    "}\n"
    "\n";

make_enum{
    glsl_type_vert = shaderc_glsl_vertex_shader,
    glsl_type_frag = shaderc_glsl_fragment_shader,
    glsl_type_comp = shaderc_glsl_compute_shader,
} enum_glsl_type;

make_struct {
  enum_glsl_type type;
  str path;
  vert_form form;
  h_shader_module module;
  ptr(u32) spirv;
  u32 size;
  h_info_pipeline_shader_stage stage_info;
}
struct_glsl;
make_ptr(struct_glsl) glsl;

fn glsl $new_glsl(in h_device in_device, in str in_path,
                  in enum_glsl_type in_type, in vert_form in_form) {
  glsl temp_glsl = new_mem(struct_glsl, 1);
  //
  temp_glsl->type = in_type;
  temp_glsl->path = in_path;
  temp_glsl->form = in_form;
  os_file text = file_load(temp_glsl->path);

  // TODO: INSERT VERT FORMAT GLSL BEFORE TEXT
  str glsl_str = new_str(
      $glsl_vert_global,
      ((temp_glsl->form != null) ? strlen(temp_glsl->form->layout_glsl) : 0) +
          text.size + 1);
  if (temp_glsl->form != null)
    str_add(glsl_str, temp_glsl->form->layout_glsl);

  str_add(glsl_str, text.data);
  str_end(glsl_str);

  // Initialize shaderc
  shaderc_compiler_t compiler = shaderc_compiler_initialize();
  shaderc_compile_options_t options = shaderc_compile_options_initialize();

  // Compile GLSL to SPIR-V
  shaderc_compilation_result_t result =
      shaderc_compile_into_spv(compiler, glsl_str, strlen(glsl_str), in_type,
                               temp_glsl->path, "main", options);

  if (shaderc_result_get_compilation_status(result) !=
      shaderc_compilation_status_success) {
    fprintf(stderr, "GLSL compilation error: %s\n",
            shaderc_result_get_error_message(result));
  } else {
    temp_glsl->spirv = (ptr(u32))shaderc_result_get_bytes(result);
    temp_glsl->size = shaderc_result_get_length(result);

    h_info_shader_module module_info =
        h_make_info_shader_module(temp_glsl->spirv, temp_glsl->size);
    // Use the SPIR-V bytecode to create the shader module
    temp_glsl->module = h_new_shader_module(in_device, module_info);
  }

  // Release shaderc resources
  shaderc_result_release(result);
  shaderc_compile_options_release(options);
  shaderc_compiler_release(compiler);

  // Shader stage
  temp_glsl->stage_info = h_make_info_pipeline_shader_stage(
      ((in_type == glsl_type_vert) ? (VK_SHADER_STAGE_VERTEX_BIT)
                                   : (VK_SHADER_STAGE_FRAGMENT_BIT)),
      temp_glsl->module, "main", );
  //
  out temp_glsl;
}

#define new_glsl_vert(in_device, in_path, in_vert_form)                        \
  $new_glsl(in_device, in_path, glsl_type_vert, in_vert_form)
#define new_glsl_frag(in_device, in_path)                                      \
  $new_glsl(in_device, in_path, glsl_type_frag, null)

//

OBJECT(shader, // elements
       h_pipeline pipeline;
       h_pipeline_layout pipeline_layout; h_descriptor_set descriptor_set;
       , // parameters
       (in machine in_machine, in renderer in_renderer, in u32 in_glsl_count,
        in ptr(glsl) in_glsl, in h_topology in_topology), // initiate
       {
         $new_shader(in_machine, in_renderer, OBJECT_shader, in_glsl_count,
                     in_glsl, in_topology);
       })

fn pure $new_shader(in machine in_machine, in renderer in_renderer,
                    in shader in_shader, in u32 in_glsl_count,
                    in ptr(glsl) in_glsl, in h_topology in_topology) {
  vert_form temp_vert_form = null;
  ptr(h_info_pipeline_shader_stage) glsl_stages =
      new_mem(h_info_pipeline_shader_stage, in_glsl_count);
  iter(in_glsl_count, g) {
    if (in_glsl[g]->type == glsl_type_vert) {
      temp_vert_form = in_glsl[g]->form;
    }

    glsl_stages[g] = in_glsl[g]->stage_info;
  }

  //

  h_vertex_binding vert_bindings[] = {
      h_make_vertex_binding_per_vertex(0, temp_vert_form->type_size)};

  ptr(h_vertex_attribute) vert_attributes =
      new_mem(h_vertex_attribute, temp_vert_form->attrib_list->size);
  vert_attrib temp_vert_attrib = null;
  u32 offset = 0;
  iter(temp_vert_form->attrib_list->size, a) {
    temp_vert_attrib = list_get(temp_vert_form->attrib_list, vert_attrib, a);
    vert_attributes[a].location = a;
    vert_attributes[a].binding = 0;
    vert_attributes[a].format = temp_vert_attrib->format;
    vert_attributes[a].offset = offset;
    offset += temp_vert_attrib->type_size * temp_vert_attrib->size;
  }

  h_info_pipeline_vertex pipeline_vertex_info = h_make_info_pipeline_vertex(
      1, vert_bindings, temp_vert_form->attrib_list->size, vert_attributes);

  //

  h_info_pipeline_assembly pipeline_input_assembly_info =
      h_make_info_pipeline_assembly(in_topology);

  //

  h_scissor scissor = {0};
  scissor.offset = (VkOffset2D){0, 0};
  scissor.extent = in_renderer->swapchain_extent;

  h_info_pipeline_viewport viewport_info = h_make_info_pipeline_viewport(
      1, adr(in_renderer->viewport), 1, adr(scissor));

  //

  h_info_pipeline_raster raster_info = h_make_info_pipeline_raster(
      VK_POLYGON_MODE_FILL, 1.0, VK_CULL_MODE_BACK_BIT,
      VK_FRONT_FACE_CLOCKWISE);

  //

  VkPipelineMultisampleStateCreateInfo multisampling = {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  //

  VkPipelineDepthStencilStateCreateInfo depth_stencil = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_TRUE,
      .depthWriteEnable = VK_TRUE,
      .depthCompareOp = VK_COMPARE_OP_LESS,
      .depthBoundsTestEnable = VK_FALSE,
      .minDepthBounds = 0.0f,
      .maxDepthBounds = 1.0f,
      .stencilTestEnable = VK_FALSE,
      .front = {0},
      .back = {0},
  };

  //

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colorBlending = {
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;

  //

  VkPipelineDynamicStateCreateInfo dynamic_state_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = 2,
      .pDynamicStates = (VkDynamicState[]){VK_DYNAMIC_STATE_VIEWPORT,
                                           VK_DYNAMIC_STATE_SCISSOR}};

  //

  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorSetLayoutBinding bindings[] = {
      {
          .binding = 0,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      },
      {
          .binding = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
  };

  VkDescriptorSetLayoutCreateInfo layoutInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 2,
      .pBindings = &bindings,
  };

  if (vkCreateDescriptorSetLayout(in_machine->device, &layoutInfo, null,
                                  &descriptorSetLayout) != VK_SUCCESS) {
    printf("Failed to create descriptor set layout\n");
    exit(1);
  }

  h_info_pipeline_layout info_pipeline_layout =
      h_make_info_pipeline_layout(1, adr(descriptorSetLayout), null, null);

  in_shader->pipeline_layout =
      h_new_pipeline_layout(in_machine->device, info_pipeline_layout);

  //

  h_info_pipeline pipeline_info = h_make_info_pipeline(
      in_glsl_count, glsl_stages, adr(pipeline_vertex_info),
      adr(pipeline_input_assembly_info), null, adr(viewport_info),
      adr(raster_info), adr(multisampling), adr(depth_stencil),
      adr(colorBlending),
      null, // adr(dynamic_state_info),
      in_shader->pipeline_layout, in_renderer->render_pass, 0, null, null);
  pipeline_info.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

  // derivedPipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
  // derivedPipelineInfo.basePipelineHandle = basePipeline;
  // derivedPipelineInfo.basePipelineIndex = -1;

  in_shader->pipeline = h_new_pipeline(in_machine->device, pipeline_info);

  //

  VkDescriptorPool descriptorPool;
  VkDescriptorPoolSize pool_sizes[2] = {
      {
          .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = 1,
      },
      {
          .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount = 1,
      },
  };

  VkDescriptorPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = 1,
      .poolSizeCount = 2,
      .pPoolSizes = &pool_sizes,
  };

  if (vkCreateDescriptorPool(in_machine->device, &poolInfo, null,
                             &descriptorPool) != VK_SUCCESS) {
    printf("Failed to create descriptor pool\n");
    exit(1);
  }

  VkDescriptorSetAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptorPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptorSetLayout,
  };

  if (vkAllocateDescriptorSets(in_machine->device, &allocInfo,
                               &in_shader->descriptor_set) != VK_SUCCESS) {
    printf("Failed to allocate descriptor set\n");
    exit(1);
  }

  //

  free_mem(glsl_stages);
}

//

fn u32 find_memory_type(in VkPhysicalDevice physical_device,
                        in uint32_t type_filter,
                        in VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

  iter(memory_properties.memoryTypeCount, i) {
    if ((type_filter & (1 << i)) and
        (memory_properties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      out i;
    }
  }

  fprintf(stderr, "Failed to find suitable memory type\n");
  out null;
}

//

/// buffer

make_struct {
  h_buffer buff;
  h_device_mem mem;
}
h_device_buffer;

fn h_device_buffer h_new_device_buffer(in machine in_machine, VkDeviceSize size,
                                       VkBufferUsageFlags usage,
                                       VkMemoryPropertyFlags properties) {
  VkBufferCreateInfo buffer_info = {0};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  h_buffer buff = new_mem(h_buffer, 1);
  h_device_mem mem = new_mem(h_device_mem, 1);

  vkCreateBuffer(in_machine->device, &buffer_info, NULL, adr(buff));

  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(in_machine->device, buff, &mem_requirements);

  VkMemoryAllocateInfo alloc_info = {0};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = find_memory_type(
      in_machine->physical_device, mem_requirements.memoryTypeBits, properties);

  vkAllocateMemory(in_machine->device, &alloc_info, NULL, adr(mem));

  vkBindBufferMemory(in_machine->device, buff, mem, 0);

  out(h_device_buffer){.buff = buff, .mem = mem};
}

//

/// mesh

OBJECT(mesh, // elements
       h_buffer vertex_buffer;
       h_device_mem vertex_buffer_memory;

       h_buffer index_buffer; h_device_mem index_buffer_memory;

       vert_form form; list vert_list; list ind_list;,   // parameters
                                                     (), // initiate
                                                     {

                                                     })

//

OBJECT(command, // elements
       list list_pipeline;
       u32 current_frame; ptr(h_framebuffer) framebuffers;
       h_command_pool command_pool; ptr(h_command_buffer) command_buffers;
       ,   // parameters
       (), // initiate
       {
           //
       })

//

global machine default_machine = null;
global renderer default_renderer = null;
global shader default_shader = null;

global vert_attrib default_vert_attrib_pos2 = null;
global vert_attrib default_vert_attrib_pos3 = null;
global vert_attrib default_vert_attrib_uv = null;
global vert_attrib default_vert_attrib_rgb = null;
global vert_attrib default_vert_attrib_rgba = null;

global vert_form default_vert_form_3d = null;

global shader default_shader_2d;
global shader default_shader_3d;

//

fn pure $main_draw_init();
fn pure $main_draw();
fn pure $main_step_init();
fn pure $main_step();

//

fn pure $hept_init() {
  global_pool_machine = new_pool(machine);
  global_pool_renderer = new_pool(renderer);
  global_pool_shader = new_pool(shader);

  default_machine = new_machine();
  default_renderer = new_renderer(default_machine);

  default_vert_attrib_pos2 = new_vert_attrib(f32, 2);
  default_vert_attrib_pos3 = new_vert_attrib(f32, 3);
  default_vert_attrib_uv = new_vert_attrib(f32, 2);
  default_vert_attrib_rgb = new_vert_attrib(f32, 3);
  default_vert_attrib_rgba = new_vert_attrib(f32, 4);

  default_vert_form_3d = new_vert_form();
  vert_form_add_attrib(default_vert_form_3d, default_vert_attrib_pos3,
                       "in_pos");
  vert_form_add_attrib(default_vert_form_3d, default_vert_attrib_uv, "in_uv");
  vert_form_add_attrib(default_vert_form_3d, default_vert_attrib_rgb, "in_rgb");

  glsl glsl_vert = new_glsl_vert(default_machine->device, "glsl/shader.vert",
                                 default_vert_form_3d);
  glsl glsl_frag = new_glsl_frag(default_machine->device, "glsl/shader.frag");

  glsl in_glsl[] = {
      glsl_vert,
      glsl_frag,
  };

  default_shader_3d =
      new_shader(default_machine, default_renderer, 2, in_glsl, h_topology_tri);
}

make_struct {
  float pos[3];
  float uv[2];
  float rgb[3];
}
vert;

make_struct { u32 a, b, c; }
ind_tri;

make_struct { u32 a, b; }
ind_line;

global vert vertex_data[] = {
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Vertex 0
    {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // Vertex 1
    {{0.0f, 0.5f, 0.0f}, {0.5f, 1.0f}, {0.0f, 0.0f, 1.0f}},   // Vertex 2
};

global ind_tri index_data[] = {
    {0, 1, 2},
};

fn pure $main_loop() {

  h_device_buffer vertex_buffer = h_new_device_buffer(
      default_machine, sizeof(vert) * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  h_device_buffer index_buffer = h_new_device_buffer(
      default_machine, sizeof(ind_tri) * 1, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  void *data;
  vkMapMemory(default_machine->device, vertex_buffer.mem, 0, sizeof(vert) * 3,
              0, &data);
  memcpy(data, vertex_data, sizeof(vert) * 3);
  vkUnmapMemory(default_machine->device, vertex_buffer.mem);

  vkMapMemory(default_machine->device, index_buffer.mem, 0, sizeof(ind_tri) * 1,
              0, &data);
  memcpy(data, index_data, sizeof(ind_tri) * 1);
  vkUnmapMemory(default_machine->device, index_buffer.mem);

  //$main_draw_init();
  loop {
    once MSG msg = {0};
    spin(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        return null;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    //

    {
      once u32 fence = 0;

      vkWaitForFences(default_machine->device, 1,
                      adr(default_renderer->flight_fences[fence]), VK_TRUE,
                      UINT64_MAX);

      vkResetFences(default_machine->device, 1,
                    adr(default_renderer->flight_fences[fence]));

      fence =
          (default_renderer->current_frame + 1) % default_renderer->image_count;
    }

    vkAcquireNextImageKHR(
        default_machine->device, default_renderer->swapchain, UINT64_MAX,
        default_renderer->image_ready[default_renderer->current_frame],
        VK_NULL_HANDLE, adr(default_renderer->current_frame));

    // default_renderer->current_frame = image_index;

    // print("%i :: %i\n", image_index, default_renderer->current_frame);

    //

    //$main_draw();

    // iter(default_renderer->image_count, i)
    {
      VkCommandBufferBeginInfo begin_info = {
          VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

      vkBeginCommandBuffer(
          default_renderer->command_buffers[default_renderer->current_frame],
          &begin_info);
      //

      VkRenderPassBeginInfo renderPassInfo = {
          VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
      renderPassInfo.renderPass = default_renderer->render_pass;
      renderPassInfo.framebuffer =
          default_renderer->framebuffers[default_renderer->current_frame];
      renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
      renderPassInfo.renderArea.extent =
          default_renderer
              ->swapchain_extent; // (h_extent) { .width = 64, .height = 64 };

      VkClearValue clearColor = {0.125f, 0.25f, 0.5f, 1.0f};
      renderPassInfo.clearValueCount = 1;
      renderPassInfo.pClearValues = &clearColor;

      vkCmdBeginRenderPass(
          default_renderer->command_buffers[default_renderer->current_frame],
          &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
      vkCmdBindPipeline(
          default_renderer->command_buffers[default_renderer->current_frame],
          VK_PIPELINE_BIND_POINT_GRAPHICS, default_shader_3d->pipeline);

      // Bind the descriptor set containing the SSBO
      /*vkCmdBindDescriptorSets(default_renderer->command_buffers[default_renderer->current_frame],
      VK_PIPELINE_BIND_POINT_GRAPHICS, default_shader_3d->pipeline_layout,
      0, 1, &default_shader_3d->descriptor_set, 0, NULL);*/

      VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(
          default_renderer->command_buffers[default_renderer->current_frame], 0,
          1, &vertex_buffer.buff, offsets);
      vkCmdBindIndexBuffer(
          default_renderer->command_buffers[default_renderer->current_frame],
          index_buffer.buff, 0, VK_INDEX_TYPE_UINT32);

      // Draw the triangles
      // vkCmdDraw(default_renderer->command_buffers[default_renderer->current_frame],
      // 3, 1, 0, 0);
      vkCmdDrawIndexed(
          default_renderer->command_buffers[default_renderer->current_frame], 3,
          1, 0, 0, 0);
      vkCmdEndRenderPass(
          default_renderer->command_buffers[default_renderer->current_frame]);

      //
      vkEndCommandBuffer(
          default_renderer->command_buffers[default_renderer->current_frame]);
    }

    //

    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    h_info_submit submit_info = h_make_info_submit(
        1, adr(default_renderer->image_ready[default_renderer->current_frame]),
        wait_stages, 1,
        adr(default_renderer->command_buffers[default_renderer->current_frame]),
        1, adr(default_renderer->image_done[default_renderer->current_frame]));

    h_submit_queue(
        default_renderer->queue, submit_info,
        default_renderer->flight_fences[default_renderer->current_frame]);

    //

    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores =
        adr(default_renderer->image_done[default_renderer->current_frame]);
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &default_renderer->swapchain;
    presentInfo.pImageIndices = &default_renderer->current_frame;

    vkQueuePresentKHR(default_renderer->queue, &presentInfo);

    vkQueueWaitIdle(default_renderer->queue);

    //

    default_renderer->current_frame =
        (default_renderer->current_frame + 1) % default_renderer->image_count;
  }
}

fn pure $main_init();
#undef main
#undef main()
#define main                                                                   \
  int main() {                                                                 \
    $hept_init();                                                              \
    $main_init();                                                              \
    $main_loop();                                                              \
  }                                                                            \
  fn pure $main_init()

#define main_loop fn pure $main_loop()

#define main_draw fn pure $main_draw()
