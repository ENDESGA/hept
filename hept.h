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

	#define PTR( NAME, ... )                       \
		make_struct( NAME ){                         \
			__VA_ARGS__ };                             \
		make_ptr( struct( NAME ) ) NAME;             \
		global NAME default_##NAME = null;           \
		global NAME current_##NAME = null;           \
		global pile pile_##NAME = null;              \
		fn set_##NAME( in NAME in_##NAME )      \
		{                                            \
			safe_ptr_set( current_##NAME, in_##NAME ); \
		}

	#define NEW_PTR( NAME, PARAMS, ... )                       \
		NAME new_##NAME PARAMS                                   \
		{                                                        \
			NAME this_##NAME = new_mem( struct( NAME ), 1 );       \
			DEF_START                                              \
			__VA_ARGS__                                            \
			DEF_END;                                               \
			pile_add( pile_##NAME, NAME, this_##NAME );            \
			ifnull( default_##NAME ) default_##NAME = this_##NAME; \
			current_##NAME = this_##NAME;                          \
			out this_##NAME;                                       \
		}

	#define PTR_PILE( NAME, PARAMS, SET_NEW, ... ) \
		PTR( NAME, __VA_ARGS__ )                     \
		NEW_PTR( NAME, PARAMS, SET_NEW )

//

/////// /////// /////// /////// /////// /////// ///////

/// OS

	#define OS( NAME, ... )                      \
		PTR_PILE( os_##NAME, (), {}, __VA_ARGS__ ) \
		os_##NAME make_os_##NAME

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

/////// /////// /////// /////// /////// /////// ///////

/// os_core

OS(
	core,
	h_instance instance;
)
( in text in_name )
{
	os_core result = new_os_core();
	//
	volkInitialize();

	//

	h_info_app info_app = h_make_info_app(
		in_name,
		h_make_version( 1, 0, 0 ),
		"hept",
		h_make_version( 1, 0, 0 ),
		h_make_version( 1, 3, 0 )
	);

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
			if( compare_text( desired_debug_layers[ i ], available_layers[ j ].layerName ) )
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
	#if OS_WINDOWS
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	#elif OS_LINUX
		VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
	#endif
	};

	h_info_instance instance_info = h_make_info_instance(
		ref( info_app ),
		enabled_debug_layer_count,
		( ptr( const char ) ptr( const ) )debug_layers,
		extension_count,
		( ptr( const char ) ptr( const ) )extensions
	);

	result->instance = h_new_instance( instance_info );
	//
	out result;
}

//

/// os_button

make_struct( os_button )
{
	flag pressed;
	u32 down;
	flag released;
};
make_ptr( struct( os_button ) ) os_button;

global list os_inputs = null;
global list os_inputs_active = null;

global u32 os_key[ 512 ] = { 0 };

	#define __clamp_key( _ ) ( ( _ & 0x00ffu ) + 0x100u )

//

/// os_file

OS(
	file,
	text path;
	text data;
	u32 size;
)
( in text in_path )
{
	os_file result = new_os_file();
	result->path = in_path;
	#if OS_WINDOWS
	HANDLE file = CreateFile( ( LPCSTR )result->path, GENERIC_READ, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null );
	if( file == INVALID_HANDLE_VALUE )
	{
		print( "Failed to open file: %s\n", result->path );
		out result;
	}

	DWORD file_size_low, file_size_high;
	file_size_low = GetFileSize( file, ref( file_size_high ) );
	result->size = file_size_low | ( ( size_t )file_size_high << 32 );

	HANDLE file_mapping = CreateFileMapping( file, null, PAGE_READONLY, 0, 0, null );
	result->data = to_text( MapViewOfFile( file_mapping, FILE_MAP_READ, 0, 0, 0 ) );
	CloseHandle( file_mapping );
	CloseHandle( file );
	#elif OS_LINUX
	int fd = syscall( SYS_open, result->path, O_RDONLY );
	if( fd == -1 )
	{
		print( "Failed to open file: %s\n", result->path );
		out result;
	}

	struct stat st;
	syscall( SYS_fstat, fd, &st );
	result->size = st.st_size;

	result->data = to_text( mmap( NULL, result->size, PROT_READ, MAP_PRIVATE, fd, 0 ) );
	syscall( SYS_close, fd );
	#endif

	out result;
}

//

/// os_timer

OS(
	timer,
	u32 fps;
	u64 start;
	u64 end;
	u32 time_ns;
)
( in u32 in_fps )
{
	os_timer result = new_os_timer();
	//
	result->fps = in_fps;
	result->time_ns = to_u32( to_f32( nano_per_sec ) / to_f32( result->fps ) );
	//
	out result;
}

fn start_os_timer( in os_timer in_timer )
{
	in_timer->start = get_nano();
}

fn wait_os_timer( in os_timer in_timer )
{
	in_timer->end = get_nano();
	u64 time_taken = in_timer->end - in_timer->start;
	if( time_taken >= in_timer->time_ns ) out;
	sleep_nano( in_timer->time_ns - time_taken );
}

fn change_os_timer( in os_timer in_timer, in u32 in_fps )
{
	in_timer->fps = in_fps;
	in_timer->time_ns = to_u32( to_f32( nano_per_sec ) / to_f32( in_timer->fps ) );
}

//

/// os_thread

	#if OS_WINDOWS
make_type( HANDLE ) os_thread_id;
	#elif OS_LINUX
make_type( pthread_t ) os_thread_id;
	#endif

OS(
	thread,
	fn_ptr( function, ptr( pure ), in ptr( pure ) );
	os_thread_id id;
)
( fn_ptr( in_function, ptr( pure ), in ptr( pure ) ) )
{
	os_thread result = new_os_thread();
	result->function = in_function;
	//
	#if OS_WINDOWS
	result->id = CreateThread( NULL, 0, to( LPTHREAD_START_ROUTINE, result->function ), NULL, 0, NULL );
	#elif OS_LINUX
	pthread_create( ref( result->id ), NULL, to( fn_ptr(, ptr( pure ), ptr( pure ) ), result->function ), NULL );
	#endif
	//
	out result;
}
//

/// os_machine

OS(
	machine,
	os_core core;
	u32 queue_family_index;
	h_physical_device physical_device;
	h_device device;
)
()
{
	os_machine result = new_os_machine();
	result->core = current_os_core;
	result->physical_device = null;
	//
	out result;
}

//

/// os_window

	#if OS_WINDOWS
inl LRESULT CALLBACK process_os_window( in HWND hwnd, in UINT u_msg, in WPARAM w_param, in LPARAM l_param );

OS(
	window,
	os_core core;
	text name;
	fn_ptr( render_fn, pure );
	flag ready, visible;
	u32 width, height;
	HWND hwnd;
	HINSTANCE inst;
	h_surface surface;
	h_surface_capabilities surface_capabilities;
	h_surface_format surface_format;
	h_present_mode present_mode;
)
	#elif OS_LINUX
fn process_os_window( ptr( Display ) in_disp );

OS(
	window,
	os_core core;
	text name;
	fn_ptr( render_fn, pure );
	flag ready, visible;
	u32 width, height;
	ptr( Display ) xdis;
	Window xwin;
	h_surface surface;
	h_surface_capabilities surface_capabilities;
	h_surface_format surface_format;
	h_present_mode present_mode;
)
	#endif
( in text in_name, fn_ptr( in_render_fn, pure ), in u32 in_width, in u32 in_height, in flag is_borderless )
{
	os_window result = new_os_window();
	result->core = current_os_core;
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
	AdjustWindowRect( ref( rect ), window_style, no );
	s32 this_width = rect.right - rect.left, this_height = rect.bottom - rect.top;

	WNDCLASS wc = {
		.lpfnWndProc = process_os_window,
		.hInstance = GetModuleHandle( null ),
		.hbrBackground = CreateSolidBrush( RGB( 0, 0, 0 ) ),
		.lpszClassName = result->name };

	ifn( RegisterClass( ref( wc ) ) )
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
		null
	);
	#elif OS_LINUX
	result->xdis = XOpenDisplay( NULL );
	if( result->xdis == NULL )
	{
		print( "Cannot open display\n" );
		out null;
	}

	s32 screen_num = DefaultScreen( result->xdis );
	s32 screen_width = DisplayWidth( result->xdis, screen_num );
	s32 screen_height = DisplayHeight( result->xdis, screen_num );

	Window root_win = RootWindow( result->xdis, screen_num );

	result->xwin = XCreateSimpleWindow( result->xdis, root_win, ( screen_width - result->width ) / 2, ( screen_height - result->height ) / 2, result->width, result->height, 1, BlackPixel( result->xdis, screen_num ), BlackPixel( result->xdis, screen_num ) );

	XStoreName( result->xdis, result->xwin, result->name );

	XSelectInput( result->xdis, result->xwin, ExposureMask | KeyPressMask | KeyReleaseMask );

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
	create_info.dpy = result->xdis;
	create_info.window = result->xwin;

	vkCreateXlibSurfaceKHR( result->core->instance, ref( create_info ), null, ref( result->surface ) );
	#elif OS_MACOS
	VkMacOSSurfaceCreateInfoMVK create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	create_info.pView = ( __bridge void* )( in_machine->window->window.contentView );

	vkCreateMacOSSurfaceMVK( in_machine->instance, ref( create_info ), null, ref( in_machine->surface ) );
	#endif
	//
	out result;
}

fn show_os_window( in os_window in_window )
{
	#if OS_WINDOWS
	ShowWindow( in_window->hwnd, SW_SHOWDEFAULT );
	UpdateWindow( in_window->hwnd );
	#elif OS_LINUX
	XMapWindow( in_window->xdis, in_window->xwin );
	XFlush( in_window->xdis );
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

	#define FORM( NAME, ... )                        \
		PTR_PILE(                                      \
			form_##NAME,                                 \
			( in os_machine in_machine ),                \
			{ this_form_##NAME->machine = in_machine; }, \
			os_machine machine;                          \
			__VA_ARGS__                                  \
		)                                              \
		form_##NAME make_form_##NAME

//

/////// /////// /////// /////// /////// /////// ///////

/// form_buffer

FORM( buffer,
			h_buffer_usage usage;
			h_mem_properties properties; )
( in os_machine in_machine, h_buffer_usage in_usage, h_mem_properties in_properties )
{
	form_buffer result = new_form_buffer( in_machine );
	result->usage = in_usage;
	result->properties = in_properties;
	out result;
}

global form_buffer form_buffer_vertex = null;
global form_buffer form_buffer_index = null;

//

/////// /////// /////// /////// /////// /////// ///////

/// form_mesh

PTR_PILE(
	form_mesh_attrib,
	( in h_format in_format, in u32 in_type_size, in u32 in_size, in text in_type_glsl ),
	{
		this_form_mesh_attrib->format = in_format;
		this_form_mesh_attrib->type_size = in_type_size;
		this_form_mesh_attrib->size = in_size;
		this_form_mesh_attrib->type_glsl = in_type_glsl;
	},
	h_format format;
	u32 type_size;
	u32 size;
	text type_glsl;
)

	#define $format_type_u8_size_1 VK_FORMAT_R8_UINT
global text $format_type_u8_size_1_text = "uint";
	#define $format_type_u8_size_2 VK_FORMAT_R8G8_UINT
global text $format_type_u8_size_2_text = "uvec2";
	#define $format_type_u8_size_3 VK_FORMAT_R8G8B8_UINT
global text $format_type_u8_size_3_text = "uvec3";
	#define $format_type_u8_size_4 VK_FORMAT_R8G8B8A8_UINT
global text $format_type_u8_size_4_text = "uvec4";

	#define $format_type_u16_size_1 VK_FORMAT_R16_UINT
global text $format_type_u16_size_1_text = "uint";
	#define $format_type_u16_size_2 VK_FORMAT_R16G16_UINT
global text $format_type_u16_size_2_text = "uvec2";
	#define $format_type_u16_size_3 VK_FORMAT_R16G16B16_UINT
global text $format_type_u16_size_3_text = "uvec3";
	#define $format_type_u16_size_4 VK_FORMAT_R16G16B16A16_UINT
global text $format_type_u16_size_4_text = "uvec4";

	#define $format_type_u32_size_1 VK_FORMAT_R32_UINT
global text $format_type_u32_size_1_text = "uint";
	#define $format_type_u32_size_2 VK_FORMAT_R32G32_UINT
global text $format_type_u32_size_2_text = "uvec2";
	#define $format_type_u32_size_3 VK_FORMAT_R32G32B32_UINT
global text $format_type_u32_size_3_text = "uvec3";
	#define $format_type_u32_size_4 VK_FORMAT_R32G32B32A32_UINT
global text $format_type_u32_size_4_text = "uvec4";

	#define $format_type_u64_size_1 VK_FORMAT_R64_UINT
global text $format_type_u64_size_1_text = "uint";
	#define $format_type_u64_size_2 VK_FORMAT_R64G64_UINT
global text $format_type_u64_size_2_text = "uvec2";
	#define $format_type_u64_size_3 VK_FORMAT_R64G64B64_UINT
global text $format_type_u64_size_3_text = "uvec3";
	#define $format_type_u64_size_4 VK_FORMAT_R64G64B64A64_UINT
global text $format_type_u64_size_4_text = "uvec4";

//

	#define $format_type_s8_size_1 VK_FORMAT_R8_SINT
global text $format_type_s8_size_1_text = "int";
	#define $format_type_s8_size_2 VK_FORMAT_R8G8_SINT
global text $format_type_s8_size_2_text = "ivec2";
	#define $format_type_s8_size_3 VK_FORMAT_R8G8B8_SINT
global text $format_type_s8_size_3_text = "ivec3";
	#define $format_type_s8_size_4 VK_FORMAT_R8G8B8A8_SINT
global text $format_type_s8_size_4_text = "ivec4";

	#define $format_type_s16_size_1 VK_FORMAT_R16_SINT
global text $format_type_s16_size_1_text = "int";
	#define $format_type_s16_size_2 VK_FORMAT_R16G16_SINT
global text $format_type_s16_size_2_text = "ivec2";
	#define $format_type_s16_size_3 VK_FORMAT_R16G16B16_SINT
global text $format_type_s16_size_3_text = "ivec3";
	#define $format_type_s16_size_4 VK_FORMAT_R16G16B16A16_SINT
global text $format_type_s16_size_4_text = "ivec4";

	#define $format_type_s32_size_1 VK_FORMAT_R32_SINT
global text $format_type_s32_size_1_text = "int";
	#define $format_type_s32_size_2 VK_FORMAT_R32G32_SINT
global text $format_type_s32_size_2_text = "ivec2";
	#define $format_type_s32_size_3 VK_FORMAT_R32G32B32_SINT
global text $format_type_s32_size_3_text = "ivec3";
	#define $format_type_s32_size_4 VK_FORMAT_R32G32B32A32_SINT
global text $format_type_s32_size_4_text = "ivec4";

	#define $format_type_s64_size_1 VK_FORMAT_R64_SINT
global text $format_type_s64_size_1_text = "int";
	#define $format_type_s64_size_2 VK_FORMAT_R64G64_SINT
global text $format_type_s64_size_2_text = "ivec2";
	#define $format_type_s64_size_3 VK_FORMAT_R64G64B64_SINT
global text $format_type_s64_size_3_text = "ivec3";
	#define $format_type_s64_size_4 VK_FORMAT_R64G64B64A64_SINT
global text $format_type_s64_size_4_text = "ivec4";

//

	#define $format_type_f16_size_1 VK_FORMAT_R16_SFLOAT
global text $format_type_f16_size_1_text = "float";
	#define $format_type_f16_size_2 VK_FORMAT_R16G16_SFLOAT
global text $format_type_f16_size_2_text = "vec2";
	#define $format_type_f16_size_3 VK_FORMAT_R16G16B16_SFLOAT
global text $format_type_f16_size_3_text = "vec3";
	#define $format_type_f16_size_4 VK_FORMAT_R16G16B16A16_SFLOAT
global text $format_type_f16_size_4_text = "vec4";

	#define $format_type_f32_size_1 VK_FORMAT_R32_SFLOAT
global text $format_type_f32_size_1_text = "float";
	#define $format_type_f32_size_2 VK_FORMAT_R32G32_SFLOAT
global text $format_type_f32_size_2_text = "vec2";
	#define $format_type_f32_size_3 VK_FORMAT_R32G32B32_SFLOAT
global text $format_type_f32_size_3_text = "vec3";
	#define $format_type_f32_size_4 VK_FORMAT_R32G32B32A32_SFLOAT
global text $format_type_f32_size_4_text = "vec4";

	#define $format_type_f64_size_1 VK_FORMAT_R64_SFLOAT
global text $format_type_f64_size_1_text = "double";
	#define $format_type_f64_size_2 VK_FORMAT_R64G64_SFLOAT
global text $format_type_f64_size_2_text = "dvec2";
	#define $format_type_f64_size_3 VK_FORMAT_R64G64B64_SFLOAT
global text $format_type_f64_size_3_text = "dvec3";
	#define $format_type_f64_size_4 VK_FORMAT_R64G64B64A64_SFLOAT
global text $format_type_f64_size_4_text = "dvec4";

	#define make_form_mesh_attrib( type, size ) new_form_mesh_attrib( $format_type_##type##_size_##size, size_( type ), size, $format_type_##type##_size_##size##_text )

global form_mesh_attrib form_mesh_attrib_pos2 = null;
global form_mesh_attrib form_mesh_attrib_pos3 = null;
global form_mesh_attrib form_mesh_attrib_uv = null;
global form_mesh_attrib form_mesh_attrib_rgb = null;
global form_mesh_attrib form_mesh_attrib_rgba = null;

//

FORM(
	mesh,
	u32 type_size;
	list attribs;
	text layout_glsl;
)
( in os_machine in_machine, in list in_attribs, in flag generate_glsl )
{
	form_mesh result = new_form_mesh( in_machine );
	//
	result->type_size = 0;
	result->attribs = in_attribs;
	result->layout_glsl = make_text( "", 0 );
	//
	iter_list( result->attribs, a )
	{
		form_mesh_attrib this_attrib = list_get( result->attribs, form_mesh_attrib, a );

		result->type_size += this_attrib->type_size * this_attrib->size;
	}
	//
	out result;
}

global form_mesh form_mesh_2d = null;
global form_mesh form_mesh_3d = null;

//

/////// /////// /////// /////// /////// /////// ///////

/// form_image

make_enum( image_type ){
	image_type_rgba = VK_IMAGE_ASPECT_COLOR_BIT,
	image_type_depth = VK_IMAGE_ASPECT_DEPTH_BIT,
	image_type_stencil = VK_IMAGE_ASPECT_STENCIL_BIT,
};

FORM(
	image,
	enum( image_type ) type;
	h_format format;
	h_sampler sampler;
)
( in os_machine in_machine, in enum( image_type ) in_type, in h_format in_format )
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

make_enum( frame_layer_type ){
	frame_layer_type_rgba = VK_IMAGE_ASPECT_COLOR_BIT,
	frame_layer_type_depth = VK_IMAGE_ASPECT_DEPTH_BIT,
	frame_layer_type_stencil = VK_IMAGE_ASPECT_STENCIL_BIT,
};

PTR_PILE(
	form_frame_layer,
	( in enum( frame_layer_type ) in_type ),
	{
		this_form_frame_layer->type = in_type;
		//
		this_form_frame_layer->attach_ref.attachment = 0;
		this_form_frame_layer->attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
		with( this_form_frame_layer->type )
		{
			is( frame_layer_type_rgba )
			{
				this_form_frame_layer->attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				skip;
			}

			is( frame_layer_type_depth )
				is( frame_layer_type_stencil )
			{
				this_form_frame_layer->attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				skip;
			}
		}
	},
	enum( frame_layer_type ) type;
	h_attachment_reference attach_ref;
)

//

make_enum( frame_type ){
	frame_type_present,
	frame_type_shader_read,
	frame_type_general,
};

FORM(
	frame,
	enum( frame_type ) type;
	h_format format;
	h_render_pass render_pass;
	list layers;
)
( in os_machine in_machine, in enum( frame_type ) in_type, in h_format in_format, in list in_layers )
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
			is( frame_layer_type_rgba )
			{
				rgba_count++;
				skip;
			}

			is( frame_layer_type_depth )
				is( frame_layer_type_stencil )
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
			is( frame_type_present )
			{
				attachments[ a ].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				skip;
			}

			is( frame_type_shader_read )
			{
				attachments[ a ].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				skip;
			}

			is( frame_type_general )
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
	h_queue queue;
)
( in os_machine in_machine )
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
}

//

/////// /////// /////// /////// /////// /////// ///////

/// form_module

	#define module_stage_vertex h_shader_stage_vertex
	#define module_stage_geometry h_shader_stage_geometry
	#define module_stage_fragment h_shader_stage_fragment
	#define module_stage_compute h_shader_stage_compute

FORM(
	module,
	h_shader_stage stage;
)
( in os_machine in_machine, in h_shader_stage in_stage )
{
	form_module result = new_form_module( in_machine );
	//
	result->stage = in_stage;
	//
	out result;
}

//

/////// /////// /////// /////// /////// /////// ///////

/// form_shader

PTR_PILE(
	form_shader_input,
	( in h_descriptor_set_layout_binding in_binding ),
	{
		this_form_shader_input->binding = in_binding;
	},
	h_descriptor_set_layout_binding binding;
)

FORM(
	shader,
	h_topology topology;
	list inputs;
)
( in os_machine in_machine, in h_topology in_topology, in list in_inputs )
{
	form_shader result = new_form_shader( in_machine );
	//
	result->topology = in_topology;
	result->inputs = in_inputs;
	//
	out result;
}

//

/////// /////// /////// /////// /////// /////// ///////

/// form_event

make_enum( event_type ){
	event_type_once,
	event_type_delay,
	event_type_always,
};

FORM(
	event,
	enum( event_type ) type;
)
( in os_machine in_machine, in enum( event_type ) in_type )
{
	form_event result = new_form_event( in_machine );
	//
	result->type = in_type;
	//
	out result;
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

/// OBJECT

	#define OBJECT( NAME, IN_FORM, ... )  \
		PTR_PILE(                           \
			NAME,                             \
			( in IN_FORM in_form ),           \
			{ this_##NAME->form = in_form; }, \
			IN_FORM form;                     \
			__VA_ARGS__                       \
		)                                   \
		NAME make_##NAME

//

/// OBJECT_FN

	#define is_empty_
	#define is_empty_const , in
	#define comma( ... ) is_empty_##__VA_ARGS__

	#define _OBJECT_FN( VERB, OBJ, ... )                                         \
		make_fn_ptr( ptr_##VERB##_##OBJ, OBJ, OBJ in_##OBJ comma( __VA_ARGS__ ) ); \
		OBJ fn_##VERB##_##OBJ( OBJ in_##OBJ comma( __VA_ARGS__ ) );                \
		ptr_##VERB##_##OBJ VERB##_##OBJ = fn_##VERB##_##OBJ;                       \
		OBJ fn_##VERB##_##OBJ( OBJ in_##OBJ comma( __VA_ARGS__ ) )

	#define OBJECT_FN( VERB, OBJ, ... ) OBJ VERB##_##OBJ( OBJ in_##OBJ comma( __VA_ARGS__ ) )

//

/////// /////// /////// /////// /////// /////// ///////

/// buffer

OBJECT(
	buffer,
	form_buffer,
	h_device_buffer device_buff;
	h_device_mem device_mem;
)
( in form_buffer in_form, u64 in_size )
{
	buffer result = new_buffer( in_form );
	//
	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = in_size;
	buffer_info.usage = result->form->usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	ptr( h_device_buffer ) temp_buff = new_mem( h_device_buffer, 1 );
	result->device_buff = val( temp_buff );
	ptr( h_device_mem ) temp_mem = new_mem( h_device_mem, 1 );
	result->device_mem = val( temp_mem );

	vkCreateBuffer( result->form->machine->device, ref( buffer_info ), NULL, ref( result->device_buff ) );

	h_mem_requirements mem_requirements;
	vkGetBufferMemoryRequirements( result->form->machine->device, result->device_buff, ref( mem_requirements ) );

	h_info_mem_allocate alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_requirements.size;
	alloc_info.memoryTypeIndex = h_find_mem( result->form->machine->physical_device, mem_requirements.memoryTypeBits, result->form->properties );

	vkAllocateMemory( result->form->machine->device, ref( alloc_info ), NULL, ref( result->device_mem ) );

	vkBindBufferMemory( result->form->machine->device, result->device_buff, result->device_mem, 0 );
	//
	out result;
}

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
	h_device_mem mem;
	u32 width;
	u32 height;
)
( in form_image in_form, in enum( image_state ) in_state, in u32 in_width, in u32 in_height )
{
	image result = new_image( in_form );
	result->state = in_state;
	result->width = in_width;
	result->height = in_height;

	//
	h_extent_3d temp_extent = {
		.width = result->width, // ceilf( f32( temp_image->width ) / 32. ) * 32.,
		.height = result->height, // ceilf( f32( temp_image->height ) / 32. ) * 32.,
		.depth = 1 };

	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties( result->form->machine->physical_device, result->form->format, ref( format_properties ) );

	if( !( format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT ) )
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
		VK_SAMPLE_COUNT_1_BIT
	);

	result->ptr = h_new_image( result->form->machine->device, image_info );

	h_mem_requirements mem_requirements;
	vkGetImageMemoryRequirements( result->form->machine->device, result->ptr, ref( mem_requirements ) );

	h_info_mem_allocate alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_requirements.size; // ceil( f64( mem_requirements.size ) / 1024. ) * 1024.;
	alloc_info.memoryTypeIndex = h_find_mem(
		result->form->machine->physical_device,
		mem_requirements.memoryTypeBits,
		( ( result->state == image_state_src ) ? h_set_flags( h_mem_property_host_visible, h_mem_property_host_coherent ) : h_set_flags( h_mem_property_device_local ) )
	);

	vkAllocateMemory( result->form->machine->device, ref( alloc_info ), null, ref( result->mem ) );

	vkBindImageMemory( result->form->machine->device, result->ptr, result->mem, 0 );
	//
	out result;
}

//

/////// /////// /////// /////// /////// /////// ///////

/// frame

OBJECT(
	frame,
	form_frame,
	h_framebuffer buffer;
	list images;
	list views;
	u32 max_w, max_h;
	h_info_begin_render_pass info_begin;
	VkClearValue clear_col, clear_dep;
)
( in form_frame in_form, in list in_images )
{
	frame result = new_frame( in_form );
	result->images = in_images;
	result->views = new_list( h_image_view );
	result->max_w = 0;
	result->max_h = 0;
	//
	iter( result->images->size, i )
	{
		image temp_image = list_get( result->images, image, i );
		result->max_w = max( result->max_w, temp_image->width );
		result->max_h = max( result->max_h, temp_image->height );
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
				0,
				1,
				0,
				1 } )
		);
		list_add( result->views, h_image_view, h_new_image_view( result->form->machine->device, image_view_info ) );
	}

	h_info_framebuffer framebuffer_info = h_make_info_framebuffer(
		result->form->render_pass,
		result->views->size,
		( ptr( h_image_view ) )( result->views->data ),
		result->max_w,
		result->max_h,
		1
	);
	result->buffer = h_new_framebuffer( result->form->machine->device, framebuffer_info );

	result->clear_col = ( VkClearValue ){ 0., 0., 0., 0. };
	// result->clear_dep = (VkClearValue){ 0.,0.,0.,0. };
	result->info_begin = h_make_info_begin_render_pass(
		result->form->render_pass,
		result->buffer,
		( ( VkRect2D ){ 0, 0, result->max_w, result->max_h } ),
		1,
		ref( result->clear_col )
	);
	//
	out result;
}

