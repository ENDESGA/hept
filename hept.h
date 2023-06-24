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

/// OBJECT template

/*

OBJECT(
		name,
		type element;
		)
( in type in_type)
{
	name temp_name = global_new_name();
	//
	
	//
	out temp_name;
}

*/

#define OBJECT( OBJ, STRUCT_VARIABLES )                                           \
                                                                                  \
	make_struct                                                                     \
	{                                                                               \
		spinlock lock;                                                                \
		u32 pool_id;                                                                  \
		str object_name;                                                              \
		STRUCT_VARIABLES                                                              \
	}                                                                               \
	struct_##OBJ;                                                                   \
	make_ptr( struct_##OBJ ) OBJ;                                                   \
                                                                                  \
	global OBJ current_##OBJ = null;                                                \
	global pool global_pool_##OBJ = null;                                           \
                                                                                  \
	fn pure OBJ##_lock( in OBJ OBJ##_to_lock )                                      \
	{                                                                               \
		engage_spinlock( OBJ##_to_lock->lock );                                       \
	}                                                                               \
	fn pure OBJ##_unlock( in OBJ OBJ##_to_unlock )                                  \
	{                                                                               \
		vacate_spinlock( OBJ##_to_unlock->lock );                                     \
	}                                                                               \
                                                                                  \
	fn pure OBJ##_set_current( in OBJ set_this_##OBJ##_current )                    \
	{                                                                               \
		safe_ptr_set( current_##OBJ, set_this_##OBJ##_current );                      \
	}                                                                               \
	fn OBJ OBJ##_get_current()                                                      \
	{                                                                               \
		out( OBJ ) safe_ptr_get( current_##OBJ );                                     \
	}                                                                               \
                                                                                  \
	fn OBJ global_new_##OBJ()                                                       \
	{                                                                               \
		OBJ OBJECT_##OBJ = new_mem( struct_##OBJ, 1 );                                \
		OBJECT_##OBJ->object_name = #OBJ;                                             \
		OBJ##_set_current( OBJECT_##OBJ );                                            \
		lock_pool( global_pool_##OBJ );                                               \
		pool_add( global_pool_##OBJ, OBJ, OBJECT_##OBJ );                             \
		safe_u32_set( OBJECT_##OBJ->pool_id, global_pool_##OBJ->pos );                \
		unlock_pool( global_pool_##OBJ );                                             \
		out OBJECT_##OBJ;                                                             \
	}                                                                               \
                                                                                  \
	fn pure delete_##OBJ( in OBJ delete_this_##OBJ )                                \
	{                                                                               \
		if( delete_this_##OBJ == OBJ##_get_current() ) OBJ##_set_current( null );     \
		lock_pool( global_pool_##OBJ );                                               \
		pool_delete( global_pool_##OBJ, safe_u32_get( delete_this_##OBJ->pool_id ) ); \
		unlock_pool( global_pool_##OBJ );                                             \
		free_mem( delete_this_##OBJ );                                                \
	}                                                                               \
                                                                                  \
	fn OBJ new_##OBJ

#define OBJECT_NEW CURRENT_OBJECT

#define new( OBJ ) new_##OBJ()

//

ptr( str ) get_files_in_directory( const char* dir_path )
{
	WIN32_FIND_DATA find_data;
	HANDLE h_find;
	ptr( str ) file_list = NULL;
	size_t num_files = 0;
	char search_path[ MAX_PATH ];

	sprintf( search_path, "%s\\*", dir_path );

	h_find = FindFirstFile( search_path, &find_data );
	if( h_find != INVALID_HANDLE_VALUE )
		{
			do
				{
					if( strcmp( find_data.cFileName, "." ) != 0 && strcmp( find_data.cFileName, ".." ) != 0 )
						{
							file_list = realloc( file_list, size_( *file_list ) * ( num_files + 1 ) );
							if( !file_list )
								{
									fprintf( stderr, "Memory allocation error\n" );
									exit( EXIT_FAILURE );
								}

							file_list[ num_files ] = malloc( strlen( find_data.cFileName ) + 1 );
							if( !file_list[ num_files ] )
								{
									fprintf( stderr, "Memory allocation error\n" );
									exit( EXIT_FAILURE );
								}

							strcpy( file_list[ num_files ], find_data.cFileName );
							num_files++;
						}
			} while( FindNextFile( h_find, &find_data ) != 0 );

			FindClose( h_find );
		}

	// null terminate the list
	file_list = realloc( file_list, size_( *file_list ) * ( num_files + 1 ) );
	if( !file_list )
		{
			fprintf( stderr, "Memory allocation error\n" );
			exit( EXIT_FAILURE );
		}
	file_list[ num_files ] = NULL;

	out file_list;
}

//

/// file

make_struct
{
	str name;
	str data;
	u32 size;
}
os_file;

os_file file_load( in str file_name )
{
#ifdef OS_WINDOWS
	HANDLE file = CreateFile( ( LPCSTR )file_name, GENERIC_READ, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null );
	if( file == INVALID_HANDLE_VALUE )
		{
			fprintf( stderr, "Failed to open file: %s\n", file_name );
		}

	DWORD file_size_low, file_size_high;
	file_size_low = GetFileSize( file, adr( file_size_high ) );

	HANDLE file_mapping = CreateFileMapping( file, null, PAGE_READONLY, 0, 0, null );
	pure_ptr file_data = MapViewOfFile( file_mapping, FILE_MAP_READ, 0, 0, 0 );
	CloseHandle( file_mapping );
	CloseHandle( file );

	out( os_file ){ file_name, file_data, file_size_low | ( ( size_t )file_size_high << 32 ) };
#elif OS_LINUX
	//
#elif OS_MACOS
	//
#endif
}

fn pure file_save( str filename, str text )
{
	HANDLE file = CreateFileA( filename, GENERIC_WRITE, 0, null, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, null );
	if( file == INVALID_HANDLE_VALUE )
		{
			fprintf( stderr, "Failed to open file: %s\n", filename );
			out;
		}

	DWORD text_length = ( DWORD )strlen( text );
	DWORD bytes_written;

	if( !WriteFile( file, text, text_length, adr( bytes_written ), null ) )
		{
			fprintf( stderr, "Failed to write to file: %s\n", filename );
		}

	CloseHandle( file );
}

//

fn h_command_buffer begin_single_time_commands( h_device in_device, h_command_pool in_command_pool )
{
	h_info_command_buffer alloc_info = h_make_info_command_buffer(
			in_command_pool,
			h_command_buffer_level_primary );

	h_command_buffer command_buffer = h_new_command_buffer( in_device, alloc_info );

	VkCommandBufferBeginInfo begin_info = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };

	vkBeginCommandBuffer( command_buffer, &begin_info );

	out command_buffer;
}

fn pure end_single_time_commands( in h_device device, in h_command_pool command_pool, in h_queue graphics_queue, in h_command_buffer command_buffer )
{
	vkEndCommandBuffer( command_buffer );

	VkSubmitInfo submit_info = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &command_buffer };

	vkQueueSubmit( graphics_queue, 1, &submit_info, VK_NULL_HANDLE );
	vkQueueWaitIdle( graphics_queue );

	vkFreeCommandBuffers( device, command_pool, 1, &command_buffer );
}

//

/// window

global s32 key_state[ 256 ] = { 0 };

#if OS_WINDOWS
fn LRESULT CALLBACK window_proc( in HWND hwnd, in UINT u_msg, in WPARAM w_param, in LPARAM l_param );

OBJECT(
		window,
		str name;
		u32 w;
		u32 h;
		HWND hwnd;
		HINSTANCE inst; )
#elif OS_LINUX
OBJECT(
		window,
		str name;
		u32 w;
		u32 h;
		ptr( Display ) display;
		Window win; )
#elif OS_MACOS
OBJECT(
		window,
		str name;
		u32 w;
		u32 h;
		ptr( NSWindow ) win; )
#endif
( in str in_name, in u32 in_width, in u32 in_height )
{
	window temp_window = global_new_window();
	//
	temp_window->w = in_width;
	temp_window->h = in_height;
	//

	bool is_borderless = 0;

#if OS_WINDOWS
	temp_window->inst = GetModuleHandle( NULL );
	WNDCLASSEX wc = {
			.cbSize = sizeof( WNDCLASSEX ),
			.style = CS_HREDRAW | CS_VREDRAW,
			.lpfnWndProc = ( WNDPROC )window_proc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = temp_window->inst,
			.hIcon = NULL,
			.hCursor = LoadCursor( NULL, IDC_ARROW ),
			.hbrBackground = NULL,//CreateSolidBrush( RGB( 0, 0, 0 ) ),
			.lpszMenuName = NULL,
			.lpszClassName = in_name,
			.hIconSm = NULL };
	RegisterClassEx( &wc );

	RECT rect = { 0, 0, temp_window->w, temp_window->h };
	DWORD windowStyle = is_borderless ? WS_POPUP : WS_OVERLAPPEDWINDOW;
	DWORD exWindowStyle = is_borderless ? 0 : WS_EX_OVERLAPPEDWINDOW;

	AdjustWindowRectEx( &rect, windowStyle, FALSE, exWindowStyle );

	temp_window->hwnd = CreateWindowEx(
			exWindowStyle,
			wc.lpszClassName,
			wc.lpszClassName,
			windowStyle,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			rect.right - rect.left,
			rect.bottom - rect.top,
			NULL,
			NULL,
			temp_window->inst,
			NULL );

#elif OS_LINUX
	temp_window->display = XOpenDisplay( NULL );
	temp_window->win = XCreateSimpleWindow( temp_window->display, DefaultRootWindow( temp_window->display ), 0, 0, temp_window->w, temp_window->h, 1, BlackPixel( temp_window->display, 0 ), WhitePixel( temp_window->display, 0 ) );
	XMapWindow( temp_window->display, temp_window->win );
	XSelectInput( temp_window->display, temp_window->win, KeyPressMask | KeyReleaseMask );
#elif OS_MACOS
	NSRect frame = NSMakeRect( 0, 0, temp_window->w, temp_window->h );
	temp_window->win = [[NSWindow alloc] initWithContentRect:frame styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
	[temp_window->win makeKeyAndOrderFront:NSApp];
#endif
	//
	out temp_window;
}

fn bool window_update( in window in_window )
{
#if OS_WINDOWS
	once MSG msg = { 0 };
	while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if( msg.message == WM_QUIT )
				{
					break;
				}
			else
				{
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
		}
#elif OS_LINUX
//
#elif OS_MACOS
//
#endif
	out 1;
}

fn pure window_show( in window in_window )
{
#if OS_WINDOWS
	ShowWindow( in_window->hwnd, SW_SHOWDEFAULT );
	UpdateWindow( in_window->hwnd );
#elif OS_LINUX
	XMapRaised( in_window->display, in_window->win );
#elif OS_MACOS
	[in_window->win makeKeyAndOrderFront:NSApp];
#endif
}


//

OBJECT(
		machine,
		window window;
		h_instance instance;
		h_surface surface;
		h_surface_capabilities surface_capabilities;
		h_surface_format surface_format;
		h_present_mode present_mode;
		u32 queue_family_index;
		h_physical_device physical_device;
		h_device device; )
( in str in_name )
{
	machine temp_machine = global_new_machine();
	temp_machine->window = new_window( in_name, 512, 256 );

	volkInitialize();

	//

	h_info_app info_app = h_make_info_app(
			in_name,
			h_make_version( 1, 0, 0 ),
			"hept",
			h_make_version( 1, 0, 0 ),
			h_make_version( 1, 3, 0 ) );

	str desired_debug_layers[] = {
			"VK_LAYER_KHRONOS_validation",
			//"VK_LAYER_LUNARG_api_dump",
			//"VK_LAYER_LUNARG_device_simulation",
			//"VK_LAYER_LUNARG_monitor",
			//"VK_LAYER_LUNARG_screenshot"
	};
	u32 desired_debug_layers_count = 1;

	u32 debug_layer_count = 0;
	u32 enabled_debug_layer_count = 0;
	vkEnumerateInstanceLayerProperties( adr( debug_layer_count ), null );
	ptr( h_layer_properties ) available_layers = new_mem( h_layer_properties, debug_layer_count );
	vkEnumerateInstanceLayerProperties( adr( debug_layer_count ), available_layers );
	ptr( str ) debug_layers = new_mem( str, desired_debug_layers_count );

	iter( desired_debug_layers_count, i )
	{
		iter( debug_layer_count, j )
		{
			if( strcmp( desired_debug_layers[ i ], available_layers[ j ].layerName ) == 0 )
				{
					( debug_layers )[ enabled_debug_layer_count++ ] = desired_debug_layers[ i ];
					break;
				}
		}
	}
	free( available_layers );

	u32 extension_count = 2;
	str extensions[] =
			{
					VK_KHR_SURFACE_EXTENSION_NAME,
					VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
			};

	h_info_instance instance_info = h_make_info_instance(
			adr( info_app ),
			enabled_debug_layer_count,
			( ptr( const char ) ptr( const ) )debug_layers,
			extension_count,
			( ptr( const char ) ptr( const ) )extensions );

	temp_machine->instance = h_new_instance( instance_info );

	//

#if OS_WINDOWS
	VkWin32SurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hwnd = temp_machine->window->hwnd;
	create_info.hinstance = temp_machine->window->inst;

	vkCreateWin32SurfaceKHR( temp_machine->instance, adr( create_info ), null, adr( temp_machine->surface ) );
#elif OS_LINUX
	VkXlibSurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	create_info.dpy = in_machine->window->display;
	create_info.window = in_machine->window->window;

	vkCreateXlibSurfaceKHR( in_machine->instance, adr( create_info ), null, adr( in_machine->surface ) );
#elif OS_MACOS
	VkMacOSSurfaceCreateInfoMVK create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	create_info.pView = ( __bridge void* )( in_machine->window->window.contentView );

	vkCreateMacOSSurfaceMVK( in_machine->instance, adr( create_info ), null, adr( in_machine->surface ) );
#endif

	//

	temp_machine->physical_device = null;
	temp_machine->queue_family_index = u32_max;

	u32 physical_devices_count = 0;
	vkEnumeratePhysicalDevices( temp_machine->instance, adr( physical_devices_count ), null );
	ptr( h_physical_device ) physical_devices = new_mem( h_physical_device, physical_devices_count );
	vkEnumeratePhysicalDevices( temp_machine->instance, adr( physical_devices_count ), physical_devices );

	h_physical_device integrated = null;
	iter( physical_devices_count, i )
	{
		h_physical_device_properties dev_prop;
		vkGetPhysicalDeviceProperties( physical_devices[ i ], adr( dev_prop ) );

		if( dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU or
				dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU )
			{
				u32 queue_family_n = 0;
				vkGetPhysicalDeviceQueueFamilyProperties( physical_devices[ i ],
																									adr( queue_family_n ), null );

				ptr( VkQueueFamilyProperties ) queue_family_prop =
						new_mem( VkQueueFamilyProperties, queue_family_n );

				vkGetPhysicalDeviceQueueFamilyProperties(
						physical_devices[ i ], adr( queue_family_n ), queue_family_prop );

				iter( queue_family_n, j )
				{
					temp_machine->queue_family_index = j;
					VkBool32 support_present;
					vkGetPhysicalDeviceSurfaceSupportKHR( physical_devices[ i ], j, temp_machine->surface, adr( support_present ) );

					if( ( queue_family_prop[ j ].queueFlags & VK_QUEUE_GRAPHICS_BIT ) and ( queue_family_prop[ j ].queueFlags & VK_QUEUE_COMPUTE_BIT ) and support_present )
						{
							if( dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
								{
									temp_machine->physical_device = physical_devices[ i ];

									break;
								}
							elif( integrated == null )
							{
								integrated = physical_devices[ i ];
							}
						}
				}
				free( queue_family_prop );
			}

		if( temp_machine->physical_device != null )
			{
				break;
			}
	}
	free( physical_devices );

	if( temp_machine->physical_device == null )
		{
			if( integrated != null )
				{
					temp_machine->physical_device = integrated;
				}
			else
				{
					print( "Failed to find a suitable GPU\n" );
				}
		}

	f32 queue_priority = 1.0f;
	h_info_device_queue device_queue = h_make_info_device_queue( temp_machine->queue_family_index, 1, adr( queue_priority ) );

	h_physical_device_features features;
	vkGetPhysicalDeviceFeatures( temp_machine->physical_device, adr( features ) );

	//

	str device_ext[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	h_info_device info_device = h_make_info_device( 1, adr( device_queue ), 0, null, 1, ( ptr( const char ) ptr( const ) )device_ext, adr( features ) );

	temp_machine->device = h_new_device( temp_machine->physical_device, info_device );

	//

	u32 format_n = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR( temp_machine->physical_device, temp_machine->surface, adr( format_n ), null );
	ptr( h_surface_format ) formats = new_mem( h_surface_format, format_n );
	vkGetPhysicalDeviceSurfaceFormatsKHR( temp_machine->physical_device, temp_machine->surface, adr( format_n ), formats );
	temp_machine->surface_format = formats[ 0 ];
	free_mem( formats );

	u32 present_mode_n = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR( temp_machine->physical_device, temp_machine->surface, adr( present_mode_n ), null );
	ptr( h_present_mode ) present_modes = new_mem( h_present_mode, present_mode_n );
	vkGetPhysicalDeviceSurfacePresentModesKHR( temp_machine->physical_device, temp_machine->surface, adr( present_mode_n ), present_modes );
	iter( present_mode_n, i )
	{
		if( present_modes[ i ] == h_present_mode_vsync_optimal )
			{
				temp_machine->present_mode = h_present_mode_vsync_optimal;
				break;
			}
	}
	free_mem( present_modes );

	//

	out temp_machine;
}

//

/// buffer

fn u32 find_memory_type( in VkPhysicalDevice physical_device, in uint32_t type_filter, in VkMemoryPropertyFlags properties )
{
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties( physical_device, adr( memory_properties ) );

	iter( memory_properties.memoryTypeCount, i )
	{
		if( ( type_filter & ( 1 << i ) ) and ( memory_properties.memoryTypes[ i ].propertyFlags & properties ) == properties )
			{
				out i;
			}
	}

	fprintf( stderr, "Failed to find suitable memory type\n" );
	out 0;
}

OBJECT(
		buffer,
		machine linked_machine;
		h_device_buffer device_buff;
		h_device_memory device_mem; )
( in machine in_machine, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties )
{
	buffer temp_buffer = global_new_buffer();
	temp_buffer->linked_machine = in_machine;
	//
	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	ptr( h_device_buffer ) temp_buff = new_mem( h_device_buffer, 1 );
	temp_buffer->device_buff = val( temp_buff );
	ptr( h_device_memory ) temp_mem = new_mem( h_device_memory, 1 );
	temp_buffer->device_mem = val( temp_mem );

	vkCreateBuffer( temp_buffer->linked_machine->device, adr( buffer_info ), NULL, adr( temp_buffer->device_buff ) );

	h_mem_requirements mem_requirements;
	vkGetBufferMemoryRequirements( temp_buffer->linked_machine->device, temp_buffer->device_buff, adr( mem_requirements ) );

	h_info_mem_allocate alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_requirements.size;
	alloc_info.memoryTypeIndex = find_memory_type( temp_buffer->linked_machine->physical_device, mem_requirements.memoryTypeBits, properties );

	vkAllocateMemory( temp_buffer->linked_machine->device, adr( alloc_info ), NULL, adr( temp_buffer->device_mem ) );

	vkBindBufferMemory( temp_buffer->linked_machine->device, temp_buffer->device_buff, temp_buffer->device_mem, 0 );
	//
	out temp_buffer;
}

//

/// image_form

make_enum{
		image_type_rgba = VK_IMAGE_ASPECT_COLOR_BIT,
		image_type_depth = VK_IMAGE_ASPECT_DEPTH_BIT,
		image_type_stencil = VK_IMAGE_ASPECT_STENCIL_BIT,
} enum_image_form_type;

OBJECT(
		image_form,
		machine linked_machine;
		enum_image_form_type type;
		h_format format;
		h_sampler sampler; )
( in machine in_machine, in enum_image_form_type in_type, in h_format in_format )
{
	image_form temp_image_form = global_new_image_form();
	temp_image_form->linked_machine = in_machine;
	temp_image_form->type = in_type;
	temp_image_form->format = in_format;
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

	vkCreateSampler( in_machine->device, adr( sampler_info ), null, adr( temp_image_form->sampler ) );
	//
	out temp_image_form;
}

//

/// image

make_enum{
		image_state_src,
		image_state_dst,
} enum_image_state;

OBJECT(
		image,
		image_form form;
		enum_image_state state;
		h_image ptr;
		h_image_view view;
		h_device_memory mem;
		u32 width;
		u32 height; )
( in image_form in_image_form, in enum_image_state in_state, in u32 in_w, in u32 in_h )
{
	image temp_image = global_new_image();
	temp_image->form = in_image_form;
	temp_image->state = in_state;
	temp_image->width = in_w;
	temp_image->height = in_h;
	//
	h_extent_3d temp_extent = {
			.width = temp_image->width,		//ceilf( f32( temp_image->width ) / 32. ) * 32.,
			.height = temp_image->height, //ceilf( f32( temp_image->height ) / 32. ) * 32.,
			.depth = 1 };

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties( temp_image->form->linked_machine->physical_device, temp_image->form->format, &formatProperties );

	if( !( formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT ) )
		{
			printf( "Format cannot be used as color attachment!\n" );
		}

	h_info_image image_info = h_make_info_image(
			VK_IMAGE_TYPE_2D,
			temp_extent,
			1,
			1,
			temp_image->form->format,
			( ( temp_image->state == image_state_src ) ? ( VK_IMAGE_TILING_LINEAR ) : ( VK_IMAGE_TILING_OPTIMAL ) ), // OPTIMAL REQUIRES MULTIPLES OF 32x32
			VK_IMAGE_LAYOUT_UNDEFINED,
			( ( temp_image->state == image_state_src ) ? ( VK_IMAGE_USAGE_SAMPLED_BIT ) : ( VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) ),
			VK_SHARING_MODE_EXCLUSIVE,
			VK_SAMPLE_COUNT_1_BIT );

	temp_image->ptr = h_new_image( temp_image->form->linked_machine->device, image_info );

	h_mem_requirements mem_requirements;
	vkGetImageMemoryRequirements( temp_image->form->linked_machine->device, temp_image->ptr, &mem_requirements );

	h_info_mem_allocate alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_requirements.size; //ceil( f64( mem_requirements.size ) / 1024. ) * 1024.;
	alloc_info.memoryTypeIndex = find_memory_type( temp_image->form->linked_machine->physical_device, mem_requirements.memoryTypeBits,
																								 ( ( temp_image->state == image_state_src ) ? ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) : ( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) ) );

	vkAllocateMemory( temp_image->form->linked_machine->device, &alloc_info, null, adr( temp_image->mem ) );

	vkBindImageMemory( temp_image->form->linked_machine->device, temp_image->ptr, temp_image->mem, 0 );

	//

	out temp_image;
}

fn pure image_make_view( in image in_image )
{
	h_component_mapping temp_mapping = ( h_component_mapping ){
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY };
	h_image_subresource_range temp_range = ( h_image_subresource_range ){
			in_image->form->type,
			0, 1, 0, 1 };
	h_info_image_view image_view_info = h_make_info_image_view(
			in_image->ptr,
			VK_IMAGE_VIEW_TYPE_2D,
			in_image->form->format,
			temp_mapping,
			temp_range );

	in_image->view = h_new_image_view( in_image->form->linked_machine->device, image_view_info );
}

//fn pure

/*fn pure image_transition(in image in_image, in h_image_layout in_layout, in h_command_pool in_command_pool)
{
	h_command_buffer temp_cmd = begin_single_time_commands( in_image->form->linked_machine->device, in_command_pool );
	VkImageMemoryBarrier barrier = { 0 };
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = in_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = in_image->ptr;

	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags src_stage;
	VkPipelineStageFlags dst_stage;

	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	vkCmdPipelineBarrier( temp_cmd, src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier );
	end_single_time_commands( in_image->form->linked_machine->device, in_command_pool, default_renderer->queue, temp_cmd );
}*/

//

/// subframe

make_enum{
		subframe_type_rgba = VK_IMAGE_ASPECT_COLOR_BIT,
		subframe_type_depth = VK_IMAGE_ASPECT_DEPTH_BIT,
		subframe_type_stencil = VK_IMAGE_ASPECT_STENCIL_BIT,
} enum_subframe_type;

OBJECT(
		subframe,
		enum_subframe_type type;
		h_attachment_reference attach_ref; )
( in enum_subframe_type in_type )
{
	subframe temp_subframe = global_new_subframe();
	temp_subframe->type = in_type;
	//
	temp_subframe->attach_ref.attachment = 0;
	temp_subframe->attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
	with( temp_subframe->type )
	{
		is( subframe_type_rgba )
		{
			temp_subframe->attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			skip;
		}

		is( subframe_type_depth )
				is( subframe_type_stencil )
		{
			temp_subframe->attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			skip;
		}
	}
	//
	out temp_subframe;
}

//

/// frame_form

make_enum{
		frame_form_type_present,
		frame_form_type_shader_read,
		frame_form_type_general,
} enum_frame_form_type;

OBJECT(
		frame_form,
		enum_frame_form_type type;
		h_format format;
		h_render_pass render_pass;
		list list_subframes; )
( in machine in_machine, in enum_frame_form_type in_type, in h_format in_format, in list in_list_subframes )
{
	frame_form temp_frame_form = global_new_frame_form();
	temp_frame_form->type = in_type;
	temp_frame_form->format = in_format;
	temp_frame_form->list_subframes = in_list_subframes;
	//

	ptr( h_attachment_reference ) attach_ref_depth_stencil = null;

	u32 rgba_count = 0;
	iter( temp_frame_form->list_subframes->size, s )
	{
		subframe temp_subframe = list_get( temp_frame_form->list_subframes, subframe, s );
		temp_subframe->attach_ref.attachment = s;
		with( temp_subframe->type )
		{
			is( subframe_type_rgba )
			{
				rgba_count++;
				skip;
			}
			is( subframe_type_depth )
					is( subframe_type_stencil )
			{
				attach_ref_depth_stencil = adr( temp_subframe->attach_ref );
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
				subframe temp_subframe = list_get( temp_frame_form->list_subframes, subframe, r );
				attach_ref_rgba[ r ] = temp_subframe->attach_ref;
			}
		}

	h_subpass_description subpass = { 0 };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = rgba_count;
	subpass.pColorAttachments = attach_ref_rgba;
	subpass.pDepthStencilAttachment = attach_ref_depth_stencil;

	ptr( h_attachment_description ) attachments = new_mem( h_attachment_description, temp_frame_form->list_subframes->size );
	iter( temp_frame_form->list_subframes->size, a )
	{
		attachments[ a ].format = temp_frame_form->format;
		attachments[ a ].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[ a ].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[ a ].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[ a ].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[ a ].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[ a ].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		with( temp_frame_form->type )
		{
			is( frame_form_type_present )
			{
				attachments[ a ].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				skip;
			}
			is( frame_form_type_shader_read )
			{
				attachments[ a ].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				skip;
			}
			is( frame_form_type_general )
			{
				attachments[ a ].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
				skip;
			}
		}
	}

	h_info_render_pass render_pass_info = h_make_info_render_pass( temp_frame_form->list_subframes->size, attachments, 1, adr( subpass ), 0, null );
	temp_frame_form->render_pass = h_new_render_pass( in_machine->device, render_pass_info );
	//
	out temp_frame_form;
}

//

OBJECT(
		frame,
		machine linked_machine;
		frame_form form;
		h_framebuffer buffer;
		list list_images;
		list list_views; )
( in machine in_machine, in frame_form in_frame_form, in list in_list_images )
{
	frame temp_frame = global_new_frame();
	temp_frame->form = in_frame_form;
	temp_frame->list_images = in_list_images;
	temp_frame->list_views = new_list( h_image_view );
	//

	u32 max_w = 0, max_h = 0;
	iter( temp_frame->list_images->size, i )
	{
		image temp_image = list_get( temp_frame->list_images, image, i );
		max_w = max( max_w, temp_image->width );
		max_h = max( max_h, temp_image->height );
	}

	//

	iter( temp_frame->form->list_subframes->size, v )
	{
		image temp_image = list_get( temp_frame->list_images, image, v );
		h_info_image_view image_view_info = h_make_info_image_view(
				temp_image->ptr,
				VK_IMAGE_VIEW_TYPE_2D,
				temp_frame->form->format,
				( ( h_component_mapping ){
						VK_COMPONENT_SWIZZLE_IDENTITY,
						VK_COMPONENT_SWIZZLE_IDENTITY,
						VK_COMPONENT_SWIZZLE_IDENTITY,
						VK_COMPONENT_SWIZZLE_IDENTITY } ),
				( ( h_image_subresource_range ){
						list_get( temp_frame->form->list_subframes, subframe, v )->type,
						0, 1, 0, 1 } ) );
		//temp_image_views[ v ] = h_new_image_view( in_machine->device, image_view_info );
		list_add( temp_frame->list_views, h_image_view, h_new_image_view( in_machine->device, image_view_info ) );
	}

	h_info_framebuffer framebuffer_info = h_make_info_framebuffer(
			in_frame_form->render_pass,
			temp_frame->list_views->size,
			( ptr( h_image_view ) )( temp_frame->list_views->data ),
			max_w,
			max_h,
			1 );
	temp_frame->buffer = h_new_framebuffer( in_machine->device, framebuffer_info );
	//
	out temp_frame;
}

//

/// renderer

OBJECT(
		renderer,
		machine linked_machine;
		h_queue queue;
		h_viewport viewport;
		h_swapchain swapchain;
		h_format swapchain_format;
		h_extent swapchain_extent;
		//ptr( h_image_view ) image_views;
		//ptr( h_image ) images;
		//u32 image_count;
		u32 current_frame;
		//h_render_pass render_pass;
		//ptr( h_framebuffer ) framebuffers;
		frame_form form;
		frame frame_window;
		list list_swapchain_frames;
		h_command_pool command_pool;
		ptr( h_command_buffer ) command_buffers;
		ptr( h_semaphore ) image_ready;
		ptr( h_semaphore ) image_done;
		ptr( h_fence ) flight_fences; )
( in machine in_machine )
{
	renderer temp_renderer = global_new_renderer();
	temp_renderer->linked_machine = in_machine;
	temp_renderer->queue = h_new_queue( temp_renderer->linked_machine->device, temp_renderer->linked_machine->queue_family_index, 0 );
	temp_renderer->frame_window = null;
	temp_renderer->list_swapchain_frames = new_list( frame );
	temp_renderer->command_pool = null;
	temp_renderer->swapchain = null;
	//
	out temp_renderer;
}

fn pure renderer_update( in renderer in_renderer )
{
	vkDeviceWaitIdle( in_renderer->linked_machine->device );

	//

	if( in_renderer->swapchain != null ) vkDestroySwapchainKHR( in_renderer->linked_machine->device, in_renderer->swapchain, null );

	if( in_renderer->list_swapchain_frames->size != 0 )
		{
			iter( in_renderer->list_swapchain_frames->size, i )
			{
				frame temp_frame = list_get( in_renderer->list_swapchain_frames, frame, i );
				iter( temp_frame->list_images->size, j )
				{
					image temp_image = list_get( temp_frame->list_images, image, j );
					delete_image( temp_image );
				}
				delete_frame( temp_frame );
				vkDestroySemaphore( in_renderer->linked_machine->device, in_renderer->image_ready[ i ], null );
				vkDestroySemaphore( in_renderer->linked_machine->device, in_renderer->image_done[ i ], null );
				vkDestroyFence( in_renderer->linked_machine->device, in_renderer->flight_fences[ i ], null );
			}
			in_renderer->list_swapchain_frames->size = 0;
		}

	if( in_renderer->command_pool != null ) vkDestroyCommandPool( in_renderer->linked_machine->device, in_renderer->command_pool, null ); //
	h_info_command_pool command_pool_info = h_make_info_command_pool( in_renderer->linked_machine->queue_family_index );
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	in_renderer->command_pool = h_new_command_pool( in_renderer->linked_machine->device, command_pool_info );

	//

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( in_renderer->linked_machine->physical_device, in_renderer->linked_machine->surface, adr( in_renderer->linked_machine->surface_capabilities ) );

	h_info_swapchain swapchain_info = h_make_info_swapchain(
			in_renderer->linked_machine->surface,
			in_renderer->linked_machine->surface_capabilities.minImageCount + 1,
			in_renderer->linked_machine->surface_format.format,
			in_renderer->linked_machine->surface_format.colorSpace,
			in_renderer->linked_machine->surface_capabilities.currentExtent,
			1,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			null,
			in_renderer->linked_machine->surface_capabilities.currentTransform,
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			in_renderer->linked_machine->present_mode, VK_TRUE,
			null );

	in_renderer->swapchain = h_new_swapchain( in_renderer->linked_machine->device, swapchain_info );
	in_renderer->swapchain_format = in_renderer->linked_machine->surface_format.format;
	in_renderer->swapchain_extent = in_renderer->linked_machine->surface_capabilities.currentExtent;

	in_renderer->viewport = h_make_viewport(
			0.0,
			0.0,
			f32( in_renderer->swapchain_extent.width ),
			f32( in_renderer->swapchain_extent.height ),
			0.0,
			1.0 );


	u32 temp_count = 0;
	ptr( h_image ) temp_images = null;
	vkGetSwapchainImagesKHR( in_renderer->linked_machine->device, in_renderer->swapchain, adr( temp_count ), null );
	temp_images = new_mem( h_image, temp_count );
	vkGetSwapchainImagesKHR( in_renderer->linked_machine->device, in_renderer->swapchain, adr( temp_count ), temp_images );

	list temp_list_subframes = new_list( subframe );
	subframe temp_subframe = new_subframe( subframe_type_rgba );
	list_add( temp_list_subframes, subframe, temp_subframe );

	in_renderer->form = new_frame_form(
			in_renderer->linked_machine,
			frame_form_type_present,
			in_renderer->swapchain_format,
			temp_list_subframes );

	iter( temp_count, i )
	{
		list temp_list_images = new_list( image );
		image temp_image = global_new_image();
		temp_image->ptr = temp_images[ i ];
		temp_image->width = in_renderer->swapchain_extent.width;
		temp_image->height = in_renderer->swapchain_extent.height;
		list_add( temp_list_images, image, temp_image );
		frame temp_frame = new_frame( in_renderer->linked_machine, in_renderer->form, temp_list_images );
		list_add( in_renderer->list_swapchain_frames, frame, temp_frame );
	}

	//

	h_info_semaphore semaphore_info = h_make_info_semaphore();
	h_info_fence fence_info = h_make_info_fence();
	h_info_command_buffer command_buffer_info = h_make_info_command_buffer(
			in_renderer->command_pool,
			h_command_buffer_level_primary );

	in_renderer->image_ready = new_mem( h_semaphore, in_renderer->list_swapchain_frames->size );
	in_renderer->image_done = new_mem( h_semaphore, in_renderer->list_swapchain_frames->size );
	in_renderer->flight_fences = new_mem( h_fence, in_renderer->list_swapchain_frames->size );
	in_renderer->command_buffers = new_mem( h_command_buffer, in_renderer->list_swapchain_frames->size );

	iter( in_renderer->list_swapchain_frames->size, i )
	{
		in_renderer->image_ready[ i ] = h_new_semaphore( in_renderer->linked_machine->device, semaphore_info );
		in_renderer->image_done[ i ] = h_new_semaphore( in_renderer->linked_machine->device, semaphore_info );
		in_renderer->flight_fences[ i ] = h_new_fence( in_renderer->linked_machine->device, fence_info );
		in_renderer->command_buffers[ i ] = h_new_command_buffer( in_renderer->linked_machine->device, command_buffer_info );
	}
	//
	in_renderer->current_frame = 0;

	//

	if( in_renderer->frame_window != null )
		{
			iter( in_renderer->frame_window->list_images->size, j )
			{
				image temp_image = list_get( in_renderer->frame_window->list_images, image, j );
				delete_image( temp_image );
			}
			delete_frame( in_renderer->frame_window );
		}

	{
		image_form frame_image_form = new_image_form(
				in_renderer->linked_machine,
				image_type_rgba,
				in_renderer->swapchain_format );

		image temp_image = new_image(
				frame_image_form,
				image_state_dst,
				in_renderer->swapchain_extent.width,
				in_renderer->swapchain_extent.height );
		//
		list temp_list_images = new_list( image );
		list_add( temp_list_images, image, temp_image );
		in_renderer->frame_window = new_frame( in_renderer->linked_machine, in_renderer->form, temp_list_images );
	}
}

//

//

///  vert_attrib

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

OBJECT(
		vert_attrib,
		h_format format;
		u32 type_size;
		u32 size;
		str type_glsl; )
( in h_format in_format, in u32 in_type_size, in u32 in_size, in str in_type_glsl )
{
	vert_attrib temp_vert_attrib = global_new_vert_attrib();
	//
	temp_vert_attrib->format = in_format;
	temp_vert_attrib->type_size = in_type_size;
	temp_vert_attrib->size = in_size;
	temp_vert_attrib->type_glsl = in_type_glsl;
	//
	out temp_vert_attrib;
}
#define new_vert_attrib( type, size ) new_vert_attrib( $format_type_##type##_size_##size, size_( type ), size, $format_type_##type##_size_##size##_str )

//

OBJECT(
		vert_form,
		u32 type_size;
		list attrib_list;
		str layout_glsl; )
()
{
	vert_form temp_vert_form = global_new_vert_form();
	//
	temp_vert_form->type_size = 0;
	temp_vert_form->attrib_list = new_list( vert_attrib );
	temp_vert_form->layout_glsl = new_str( "", 0 );
	//
	out temp_vert_form;
}

fn pure vert_form_add_attrib( in vert_form in_vert_form,
															in vert_attrib in_attrib,
															in str in_attrib_var )
{
	str layout_str = new_str( "layout(location = ", str_len( in_attrib->type_glsl ) + 6 );
	str_add( layout_str, dec_to_str[ in_vert_form->attrib_list->size ] );
	str_add( layout_str, ") in " );
	str_add( layout_str, in_attrib->type_glsl );
	str_end( layout_str );

	str temp_str = new_str( in_vert_form->layout_glsl,
													str_len( layout_str ) + str_len( in_attrib_var ) + 3 );
	str_add( temp_str, layout_str );
	str_add( temp_str, " " );
	str_add( temp_str, in_attrib_var );
	str_add( temp_str, ";\n" );
	str_end( temp_str );

	delete_str( layout_str );
	delete_str( in_vert_form->layout_glsl );

	list_add( in_vert_form->attrib_list, vert_attrib, in_attrib );
	in_vert_form->type_size += in_attrib->type_size * in_attrib->size;
	in_vert_form->layout_glsl = temp_str;
}

//

make_struct
{
	struct_vec2 pos;
	struct_vec2 uv;
	struct_vec3 rgb;
}
struct_vert_2d;

make_struct
{
	struct_vec3 pos;
	struct_vec2 uv;
	struct_vec3 rgb;
}
struct_vert_3d;

make_struct
{
	u32 a, b, c;
}
ind_tri;

make_struct
{
	u32 a, b;
}
ind_line;

//

/// mesh

OBJECT(
		mesh,
		machine linked_machine;
		buffer vertex_buffer;
		buffer index_buffer;
		vert_form form;
		list vert_list;
		list ind_list;
		u32 vert_n; )
( in machine in_machine, in vert_form in_vert_form )
{
	mesh temp_mesh = global_new_mesh();
	temp_mesh->linked_machine = in_machine;
	//
	temp_mesh->form = in_vert_form;
	temp_mesh->vert_list = $new_list( 0, 1, temp_mesh->form->type_size, new_mem( u8, temp_mesh->form->type_size ) );
	temp_mesh->ind_list = new_list( u32 );
	//
	out temp_mesh;
}

#define mesh_add_quad( var, vert_struct, tl, tr, br, bl )       \
	do                                                            \
		{                                                           \
			list_add( var->ind_list, u32, var->vert_list->size );     \
			list_add( var->ind_list, u32, var->vert_list->size + 1 ); \
			list_add( var->ind_list, u32, var->vert_list->size + 2 ); \
			list_add( var->ind_list, u32, var->vert_list->size );     \
			list_add( var->ind_list, u32, var->vert_list->size + 2 ); \
			list_add( var->ind_list, u32, var->vert_list->size + 3 ); \
			list_add( var->vert_list, vert_struct, tl );              \
			list_add( var->vert_list, vert_struct, tr );              \
			list_add( var->vert_list, vert_struct, br );              \
			list_add( var->vert_list, vert_struct, bl );              \
	}                                                             \
	spin( 0 )

fn pure mesh_update( in mesh in_mesh )
{
	in_mesh->vertex_buffer = new_buffer(
			in_mesh->linked_machine,
			size_( struct_vert_3d ) * in_mesh->vert_list->size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

	in_mesh->index_buffer = new_buffer(
			in_mesh->linked_machine,
			size_( u32 ) * in_mesh->ind_list->size,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

	pure_ptr data;
	vkMapMemory( in_mesh->linked_machine->device, in_mesh->vertex_buffer->device_mem, 0, size_( struct_vert_3d ) * in_mesh->vert_list->size, 0, adr( data ) );
	memcpy( data, in_mesh->vert_list->data, size_( struct_vert_3d ) * in_mesh->vert_list->size );
	vkUnmapMemory( in_mesh->linked_machine->device, in_mesh->vertex_buffer->device_mem );

	vkMapMemory( in_mesh->linked_machine->device, in_mesh->index_buffer->device_mem, 0, size_( u32 ) * in_mesh->ind_list->size, 0, adr( data ) );
	memcpy( data, in_mesh->ind_list->data, size_( u32 ) * in_mesh->ind_list->size );
	vkUnmapMemory( in_mesh->linked_machine->device, in_mesh->index_buffer->device_mem );
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
		glsl_type_vert,
		glsl_type_frag,
		glsl_type_comp } enum_glsl_type;

const char* get_extension( enum_glsl_type type )
{
	static const char* extensions[] = { ".vert", ".frag", ".comp" };
	return extensions[ type ];
}

fn pure compile_glsl_to_spirv( str glsl_str, enum_glsl_type type, const char* spirv_filename )
{
	str temp_name = new_str( "glsl_to_spirv_temp", 5 );
	str_add( temp_name, get_extension( type ) );
	str_end( temp_name );

	// Write the GLSL string to the temp file
	file_save( temp_name, glsl_str );

	// Write out the command string
	char command[ 512 ];
	snprintf( command, size_( command ), "glslangValidator -V %s -o %s", temp_name, spirv_filename );
	int result = system( command );
	if( result != 0 )
		{
			printf( "Failed to compile GLSL to SPIR-V: %d\n", result );
		}

	// Clean up the temporary file
	remove( temp_name );
}

OBJECT(
		glsl,
		enum_glsl_type type;
		str path;
		vert_form form;
		h_shader_module module;
		ptr( u32 ) spirv;
		u32 size;
		h_info_pipeline_shader_stage stage_info; )
( in h_device in_device, in str in_path, in enum_glsl_type in_type, in vert_form in_form )
{
	glsl temp_glsl = global_new_glsl();
	//
	temp_glsl->type = in_type;
	temp_glsl->path = in_path;
	temp_glsl->form = in_form;
	os_file text = file_load( temp_glsl->path );

	str glsl_str = new_str(
			$glsl_vert_global,
			( ( temp_glsl->form != null ) ? strlen( temp_glsl->form->layout_glsl ) : 0 ) +
					text.size + 1 );
	if( temp_glsl->form != null )
		str_add( glsl_str, temp_glsl->form->layout_glsl );

	str_add( glsl_str, text.data );
	str_end( glsl_str );

	//

	str spirv_path = new_str( in_path, 6 );
	str_add( spirv_path, ".spirv" );
	str_end( spirv_path );
	compile_glsl_to_spirv( glsl_str, in_type, spirv_path );

	//

	os_file spirv_text = file_load( spirv_path );

	temp_glsl->spirv = ( ptr( u32 ) )spirv_text.data;
	temp_glsl->size = spirv_text.size;

	h_info_shader_module module_info = h_make_info_shader_module( temp_glsl->spirv, temp_glsl->size );
	// Use the SPIR-V bytecode to create the shader module
	temp_glsl->module = h_new_shader_module( in_device, module_info );

	// Shader stage
	temp_glsl->stage_info = h_make_info_pipeline_shader_stage( ( ( in_type == glsl_type_vert ) ? ( VK_SHADER_STAGE_VERTEX_BIT ) : ( VK_SHADER_STAGE_FRAGMENT_BIT ) ), temp_glsl->module, "main" );
	//
	out temp_glsl;
}

#define new_glsl_vert( in_device, in_path, in_vert_form ) new_glsl( in_device, in_path, glsl_type_vert, in_vert_form )
#define new_glsl_frag( in_device, in_path ) new_glsl( in_device, in_path, glsl_type_frag, null )

//

OBJECT(
		shader,
		renderer linked_renderer;
		u32 glsl_count;
		ptr( glsl ) glsl_data;
		h_topology topology;
		h_pipeline pipeline;
		h_pipeline_layout pipeline_layout;
		h_descriptor_set descriptor_set; )
( in renderer in_renderer, in u32 in_glsl_count, ptr( glsl ) in_glsl, in h_topology in_topology )
{
	shader temp_shader = global_new_shader();
	temp_shader->linked_renderer = in_renderer;
	temp_shader->glsl_count = in_glsl_count;
	temp_shader->glsl_data = in_glsl;
	temp_shader->topology = in_topology;
	//
	vert_form temp_vert_form = null;
	ptr( h_info_pipeline_shader_stage ) glsl_stages =
			new_mem( h_info_pipeline_shader_stage, temp_shader->glsl_count );
	iter( temp_shader->glsl_count, g )
	{
		if( temp_shader->glsl_data[ g ]->type == glsl_type_vert )
			{
				temp_vert_form = temp_shader->glsl_data[ g ]->form;
			}

		glsl_stages[ g ] = temp_shader->glsl_data[ g ]->stage_info;
	}

	//

	h_vertex_binding vert_bindings[] = {
			h_make_vertex_binding_per_vertex( 0, temp_vert_form->type_size ) };

	ptr( h_vertex_attribute ) vert_attributes =
			new_mem( h_vertex_attribute, temp_vert_form->attrib_list->size );
	vert_attrib temp_vert_attrib = null;
	u32 offset = 0;
	iter( temp_vert_form->attrib_list->size, a )
	{
		temp_vert_attrib = list_get( temp_vert_form->attrib_list, vert_attrib, a );
		vert_attributes[ a ].location = a;
		vert_attributes[ a ].binding = 0;
		vert_attributes[ a ].format = temp_vert_attrib->format;
		vert_attributes[ a ].offset = offset;
		offset += temp_vert_attrib->type_size * temp_vert_attrib->size;
	}

	h_info_pipeline_vertex pipeline_vertex_info = h_make_info_pipeline_vertex(
			1, vert_bindings, temp_vert_form->attrib_list->size, vert_attributes );

	//

	h_info_pipeline_assembly pipeline_input_assembly_info =
			h_make_info_pipeline_assembly( temp_shader->topology );

	//

	h_scissor scissor = { 0 };
	scissor.offset = ( VkOffset2D ){ 0, 0 };
	scissor.extent = temp_shader->linked_renderer->swapchain_extent;
	scissor.extent.width /= 2;

	h_info_pipeline_viewport viewport_info = h_make_info_pipeline_viewport(
			1, adr( temp_shader->linked_renderer->viewport ), 1, adr( scissor ) );

	//

	h_info_pipeline_raster raster_info = h_make_info_pipeline_raster(
			VK_POLYGON_MODE_FILL, 1.0, VK_CULL_MODE_BACK_BIT,
			VK_FRONT_FACE_CLOCKWISE );

	//

	VkPipelineMultisampleStateCreateInfo multisampling = {
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
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
			.front = { 0 },
			.back = { 0 },
	};

	//

	VkPipelineColorBlendAttachmentState colorBlendAttachment = { 0 };
	colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlending = {
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = adr( colorBlendAttachment );

	//

	VkPipelineDynamicStateCreateInfo dynamic_state_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = 2,
			.pDynamicStates = ( VkDynamicState[] ){ VK_DYNAMIC_STATE_VIEWPORT,
																							VK_DYNAMIC_STATE_SCISSOR } };

	//

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSetLayoutBinding bindings[] = {
			/*{
					.binding = 0,
					.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					.descriptorCount = 1,
					.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			},*/
			{
					.binding = 0, //1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 1,
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			},
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = 1, //2,
			.pBindings = ( const ptr( VkDescriptorSetLayoutBinding ) )( adr( bindings ) ),
	};

	if( vkCreateDescriptorSetLayout( temp_shader->linked_renderer->linked_machine->device, adr( layoutInfo ), null,
																	 adr( descriptorSetLayout ) ) != VK_SUCCESS )
		{
			printf( "Failed to create descriptor set layout\n" );
			exit( 1 );
		}

	h_info_pipeline_layout info_pipeline_layout = h_make_info_pipeline_layout( 1, adr( descriptorSetLayout ), 0, null );

	temp_shader->pipeline_layout = h_new_pipeline_layout( temp_shader->linked_renderer->linked_machine->device, info_pipeline_layout );

	//

	h_info_pipeline pipeline_info = h_make_info_pipeline(
			temp_shader->glsl_count,
			glsl_stages,
			adr( pipeline_vertex_info ),
			adr( pipeline_input_assembly_info ),
			null,
			adr( viewport_info ),
			adr( raster_info ),
			adr( multisampling ),
			adr( depth_stencil ),
			adr( colorBlending ),
			adr( dynamic_state_info ),
			temp_shader->pipeline_layout,
			temp_shader->linked_renderer->form->render_pass,
			0,
			null, 0 );
	pipeline_info.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

	// derivedPipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
	// derivedPipelineInfo.basePipelineHandle = basePipeline;
	// derivedPipelineInfo.basePipelineIndex = -1;

	temp_shader->pipeline = h_new_pipeline( temp_shader->linked_renderer->linked_machine->device, pipeline_info );

	//

	VkDescriptorPool descriptorPool;
	VkDescriptorPoolSize pool_sizes[] = {
			/*{
					.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					.descriptorCount = 1,
			},*/
			{
					.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 1,
			},
	};

	VkDescriptorPoolCreateInfo poolInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = 1,
			.poolSizeCount = 1, //2,
			.pPoolSizes = ( const ptr( VkDescriptorPoolSize ) ) & pool_sizes,
	};

	if( vkCreateDescriptorPool( temp_shader->linked_renderer->linked_machine->device, adr( poolInfo ), null,
															adr( descriptorPool ) ) != VK_SUCCESS )
		{
			printf( "Failed to create descriptor pool\n" );
			exit( 1 );
		}

	VkDescriptorSetAllocateInfo allocInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = adr( descriptorSetLayout ),
	};

	if( vkAllocateDescriptorSets( temp_shader->linked_renderer->linked_machine->device, adr( allocInfo ),
																adr( temp_shader->descriptor_set ) ) != VK_SUCCESS )
		{
			printf( "Failed to allocate descriptor set\n" );
			exit( 1 );
		}

	//

	free_mem( glsl_stages );

	//

	out temp_shader;
}

//

/// defaults

global machine default_machine = null;

global image_form default_image_form = null;
//global image_form default_frame_image_form = null;

//global image default_frame_image = null;
global image default_image = null;

//global frame_form default_frame_form = null;
//global frame default_frame = null;

global renderer default_renderer = null;

global vert_attrib default_vert_attrib_pos2 = null;
global vert_attrib default_vert_attrib_pos3 = null;
global vert_attrib default_vert_attrib_uv = null;
global vert_attrib default_vert_attrib_rgb = null;
global vert_attrib default_vert_attrib_rgba = null;

global vert_form default_vert_form_2d = null;
global vert_form default_vert_form_3d = null;

global glsl default_glsl_2d_vert = null;
global glsl default_glsl_2d_frag = null;
global glsl default_glsl_3d_vert = null;
global glsl default_glsl_3d_frag = null;

global shader default_shader_2d = null;
global shader default_shader_3d = null;

global mesh default_mesh_2d_plane = null;
global mesh default_mesh_3d_plane = null;

//

fn pure main_draw();

//

bool resizing = false;
SIZE lastSize;
u32 this_fence = 0;

#if OS_WINDOWS
fn LRESULT CALLBACK window_proc( in HWND hwnd, in UINT u_msg, in WPARAM w_param, in LPARAM l_param )
{
	if( u_msg == WM_DESTROY )
		{
			PostQuitMessage( 0 );
			out 0;
		}
	else
		{
			switch( u_msg )
				{
					//case WM_ERASEBKGND:
						//break;
					case WM_SIZE:
						/*if (w_param != SIZE_MINIMIZED) {
								lastSize.cx = LOWORD(l_param);
								lastSize.cy = HIWORD(l_param);
								if (!resizing) {
										resizing = true;
										SetTimer(hwnd, 1, 100, NULL);  // Set a timer for 100 ms.
									}
							}
						return 0;*/
					//case WM_TIMER:
						//KillTimer(hwnd, 1);
						renderer_update(default_renderer); // recreate swapchain
						this_fence = 0;
						main_draw();
						//resizing = false;
						return 1;
					case WM_KEYDOWN:
					case WM_SYSKEYDOWN:
						key_state[ w_param ] = true;
						break;
					case WM_KEYUP:
					case WM_SYSKEYUP:
						key_state[ w_param ] = false;
						break;
				}
		}
	out DefWindowProc( hwnd, u_msg, w_param, l_param );
}
#endif

//

/// main

fn pure $main_init()
{
	global_pool_window = new_pool( window );
	global_pool_machine = new_pool( machine );
	global_pool_buffer = new_pool( buffer );
	global_pool_image_form = new_pool( image_form );
	global_pool_image = new_pool( image );
	global_pool_subframe = new_pool( subframe );
	global_pool_frame_form = new_pool( frame_form );
	global_pool_frame = new_pool( frame );
	global_pool_renderer = new_pool( renderer );
	global_pool_vert_attrib = new_pool( vert_attrib );
	global_pool_vert_form = new_pool( vert_form );
	global_pool_mesh = new_pool( mesh );
	global_pool_glsl = new_pool( glsl );
	global_pool_shader = new_pool( shader );

	default_machine = new_machine( "hept" );

	//
	default_image_form = new_image_form(
			default_machine,
			image_type_rgba,
			VK_FORMAT_R8G8B8A8_UNORM );

	default_image = new_image(
			default_image_form,
			image_state_src,
			32,
			32 );
	image_make_view(default_image);
	//

	default_renderer = new_renderer( default_machine );
	renderer_update( default_renderer );
	//
	/*
	list temp_list_subframes = new_list( subframe );
	subframe temp_subframe_rgba = new_subframe( subframe_type_rgba );
	//subframe temp_subframe_depth = new_subframe( subframe_type_depth );
	list_add( temp_list_subframes, subframe, temp_subframe_rgba );
	//list_add( temp_list_subframes, subframe, temp_subframe_depth );

	default_frame_form = new_frame_form(
			default_machine,
			frame_form_type_general,
			default_renderer->swapchain_format,
			temp_list_subframes );

	list temp_list_images = new_list( image );*/


	//

	default_vert_attrib_pos2 = new_vert_attrib( f32, 2 );
	default_vert_attrib_pos3 = new_vert_attrib( f32, 3 );
	default_vert_attrib_uv = new_vert_attrib( f32, 2 );
	default_vert_attrib_rgb = new_vert_attrib( f32, 3 );
	default_vert_attrib_rgba = new_vert_attrib( f32, 4 );

	{
		default_vert_form_2d = new_vert_form();
		vert_form_add_attrib( default_vert_form_2d, default_vert_attrib_pos2, "in_pos" );
		vert_form_add_attrib( default_vert_form_2d, default_vert_attrib_uv, "in_uv" );
		vert_form_add_attrib( default_vert_form_2d, default_vert_attrib_rgb, "in_rgb" );
		default_glsl_2d_vert = new_glsl_vert( default_machine->device, "glsl/default_2d.vert", default_vert_form_2d );
		default_glsl_2d_frag = new_glsl_frag( default_machine->device, "glsl/default_2d.frag" );
		glsl in_glsl[] = {
				default_glsl_2d_vert,
				default_glsl_2d_frag,
		};
		default_shader_2d = new_shader( default_renderer, 2, in_glsl, h_topology_tri );

		default_mesh_2d_plane = new_mesh( default_machine, default_vert_form_2d );
		struct_vert_2d
				tl = ( struct_vert_2d ){ .pos = { -.95f, -.95f }, .uv = { 0.f, 0.f }, .rgb = { 1.f, 0.f, 0.f } },
				tr = ( struct_vert_2d ){ .pos = { .95f, -.95f }, .uv = { 1.f, 0.f }, .rgb = { 0.f, 1.f, 0.f } },
				br = ( struct_vert_2d ){ .pos = { .95f, .95f }, .uv = { 1.f, 1.f }, .rgb = { 0.f, 0.f, 1.f } },
				bl = ( struct_vert_2d ){ .pos = { -.95f, .95f }, .uv = { 0.f, 1.f }, .rgb = { 1.f, 1.f, 1.f } };
		mesh_add_quad( default_mesh_2d_plane, struct_vert_2d, tl, tr, br, bl );
		mesh_update( default_mesh_2d_plane );
	}

	{
		default_vert_form_3d = new_vert_form();
		vert_form_add_attrib( default_vert_form_3d, default_vert_attrib_pos3, "in_pos" );
		vert_form_add_attrib( default_vert_form_3d, default_vert_attrib_uv, "in_uv" );
		vert_form_add_attrib( default_vert_form_3d, default_vert_attrib_rgb, "in_rgb" );
		default_glsl_3d_vert = new_glsl_vert( default_machine->device, "glsl/default_3d.vert", default_vert_form_3d );
		default_glsl_3d_frag = new_glsl_frag( default_machine->device, "glsl/default_3d.frag" );
		glsl in_glsl[] = {
				default_glsl_3d_vert,
				default_glsl_3d_frag,
		};
		default_shader_3d = new_shader( default_renderer, 2, in_glsl, h_topology_tri );
	}

	default_mesh_3d_plane = new_mesh( default_machine, default_vert_form_3d );
	struct_vert_3d
			tl = ( struct_vert_3d ){ .pos = { -.5f, -.5f, 0. }, .uv = { 0.f, 0.f }, .rgb = { 1.f, 0.f, 0.f } },
			tr = ( struct_vert_3d ){ .pos = { .5f, -.5f, 0. }, .uv = { 1.f, 0.f }, .rgb = { 0.f, 1.f, 0.f } },
			br = ( struct_vert_3d ){ .pos = { .5f, .5f, 0. }, .uv = { 1.f, 1.f }, .rgb = { 0.f, 0.f, 1.f } },
			bl = ( struct_vert_3d ){ .pos = { -.5f, .5f, 0. }, .uv = { 0.f, 1.f }, .rgb = { 1.f, 1.f, 1.f } };
	tl.pos.x -= .2;
	tr.pos.x -= .2;
	br.pos.x -= .2;
	bl.pos.x -= .2;
	tl.pos.y -= .1;
	tr.pos.y -= .1;
	br.pos.y -= .1;
	bl.pos.y -= .1;
	mesh_add_quad( default_mesh_3d_plane, struct_vert_3d, tl, tr, br, bl );

	tl = ( struct_vert_3d ){ .pos = { -.5f, -.5f, 0. }, .uv = { 0.f, 0.f }, .rgb = { 1.f, 0.f, 0.f } },
	tr = ( struct_vert_3d ){ .pos = { .5f, -.5f, 0. }, .uv = { 1.f, 0.f }, .rgb = { 0.f, 1.f, 0.f } },
	br = ( struct_vert_3d ){ .pos = { .5f, .5f, 0. }, .uv = { 1.f, 1.f }, .rgb = { 0.f, 0.f, 1.f } },
	bl = ( struct_vert_3d ){ .pos = { -.5f, .5f, 0. }, .uv = { 0.f, 1.f }, .rgb = { 1.f, 1.f, 1.f } };
	tl.pos.x += .2;
	tr.pos.x += .2;
	br.pos.x += .2;
	bl.pos.x += .2;
	tl.pos.y += .1;
	tr.pos.y += .1;
	br.pos.y += .1;
	bl.pos.y += .1;
	mesh_add_quad( default_mesh_3d_plane, struct_vert_3d, tl, tr, br, bl );

	mesh_update( default_mesh_3d_plane );

	//

	{
		VkDescriptorImageInfo image_info = {
				.sampler = default_image->form->sampler,
				.imageView = default_image->view,												//list_get( default_frame->list_views, h_image_view, 0 ), // TODO: MAKE VIEW VIA IMAGE_FORM
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL // COLOR_ATTACHMENT_OPTIMAL//VK_IMAGE_LAYOUT_GENERAL,
		};

		VkWriteDescriptorSet descriptor_write[ 1 ] = {
				/*{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = default_shader_3d->descriptor_set,
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &buffer_info,
			},*/
				{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = default_shader_2d->descriptor_set,
					.dstBinding = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.pImageInfo = &image_info },
		};
		vkUpdateDescriptorSets( default_machine->device, 1, descriptor_write, 0, NULL );
	}
}

pure $main_loop_init();

fn pure $main_loop()
{
	$main_loop_init();
	//
	do_once
	{
		h_command_buffer temp_cmd = begin_single_time_commands( default_machine->device, default_renderer->command_pool );
		VkImageMemoryBarrier barrier = { 0 };
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; /////////////////////////////////////////////////
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = default_image->ptr;

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags src_stage;
		VkPipelineStageFlags dst_stage;

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		vkCmdPipelineBarrier( temp_cmd, src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier );
		end_single_time_commands( default_machine->device, default_renderer->command_pool, default_renderer->queue, temp_cmd );

		//

		pure_ptr data;
		{
			list pixels = new_list( rgba );
			u32 image_area = default_image->width * default_image->height;
			iter( image_area, p )
			{
				rgba c = make_rgba( rand(), rand(), rand(), 255 );
				list_add( pixels, rgba, c );
			}
			vkMapMemory( default_machine->device, default_image->mem, 0, sizeof( rgba ) * ( image_area ), 0, &data );
			memcpy( data, pixels->data, sizeof( rgba ) * ( image_area ) );
			vkUnmapMemory( default_machine->device, default_image->mem );
		}
	}
	//
	loop
	{

		//

		if( key_state[ 'A' ] ) print_str( "A" );

		//

		main_draw();

		/*if( present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR )
			{
				renderer_update( default_renderer );
				vkQueueWaitIdle( default_renderer->queue );
				this_fence = 0;
				default_renderer->current_frame = 0;
			}*/

		do_once window_show( default_machine->window );
		ifn( window_update( default_machine->window ) ) break;
	}
}

fn pure main_draw()
{
	do {
			{


				vkWaitForFences( default_machine->device, 1, adr( default_renderer->flight_fences[ this_fence ] ), VK_TRUE, UINT64_MAX );

				VkResult aquire_result = vkAcquireNextImageKHR(
						default_machine->device, default_renderer->swapchain, UINT64_MAX,
						default_renderer->image_ready[ default_renderer->current_frame ],
						VK_NULL_HANDLE, adr( default_renderer->current_frame ) );

				if( aquire_result == VK_ERROR_OUT_OF_DATE_KHR || aquire_result == VK_SUBOPTIMAL_KHR )
					{
						//recreateSwapChain();
						//return;
						print_str( "RESIZE2" );

						vkResetFences( default_machine->device, 1, adr( default_renderer->flight_fences[ this_fence ] ) );
						renderer_update( default_renderer );
						this_fence = 0;
						break;

						//loop{};
					}


				vkResetFences( default_machine->device, 1, adr( default_renderer->flight_fences[ this_fence ] ) );

				this_fence = ( default_renderer->current_frame + 1 ) % default_renderer->list_swapchain_frames->size;
			}

			frame this_frame = list_get( default_renderer->list_swapchain_frames, frame, default_renderer->current_frame );
			image this_image = list_get( this_frame->list_images, image, 0 );

			{
				VkCommandBufferBeginInfo begin_info = {
						VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

				vkBeginCommandBuffer(
						default_renderer->command_buffers[ default_renderer->current_frame ],
						adr( begin_info ) );
				//
				{
					h_scissor scissor = { 0 };
					scissor.offset = ( VkOffset2D ){ 0, 0 };
					scissor.extent = default_renderer->swapchain_extent;
					//scissor.extent.width /= 2;

					vkCmdSetScissor( default_renderer->command_buffers[ default_renderer->current_frame ], 0, 1, adr( scissor ) );
					h_viewport viewport = h_make_viewport(
							0.0,
							0.0,
							f32( default_renderer->swapchain_extent.width ),
							f32( default_renderer->swapchain_extent.height ),
							0.0,
							1.0 );
					vkCmdSetViewport( default_renderer->command_buffers[ default_renderer->current_frame ], 0, 1, adr( viewport ) );
				}
				//
				if( 1 )
					{ //

						VkRenderPassBeginInfo renderPassInfo = {
								VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
						renderPassInfo.renderPass = default_renderer->form->render_pass;
						renderPassInfo.framebuffer = default_renderer->frame_window->buffer; //default_frame->buffer;
						//default_renderer->framebuffers[ default_renderer->current_frame ];
						renderPassInfo.renderArea.offset = ( VkOffset2D ){ 0, 0 };
						renderPassInfo.renderArea.extent = default_renderer->swapchain_extent; // (h_extent) { .width = 64, .height = 64 };

						VkClearValue clearColor = { 1., 0., 0.25, 1.0f };
						renderPassInfo.clearValueCount = 1;
						renderPassInfo.pClearValues = adr( clearColor );

						//
						vkCmdBeginRenderPass(
								default_renderer->command_buffers[ default_renderer->current_frame ],
								adr( renderPassInfo ), VK_SUBPASS_CONTENTS_INLINE );
						//
						if( 1 )
							{


								vkCmdBindPipeline( default_renderer->command_buffers[ default_renderer->current_frame ], VK_PIPELINE_BIND_POINT_GRAPHICS, default_shader_2d->pipeline );

								// Bind the descriptor set containing the SSBO
								vkCmdBindDescriptorSets( default_renderer->command_buffers[ default_renderer->current_frame ], VK_PIPELINE_BIND_POINT_GRAPHICS, default_shader_2d->pipeline_layout, 0, 1, &default_shader_2d->descriptor_set, 0, NULL );

								VkDeviceSize offsets[] = { 0 };
								vkCmdBindVertexBuffers( default_renderer->command_buffers[ default_renderer->current_frame ], 0, 1, &default_mesh_2d_plane->vertex_buffer->device_buff, offsets );
								vkCmdBindIndexBuffer( default_renderer->command_buffers[ default_renderer->current_frame ], default_mesh_2d_plane->index_buffer->device_buff, 0, VK_INDEX_TYPE_UINT32 );

								vkCmdDrawIndexed( default_renderer->command_buffers[ default_renderer->current_frame ], default_mesh_2d_plane->ind_list->size, 1, 0, 0, 0 );
							} //
						vkCmdEndRenderPass(
								default_renderer->command_buffers[ default_renderer->current_frame ] );
						//
					}

				//

				//
				//
				//

				VkRenderPassBeginInfo renderPassInfo = {
						VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
				renderPassInfo.renderPass = default_renderer->form->render_pass;
				renderPassInfo.framebuffer = this_frame->buffer;
				//default_renderer->framebuffers[ default_renderer->current_frame ];
				renderPassInfo.renderArea.offset = ( VkOffset2D ){ 0, 0 };
				renderPassInfo.renderArea.extent = default_renderer->swapchain_extent; // (h_extent) { .width = 64, .height = 64 };

				//VkClearValue clearColor = { 0.125f, 0.25f, 0.5f, 1.0f };
				VkClearValue clearColor = { 0, 0, 0, 0 };
				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = adr( clearColor );

				//
				vkCmdBeginRenderPass(
						default_renderer->command_buffers[ default_renderer->current_frame ],
						adr( renderPassInfo ), VK_SUBPASS_CONTENTS_INLINE );
				//
				if( 0 )
					{
						vkCmdBindPipeline( default_renderer->command_buffers[ default_renderer->current_frame ], VK_PIPELINE_BIND_POINT_GRAPHICS, default_shader_2d->pipeline );

						// Bind the descriptor set containing the SSBO
						vkCmdBindDescriptorSets( default_renderer->command_buffers[ default_renderer->current_frame ], VK_PIPELINE_BIND_POINT_GRAPHICS, default_shader_2d->pipeline_layout, 0, 1, &default_shader_2d->descriptor_set, 0, NULL );

						VkDeviceSize offsets[] = { 0 };
						vkCmdBindVertexBuffers( default_renderer->command_buffers[ default_renderer->current_frame ], 0, 1, &default_mesh_2d_plane->vertex_buffer->device_buff, offsets );
						vkCmdBindIndexBuffer( default_renderer->command_buffers[ default_renderer->current_frame ], default_mesh_2d_plane->index_buffer->device_buff, 0, VK_INDEX_TYPE_UINT32 );

						vkCmdDrawIndexed( default_renderer->command_buffers[ default_renderer->current_frame ], default_mesh_2d_plane->ind_list->size, 1, 0, 0, 0 );
					} //
				vkCmdEndRenderPass(
						default_renderer->command_buffers[ default_renderer->current_frame ] );
				//
				{
					image temp_image = list_get( default_renderer->frame_window->list_images, image, 0 );

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

							vkCmdPipelineBarrier( default_renderer->command_buffers[ default_renderer->current_frame ], src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier );

							barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
							barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
							barrier.image = this_image->ptr;
							barrier.srcAccessMask = 0;
							barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
							src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
							dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

							vkCmdPipelineBarrier( default_renderer->command_buffers[ default_renderer->current_frame ], src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier );
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
					blit.dstOffsets[ 1 ] = ( h_offset_3d ){ default_renderer->swapchain_extent.width, default_renderer->swapchain_extent.height, 1 };
					blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					blit.dstSubresource.mipLevel = 0;
					blit.dstSubresource.baseArrayLayer = 0;
					blit.dstSubresource.layerCount = 1;

					vkCmdBlitImage(
							default_renderer->command_buffers[ default_renderer->current_frame ],
							temp_image->ptr, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							this_image->ptr, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							1, &blit,
							VK_FILTER_LINEAR );
				}
				//

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

						vkCmdPipelineBarrier( default_renderer->command_buffers[ default_renderer->current_frame ], src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier );
					}

				vkEndCommandBuffer(
						default_renderer->command_buffers[ default_renderer->current_frame ] );
			}

			//

			VkPipelineStageFlags wait_stages[] = {
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

			h_info_submit submit_info = h_make_info_submit(
					1, adr( default_renderer->image_ready[ default_renderer->current_frame ] ),
					wait_stages,
					1, adr( default_renderer->command_buffers[ default_renderer->current_frame ] ),
					1, adr( default_renderer->image_done[ default_renderer->current_frame ] ) );

			h_submit_queue(
					default_renderer->queue, submit_info,
					default_renderer->flight_fences[ default_renderer->current_frame ] );

			//

			VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = adr( default_renderer->image_done[ default_renderer->current_frame ] );
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = adr( default_renderer->swapchain );
			presentInfo.pImageIndices = adr( default_renderer->current_frame );

			VkResult present_result = vkQueuePresentKHR( default_renderer->queue, adr( presentInfo ) );

			if( present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR )
				{
					renderer_update( default_renderer );
					vkQueueWaitIdle( default_renderer->queue );
					this_fence = 0;
					default_renderer->current_frame = 0;
				}

			default_renderer->current_frame = ( default_renderer->current_frame + 1 ) % default_renderer->list_swapchain_frames->size;
	} while(0);
}

#undef main
#define main      \
	int main()      \
	{               \
		$main_init(); \
		$main_loop(); \
		out 0;        \
	}               \
	pure $main_loop_init()

#endif