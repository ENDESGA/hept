// // // // // // //
// > hept _
// -------
// minimal game engine language and system framework
// requires: hephaestus.h, c7h16.h
// @ENDESGA 2023
// // // // // // //

#pragma once
#ifndef HEPT_H
	#define HEPT_H

	#include <c7h16.h>

	#if OS_WINDOWS
		#define VK_USE_PLATFORM_WIN32_KHR
	#elif OS_LINUX
		#define VK_USE_PLATFORM_XLIB_KHR
	#elif OS_MACOS
		#define VK_USE_PLATFORM_MACOS_MVK
	#endif

	#include <hephaestus.h>

//

/////// /////// /////// /////// /////// /////// ///////

/// PTR PILE

	#define PTR( NAME, ... )             \
		make_struct( NAME ){               \
				__VA_ARGS__ };                 \
		make_ptr( struct( NAME ) ) NAME;   \
		global NAME current_##NAME = null; \
		global pile pile_##NAME = null;

	#define NEW_PTR( NAME, PARAMS, ... )                 \
		NAME new_##NAME PARAMS                             \
		{                                                  \
			NAME this_##NAME = new_mem( struct( NAME ), 1 ); \
			DEF_START                                        \
			__VA_ARGS__                                      \
			DEF_END;                                         \
			pile_add( pile_##NAME, NAME, this_##NAME );      \
			current_##NAME = this_##NAME;                    \
			out this_##NAME;                                 \
		}

	#define PTR_PILE( NAME, PARAMS, SET_NEW, ... ) \
		PTR( NAME, __VA_ARGS__ )                     \
		NEW_PTR( NAME, PARAMS, SET_NEW )

//

/////// /////// /////// /////// /////// /////// ///////

/// HEPT

	#define HEPT( NAME, ... )                      \
		PTR_PILE( hept_##NAME, (), {}, __VA_ARGS__ ) \
		hept_##NAME create_hept_##NAME

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

/////// /////// /////// /////// /////// /////// ///////

/// hept_core

HEPT(
		core,
		h_instance instance; )
( in text in_name )
{
	hept_core result = new_hept_core();
	//
	volkInitialize();

	//

	h_info_app info_app = h_make_info_app(
			in_name,
			h_make_version( 1, 0, 0 ),
			"hept",
			h_make_version( 1, 0, 0 ),
			h_make_version( 1, 3, 0 ) );

	text desired_debug_layers[] = {
			"VK_LAYER_KHRONOS_validation",
			//"VK_LAYER_LUNARG_api_dump",
			//"VK_LAYER_LUNARG_device_simulation",
			//"VK_LAYER_LUNARG_monitor",
			//"VK_LAYER_LUNARG_screenshot"
	};
	u32 desired_debug_layers_count = 1;

	u32 debug_layer_count = 0;
	u32 enabled_debug_layer_count = 0;
	vkEnumerateInstanceLayerProperties( ref( debug_layer_count ), null );
	ptr( h_layer_properties ) available_layers = new_mem( h_layer_properties, debug_layer_count );
	vkEnumerateInstanceLayerProperties( ref( debug_layer_count ), available_layers );
	ptr( text ) debug_layers = new_mem( text, desired_debug_layers_count );

	iter( desired_debug_layers_count, i )
	{
		iter( debug_layer_count, j )
		{
			if( strcmp( desired_debug_layers[ i ], available_layers[ j ].layerName ) == 0 )
			{
				( debug_layers )[ enabled_debug_layer_count++ ] = desired_debug_layers[ i ];
				skip;
			}
		}
	}
	free_mem( available_layers );

	u32 extension_count = 2;
	text extensions[] =
			{
					VK_KHR_SURFACE_EXTENSION_NAME,
					VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
			};

	h_info_instance instance_info = h_make_info_instance(
			ref( info_app ),
			enabled_debug_layer_count,
			( ptr( const char ) ptr( const ) )debug_layers,
			extension_count,
			( ptr( const char ) ptr( const ) )extensions );

	result->instance = h_new_instance( instance_info );
	//
	out result;
}

//

/// hept_machine

HEPT(
		machine,
		hept_core core;
		u32 queue_family_index;
		h_physical_device physical_device;
		h_device device; )
()
{
	hept_machine result = new_hept_machine();
	result->core = current_hept_core;
	result->physical_device = null;
	//
	out result;
}

//

/// hept_window

	#if OS_WINDOWS
fn LRESULT CALLBACK process_hept_window( in HWND hwnd, in UINT u_msg, in WPARAM w_param, in LPARAM l_param );

HEPT(
		window,
		hept_core core;
		text name;
		fn_ptr( render_fn, pure );
		flag ready, visible;
		u32 width, height;
		HWND hwnd;
		HINSTANCE inst;
		h_surface surface;
		h_surface_capabilities surface_capabilities;
		h_surface_format surface_format;
		h_present_mode present_mode; )
	#elif OS_LINUX
fn pure process_hept_window( ptr( Display ) in_disp );

HEPT(
		window,
		hept_core core;
		text name;
		fn_ptr( render_fn, pure );
		flag ready, visible;
		u32 width, height;
		ptr( Display ) disp;
		Window xlib_window;
		h_surface surface;
		h_surface_capabilities surface_capabilities;
		h_surface_format surface_format;
		h_present_mode present_mode; )
	#endif
( in text in_name, fn_ptr( in_render_fn, pure ), in u32 in_width, in u32 in_height, in flag is_borderless )
{
	hept_window result = new_hept_window();
	result->core = current_hept_core;
	result->name = in_name;
	result->render_fn = in_render_fn;
	result->ready = no;
	result->visible = no;
	result->width = in_width;
	result->height = in_height;
	//
	#if OS_WINDOWS
	HWND hwnd = GetConsoleWindow();
	DWORD window_style = is_borderless ? WS_POPUP : WS_OVERLAPPEDWINDOW;
	RECT rect = { 0, 0, result->width, result->height };
	AdjustWindowRect( &rect, window_style, no );
	s32 this_width = rect.right - rect.left, this_height = rect.bottom - rect.top;

	WNDCLASS wc = {
			.lpfnWndProc = process_hept_window,
			.hInstance = GetModuleHandle( null ),
			.hbrBackground = CreateSolidBrush( RGB( 0, 0, 0 ) ),
			.lpszClassName = result->name };

	ifn( RegisterClass( &wc ) )
	{
		print( "COULD NOT MAKE WINDOW\n" );
		out null;
	}

	result->hwnd = CreateWindowEx(
			0,
			wc.lpszClassName,
			wc.lpszClassName,
			window_style,
			( GetSystemMetrics( SM_CXSCREEN ) - this_width ) / 2,
			( GetSystemMetrics( SM_CYSCREEN ) - this_height ) / 2,
			this_width,
			this_height,
			null,
			null,
			wc.hInstance,
			null );
	#elif OS_LINUX
	Display* disp = XOpenDisplay( NULL );
	if( disp == NULL )
	{
		print( "Cannot open display\n" );
		out null;
	}

	s32 screen_num = DefaultScreen( disp );
	s32 screen_width = DisplayWidth( disp, screen_num );
	s32 screen_height = DisplayHeight( disp, screen_num );

	Window root_win = RootWindow( disp, screen_num );

	result->xlib_window = XCreateSimpleWindow( disp, root_win, ( screen_width - result->width ) / 2, ( screen_height - result->height ) / 2, result->width, result->height, 1, BlackPixel( disp, screen_num ),
																						 BlackPixel( disp, screen_num ) );

	XStoreName( disp, result->xlib_window, result->name );

	result->disp = disp;
	#endif
	//
	#if OS_WINDOWS
	VkWin32SurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hwnd = result->hwnd;
	create_info.hinstance = result->inst;

	vkCreateWin32SurfaceKHR( result->core->instance, ref( create_info ), null, ref( result->surface ) );
	#elif OS_LINUX
	VkXlibSurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	create_info.dpy = result->display;
	create_info.window = result->window;

	vkCreateXlibSurfaceKHR( result->core->instance, ref( create_info ), null, ref( in_machine->surface ) );
	#elif OS_MACOS
	VkMacOSSurfaceCreateInfoMVK create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	create_info.pView = ( __bridge void* )( in_machine->window->window.contentView );

	vkCreateMacOSSurfaceMVK( in_machine->instance, ref( create_info ), null, ref( in_machine->surface ) );
	#endif
	//
	out result;
}

fn pure show_hept_window( in hept_window in_window )
{
	#if OS_WINDOWS
	ShowWindow( in_window->hwnd, SW_SHOWDEFAULT );
	UpdateWindow( in_window->hwnd );
	#elif OS_LINUX
	XMapWindow( in_window->disp, in_window->xlib_window );
	XFlush( in_window->disp );
	#endif
	in_window->visible = yes;
}

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

/////// /////// /////// /////// /////// /////// ///////

/// FORM

	#define FORM( NAME, ... )                          \
		PTR_PILE(                                        \
				form_##NAME,                                 \
				( in hept_machine in_machine ),              \
				{ this_form_##NAME->machine = in_machine; }, \
				hept_machine machine;                        \
				__VA_ARGS__ )                                \
		form_##NAME create_form_##NAME

//

/////// /////// /////// /////// /////// /////// ///////

/// form_mesh

PTR_PILE(
		form_mesh_attrib,
		( in h_format in_format, in u32 in_type_size, in u32 in_size, in text in_type_glsl ),
		{

		},
		h_format format;
		u32 type_size;
		u32 size;
		text type_glsl; )

//

FORM(
		mesh,
		u32 type_size;
		list attribs;
		text layout_glsl; )
( in list in_attribs )
{

}

//

/////// /////// /////// /////// /////// /////// ///////

/// form_image

make_enum( form_image_type ){
		form_image_rgba = VK_IMAGE_ASPECT_COLOR_BIT,
		form_image_depth = VK_IMAGE_ASPECT_DEPTH_BIT,
		form_image_stencil = VK_IMAGE_ASPECT_STENCIL_BIT,
};

FORM(
		image,
		enum( form_image_type ) type;
		h_format format;
		h_sampler sampler; )
( in hept_machine in_machine, in enum( form_image_type ) in_type, in h_format in_format )
{
	form_image result = new_form_image( in_machine );
	result->type = in_type;
	result->format = in_format;
	//
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
			.unnormalizedCoordinates = VK_FALSE };

	vkCreateSampler( result->machine->device, ref( sampler_info ), null, ref( result->sampler ) );
	//
	out result;
}

//

/////// /////// /////// /////// /////// /////// ///////

/// form_frame

make_enum( form_frame_layer_type ){
		form_frame_layer_rgba = VK_IMAGE_ASPECT_COLOR_BIT,
		form_frame_layer_depth = VK_IMAGE_ASPECT_DEPTH_BIT,
		form_frame_layer_stencil = VK_IMAGE_ASPECT_STENCIL_BIT,
};

PTR_PILE(
		form_frame_layer,
		( in enum( form_frame_layer_type ) in_type ),
		{
			this_form_frame_layer->type = in_type;
			//
			this_form_frame_layer->attach_ref.attachment = 0;
			this_form_frame_layer->attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
			with( this_form_frame_layer->type )
			{
				is( form_frame_layer_rgba )
				{
					this_form_frame_layer->attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					skip;
				}

				is( form_frame_layer_depth )
						is( form_frame_layer_stencil )
				{
					this_form_frame_layer->attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					skip;
				}
			}
		},
		enum( form_frame_layer_type ) type;
		h_attachment_reference attach_ref; )

//

make_enum( form_frame_type ){
		form_frame_present,
		form_frame_shader_read,
		form_frame_general,
};

FORM(
		frame,
		enum( form_frame_type ) type;
		h_format format;
		h_render_pass render_pass;
		list layers; )
( in hept_machine in_machine, in enum( form_frame_type ) in_type, in h_format in_format, in list in_layers )
{
	form_frame result = new_form_frame( in_machine );
	result->type = in_type;
	result->format = in_format;
	result->layers = in_layers;
	//
	ptr( h_attachment_reference ) attach_ref_depth_stencil = null;
	u32 rgba_count = 0;

	iter( result->layers->size, s )
	{
		form_frame_layer this_layer = list_get( result->layers, form_frame_layer, s );
		this_layer->attach_ref.attachment = s;
		with( this_layer->type )
		{
			is( form_frame_layer_rgba )
			{
				rgba_count++;
				skip;
			}

			is( form_frame_layer_depth )
					is( form_frame_layer_stencil )
			{
				attach_ref_depth_stencil = ref( this_layer->attach_ref );
				skip;
			}
		}
	}

	ptr( h_attachment_reference ) attach_ref_rgba = null;
	if( rgba_count )
	{
		attach_ref_rgba = new_mem( h_attachment_reference, rgba_count );

		iter( rgba_count, r )
		{
			form_frame_layer this_layer = list_get( result->layers, form_frame_layer, r );
			attach_ref_rgba[ r ] = this_layer->attach_ref;
		}
	}

	h_subpass_description subpass = { 0 };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = rgba_count;
	subpass.pColorAttachments = attach_ref_rgba;
	subpass.pDepthStencilAttachment = attach_ref_depth_stencil;

	ptr( h_attachment_description ) attachments = new_mem( h_attachment_description, result->layers->size );
	iter( result->layers->size, a )
	{
		attachments[ a ].format = result->format;
		attachments[ a ].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[ a ].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[ a ].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[ a ].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[ a ].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[ a ].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		with( result->type )
		{
			is( form_frame_present )
			{
				attachments[ a ].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				skip;
			}

			is( form_frame_shader_read )
			{
				attachments[ a ].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				skip;
			}

			is( form_frame_general )
			{
				attachments[ a ].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
				skip;
			}
		}
	}

	h_info_render_pass render_pass_info = h_make_info_render_pass( result->layers->size, attachments, 1, ref( subpass ), 0, null );
	result->render_pass = h_new_render_pass( result->machine->device, render_pass_info );
	//
	out result;
}

//

/////// /////// /////// /////// /////// /////// ///////

/// form_renderer

FORM(
		renderer,
		h_command_pool command_pool;
		h_queue queue; )
( in hept_machine in_machine )
{
	form_renderer result = new_form_renderer( in_machine );
	//
	result->queue = h_get_queue( result->machine->device, result->machine->queue_family_index, 0 );
	if( result->command_pool != null ) vkDestroyCommandPool( result->machine->device, result->command_pool, null ); //
	h_info_command_pool command_pool_info = h_make_info_command_pool( result->machine->queue_family_index );
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	result->command_pool = h_new_command_pool( result->machine->device, command_pool_info );
	//
	out result;
};

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

/////// /////// /////// /////// /////// /////// ///////

/// OBJECT

	#define OBJECT( NAME, IN_FORM, ... )    \
		PTR_PILE(                             \
				NAME,                             \
				( in IN_FORM in_form ),           \
				{ this_##NAME->form = in_form; }, \
				IN_FORM form;                     \
				__VA_ARGS__ )                     \
		NAME create_##NAME

//

/// OBJECT_FN

	#define OBJECT_FN( VERB, OBJ, ... )                                 \
		make_fn_ptr( ptr_##VERB##_##OBJ, OBJ, OBJ in_##OBJ __VA_ARGS__ ); \
		OBJ fn_##VERB##_##OBJ( OBJ in_##OBJ __VA_ARGS__ );                \
		ptr_##VERB##_##OBJ VERB##_##OBJ = fn_##VERB##_##OBJ;              \
		OBJ fn_##VERB##_##OBJ( OBJ in_##OBJ __VA_ARGS__ )

//

/////// /////// /////// /////// /////// /////// ///////

/// image

make_enum( image_state ){
		image_state_src,
		image_state_dst,
};

OBJECT(
		image,
		form_image,
		enum( image_state ) state;
		h_image ptr;
		h_image_view view;
		h_device_memory mem;
		u32 width;
		u32 height; )
( in form_image in_form, in enum( image_state ) in_state, in u32 in_width, in u32 in_height )
{
	image result = new_image( in_form );
	result->state = in_state;
	result->width = in_width;
	result->height = in_height;
	//
	h_extent_3d temp_extent = {
			.width = result->width,		//ceilf( f32( temp_image->width ) / 32. ) * 32.,
			.height = result->height, //ceilf( f32( temp_image->height ) / 32. ) * 32.,
			.depth = 1 };

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties( result->form->machine->physical_device, result->form->format, &formatProperties );

	if( !( formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT ) )
	{
		print( "Format cannot be used as color attachment!\n" );
	}

	h_info_image image_info = h_make_info_image(
			VK_IMAGE_TYPE_2D,
			temp_extent,
			1,
			1,
			result->form->format,
			( ( result->state == image_state_src ) ? ( VK_IMAGE_TILING_LINEAR ) : ( VK_IMAGE_TILING_OPTIMAL ) ), // OPTIMAL REQUIRES MULTIPLES OF 32x32
			VK_IMAGE_LAYOUT_UNDEFINED,
			( ( result->state == image_state_src ) ? ( VK_IMAGE_USAGE_SAMPLED_BIT ) : ( VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) ),
			VK_SHARING_MODE_EXCLUSIVE,
			VK_SAMPLE_COUNT_1_BIT );

	result->ptr = h_new_image( result->form->machine->device, image_info );

	h_mem_requirements mem_requirements;
	vkGetImageMemoryRequirements( result->form->machine->device, result->ptr, &mem_requirements );

	h_info_mem_allocate alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_requirements.size; //ceil( f64( mem_requirements.size ) / 1024. ) * 1024.;
	alloc_info.memoryTypeIndex = h_find_memory( result->form->machine->physical_device, mem_requirements.memoryTypeBits,
																							( ( result->state == image_state_src ) ? ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) : ( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) ) );

	vkAllocateMemory( result->form->machine->device, &alloc_info, null, ref( result->mem ) );

	vkBindImageMemory( result->form->machine->device, result->ptr, result->mem, 0 );
	//
	out result;
}

//

/// frame

OBJECT(
		frame,
		form_frame,
		h_framebuffer buffer;
		list images;
		list views; )
( in form_frame in_form, in list in_images )
{
	frame result = new_frame( in_form );
	result->images = in_images;
	result->views = new_list( h_image_view );
	//
	u32 max_w = 0, max_h = 0;
	iter( result->images->size, i )
	{
		image temp_image = list_get( result->images, image, i );
		max_w = max( max_w, temp_image->width );
		max_h = max( max_h, temp_image->height );
	}

	//

	iter( result->form->layers->size, v )
	{
		image temp_image = list_get( result->images, image, v );
		h_info_image_view image_view_info = h_make_info_image_view(
				temp_image->ptr,
				VK_IMAGE_VIEW_TYPE_2D,
				result->form->format,
				( ( h_component_mapping ){
						VK_COMPONENT_SWIZZLE_IDENTITY,
						VK_COMPONENT_SWIZZLE_IDENTITY,
						VK_COMPONENT_SWIZZLE_IDENTITY,
						VK_COMPONENT_SWIZZLE_IDENTITY } ),
				( ( h_image_subresource_range ){
						list_get( result->form->layers, form_frame_layer, v )->type,
						0, 1, 0, 1 } ) );
		list_add( result->views, h_image_view, h_new_image_view( result->form->machine->device, image_view_info ) );
	}

	h_info_framebuffer framebuffer_info = h_make_info_framebuffer(
			result->form->render_pass,
			result->views->size,
			( ptr( h_image_view ) )( result->views->data ),
			max_w,
			max_h,
			1 );
	result->buffer = h_new_framebuffer( result->form->machine->device, framebuffer_info );
	//
	out result;
}

//

/// renderer

OBJECT(
		renderer,
		form_renderer,
		flag changed;
		hept_window window;
		h_viewport viewport;
		h_swapchain swapchain;
		h_format swapchain_format;
		h_extent swapchain_extent;
		u32 current_frame;
		form_frame form_window;
		frame frame_window;
		list frames;
		u32 fence_id;
		ptr( h_command_buffer ) command_buffers;
		ptr( h_semaphore ) image_ready;
		ptr( h_semaphore ) image_done;
		ptr( h_fence ) flight_fences; )
( in form_renderer in_form, in hept_window in_window )
{
	renderer result = new_renderer( in_form );
	result->changed = yes;
	result->window = in_window;
	result->swapchain = null;
	result->current_frame = 0;
	result->frame_window = null;
	result->frames = new_list( frame );
	result->fence_id = 0;

	//
	out result;
};

OBJECT_FN( refresh, renderer )
{
	vkDeviceWaitIdle( in_renderer->form->machine->device );

	//

	if( in_renderer->swapchain != null ) vkDestroySwapchainKHR( in_renderer->form->machine->device, in_renderer->swapchain, null );

	if( in_renderer->frames->size != 0 )
	{
		iter( in_renderer->frames->size, i )
		{
			frame temp_frame = list_get( in_renderer->frames, frame, i );
			iter( temp_frame->images->size, j )
			{
				image temp_image = list_get( temp_frame->images, image, j );
				//delete_image( temp_image );
			}
			//delete_frame( temp_frame );
			vkDestroySemaphore( in_renderer->form->machine->device, in_renderer->image_ready[ i ], null );
			vkDestroySemaphore( in_renderer->form->machine->device, in_renderer->image_done[ i ], null );
			vkDestroyFence( in_renderer->form->machine->device, in_renderer->flight_fences[ i ], null );
		}
		in_renderer->frames->size = 0;
	}

	//

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( in_renderer->form->machine->physical_device, in_renderer->window->surface, ref( in_renderer->window->surface_capabilities ) );

	h_info_swapchain swapchain_info = h_make_info_swapchain(
			in_renderer->window->surface,
			in_renderer->window->surface_capabilities.minImageCount + 1,
			in_renderer->window->surface_format.format,
			in_renderer->window->surface_format.colorSpace,
			in_renderer->window->surface_capabilities.currentExtent,
			1,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			null,
			in_renderer->window->surface_capabilities.currentTransform,
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			in_renderer->window->present_mode, VK_TRUE,
			null );

	in_renderer->swapchain = h_new_swapchain( in_renderer->form->machine->device, swapchain_info );
	in_renderer->swapchain_format = in_renderer->window->surface_format.format;
	in_renderer->swapchain_extent = in_renderer->window->surface_capabilities.currentExtent;

	in_renderer->viewport = h_make_viewport(
			0.0,
			0.0,
			f32( in_renderer->swapchain_extent.width ),
			f32( in_renderer->swapchain_extent.height ),
			0.0,
			1.0 );

	u32 temp_count = 0;
	ptr( h_image ) temp_images = null;
	vkGetSwapchainImagesKHR( in_renderer->form->machine->device, in_renderer->swapchain, ref( temp_count ), null );
	temp_images = new_mem( h_image, temp_count );
	vkGetSwapchainImagesKHR( in_renderer->form->machine->device, in_renderer->swapchain, ref( temp_count ), temp_images );

	list layers = new_list( form_frame_layer );
	form_frame_layer layer_rgba = new_form_frame_layer( form_frame_layer_rgba );
	list_add( layers, form_frame_layer, layer_rgba );

	//if( in_renderer->form != null ) delete_frame_form( in_renderer->form );
	in_renderer->form_window = create_form_frame(
			in_renderer->form->machine,
			form_frame_present,
			in_renderer->swapchain_format,
			layers );

	iter( temp_count, i )
	{
		list temp_list_images = new_list( image );
		image temp_image = new_image( null );
		temp_image->ptr = temp_images[ i ];
		temp_image->width = in_renderer->swapchain_extent.width;
		temp_image->height = in_renderer->swapchain_extent.height;
		list_add( temp_list_images, image, temp_image );
		frame temp_frame = create_frame( in_renderer->form_window, temp_list_images );
		list_add( in_renderer->frames, frame, temp_frame );
	}

	//

	in_renderer->image_ready = new_mem( h_semaphore, in_renderer->frames->size );
	in_renderer->image_done = new_mem( h_semaphore, in_renderer->frames->size );
	in_renderer->flight_fences = new_mem( h_fence, in_renderer->frames->size );
	in_renderer->command_buffers = new_mem( h_command_buffer, in_renderer->frames->size );

	h_info_semaphore semaphore_info = h_make_info_semaphore();
	h_info_fence fence_info = h_make_info_fence();
	h_info_command_buffer command_buffers_info = h_make_info_command_buffer(
			in_renderer->form->command_pool,
			h_command_buffer_level_primary,
			in_renderer->frames->size );

	h_allocate_command_buffers( in_renderer->form->machine->device, command_buffers_info, in_renderer->command_buffers );

	iter( in_renderer->frames->size, i )
	{
		in_renderer->image_ready[ i ] = h_new_semaphore( in_renderer->form->machine->device, semaphore_info );
		in_renderer->image_done[ i ] = h_new_semaphore( in_renderer->form->machine->device, semaphore_info );
		in_renderer->flight_fences[ i ] = h_new_fence( in_renderer->form->machine->device, fence_info );
	}
	//

	in_renderer->current_frame = 0;
	in_renderer->fence_id = 0;

	//

	if( in_renderer->frame_window != null )
	{
		iter( in_renderer->frame_window->images->size, j )
		{
			image temp_image = list_get( in_renderer->frame_window->images, image, j );
			//delete_image( temp_image );
		}
		//delete_frame( in_renderer->frame_window );
	}

	form_image frame_image_form = create_form_image(
			in_renderer->form->machine,
			form_image_rgba,
			in_renderer->swapchain_format );

	image temp_image = create_image(
			frame_image_form,
			image_state_dst,
			in_renderer->swapchain_extent.width,
			in_renderer->swapchain_extent.height );
	//
	list temp_list_images = new_list( image );
	list_add( temp_list_images, image, temp_image );
	in_renderer->frame_window = create_frame( in_renderer->form_window, temp_list_images );

	in_renderer->changed = no;
}

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

/////// /////// /////// /////// /////// /////// ///////

/// global main defaults

global hept_core main_hept_core = null;
global hept_machine main_machine = null;
global hept_window main_hept_window = null;

global form_renderer main_form_renderer = null;

/////// /////// /////// /////// /////// /////// ///////

/// main setup

fn pure main_init( in hept_machine in_machine );
fn pure main_draw( in hept_machine in_machine );
fn pure main_step( in hept_machine in_machine );

//

/// global piles update

fn pure main_update_hept_machines()
{
	iter_pile( pile_hept_machine, m )
	{
		maybe maybe_machine = pile_find( pile_hept_machine, hept_machine, m );
		ifn( maybe_machine.valid ) next;
		hept_machine this_machine = cast( maybe_machine.value, hept_machine );
		if( this_machine->physical_device != null ) next;
		//
		this_machine->queue_family_index = u32_max;

		u32 physical_devices_count = 0;
		vkEnumeratePhysicalDevices( this_machine->core->instance, ref( physical_devices_count ), null );
		ptr( h_physical_device ) physical_devices = new_mem( h_physical_device, physical_devices_count );
		vkEnumeratePhysicalDevices( this_machine->core->instance, ref( physical_devices_count ), physical_devices );

		h_physical_device integrated = null;
		iter( physical_devices_count, i )
		{
			h_physical_device_properties dev_prop;
			vkGetPhysicalDeviceProperties( physical_devices[ i ], ref( dev_prop ) );

			if( dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU or
					dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU )
			{
				u32 queue_family_n = 0;
				vkGetPhysicalDeviceQueueFamilyProperties( physical_devices[ i ], ref( queue_family_n ), null );
				ptr( VkQueueFamilyProperties ) queue_family_prop = new_mem( VkQueueFamilyProperties, queue_family_n );
				vkGetPhysicalDeviceQueueFamilyProperties( physical_devices[ i ], ref( queue_family_n ), queue_family_prop );
				iter( queue_family_n, j )
				{
					this_machine->queue_family_index = j;
					VkBool32 support_present;
					vkGetPhysicalDeviceSurfaceSupportKHR( physical_devices[ i ], j, current_hept_window->surface, ref( support_present ) );

					if( ( queue_family_prop[ j ].queueFlags & VK_QUEUE_GRAPHICS_BIT ) and ( queue_family_prop[ j ].queueFlags & VK_QUEUE_COMPUTE_BIT ) and support_present )
					{
						if( dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
						{
							this_machine->physical_device = physical_devices[ i ];
							skip;
						}
						elif( integrated == null )
						{
							integrated = physical_devices[ i ];
						}
					}
				}
				free_mem( queue_family_prop );
			}

			if( this_machine->physical_device != null )
			{
				skip;
			}
		}
		free_mem( physical_devices );

		if( this_machine->physical_device == null )
		{
			if( integrated != null )
			{
				this_machine->physical_device = integrated;
			}
			else
			{
				print( "Failed to find a suitable GPU\n" );
			}
		}

		f32 queue_priority = 1.0f;
		h_info_device_queue device_queue = h_make_info_device_queue( this_machine->queue_family_index, 1, ref( queue_priority ) );

		h_physical_device_features features;
		vkGetPhysicalDeviceFeatures( this_machine->physical_device, ref( features ) );

		//

		text device_ext[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		h_info_device info_device = h_make_info_device( 1, ref( device_queue ), 0, null, 1, ( ptr( const char ) ptr( const ) )device_ext, ref( features ) );

		this_machine->device = h_new_device( this_machine->physical_device, info_device );
	}
}

fn pure main_update_hept_windows( in hept_machine in_machine )
{
	iter_pile( pile_hept_window, w )
	{
		maybe maybe_window = pile_find( pile_hept_window, hept_window, w );
		ifn( maybe_window.valid ) next;
		hept_window this_window = cast( maybe_window.value, hept_window );

		//

	#if OS_WINDOWS
		once MSG msg = { 0 };

		while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if( msg.message == WM_QUIT )
			{
				//exit( 0 );
				skip;
			}
			else
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}

	#elif OS_LINUX
		process_hept_window( in_window->disp );
	#endif

		//

		u32 format_n = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR( in_machine->physical_device, this_window->surface, ref( format_n ), null );
		ptr( h_surface_format ) formats = new_mem( h_surface_format, format_n );
		vkGetPhysicalDeviceSurfaceFormatsKHR( in_machine->physical_device, this_window->surface, ref( format_n ), formats );
		this_window->surface_format = formats[ 0 ];
		free_mem( formats );

		u32 present_mode_n = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR( in_machine->physical_device, this_window->surface, ref( present_mode_n ), null );
		ptr( h_present_mode ) present_modes = new_mem( h_present_mode, present_mode_n );
		vkGetPhysicalDeviceSurfacePresentModesKHR( in_machine->physical_device, this_window->surface, ref( present_mode_n ), present_modes );
		iter( present_mode_n, i )
		{
			if( present_modes[ i ] == h_present_mode_vsync_optimal )
			{
				this_window->present_mode = h_present_mode_vsync_optimal;
				skip;
			}
		}
		free_mem( present_modes );

		//

		if( this_window->ready )
		{
			ifn( this_window->visible )
			{
				show_hept_window( this_window );
			}
		}
		else
		{
			create_renderer( main_form_renderer, this_window );
			this_window->ready = yes;
		}
	}
}

//

fn pure main_update_renderers()
{
	iter_pile( pile_renderer, r )
	{
		maybe maybe_renderer = pile_find( pile_renderer, renderer, r );
		ifn( maybe_renderer.valid ) next;

		renderer this_renderer = cast( maybe_renderer.value, renderer );
		current_renderer = this_renderer;

		//

		if( this_renderer->changed ) refresh_renderer( this_renderer );

		//

		{
			vkWaitForFences( this_renderer->form->machine->device, 1, ref( this_renderer->flight_fences[ this_renderer->fence_id ] ), VK_TRUE, UINT64_MAX );

			VkResult aquire_result = vkAcquireNextImageKHR(
					this_renderer->form->machine->device, this_renderer->swapchain, UINT64_MAX,
					this_renderer->image_ready[ this_renderer->current_frame ],
					VK_NULL_HANDLE, ref( this_renderer->current_frame ) );

			if( aquire_result == VK_ERROR_OUT_OF_DATE_KHR || aquire_result == VK_SUBOPTIMAL_KHR )
			{
				//recreateSwapChain();
				//return;
				//print( "RESIZE2" );

				//vkResetFences( this_renderer->form->machine->device, 1, ref( this_renderer->flight_fences[ this_fence ] ) );
				//renderer_update( this_renderer );
				//this_fence = 0;
				//vkResetFences( this_renderer->form->machine->device, 1, ref( this_renderer->flight_fences[ this_renderer->fence_id ] ) );
				refresh_renderer( this_renderer );
				next;
				//loop{};
				//abort();
				/*
				refresh_renderer(this_renderer);
				this_renderer->fence_id = 0;
				next;*/
			}

			vkResetFences( this_renderer->form->machine->device, 1, ref( this_renderer->flight_fences[ this_renderer->fence_id ] ) );

			this_renderer->fence_id = ( this_renderer->current_frame + 1 ) % this_renderer->frames->size;
		}

		//

		frame this_frame = list_get( this_renderer->frames, frame, this_renderer->current_frame );
		image this_image = list_get( this_frame->images, image, 0 );

		VkCommandBufferBeginInfo begin_info = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

		vkBeginCommandBuffer(
				this_renderer->command_buffers[ this_renderer->current_frame ],
				ref( begin_info ) );
		//
		{
			h_scissor scissor = { 0 };
			scissor.offset = ( VkOffset2D ){ 0, 0 };
			scissor.extent = this_renderer->swapchain_extent;
			//scissor.extent.width /= 2;

			vkCmdSetScissor( this_renderer->command_buffers[ this_renderer->current_frame ], 0, 1, ref( scissor ) );
			h_viewport viewport = h_make_viewport(
					0.0,
					0.0,
					f32( this_renderer->swapchain_extent.width ),
					f32( this_renderer->swapchain_extent.height ),
					0.0,
					1.0 );
			vkCmdSetViewport( this_renderer->command_buffers[ this_renderer->current_frame ], 0, 1, ref( viewport ) );
		}
		//

		this_renderer->window->render_fn();

		//

		if( 1 )
		{
			image temp_image = list_get( this_renderer->frame_window->images, image, 0 );

			if( 1 )
			{ //
				VkImageMemoryBarrier barrier = { 0 };
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.image = temp_image->ptr; //default_frame_image->ptr;

				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				barrier.subresourceRange.baseMipLevel = 0;
				barrier.subresourceRange.levelCount = 1;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;

				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

				vkCmdPipelineBarrier( this_renderer->command_buffers[ this_renderer->current_frame ], src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier );

				barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.image = this_image->ptr;
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

				vkCmdPipelineBarrier( this_renderer->command_buffers[ this_renderer->current_frame ], src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier );
			}

			//

			VkImageBlit blit = {};
			blit.srcOffsets[ 0 ] = ( h_offset_3d ){ 0, 0, 0 };
			blit.srcOffsets[ 1 ] = ( h_offset_3d ){ temp_image->width, temp_image->height, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = 0;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;

			blit.dstOffsets[ 0 ] = ( h_offset_3d ){ 0, 0, 0 };
			blit.dstOffsets[ 1 ] = ( h_offset_3d ){ this_renderer->swapchain_extent.width, this_renderer->swapchain_extent.height, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = 0;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(
					this_renderer->command_buffers[ this_renderer->current_frame ],
					temp_image->ptr, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					this_image->ptr, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &blit,
					VK_FILTER_LINEAR );
		}

		if( 1 )
		{ //
			VkImageMemoryBarrier barrier = { 0 };
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = this_image->ptr;

			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = 0;

			VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

			vkCmdPipelineBarrier( this_renderer->command_buffers[ this_renderer->current_frame ], src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier );
		}

		vkEndCommandBuffer( this_renderer->command_buffers[ this_renderer->current_frame ] );

		//
	}
}

//

/////// /////// /////// /////// /////// /////// ///////

/// hept setup

fn pure hept_init( in hept_machine in_machine )
{
	//main_hept_window = create_hept_window( in_machine->core, "hept", main_draw, 640, 480, no );
	//list_add( in_machine->windows, hept_window, main_hept_window );

	//list_add( in_machine->windows, hept_window, main_hept_window2 );
	//update_hept_machine( in_machine );
	//main_update_hept_machines();
	main_update_hept_machines();
	main_form_renderer = create_form_renderer( in_machine );
}

fn pure hept_draw( in hept_machine in_machine )
{
	main_update_hept_windows( in_machine );
	main_update_renderers();
}

fn pure hept_present( in hept_machine in_machine )
{
	iter_pile( pile_renderer, r )
	{
		maybe maybe_renderer = pile_find( pile_renderer, renderer, r );
		ifn( maybe_renderer.valid ) next;
		renderer this_renderer = maybe_renderer.value;

		//

		VkPipelineStageFlags wait_stages[] = {
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		h_info_submit submit_info = h_make_info_submit(
				1, ref( this_renderer->image_ready[ this_renderer->current_frame ] ),
				wait_stages,
				1, ref( this_renderer->command_buffers[ this_renderer->current_frame ] ),
				1, ref( this_renderer->image_done[ this_renderer->current_frame ] ) );

		h_submit_queue(
				this_renderer->form->queue, submit_info,
				this_renderer->flight_fences[ this_renderer->current_frame ] );

		//

		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = ref( this_renderer->image_done[ this_renderer->current_frame ] );
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = ref( this_renderer->swapchain );
		presentInfo.pImageIndices = ref( this_renderer->current_frame );

		VkResult present_result = vkQueuePresentKHR( this_renderer->form->queue, ref( presentInfo ) );

		if( present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR )
		{
			//update_renderer( this_renderer );
			//vkQueueWaitIdle( this_renderer->form->queue );
			//this_fence = 0;
			//this_renderer->current_frame = 0;
			refresh_renderer( this_renderer );
			next;
			//this_renderer->fence_id = 0;
			//exit(0);
			//abort();
		}

		this_renderer->current_frame = ( this_renderer->current_frame + 1 ) % this_renderer->frames->size;
	}
}

fn pure hept_step( in hept_machine in_machine )
{
	//
}

fn pure hept_piles()
{
	// hept
	pile_hept_core = new_pile( hept_core );
	pile_hept_window = new_pile( hept_window );
	pile_hept_machine = new_pile( hept_machine );
	// form
	pile_form_image = new_pile( form_image );
	pile_form_frame_layer = new_pile( form_frame_layer );
	pile_form_frame = new_pile( form_frame );
	pile_form_renderer = new_pile( form_renderer );
	// object
	pile_image = new_pile( image );
	pile_frame = new_pile( frame );
	pile_renderer = new_pile( renderer );
}

	#if OS_WINDOWS
fn LRESULT CALLBACK process_hept_window( in HWND hwnd, in UINT u_msg, in WPARAM w_param, in LPARAM l_param )
{
	renderer temp_renderer = null;

	if( u_msg == WM_DESTROY )
	{
		PostQuitMessage( 0 );
		out 0;
	}
	else
	{
		switch( u_msg )
		{
			case WM_SIZE:
				refresh_renderer( current_renderer );
				hept_draw( current_hept_machine );
				hept_present( current_hept_machine );
				//current_renderer->fence_id = 0;
				//renderer_update(default_renderer); // recreate swapchain
				//this_fence = 0;
				//main_draw();
				//resizing = false;
				//return 1;
				//abort();
				skip; //out 1;
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				//key_state[ w_param ] = true;
				skip;
			case WM_KEYUP:
			case WM_SYSKEYUP:
				//key_state[ w_param ] = false;
				skip;
		}
	}
	out DefWindowProc( hwnd, u_msg, w_param, l_param );
}
	#elif OS_LINUX
fn pure process_hept_window( ptr( Display ) in_disp )
{
	XEvent e;
	spin( XPending( in_disp ) )
	{
		XNextEvent( in_disp, &e );
		if( e.type == Expose )
		{
			// Redraw here
		}
		else if( e.type == ClientMessage )
		{
			// Handle close event, etc
			out;
		}
	}
}
	#elif OS_MACOS
	//
	#endif

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

/////// /////// /////// /////// /////// /////// ///////

/// main

fn pure main_core()
{
	main_hept_core = create_hept_core( "hept" );
	main_machine = create_hept_machine();

	main_init( main_machine );
	hept_init( main_machine );

	loop
	{
		hept_draw( main_machine );
		hept_present( main_machine );
	}
}

//

	#define init      \
		int main()      \
		{               \
			hept_piles(); \
			main_core();  \
			out 0;        \
		}               \
		fn pure main_init( in hept_machine in_machine )

//#define draw fn pure main_draw( in hept_machine in_machine )

//#define step fn pure main_step( in hept_machine in_machine )

#endif

/////// /////// /////// /////// /////// /////// ///////