//

/////// /////// /////// /////// /////// /////// ///////

/// renderer

OBJECT(
	renderer,
	form_renderer,
	os_window ref_window;
	flag changed;
	h_viewport viewport;
	h_swapchain swapchain;
	h_format swapchain_format;
	h_extent swapchain_extent;
	u32 current_frame;
	form_frame form_frame_window;
	frame frame_window;
	list frames;
	u32 fence_id;
	ptr( h_command_buffer ) command_buffers;
	ptr( h_semaphore ) image_ready;
	ptr( h_semaphore ) image_done;
	ptr( h_fence ) flight_fences;
)
( in form_renderer in_form, in os_window in_window )
{
	renderer result = new_renderer( in_form );
	result->changed = yes;
	result->ref_window = in_window;
	result->swapchain = null;
	result->current_frame = 0;
	result->frame_window = null;
	result->frames = new_list( frame );
	result->fence_id = 0;

	//
	out result;
}

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
				// delete_image( temp_image );
			}
			// delete_frame( temp_frame );
			vkDestroySemaphore( in_renderer->form->machine->device, in_renderer->image_ready[ i ], null );
			vkDestroySemaphore( in_renderer->form->machine->device, in_renderer->image_done[ i ], null );
			vkDestroyFence( in_renderer->form->machine->device, in_renderer->flight_fences[ i ], null );
		}
		in_renderer->frames->size = 0;
	}

	//

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( in_renderer->form->machine->physical_device, in_renderer->ref_window->surface, ref( in_renderer->ref_window->surface_capabilities ) );

	h_info_swapchain swapchain_info = h_make_info_swapchain(
		in_renderer->ref_window->surface,
		in_renderer->ref_window->surface_capabilities.minImageCount + 1,
		in_renderer->ref_window->surface_format.format,
		in_renderer->ref_window->surface_format.colorSpace,
		in_renderer->ref_window->surface_capabilities.currentExtent,
		1,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		null,
		in_renderer->ref_window->surface_capabilities.currentTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		in_renderer->ref_window->present_mode,
		VK_TRUE,
		null
	);

	in_renderer->swapchain = h_new_swapchain( in_renderer->form->machine->device, swapchain_info );
	in_renderer->swapchain_format = in_renderer->ref_window->surface_format.format;
	in_renderer->swapchain_extent = in_renderer->ref_window->surface_capabilities.currentExtent;

	in_renderer->ref_window->width = in_renderer->swapchain_extent.width;
	in_renderer->ref_window->height = in_renderer->swapchain_extent.height;

	in_renderer->viewport = h_make_viewport(
		0.0,
		0.0,
		to_f32( in_renderer->swapchain_extent.width ),
		to_f32( in_renderer->swapchain_extent.height ),
		0.0,
		1.0
	);

	u32 temp_count = 0;
	ptr( h_image ) temp_images = null;
	vkGetSwapchainImagesKHR( in_renderer->form->machine->device, in_renderer->swapchain, ref( temp_count ), null );
	temp_images = new_mem( h_image, temp_count );
	vkGetSwapchainImagesKHR( in_renderer->form->machine->device, in_renderer->swapchain, ref( temp_count ), temp_images );

	list layers = new_list( form_frame_layer );
	form_frame_layer layer_rgba = new_form_frame_layer( frame_layer_type_rgba );
	list_add( layers, form_frame_layer, layer_rgba );

	// if( in_renderer->form != null ) delete_frame_form( in_renderer->form );
	in_renderer->form_frame_window = make_form_frame(
		in_renderer->form->machine,
		frame_type_present,
		in_renderer->swapchain_format, // VK_FORMAT_R8G8B8A8_UNORM,//in_renderer->swapchain_format,
		layers
	);

	iter( temp_count, i )
	{
		list temp_list_images = new_list( image );
		image temp_image = new_image( null );
		temp_image->ptr = temp_images[ i ];
		temp_image->width = in_renderer->swapchain_extent.width;
		temp_image->height = in_renderer->swapchain_extent.height;
		list_add( temp_list_images, image, temp_image );
		frame temp_frame = make_frame( in_renderer->form_frame_window, temp_list_images );
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
		in_renderer->frames->size
	);

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
			// delete_image( temp_image );
		}
		// delete_frame( in_renderer->frame_window );
	}

	form_image frame_image_form = make_form_image(
		in_renderer->form->machine,
		image_type_rgba,
		in_renderer->swapchain_format // VK_FORMAT_R8G8B8A8_UNORM// in_renderer->swapchain_format
	);

	image temp_image = make_image(
		frame_image_form,
		image_state_dst,
		in_renderer->swapchain_extent.width,
		in_renderer->swapchain_extent.height
	);
	//
	list temp_list_images = new_list( image );
	list_add( temp_list_images, image, temp_image );
	in_renderer->frame_window = make_frame( default_form_frame, temp_list_images );

	in_renderer->changed = no;

	out in_renderer;
}

