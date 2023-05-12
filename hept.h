#pragma once

#include "hephaestus.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

#include "c7h16.h"

//

typedef ptr(SDL_Window) h_window;
typedef ptr(SDL_Thread) h_thread;

//

/// timer

#define ns_now s64_(SDL_GetTicksNS())

make_type( s64 ) timer;

#define new_timer() ns_now

#define timer_get_sec( t ) (f64_(ns_now - t) / 1000000000.L)
#define timer_get_milli( t ) (f64_(ns_now - t) / 1000000.L)
#define timer_get_micro( t ) (f64_(ns_now - t) / 1000.L)
#define timer_get_nano( t ) (f64_(ns_now - t))

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

#define OBJECT( OBJ, STRUCT_VARIABLES, NEW_PARAMS, NEW_SET )																	\
make_struct{					   																								\
	volatile spinlock lock;					   																					\
	volatile u32 pool_id;				   																						\
	volatile str object_name;																									\
	STRUCT_VARIABLES			   																								\
} OBJ##_struct;				   																									\
make_ptr(OBJ##_struct) OBJ;	   																									\
																																\
global OBJ current_##OBJ = null;		   																						\
global pool global_pool_##OBJ = null;	   																						\
																																\
fn pure OBJ##_lock( in OBJ OBJ##_to_lock ){spinlock_engage(OBJ##_to_lock->lock);}												\
fn pure OBJ##_unlock( in OBJ OBJ##_to_unlock ){spinlock_vacate(OBJ##_to_unlock->lock);}											\
																																\
fn pure OBJ##_set_current( in OBJ set_this_##OBJ##_current ){safe_ptr_set(current_##OBJ, set_this_##OBJ##_current);}			\
fn OBJ OBJ##_get_current(){out safe_ptr_get(current_##OBJ);}																	\
																																\
fn OBJ new_##OBJ NEW_PARAMS																										\
{																																\
	OBJ OBJECT_##OBJ = new_mem(OBJ##_struct, 1);																				\
	OBJECT_##OBJ->object_name = #OBJ;																							\
	NEW_SET																														\
	OBJ##_set_current(OBJECT_##OBJ);																							\
	pool_lock(global_pool_##OBJ);																								\
	pool_add(global_pool_##OBJ, OBJ, OBJECT_##OBJ);																				\
	safe_u32_set(OBJECT_##OBJ->pool_id, global_pool_##OBJ->pos_prev);															\
	pool_unlock(global_pool_##OBJ);																								\
	out OBJECT_##OBJ;																											\
}																																\
																																\
fn pure delete_##OBJ( in OBJ delete_this_##OBJ )																				\
{																																\
	if(delete_this_##OBJ == OBJ##_get_current()) OBJ##_set_current(null);														\
	pool_lock(global_pool_##OBJ);																								\
	pool_free(global_pool_##OBJ, OBJ, safe_u32_get(delete_this_##OBJ->pool_id));												\
	pool_unlock(global_pool_##OBJ);																								\
	free(delete_this_##OBJ);																									\
}																																\

#define new( OBJ ) new_##OBJ()

//

VkCommandBuffer begin_single_time_commands(in h_device device, in h_command_pool command_pool) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = command_pool,
        .commandBufferCount = 1
    };

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

void end_single_time_commands(in h_device device, in h_command_pool command_pool, in h_queue graphics_queue, in h_command_buffer command_buffer) {
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer
    };

    vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}


make_struct {
  f32 x, y;
  f32 u, v;
} sprite_struct;

uint32_t find_memory_type(VkPhysicalDevice physical_device,
                          uint32_t type_filter,
                          VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

  // printf("Memory type filter: 0x%X\n", type_filter);
  // printf("Desired properties: 0x%X\n", properties);

  for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
    // printf("Memory type %u properties: 0x%X\n", i,
    // memory_properties.memoryTypes[i].propertyFlags);

    if ((type_filter & (1 << i)) &&
        (memory_properties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      return i;
    }
  }

  fprintf(stderr, "Failed to find suitable memory type\n");
}

make_struct{
	VkBuffer buff;
	VkDeviceMemory mem;
	u32 type_size;
	u32 size;
} storage_struct;
make_ptr(storage_struct) storage;

storage new_storage(VkDevice device, VkPhysicalDevice physical_device, u32 data_size) {

	storage temp_storage = new_mem(storage_struct, 1);

	// Create SSBO
	VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = data_size,
		.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	if (vkCreateBuffer(device, &bufferInfo, NULL, adr(temp_storage->buff)) != VK_SUCCESS) {
	printf("Failed to create SSBO\n");
	exit(1);
	}

	// Allocate memory for SSBO
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, temp_storage->buff, &memRequirements);

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

	VkMemoryAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = find_memory_type(physical_device, memRequirements.memoryTypeBits,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	};

	if (vkAllocateMemory(device, &allocInfo, NULL, &temp_storage->mem) != VK_SUCCESS) {
	printf("Failed to allocate SSBO memory\n");
	exit(1);
	}

	vkBindBufferMemory(device, temp_storage->buff, temp_storage->mem, 0);

	out temp_storage;
}

make_struct{
	h_image img;
	VkDeviceMemory mem;
	VkImageView view;
	VkSampler sampler;
	u32 width;
	u32 height;
} image_struct;
make_ptr(image_struct) image;

image new_image(VkDevice device, VkPhysicalDevice physical_device, u32 width, u32 height) {
	image temp_image = (image)malloc(sizeof(image_struct));

	VkImageCreateInfo image_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.extent = {.width = width, .height = height, .depth = 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.samples = VK_SAMPLE_COUNT_1_BIT
	};

	if (vkCreateImage(device, &image_info, NULL, &(temp_image->img)) != VK_SUCCESS) {
		printf("Failed to create Image\n");
		exit(1);
	}

	VkMemoryRequirements mem_requirements;
	vkGetImageMemoryRequirements(device, temp_image->img, &mem_requirements);

	VkMemoryAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = mem_requirements.size,
		.memoryTypeIndex = find_memory_type(physical_device, mem_requirements.memoryTypeBits,
											VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
											VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	};

	if (vkAllocateMemory(device, &alloc_info, NULL, &(temp_image->mem)) != VK_SUCCESS) {
		printf("Failed to allocate Image memory\n");
		exit(1);
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

	if (vkCreateImageView(device, &view_info, NULL, &(temp_image->view)) != VK_SUCCESS) {
		printf("Failed to create Image View\n");
		exit(1);
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

	if (vkCreateSampler(device, &sampler_info, NULL, &(temp_image->sampler)) != VK_SUCCESS) {
		printf("Failed to create Image Sampler\n");
		exit(1);
	}

	temp_image->width = width;
	temp_image->height = height;

	return temp_image;
}

void transition_image_layout(VkDevice device, VkCommandPool cmd_pool, h_queue queue, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout) {
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

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = 0;
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
    VkCommandBuffer commandBuffer = begin_single_time_commands(device, commandPool);

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

    end_single_time_commands(device, commandPool, graphicsQueue, commandBuffer);
}

VkShaderModule new_shader(VkDevice device, const char *path, shaderc_shader_kind kind) {
  size_t s;
  const char *source = SDL_LoadFile(path, &s);
  VkShaderModule shaderModule = null;

  // Initialize shaderc
  shaderc_compiler_t compiler = shaderc_compiler_initialize();
  shaderc_compile_options_t options = shaderc_compile_options_initialize();

  // Compile GLSL to SPIR-V
  shaderc_compilation_result_t result = shaderc_compile_into_spv( compiler, source, s, kind, path, "main", options);

  if (shaderc_result_get_compilation_status(result) !=
	  shaderc_compilation_status_success) {
	fprintf(stderr, "GLSL compilation error: %s\n",
			shaderc_result_get_error_message(result));
  } else {
	const uint32_t *spirv = (const uint32_t *)shaderc_result_get_bytes(result);
	const size_t spirv_len = shaderc_result_get_length(result);

	// Use the SPIR-V bytecode to create the shader module
	VkShaderModuleCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	createInfo.codeSize = spirv_len;
	createInfo.pCode = spirv;

	if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
	  print("Failed to create shader module\n");
	  print("%s\n", SDL_GetError());
	}
  }

  // Release shaderc resources
  shaderc_result_release(result);
  shaderc_compile_options_release(options);
  shaderc_compiler_release(compiler);

  return shaderModule;
}

u32 find_debug_layers( in ptr(str) desired_layers, in u32 desired_count, ptr(ptr(str)) out_layers )
{
    u32 debug_layer_count = 0;
    vkEnumerateInstanceLayerProperties(adr(debug_layer_count), null);
	ptr(h_layer_properties) available_layers = new_mem(h_layer_properties, debug_layer_count);
    vkEnumerateInstanceLayerProperties(adr(debug_layer_count), available_layers);
    val(out_layers) = new_mem(str, desired_count);
	//
    u32 enabled_count = 0;
	iter(desired_count, i)
    {
		iter(debug_layer_count, j)
        {
            if(strcmp(desired_layers[i], available_layers[j].layerName) == 0)
			{
                (val(out_layers))[enabled_count++] = desired_layers[i];
                break;
            }
        }
    }
    free(available_layers);
	//
    out enabled_count;
}

//

u32 find_extensions( ptr(ptr(str)) out_extensions )
{
	u32 extension_count = 0;
	SDL_Vulkan_GetInstanceExtensions(adr(extension_count), null);
	val(out_extensions) = new_mem(str, extension_count);
	SDL_Vulkan_GetInstanceExtensions(adr(extension_count), val(out_extensions));
	//
	out extension_count;
}

//

h_physical_device_features get_physical_device_features(h_physical_device in_physical_device)
{
    h_physical_device_features temp_physical_device_features;
    vkGetPhysicalDeviceFeatures(in_physical_device, adr(temp_physical_device_features));
    out temp_physical_device_features;
}

u32 find_physical_device(in h_instance in_instance, in h_surface in_surface, ptr(h_physical_device) out_physical_device )
{
	val(out_physical_device) = null;
	u32 queue_family_index = u32_max;

	u32 physical_devices_count = 0;
	vkEnumeratePhysicalDevices(in_instance, adr(physical_devices_count), null);
	ptr(h_physical_device) physical_devices = new_mem(h_physical_device, physical_devices_count);
	vkEnumeratePhysicalDevices(in_instance, adr(physical_devices_count), physical_devices);

	h_physical_device integrated = null;
	iter(physical_devices_count, i)
	{
			
		h_physical_device_properties dev_prop;
		vkGetPhysicalDeviceProperties(physical_devices[i], adr(dev_prop));

		if (dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU or
			dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
			u32 queue_family_n;
			vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], adr(queue_family_n), null);

			ptr(VkQueueFamilyProperties) queue_family_prop = new_mem(VkQueueFamilyProperties, queue_family_n);

			vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], adr(queue_family_n), queue_family_prop);

			iter(queue_family_n, j)
			{
				queue_family_index = j;
				VkBool32 support_present;
				vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices[i], j, in_surface, adr(support_present));

				if ((queue_family_prop[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) and support_present) {
					if (dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
						val(out_physical_device) = physical_devices[i];
							
						break;
					} else if (integrated == null) {
						integrated = physical_devices[i];
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
}

OBJECT(
	machine
	, // elements
	h_window window;
	h_instance instance;
	h_surface surface;
	u32 queue_family_index;
	h_physical_device physical_device;
	h_device device;
	, // parameters
	()
	, // initiate
	{
		//
	}
)

//

h_surface_capabilities get_surface_capabilities( in h_physical_device in_physical_device, in h_surface in_surface)
	{
	h_surface_capabilities temp_capabilities;
	if(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(in_physical_device, in_surface, adr(temp_capabilities)) != VK_SUCCESS) {
		printf("Failed to find surface capabilities\n");
	}
	out temp_capabilities;
}

h_surface_format get_surface_format(in h_physical_device in_physical_device, in h_surface in_surface)
{
	h_surface_format temp_surface_format;
	uint32_t format_n = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(in_physical_device, in_surface, adr(format_n), null);
	ptr(h_surface_format) formats = new_mem(h_surface_format, format_n);
	vkGetPhysicalDeviceSurfaceFormatsKHR(in_physical_device, in_surface, adr(format_n), formats);
	temp_surface_format = formats[0];
	free(formats);
	out temp_surface_format;
}

h_present_mode get_present_mode(in h_physical_device in_physical_device, in h_surface in_surface)
{
	h_present_mode temp_present_mode = h_present_mode_vsync_off;
	u32 present_mode_n;
	vkGetPhysicalDeviceSurfacePresentModesKHR(in_physical_device, in_surface, adr(present_mode_n), null);
	ptr(h_present_mode) present_modes = new_mem(h_present_mode, present_mode_n);
	vkGetPhysicalDeviceSurfacePresentModesKHR(in_physical_device, in_surface,adr(present_mode_n), present_modes);
	iter(present_mode_n,i)
	{
		if (present_modes[i] == h_present_mode_vsync_optimal) {
		temp_present_mode = h_present_mode_vsync_optimal;
		break;
		}
	}
	free(present_modes);
	out temp_present_mode;
}

OBJECT(
	renderer
	, // elements
	h_queue queue;
	h_swapchain swapchain;
	h_format swapchain_image_format;
	h_extent swapchain_extent;
	h_image_view *image_views;
	u32 image_count;
	h_render_pass render_pass;
	ptr(h_semaphore) image_ready;
	ptr(h_semaphore) image_done;
	ptr(h_fence) flight_fences;
	, // parameters
	()
	, // initiate
	{
		//
	}
)

//

OBJECT(
	pipeline
	, // elements
	h_pipeline ptr;
	h_pipeline_layout pipeline_layout;
	h_descriptor_set descriptor_set;
	//VkPipelineLayoutCreateInfo layout_info;
	, // parameters
	()
	, // initiate
	{
		//
	}
)

//

OBJECT(
	command
	, // elements
	list list_pipeline;
	u32 current_frame;
	ptr(h_framebuffer) framebuffers;
	h_command_pool command_pool;
	ptr(h_command_buffer) command_buffers;
	, // parameters
	()
	, // initiate
	{
		//
	}
)

//