//

/*OBJECT_FN( begin, renderer, in frame in_frame )
{
	vkCmdBeginRenderPass( in_renderer->command_buffers[ in_renderer->current_frame ], ref( in_frame->info_begin ), VK_SUBPASS_CONTENTS_INLINE );
	out in_renderer;
}

OBJECT_FN( end, renderer )
{
	vkCmdEndRenderPass( in_renderer->command_buffers[ in_renderer->current_frame ] );
	out in_renderer;
}*/

	#define start_drawing vkCmdBeginRenderPass( current_renderer->command_buffers[ current_renderer->current_frame ], ref( current_frame->info_begin ), VK_SUBPASS_CONTENTS_INLINE )

	#define stop_drawing vkCmdEndRenderPass( current_renderer->command_buffers[ current_renderer->current_frame ] )

//

/////// /////// /////// /////// /////// /////// ///////

/// mesh

make_struct( vertex_2d )
{
	struct( fvec2 ) pos;
	struct( fvec2 ) uv;
	struct( fvec3 ) rgb;
};
	#define make_struct_vertex_2d( x, y, u, v, r, g, b ) make( struct( vertex_2d ), .pos = { x, y }, .uv = { u, v }, .rgb = { r, g, b } )

make_struct( vertex_3d )
{
	struct( fvec3 ) pos;
	struct( fvec2 ) uv;
	struct( fvec3 ) rgb;
};

make_struct( ind_tri )
{
	u32 a, b, c;
};

make_struct( ind_line )
{
	u32 a, b;
};

//

global list list_update_mesh = null;

OBJECT(
	mesh,
	form_mesh,
	spinlock lock;
	flag update;
	buffer vertex_buffer;
	buffer index_buffer;
	list vertices;
	list indices;
	u32 vertex_n;
)
( in form_mesh in_form )
{
	mesh result = new_mesh( in_form );
	//
	result->update = yes;
	result->vertices = new_list( u32 ); //__new_list( 0, 1, result->form->type_size, new_mem( u8, result->form->type_size ) );
	result->vertices->size_type = result->form->type_size;
	result->indices = new_list( u32 );
	//
	out result;
}

global mesh mesh_square = null;

	#define mesh_add_quad( var, vertex_struct, tl, tr, br, bl ) \
		DEF_START                                                 \
		list_add( var->indices, u32, var->vertices->size );       \
		list_add( var->indices, u32, var->vertices->size + 1 );   \
		list_add( var->indices, u32, var->vertices->size + 2 );   \
		list_add( var->indices, u32, var->vertices->size );       \
		list_add( var->indices, u32, var->vertices->size + 2 );   \
		list_add( var->indices, u32, var->vertices->size + 3 );   \
		list_add( var->vertices, vertex_struct, tl );             \
		list_add( var->vertices, vertex_struct, tr );             \
		list_add( var->vertices, vertex_struct, br );             \
		list_add( var->vertices, vertex_struct, bl );             \
		DEF_END

OBJECT_FN( update, mesh )
{
	engage_spinlock( in_mesh->lock );
	if( in_mesh->update )
	{
		list_add( list_update_mesh, mesh, in_mesh );
		in_mesh->update = no;
	}
	vacate_spinlock( in_mesh->lock );
	out in_mesh;
}

OBJECT_FN( draw, mesh )
{
	engage_spinlock( in_mesh->lock );
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers( current_renderer->command_buffers[ current_renderer->current_frame ], 0, 1, ref( in_mesh->vertex_buffer->device_buff ), offsets );
	vkCmdBindIndexBuffer( current_renderer->command_buffers[ current_renderer->current_frame ], in_mesh->index_buffer->device_buff, 0, VK_INDEX_TYPE_UINT32 );
	vkCmdDrawIndexed( current_renderer->command_buffers[ current_renderer->current_frame ], in_mesh->indices->size, 1, 0, 0, 0 );
	vacate_spinlock( in_mesh->lock );
	out in_mesh;
}

//

//

/////// /////// /////// /////// /////// /////// ///////

/// module

OBJECT(
	module,
	form_module,
	form_mesh ref_form_mesh;
	text path;
	h_shader_module shader_module;
	os_file file;
	h_info_pipeline_shader_stage stage_info;
)
( in form_module in_form, in text in_path, in form_mesh in_form_mesh )
{
	module result = new_module( in_form );
	//
	result->ref_form_mesh = in_form_mesh;

	text glsl_name = make_text( in_path, 5 );
	join_text( glsl_name, ( ( in_form->stage == module_stage_vertex ) ? ( ".vert" ) : ( ".frag" ) ) );

	text spirv_name = make_text( glsl_name, 4 );
	join_text( spirv_name, ".spv" );

	//
	{
		text command = format_text( "glslangValidator -V %s -o %s", glsl_name, spirv_name );
		int sys_result = system( command );
		if( sys_result != 0 )
		{
			print( "failed to compile GLSL to SPIR-V\n" );
		}
	}
	//

	result->file = make_os_file( spirv_name );

	h_info_shader_module module_info = h_make_info_shader_module( result->file->data, result->file->size );
	result->shader_module = h_new_shader_module( result->form->machine->device, module_info );
	result->stage_info = h_make_info_pipeline_shader_stage( result->form->stage, result->shader_module, "main" );
	//
	out result;
}

//

/////// /////// /////// /////// /////// /////// ///////

/// shader

OBJECT(
	shader,
	form_shader,
	form_frame ref_form_frame;
	h_pipeline pipeline;
	h_pipeline_layout pipeline_layout;
	h_descriptor_set descriptor_set;
	list modules;
)
( in form_shader in_form, in form_frame in_form_frame, in list in_modules )
{
	shader result = new_shader( in_form );
	//
	result->ref_form_frame = in_form_frame;
	result->modules = in_modules;
	//
	form_mesh vert_form_mesh = null;
	ptr( h_info_pipeline_shader_stage ) stages = new_mem( h_info_pipeline_shader_stage, result->modules->size );
	iter_list( result->modules, m )
	{
		module this_module = list_get( result->modules, module, m );
		if( this_module->form->stage == module_stage_vertex )
		{
			vert_form_mesh = this_module->ref_form_mesh;
		}

		stages[ m ] = this_module->stage_info;
	}

	//

	h_vertex_binding vert_bindings[] = {
		h_make_vertex_binding_per_vertex( 0, vert_form_mesh->type_size ) };

	ptr( h_vertex_attribute ) vert_attributes = new_mem( h_vertex_attribute, vert_form_mesh->attribs->size );
	form_mesh_attrib temp_vert_attrib = null;
	u32 offset = 0;
	iter( vert_form_mesh->attribs->size, a )
	{
		temp_vert_attrib = list_get( vert_form_mesh->attribs, form_mesh_attrib, a );
		vert_attributes[ a ].location = a;
		vert_attributes[ a ].binding = 0;
		vert_attributes[ a ].format = temp_vert_attrib->format;
		vert_attributes[ a ].offset = offset;
		offset += temp_vert_attrib->type_size * temp_vert_attrib->size;
	}

	h_info_pipeline_vertex pipeline_vertex_info = h_make_info_pipeline_vertex(
		1, vert_bindings, vert_form_mesh->attribs->size, vert_attributes
	);

	//

	h_info_pipeline_assembly pipeline_input_assembly_info =
		h_make_info_pipeline_assembly( result->form->topology );

	//

	h_info_pipeline_raster raster_info = h_make_info_pipeline_raster(
		VK_POLYGON_MODE_FILL, 1.0, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE
	);

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
	colorBlending.pAttachments = ref( colorBlendAttachment );

	//

	VkPipelineDynamicStateCreateInfo dynamic_state_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = ( VkDynamicState[] ){VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}
	};

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
			.binding = 0, // 1,
 .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		 },
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1, // 2,
		.pBindings = ( const ptr( VkDescriptorSetLayoutBinding ) )( ref( bindings ) ),
	};

	if( vkCreateDescriptorSetLayout( result->form->machine->device, ref( layoutInfo ), null, ref( descriptorSetLayout ) ) != VK_SUCCESS )
	{
		print( "Failed to create descriptor set layout\n" );
	}

	h_info_pipeline_layout info_pipeline_layout = h_make_info_pipeline_layout( 1, ref( descriptorSetLayout ), 0, null );

	result->pipeline_layout = h_new_pipeline_layout( result->form->machine->device, info_pipeline_layout );

	//
	h_scissor scissor = { 0 };
	scissor.offset = ( VkOffset2D ){ 0, 0 };
	scissor.extent = ( h_extent ){ 256, 256 };

	h_info_pipeline_viewport viewport_info = h_make_info_pipeline_viewport(
		1, null, 1, null
	);
	//

	h_info_pipeline pipeline_info = h_make_info_pipeline(
		result->modules->size,
		stages,
		ref( pipeline_vertex_info ),
		ref( pipeline_input_assembly_info ),
		null,
		ref( viewport_info ),
		ref( raster_info ),
		ref( multisampling ),
		ref( depth_stencil ),
		ref( colorBlending ),
		ref( dynamic_state_info ),
		result->pipeline_layout,
		result->ref_form_frame->render_pass,
		0,
		null,
		0
	);
	pipeline_info.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

	// derivedPipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
	// derivedPipelineInfo.basePipelineHandle = basePipeline;
	// derivedPipelineInfo.basePipelineIndex = -1;

	result->pipeline = h_new_pipeline( result->form->machine->device, pipeline_info );

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
		.poolSizeCount = 1, // 2,
		.pPoolSizes = ( const ptr( VkDescriptorPoolSize ) ) & pool_sizes,
	};

	if( vkCreateDescriptorPool( result->form->machine->device, ref( poolInfo ), null, ref( descriptorPool ) ) != VK_SUCCESS )
	{
		print( "Failed to create descriptor pool\n" );
	}

	VkDescriptorSetAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = ref( descriptorSetLayout ),
	};

	if( vkAllocateDescriptorSets( result->form->machine->device, ref( allocInfo ), ref( result->descriptor_set ) ) != VK_SUCCESS )
	{
		print( "Failed to allocate descriptor set\n" );
	}

	//

	free_mem( stages );
	//
	out result;
}

global shader shader_2d = null;

OBJECT_FN( use, shader )
{
	vkCmdBindPipeline( current_renderer->command_buffers[ current_renderer->current_frame ], VK_PIPELINE_BIND_POINT_GRAPHICS, in_shader->pipeline );
	vkCmdBindDescriptorSets( current_renderer->command_buffers[ current_renderer->current_frame ], VK_PIPELINE_BIND_POINT_GRAPHICS, in_shader->pipeline_layout, 0, 1, ref( in_shader->descriptor_set ), 0, NULL );
	out in_shader;
}

//

/////// /////// /////// /////// /////// /////// ///////

OBJECT(
	event,
	form_event,
	list functions;
)
( in form_event in_form, in list in_functions )
{
	event result = new_event( in_form );
	//
	result->functions = in_functions;
	//
	out result;
}

make_type( fn_ptr( ( fn_event ), pure ) );

inl event perform_event( in event in_event )
{
	iter_list( in_event->functions, f )
	{
		list_get( in_event->functions, fn_event, f )();
	}
	out in_event;
}

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

/////// /////// /////// /////// /////// /////// ///////

/// global main defaults

global os_core main_os_core = null;
global os_machine main_machine = null;
global os_window main_os_window = null;

global form_renderer main_form_renderer = null;

/////// /////// /////// /////// /////// /////// ///////

/// main setup

fn main_init( in os_machine in_machine );
// fn main_draw( in os_machine in_machine );
// fn main_step( in os_machine in_machine );

//

/// global piles update

fn main_update_os_machines()
{
	iter_pile( pile_os_machine, m )
	{
		maybe maybe_machine = pile_find( pile_os_machine, os_machine, m );
		ifn( maybe_machine.valid ) next;
		os_machine this_machine = to( os_machine, maybe_machine.value );
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

			if( dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU or dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU )
			{
				u32 queue_family_n = 0;
				vkGetPhysicalDeviceQueueFamilyProperties( physical_devices[ i ], ref( queue_family_n ), null );
				ptr( VkQueueFamilyProperties ) queue_family_prop = new_mem( VkQueueFamilyProperties, queue_family_n );
				vkGetPhysicalDeviceQueueFamilyProperties( physical_devices[ i ], ref( queue_family_n ), queue_family_prop );
				iter( queue_family_n, j )
				{
					this_machine->queue_family_index = j;
					VkBool32 support_present;
					vkGetPhysicalDeviceSurfaceSupportKHR( physical_devices[ i ], j, current_os_window->surface, ref( support_present ) );

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

fn main_update_os_windows()
{
	iter_pile( pile_os_window, w )
	{
		maybe maybe_window = pile_find( pile_os_window, os_window, w );
		ifn( maybe_window.valid ) next;
		os_window this_window = to( os_window, maybe_window.value );

		//

	#if OS_WINDOWS
		once MSG msg = { 0 };

		as( PeekMessage( ref( msg ), NULL, 0, 0, PM_REMOVE ) )
		{
			if( msg.message == WM_QUIT )
			{
				// exit( 0 );
				skip;
			}
			else
			{
				TranslateMessage( ref( msg ) );
				DispatchMessage( ref( msg ) );
			}
		}

	#elif OS_LINUX
		process_os_window( this_window->xdis );
	#endif

		//

		u32 format_n = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR( current_os_machine->physical_device, this_window->surface, ref( format_n ), null );
		ptr( h_surface_format ) formats = new_mem( h_surface_format, format_n );
		vkGetPhysicalDeviceSurfaceFormatsKHR( current_os_machine->physical_device, this_window->surface, ref( format_n ), formats );
		this_window->surface_format = formats[ 0 ];
		free_mem( formats );

		u32 present_mode_n = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR( current_os_machine->physical_device, this_window->surface, ref( present_mode_n ), null );
		ptr( h_present_mode ) present_modes = new_mem( h_present_mode, present_mode_n );
		vkGetPhysicalDeviceSurfacePresentModesKHR( current_os_machine->physical_device, this_window->surface, ref( present_mode_n ), present_modes );
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
				show_os_window( this_window );
			}
		}
		else
		{
			make_renderer( main_form_renderer, this_window );
			this_window->ready = yes;
		}
	}
}

//

fn main_update_renderers()
{
	iter_pile( pile_renderer, r )
	{
		maybe maybe_renderer = pile_find( pile_renderer, renderer, r );
		ifn( maybe_renderer.valid ) next;

		renderer this_renderer = cast( maybe_renderer.value, renderer );
		set_renderer( this_renderer );

		//

		if( this_renderer->changed ) refresh_renderer( this_renderer );

		//

		{
			vkWaitForFences( this_renderer->form->machine->device, 1, ref( this_renderer->flight_fences[ this_renderer->fence_id ] ), VK_TRUE, UINT64_MAX );

			VkResult aquire_result = vkAcquireNextImageKHR(
				this_renderer->form->machine->device, this_renderer->swapchain, UINT64_MAX, this_renderer->image_ready[ this_renderer->current_frame ], VK_NULL_HANDLE, ref( this_renderer->current_frame )
			);

			if( aquire_result == VK_ERROR_OUT_OF_DATE_KHR || aquire_result == VK_SUBOPTIMAL_KHR )
			{
				refresh_renderer( this_renderer );
				main_update_renderers();
				next;
				// recreateSwapChain();
				// return;
				// print( "RESIZE2" );

				// vkResetFences( this_renderer->form->machine->device, 1, ref( this_renderer->flight_fences[ this_fence ] ) );
				// renderer_update( this_renderer );
				// this_fence = 0;
				// vkResetFences( this_renderer->form->machine->device, 1, ref( this_renderer->flight_fences[ this_renderer->fence_id ] ) );
				// refresh_renderer( this_renderer );
				// next;
				// loop{};
				// abort();
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
			ref( begin_info )
		);
		//

		{
			h_scissor scissor = { 0 };
			scissor.offset = ( VkOffset2D ){ 0, 0 };
			scissor.extent = this_renderer->swapchain_extent;
			// scissor.extent.width /= 2;

			vkCmdSetScissor( this_renderer->command_buffers[ this_renderer->current_frame ], 0, 1, ref( scissor ) );
			h_viewport viewport = h_make_viewport(
				0.0,
				0.0,
				to_f32( this_renderer->swapchain_extent.width ),
				to_f32( this_renderer->swapchain_extent.height ),
				0.0,
				1.0
			);
			vkCmdSetViewport( this_renderer->command_buffers[ this_renderer->current_frame ], 0, 1, ref( viewport ) );
		}

		//

		iter_list( list_update_mesh, m )
		{
			mesh this_mesh = list_get( list_update_mesh, mesh, m );
			engage_spinlock(this_mesh->lock);
			if( this_mesh->vertex_buffer == null )
			{
				this_mesh->vertex_buffer = make_buffer( form_buffer_vertex, this_mesh->form->type_size * this_mesh->vertices->size );
			}

			if( this_mesh->index_buffer == null )
			{
				this_mesh->index_buffer = make_buffer( form_buffer_index, size_u32 * this_mesh->indices->size );
			}

			vkCmdUpdateBuffer( default_renderer->command_buffers[ default_renderer->current_frame ], this_mesh->vertex_buffer->device_buff, 0, this_mesh->form->type_size * this_mesh->vertices->size, this_mesh->vertices->data );
			vkCmdUpdateBuffer( default_renderer->command_buffers[ default_renderer->current_frame ], this_mesh->index_buffer->device_buff, 0, size_u32 * this_mesh->indices->size, this_mesh->indices->data );
			vacate_spinlock(this_mesh->lock);
		}

		//

		this_renderer->ref_window->render_fn();

		//

		// if( 0 )
		{
			image temp_image = list_get( this_renderer->frame_window->images, image, 0 );

			// if( 0 )
			{ //
				VkImageMemoryBarrier barrier = { 0 };
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.image = temp_image->ptr; // default_frame_image->ptr;

				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				barrier.subresourceRange.baseMipLevel = 0;
				barrier.subresourceRange.levelCount = 1;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;

				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

				vkCmdPipelineBarrier( this_renderer->command_buffers[ this_renderer->current_frame ], src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, ref( barrier ) );

				barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.image = this_image->ptr;
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

				vkCmdPipelineBarrier( this_renderer->command_buffers[ this_renderer->current_frame ], src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, ref( barrier ) );
			}

			//

			VkImageBlit blit = { 0 };
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
				temp_image->ptr,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				this_image->ptr,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				ref( blit ),
				VK_FILTER_LINEAR
			);
		}

		// if( 0 )
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

			vkCmdPipelineBarrier( this_renderer->command_buffers[ this_renderer->current_frame ], src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, ref( barrier ) );
		}

		vkEndCommandBuffer( this_renderer->command_buffers[ this_renderer->current_frame ] );

		//

		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		h_info_submit submit_info = h_make_info_submit(
			1, ref( this_renderer->image_ready[ this_renderer->current_frame ] ), wait_stages, 1, ref( this_renderer->command_buffers[ this_renderer->current_frame ] ), 1, ref( this_renderer->image_done[ this_renderer->current_frame ] )
		);

		h_submit_queue(
			this_renderer->form->queue, submit_info, this_renderer->flight_fences[ this_renderer->current_frame ]
		);

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
			// print("AHHHHH\n");
			// update_renderer( this_renderer );
			// vkQueueWaitIdle( this_renderer->form->queue );
			// this_fence = 0;
			// this_renderer->current_frame = 0;
			refresh_renderer( this_renderer );
			main_update_renderers();
			next;
			// this_renderer->fence_id = 0;
			// exit(0);
			// abort();
		}

		this_renderer->current_frame = ( this_renderer->current_frame + 1 ) % this_renderer->frames->size;
	}
}

//

fn main_update_meshes()
{
	/*iter_pile( pile_mesh, e )
	{
	maybe maybe_mesh = pile_find( pile_mesh, mesh, e );
	ifn( maybe_mesh.valid ) next;

	mesh this_mesh = to(mesh, maybe_mesh.value);
	set_mesh( this_mesh );
	//

	ifn(this_mesh->update) next;
	update_mesh(this_mesh);
	}*/
}

//

fn main_update_events()
{
	iter_pile( pile_event, e )
	{
		maybe maybe_event = pile_find( pile_event, event, e );
		ifn( maybe_event.valid ) next;

		event this_event = to( event, maybe_event.value );
		set_event( this_event );
		//

		perform_event( this_event );
	}
}

//

/////// /////// /////// /////// /////// /////// ///////

/// step thread

fn reset_button_events()
{
	// lock_list(os_inputs_active);
	iter_list( os_inputs_active, i )
	{
		// os_button button = list_pop_front( os_inputs_active, os_button );
		os_button button = to(os_button,safe_ptr_get( list_get( os_inputs_active, os_button, 0 ) ));
		list_shift( os_inputs_active, -1 );
		safe_u8_set( button->pressed, no );
		safe_u8_set( button->released, no );
	}
	// unlock_list(os_inputs_active);
}

global os_timer timer_main_thread = null;

inl ptr( pure ) main_thread( in ptr( pure ) in_ptr )
{
	timer_main_thread = make_os_timer( 120 );
	loop
	{
		start_os_timer( timer_main_thread );
		main_update_events();
		reset_button_events();
		wait_os_timer( timer_main_thread );
	}
}

//

/////// /////// /////// /////// /////// /////// ///////

/// hept setup

fn hept_forms( in os_machine in_machine )
{
	//
	form_mesh_attrib_pos2 = make_form_mesh_attrib( f32, 2 );
	form_mesh_attrib_pos3 = make_form_mesh_attrib( f32, 3 );
	form_mesh_attrib_uv = make_form_mesh_attrib( f32, 2 );
	form_mesh_attrib_rgb = make_form_mesh_attrib( f32, 3 );
	form_mesh_attrib_rgba = make_form_mesh_attrib( f32, 4 );
	//
	list attribs_2d = new_list( form_mesh_attrib );
	list_add( attribs_2d, form_mesh_attrib, form_mesh_attrib_pos2 );
	list_add( attribs_2d, form_mesh_attrib, form_mesh_attrib_uv );
	list_add( attribs_2d, form_mesh_attrib, form_mesh_attrib_rgb );
	form_mesh_2d = make_form_mesh( in_machine, attribs_2d, yes );
	//
	list_update_mesh = new_list( mesh );
	//
	form_buffer_vertex = make_form_buffer(
		in_machine,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, // h_set_flags( h_buffer_usage_vertex, h_buffer_usage_transfer_dst),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	form_buffer_index = make_form_buffer(
		in_machine,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, // h_set_flags( h_buffer_usage_index, h_buffer_usage_transfer_dst),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

fn hept_defaults( in os_machine in_machine )
{
	list layers = new_list( form_frame_layer );
	list_add( layers, form_frame_layer, new_form_frame_layer( frame_layer_type_rgba ) );
	default_form_frame = make_form_frame( in_machine, frame_type_general, VK_FORMAT_B8G8R8A8_UNORM, layers );
}

fn hept_init( in os_machine in_machine )
{
	main_update_os_machines();
	//
	hept_defaults( in_machine );
	//

	//
	main_form_renderer = make_form_renderer( in_machine );

	default_os_thread = make_os_thread( main_thread ); //////////////////////////////////////////////

	//

	//
	mesh_square = make_mesh( form_mesh_2d );
	mesh_add_quad(
		mesh_square,
		struct( vertex_2d ),
		make_struct_vertex_2d( 0, 0, 0.f, 0.f, 0.f, 1.f, .75f ),
		make_struct_vertex_2d( 0, 0, 0.f, 0.f, 0.f, 1.f, .75f ),
		make_struct_vertex_2d( 0, 0, 0.f, 0.f, 0.f, 1.f, .75f ),
		make_struct_vertex_2d( 0, 0, 0.f, 0.f, 0.f, 1.f, .75f )
	);
	update_mesh( mesh_square );
	//
	form_module form_module_vert = make_form_module( in_machine, module_stage_vertex );
	form_module form_module_geom = make_form_module( in_machine, module_stage_geometry );
	form_module form_module_frag = make_form_module( in_machine, module_stage_fragment );
	form_module form_module_comp = make_form_module( in_machine, module_stage_compute );
	//
	module module_2d_vert = make_module( form_module_vert, "default_2d", form_mesh_2d );
	module module_2d_frag = make_module( form_module_frag, "default_2d", null );
	//
	// list inputs = new_list(form_shader_input);
	// list_add(inputs,form_shader_input, new_form_shader_input(shader_binding));
	form_shader form_shader_2d_tri = make_form_shader( in_machine, h_topology_tri, null );
	//
	list modules = new_list( module );
	list_add( modules, module, module_2d_vert );
	list_add( modules, module, module_2d_frag );
	shader_2d = make_shader( form_shader_2d_tri, default_form_frame, modules );
}

fn hept_update()
{
	main_update_os_windows();
	main_update_meshes();
	main_update_renderers();
	// main_update_shaders();
	// main_update_events();
}

fn os_step( in os_machine in_machine )
{
	//
}

fn hept_piles()
{
	// os
	pile_os_core = new_pile( os_core );
	pile_os_file = new_pile( os_file );
	pile_os_timer = new_pile( os_timer );
	pile_os_thread = new_pile( os_thread );
	pile_os_window = new_pile( os_window );
	pile_os_machine = new_pile( os_machine );
	// form
	pile_form_buffer = new_pile( form_buffer );
	pile_form_mesh_attrib = new_pile( form_mesh_attrib );
	pile_form_mesh = new_pile( form_mesh );
	pile_form_image = new_pile( form_image );
	pile_form_frame_layer = new_pile( form_frame_layer );
	pile_form_frame = new_pile( form_frame );
	pile_form_renderer = new_pile( form_renderer );
	pile_form_module = new_pile( form_module );
	pile_form_shader = new_pile( form_shader );
	pile_form_event = new_pile( form_event );
	// object
	pile_buffer = new_pile( buffer );
	pile_mesh = new_pile( mesh );
	pile_image = new_pile( image );
	pile_frame = new_pile( frame );
	pile_renderer = new_pile( renderer );
	pile_module = new_pile( module );
	pile_shader = new_pile( shader );
	pile_event = new_pile( event );
}

//

	#define get_key( _ )

	#define pressed_key( _ ) safe_u8_get( ( list_get( os_inputs, os_button, os_key[ ( _ ) ] ) )->pressed )
	#define held_key( _ ) ( safe_u32_get( ( list_get( os_inputs, os_button, os_key[ ( _ ) ] ) )->down ) > 0 )
	#define released_key( _ ) safe_u8_get( ( list_get( os_inputs, os_button, os_key[ ( _ ) ] ) )->released )

	#if OS_WINDOWS
inl LRESULT CALLBACK process_os_window( HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param )
{
	//

	switch( u_msg )
	{
	case WM_DESTROY:
		PostQuitMessage( 0 );
		return 0;
	case WM_SIZE:
		refresh_renderer( current_renderer );
		hept_update( );
		return 1;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			os_button button = null;
			if( os_key[ w_param ] == 0 )
			{
				os_key[ w_param ] = os_inputs->size;
				button = new_mem( struct( os_button ), 1 );
				list_add( os_inputs, os_button, button );
			}
			else
			{
				button = list_get( os_inputs, os_button, os_key[ w_param ] );
			}

			if( button->down == 0 )
			{
				button->pressed = yes;
				list_add( os_inputs_active, os_button, button );
			}
			button->down++;
			break;
		}

	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			if( os_key[ w_param ] != 0 )
			{
				os_button button = list_get( os_inputs, os_button, os_key[ w_param ] );
				if( button->down > 0 )
				{
					button->released = yes;
					list_add( os_inputs_active, os_button, button );
				}
				button->down = 0;
			}
			break;
		}

	case WM_LBUTTONDOWN:
		// handle_button_event( &mouse.left_button, yes );
		break;
	/*case WM_RBUTTONDOWN:
	handle_button_event( &mouse.right_button, yes );
	break;
	case WM_MBUTTONDOWN:
	handle_button_event( &mouse.middle_button, yes );
	break;
	case WM_LBUTTONUP:
	handle_button_event( &mouse.left_button, no );
	break;
	case WM_RBUTTONUP:
	handle_button_event( &mouse.right_button, no );
	break;
	case WM_MBUTTONUP:
	handle_button_event( &mouse.middle_button, no );
	break;
	case WM_MOUSEMOVE:
	mouse.x = LOWORD( l_param );
	mouse.y = HIWORD( l_param );
	break;*/
	default:
		return DefWindowProc( hwnd, u_msg, w_param, l_param );
	}
	return 0;
}
	#elif OS_LINUX
void process_os_window( Display* in_disp )
{
	XEvent e;

	as( XPending( in_disp ) )
	{
		XNextEvent( in_disp, &e );

		with( e.type )
		{
			is( Expose )
			{
				refresh_renderer( current_renderer );
				hept_update();
				skip;
			}

			is( KeyPress )
			{
				u32 key = XLookupKeysym( &e.xkey, 0 );

				if( key >= 256 ) key = __clamp_key( key );

				os_button button = null;
				if( safe_u32_get( os_key[ key ] ) == 0 )
				{
					safe_u32_set( os_key[ key ], os_inputs->size );
					button = new_mem( struct( os_button ), 1 );
					list_safe_add( os_inputs, os_button, button );
				}
				else
				{
					button = safe_ptr_get( list_get( os_inputs, os_button, os_key[ key ] ) );
				}

				if( safe_u32_get( button->pressed ) == no and safe_u32_get( button->down ) == 0 )
				{
					safe_u8_set( button->pressed, yes );
					list_safe_add( os_inputs_active, os_button, button );
				}
				safe_u32_inc( button->down );
				skip;
			}

			is( KeyRelease )
			{
				u32 key = XLookupKeysym( &e.xkey, 0 );

				if( key >= 256 ) key = __clamp_key( key );

				if( safe_u32_get( os_key[ key ] ) != 0 )
				{
					os_button button = safe_ptr_get( list_get( os_inputs, os_button, os_key[ key ] ) );
					safe_u8_set( button->pressed, no );
					if( safe_u32_get( button->down ) > 0 )
					{
						safe_u8_set( button->released, yes );
						list_safe_add( os_inputs_active, os_button, button );
					}
					safe_u32_set( button->down, 0 );
				}
				skip;
			}

			/*	break;
			case KeyPress:
			handle_button_event( &keys[ XLookupKeysym( &e.xkey, 0 ) ], yes );
			break;
			case KeyRelease:
			handle_button_event( &keys[ XLookupKeysym( &e.xkey, 0 ) ], no );
			break;
			case ButtonPress:
			if( e.xbutton.button == Button1 )
			handle_button_event( &mouse.left_button, yes );
			else if( e.xbutton.button == Button2 )
			handle_button_event( &mouse.middle_button, yes );
			else if( e.xbutton.button == Button3 )
			handle_button_event( &mouse.right_button, yes );
			break;
			case ButtonRelease:
			if( e.xbutton.button == Button1 )
			handle_button_event( &mouse.left_button, no );
			else if( e.xbutton.button == Button2 )
			handle_button_event( &mouse.middle_button, no );
			else if( e.xbutton.button == Button3 )
			handle_button_event( &mouse.right_button, no );
			break;
			case MotionNotify:
			mouse.x = e.xmotion.x;
			mouse.y = e.xmotion.y;
			break;*/
		}
	}
}
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

fn main_core()
{
	os_inputs = new_list( os_button );
	os_inputs_active = new_list( os_button );
	os_button button = new_mem( struct( os_button ), 1 );
	list_add( os_inputs, os_button, button );

	main_os_core = make_os_core( "hept" );
	main_machine = make_os_machine();

	hept_forms( main_machine );

	main_init( main_machine );
	hept_init( main_machine );

	loop
	{
		hept_update();
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
		fn main_init( in os_machine in_machine )

#ifdef COMPILER_MSVC
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) { return main(); };
#endif

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

/// thing

	#define make_thing( NAME, PARTS, ... )               \
		make_struct( NAME ) PARTS;                         \
		make_ptr( struct( NAME ) ) NAME;                   \
		NAME new_##NAME()                                  \
		{                                                  \
			NAME this_##NAME = new_mem( struct( NAME ), 1 ); \
			out this_##NAME;                                 \
		}                                                  \
		NAME make_##NAME __VA_ARGS__

/*
#define __new_thing( NAME )           \
NAME new_##NAME()                   \
{                                   \
out new_mem( struct( NAME ), 1 ); \
}

#define make_thing( NAME, ... )       \
make_struct( NAME ) __VA_ARGS__ ; \
make_ptr( struct( NAME ) ) NAME;    \
__new_thing( NAME );                \
global NAME default_##NAME = null;
*/

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

//

/// keycodes

	#if OS_WINDOWS
		#define key_a 'A'
		#define key_b 'B'
		#define key_c 'C'
		#define key_d 'D'
		#define key_e 'E'
		#define key_f 'F'
		#define key_g 'G'
		#define key_h 'H'
		#define key_i 'I'
		#define key_j 'J'
		#define key_k 'K'
		#define key_l 'L'
		#define key_m 'M'
		#define key_n 'N'
		#define key_o 'O'
		#define key_p 'P'
		#define key_q 'Q'
		#define key_r 'R'
		#define key_s 'S'
		#define key_t 'T'
		#define key_u 'U'
		#define key_v 'V'
		#define key_w 'W'
		#define key_x 'X'
		#define key_y 'Y'
		#define key_z 'Z'
		#define key_0 '0'
		#define key_1 '1'
		#define key_2 '2'
		#define key_3 '3'
		#define key_4 '4'
		#define key_5 '5'
		#define key_6 '6'
		#define key_7 '7'
		#define key_8 '8'
		#define key_9 '9'
		#define key_backspace VK_BACK
		#define key_tab VK_TAB
		#define key_return VK_RETURN
		#define key_escape VK_ESCAPE
		#define key_space VK_SPACE
		#define key_up VK_UP
		#define key_down VK_DOWN
		#define key_left VK_LEFT
		#define key_right VK_RIGHT
	#elif OS_LINUX
		#define key_a XK_a
		#define key_b XK_b
		#define key_c XK_c
		#define key_d XK_d
		#define key_e XK_e
		#define key_f XK_f
		#define key_g XK_g
		#define key_h XK_h
		#define key_i XK_i
		#define key_j XK_j
		#define key_k XK_k
		#define key_l XK_l
		#define key_m XK_m
		#define key_n XK_n
		#define key_o XK_o
		#define key_p XK_p
		#define key_q XK_q
		#define key_r XK_r
		#define key_s XK_s
		#define key_t XK_t
		#define key_u XK_u
		#define key_v XK_v
		#define key_w XK_w
		#define key_x XK_x
		#define key_y XK_y
		#define key_z XK_z
		#define key_0 XK_0
		#define key_1 XK_1
		#define key_2 XK_2
		#define key_3 XK_3
		#define key_4 XK_4
		#define key_5 XK_5
		#define key_6 XK_6
		#define key_7 XK_7
		#define key_8 XK_8
		#define key_9 XK_9
		#define key_space XK_space

		#define key_backspace __clamp_key( XK_BackSpace )
		#define key_tab __clamp_key( XK_Tab )
		#define key_return __clamp_key( XK_Return )
		#define key_escape __clamp_key( XK_Escape )
		#define key_up __clamp_key( XK_Up )
		#define key_down __clamp_key( XK_Down )
		#define key_left __clamp_key( XK_Left )
		#define key_right __clamp_key( XK_Right )
	#endif

//

#endif

/////// /////// /////// /////// /////// /////// ///////