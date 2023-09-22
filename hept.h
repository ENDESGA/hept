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

	#include <Hephaestus.h>

//

global flag hept_exit = no;

//

/////// /////// /////// /////// /////// /////// ///////

global list list_object_piles = null;

	#define make_object( NAME, ... )                             \
		make_struct( NAME )                                        \
		{                                                          \
			u32 pile_id;                                             \
			__VA_ARGS__                                              \
		};                                                         \
		make_ptr( struct( NAME ) ) NAME;                           \
		global NAME current_##NAME = null;                         \
		global pile pile_##NAME = null;                            \
		fn set_current_##NAME( in NAME in_##NAME )                 \
		{                                                          \
			safe_ptr_set( current_##NAME, in_##NAME );               \
		}                                                          \
		inl NAME assign_##NAME()                                   \
		{                                                          \
			NAME this = new_mem( struct( NAME ), 1 );                \
			if( safe_ptr_get( pile_##NAME ) == null )                                \
			{                                                        \
				safe_ptr_set(pile_##NAME, new_pile( NAME ));                        \
				list_safe_add( list_object_piles, pile, pile_##NAME );      \
			}                                                        \
			pile_safe_add( pile_##NAME, NAME, this );                     \
			this->pile_id = safe_u32_get(pile_##NAME->prev_pos);                   \
			if( current_##NAME == null ) set_current_##NAME( this ); \
			out this;                                                \
		}                                                          \
		fn delete_##NAME( in NAME in_##NAME )                      \
		{                                                          \
			pile_safe_delete( pile_##NAME, in_##NAME->pile_id );          \
			free_mem( in_##NAME );                                   \
		}                                                          \
		NAME new_##NAME

//

/*
//

/////// /////// /////// /////// /////// /////// ///////

// NAME
// -------
// does thing

make_object(
	NAME,
	ELEMENTS;
	ELEMENTS;
	ELEMENTS;
)( in PARAMS, in PARAMS )
{
	#ifdef hept_debug
	print_error( PARAMS == null, "NAME: PARAMS is null" );
	#endif
	//
	NAME this = assign_NAME();
	//

	//
	#ifdef hept_trace
	print_trace( "new NAME: ID: %d",this->pile_id );
	#endif
	//
	out this;
}

//
*/

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

/// os
/*
	os_* objects are directly associated with the operating system
	hept is executed on, and are fundamental to creating a hept
	environment that interacts with files, windows, and hardware.
*/

/////// /////// /////// /////// /////// /////// ///////

// os_file
// -------
// connects and holds a file stored on the system

make_object(
	os_file,
	text path;
	text data;
	u32 size;
)( in text in_path )
{
	#ifdef hept_debug
	print_error( in_path == null, "os_file: in_path is null" );
	#endif
	//
	os_file this = assign_os_file();
	//
	this->path = in_path;
	#if OS_WINDOWS
	HANDLE file = CreateFile( ( LPCSTR )this->path, GENERIC_READ, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null );

		#ifdef hept_debug
	print_error( file == INVALID_HANDLE_VALUE, "failed to open: %s", this->path );
		#endif

	DWORD file_size_low, file_size_high;
	file_size_low = GetFileSize( file, ref( file_size_high ) );
	this->size = file_size_low | ( ( size_t )file_size_high << 32 );

	HANDLE file_mapping = CreateFileMapping( file, null, PAGE_READONLY, 0, 0, null );
	this->data = to_text( MapViewOfFile( file_mapping, FILE_MAP_READ, 0, 0, 0 ) );
	CloseHandle( file_mapping );
	CloseHandle( file );
	#elif OS_LINUX
	s32 fd = syscall( SYS_open, this->path, O_RDONLY );

		#ifdef hept_debug
	print_error( fd == -1, "os_file: failed to open: %s", this->path );
		#endif

	struct( stat ) st;
	syscall( SYS_fstat, fd, ref( st ) );
	this->size = st.st_size;

	this->data = to_text( mmap( NULL, this->size, PROT_READ, MAP_PRIVATE, fd, 0 ) );
	syscall( SYS_close, fd );
	#endif
	//
	#ifdef hept_trace
	print_trace( "new os_file: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

fn write_file( in text in_path, in text in_contents )
{
	#if OS_WINDOWS
	HANDLE hFile = CreateFileA( in_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile != INVALID_HANDLE_VALUE )
	{
		DWORD bytesWritten;
		WriteFile( hFile, in_contents, text_length( in_contents ), &bytesWritten, NULL );
		CloseHandle( hFile );
	}
	#elif OS_LINUX
	s32 fd = open( in_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );
	if( fd != -1 )
	{
		write( fd, in_contents, text_length( in_contents ) );
		close( fd );
	}
	#endif
}

flag check_file( in text in_path )
{
	#if OS_WINDOWS
	DWORD file_attr = GetFileAttributesA( in_path );
	out( file_attr != INVALID_FILE_ATTRIBUTES ) and !( file_attr & FILE_ATTRIBUTE_DIRECTORY );
	#elif OS_LINUX
	out access( in_path, F_OK ) != -1;
	#endif
}

// folder

flag check_folder( in text in_path )
{
	#if OS_WINDOWS
	DWORD file_attr = GetFileAttributesA( in_path );
	out( ( file_attr != INVALID_FILE_ATTRIBUTES ) and ( file_attr & FILE_ATTRIBUTE_DIRECTORY ) );
	#elif OS_LINUX
	struct stat st;
	out( stat( in_path, &st ) == 0 ) and S_ISDIR( st.st_mode );
	#endif
}

fn make_folder( in text in_path )
{
	if( check_folder( in_path ) ) out;
	#if OS_WINDOWS
	CreateDirectoryA( in_path, NULL );
	#elif OS_LINUX
	mkdir( in_path, 0755 );
	#endif
}

fn copy_folder( in text in_src_path, in text in_dest_path )
{
	ifn( check_folder( in_src_path ) ) out;
	#if OS_WINDOWS
	SHFILEOPSTRUCTA shFileOp = { 0 };
	shFileOp.wFunc = FO_COPY;
	shFileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;
	shFileOp.pFrom = in_src_path;
	shFileOp.pTo = in_dest_path;
	SHFileOperationA( &shFileOp );
	#elif OS_LINUX
	text command = text_format( "cp -r %s %s", in_src_path, in_dest_path );
	system( command );
	#endif
}

fn delete_folder( in text in_path )
{
	ifn( check_folder( in_path ) ) out;
	#if OS_WINDOWS
	SHFILEOPSTRUCTA shFileOp = { 0 };
	shFileOp.wFunc = FO_DELETE;
	shFileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;
	shFileOp.pFrom = in_path;
	SHFileOperationA( &shFileOp );
	#elif OS_LINUX
	text command = text_format( "rm -r %s", in_path );
	system( command );
	#endif
}

//

/////// /////// /////// /////// /////// /////// ///////

// os_pacer
// -------
// maintains a constant loop frequency

make_object(
	os_pacer,
	u32 fps;
	u64 start;
	u64 end;
	u32 time_ns;
)( in u32 in_fps )
{
	#ifdef hept_debug
	print_error( in_fps == 0, "os_pacer: in_fps is 0" );
	#endif
	//
	os_pacer this = assign_os_pacer();
	//
	this->fps = in_fps;
	this->time_ns = to_u32( to_f64( nano_per_sec ) / to_f64( this->fps ) );
	//
	#ifdef hept_trace
	print_trace( "new os_pacer: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

fn start_os_pacer( in os_pacer in_pacer )
{
	in_pacer->start = get_ns();
}

fn wait_os_pacer( in os_pacer in_pacer )
{
	in_pacer->end = get_ns();
	u64 time_taken = in_pacer->end - in_pacer->start;
	if( time_taken >= in_pacer->time_ns ) out;
	sleep_ns( in_pacer->time_ns - time_taken );
}

fn change_os_pacer( in os_pacer in_pacer, in u32 in_fps )
{
	in_pacer->fps = in_fps;
	in_pacer->time_ns = to_u32( to_f64( nano_per_sec ) / to_f64( in_pacer->fps ) );
}

//

/////// /////// /////// /////// /////// /////// ///////

// os_thread
// -------
// creates and controls a CPU thread

	#if OS_WINDOWS
make_type( HANDLE ) os_thread_id;
	#elif OS_LINUX
make_type( pthread_t ) os_thread_id;
	#endif

make_object(
	os_thread,
	fn_ptr( ptr( pure ), function, in ptr( pure ) );
	os_thread_id id;
)( fn_ptr( ptr( pure ), in_function, in ptr( pure ) ) )
{
	#ifdef hept_debug
	print_error( in_function == null, "os_thread: in_function is null" );
	#endif
	//
	os_thread this = assign_os_thread();
	//
	this->function = in_function;
	#if OS_WINDOWS
	this->id = CreateThread( NULL, 0, to( LPTHREAD_START_ROUTINE, this->function ), null, 0, null );
	#elif OS_LINUX
	pthread_create( ref( this->id ), null, to( fn_ptr( ptr( pure ), , ptr( pure ) ), this->function ), null );
	#endif

	#ifdef hept_debug
	print_error( this->id == 0, "os_thread: could not create thread" );
	#endif
	//
	#ifdef hept_trace
	print_trace( "new os_thread: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global os_thread default_os_thread = null;

//

/////// /////// /////// /////// /////// /////// ///////

// os_core
// -------
// holds the Hephaestus instance.

global u32 os_core_version = 1;

make_object(
	os_core,
	H_instance instance;
)( in text in_name )
{
	#ifdef hept_debug
	print_error( in_name == null, "os_core: in_name is null" );
	#endif
	//
	os_core this = assign_os_core();
	//
	H_info_app info_app = H_new_info( in_name, os_core_version );

	text desired_debug_layers[] = {
		"VK_LAYER_KHRONOS_validation",
	//"VK_LAYER_LUNARG_api_dump",
	//"VK_LAYER_LUNARG_device_simulation",
	#ifdef hept_debug
		"VK_LAYER_LUNARG_monitor",
	#endif
		//"VK_LAYER_LUNARG_screenshot"
	};
	u32 desired_debug_layers_count = 1;
	#ifdef hept_debug
	desired_debug_layers_count++;
	#endif

	u32 debug_layer_count = H_get_debug_layers( null ), enabled_debug_layer_count = 0;
	ptr( H_layer_properties ) available_layers = new_mem( H_layer_properties, debug_layer_count );
	H_get_debug_layers( available_layers );
	ptr( text ) debug_layers = new_mem( text, desired_debug_layers_count );

	iter( desired_debug_layers_count, i )
	{
		iter( debug_layer_count, j )
		{
			if( compare_text( desired_debug_layers[ i ], available_layers[ j ].layerName ) )
			{
	#ifdef hept_debug
				print_debug( "activated layer: %s", available_layers[ j ].layerName );
	#endif
				( debug_layers )[ enabled_debug_layer_count++ ] = desired_debug_layers[ i ];
				skip;
			}
		}
	}

	#ifdef hept_debug
	print_error( enabled_debug_layer_count < desired_debug_layers_count, "os_core: debug layers not found" );
	#endif

	free_mem( available_layers );

	text extensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
	#if OS_WINDOWS
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	#elif OS_LINUX
		VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
	#endif
	};
	u32 extension_count = 2;

	H_info_instance instance_info = H_create_info_instance(
		ref( info_app ),
		enabled_debug_layer_count,
		( ptr( const char ) ptr( const ) )debug_layers,
		extension_count,
		( ptr( const char ) ptr( const ) )extensions
	);
	this->instance = H_new_instance( instance_info );

	#ifdef hept_debug
	print_error( this->instance == null, "os_core: instance could not be created" );
	#endif

	free_text( debug_layers );
	//
	#ifdef hept_trace
	print_trace( "new os_core: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

//

/////// /////// /////// /////// /////// /////// ///////

// os_machine
// -------
// links the CPU to the GPU

make_object(
	os_machine,
	u32 queue_family_index;
	H_physical_device physical_device;
	H_device device;
)()
{
	os_machine this = assign_os_machine();
	//
	this->physical_device = null;
	//
	#ifdef hept_trace
	print_trace( "new os_machine: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

//

/////// /////// /////// /////// /////// /////// ///////

// os_window
// -------
// does thing

	#if OS_WINDOWS
inl LRESULT CALLBACK process_os_window( in HWND hwnd, in UINT u_msg, in WPARAM w_param, in LPARAM l_param );

make_struct( os_window_link )
{
	HWND hwnd;
	HINSTANCE inst;
};
	#elif OS_LINUX
fn process_os_window( ptr( Display ) in_disp, Window in_win );

make_struct( os_window_link )
{
	ptr( Display ) xdis;
	Window xwin;
};
	#endif

make_object(
	os_window,
	text name;
	fn_ptr( pure, call );
	flag ready, visible;
	u32 width, height;
	H_surface surface;
	H_surface_capabilities surface_capabilities;
	H_surface_format surface_format;
	H_present_mode present_mode;
	struct( os_window_link ) link;
)( in text in_name, in u32 in_width, in u32 in_height, fn_ptr( pure, in_call ) )
{
	#ifdef hept_debug
	print_error( in_call == null, "os_window: in_call is null" );
	print_error( in_width == 0, "os_window: in_width is 0" );
	print_error( in_height == 0, "os_window: in_height is 0" );
	#endif
	//
	os_window this = assign_os_window();
	//
	this->name = in_name;
	this->call = in_call;
	this->ready = no;
	this->visible = no;
	this->width = in_width;
	this->height = in_height;

	#if OS_WINDOWS
	HWND hwnd = GetConsoleWindow();
	DWORD window_style = WS_OVERLAPPEDWINDOW;
	RECT rect = { 0, 0, this->width, this->height };
	AdjustWindowRect( ref( rect ), window_style, no );
	s32 this_width = rect.right - rect.left, this_height = rect.bottom - rect.top;

	WNDCLASS wc = {
		.lpfnWndProc = process_os_window,
		.hInstance = GetModuleHandle( null ),
		.hbrBackground = CreateSolidBrush( RGB( 0, 0, 0 ) ),
		.lpszClassName = this->name };

	flag rc = RegisterClass( ref( wc ) );

		#ifdef hept_debug
	print_error( rc == 0, "os_window: cannot create win32 window" );
		#endif

	this->link.hwnd = CreateWindowEx(
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

	VkWin32SurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hwnd = this->link.hwnd;
	create_info.hinstance = this->link.inst;

	vkCreateWin32SurfaceKHR( current_os_core->instance, ref( create_info ), null, ref( this->surface ) );

	#elif OS_LINUX
	this->link.xdis = XOpenDisplay( NULL );

		#ifdef hept_debug
	print_error( this->link.xdis == null, "os_window: cannot open X11 display" );
		#endif

	s32 screen_num = DefaultScreen( this->link.xdis );
	s32 screen_width = DisplayWidth( this->link.xdis, screen_num );
	s32 screen_height = DisplayHeight( this->link.xdis, screen_num );

	Window root_win = RootWindow( this->link.xdis, screen_num );

	this->link.xwin = XCreateSimpleWindow( this->link.xdis, root_win, ( screen_width - this->width ) / 2, ( screen_height - this->height ) / 2, this->width, this->height, 1, BlackPixel( this->link.xdis, screen_num ), BlackPixel( this->link.xdis, screen_num ) );

		#ifdef hept_debug
	print_error( this->link.xwin == 0, "os_window: cannot create X11 window" );
		#endif

	XStoreName( this->link.xdis, this->link.xwin, this->name );

	XSelectInput( this->link.xdis, this->link.xwin, StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask );

	VkXlibSurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	create_info.dpy = this->link.xdis;
	create_info.window = this->link.xwin;

	vkCreateXlibSurfaceKHR( current_os_core->instance, ref( create_info ), null, ref( this->surface ) );

	#endif
	//
	#ifdef hept_trace
	print_trace( "new os_window: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

fn show_os_window( in os_window in_window )
{
	#if OS_WINDOWS
	ShowWindow( in_window->link.hwnd, SW_SHOWDEFAULT );
	UpdateWindow( in_window->link.hwnd );
	#elif OS_LINUX
	XMapWindow( in_window->link.xdis, in_window->link.xwin );
	XFlush( in_window->link.xdis );
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

/*
//

/////// /////// /////// /////// /////// /////// ///////

// form_NAME
// -------
// holds a blueprint for NAME creation

make_object(
	form_NAME,
	ELEMENTS;
)( in PARAMS )
{
	#ifdef hept_debug
	print_error( in_PARAMS == null, "form_NAME: in_PARAMS is null" );
	#endif
	//
	form_NAME this = assign_form_NAME();
	//

	//
	#ifdef hept_trace
	print_trace( "new form_NAME: ID: %d",this->pile_id );
	#endif
	//
	out this;
}

global form_NAME default_form_NAME_ = null;

//
*/

//

/////// /////// /////// /////// /////// /////// ///////

// form_buffer
// -------
// holds a blueprint for buffer creation

make_object(
	form_buffer,
	H_buffer_usage usage;
	H_memory_properties properties;
)( in H_buffer_usage in_usage, in H_memory_properties in_properties )
{
	#ifdef hept_debug
	print_error( in_usage == 0, "form_buffer: in_usage is 0" );
	print_error( in_properties == 0, "form_buffer: in_properties is 0" );
	#endif
	//
	form_buffer this = assign_form_buffer();
	//
	this->usage = in_usage;
	this->properties = in_properties;
	//
	#ifdef hept_trace
	print_trace( "new form_buffer: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global form_buffer default_form_buffer_vertex = null;
global form_buffer default_form_buffer_index = null;
global form_buffer default_form_buffer_storage = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_image
// -------
// holds a blueprint for image creation

make_enum( image_type ){
	image_type_null,
	image_type_rgba = VK_IMAGE_ASPECT_COLOR_BIT,
	image_type_depth = VK_IMAGE_ASPECT_DEPTH_BIT,
	image_type_stencil = VK_IMAGE_ASPECT_STENCIL_BIT,
};

make_object(
	form_image,
	enum( image_type ) type;
	H_format format;
	H_sampler sampler;
)( in enum( image_type ) in_type, in H_format in_format )
{
	#ifdef hept_debug
	print_error( in_type == image_type_null, "form_image: in_type is null" );
	print_error( in_format == 0, "form_image: in_format is 0" );
	#endif
	//
	form_image this = assign_form_image();
	//
	this->type = in_type;
	this->format = in_format;

	H_info_sampler info = H_create_info_sampler(
		VK_FILTER_NEAREST,
		VK_FILTER_NEAREST,
		VK_SAMPLER_MIPMAP_MODE_NEAREST,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		0,
		no,
		1,
		no,
		VK_COMPARE_OP_ALWAYS,
		0,
		0,
		VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		no
	);
	this->sampler = H_new_sampler( current_os_machine->device, info );
	#ifdef hept_debug
	print_error( this->sampler == null, "form_image: could not make sampler" );
	#endif
	//
	#ifdef hept_trace
	print_trace( "new form_image: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global form_image default_form_image_rgba = null;
global form_image default_form_image_depth = null;
global form_image default_form_image_stencil = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_frame_layer
// -------
// layer for form_frame

make_enum( frame_layer_type ){
	frame_layer_type_null,
	frame_layer_type_rgba = VK_IMAGE_ASPECT_COLOR_BIT,
	frame_layer_type_depth = VK_IMAGE_ASPECT_DEPTH_BIT,
	frame_layer_type_stencil = VK_IMAGE_ASPECT_STENCIL_BIT,
};

make_object(
	form_frame_layer,
	enum( frame_layer_type ) type;
	H_attachment_reference attach_ref;
	H_format format;
)( in enum( frame_layer_type ) in_type, in H_format in_format )
{
	#ifdef hept_debug
	print_error( in_type == frame_layer_type_null, "form_frame_layer: in_type is null" );
	#endif
	//
	form_frame_layer this = assign_form_frame_layer();
	//
	this->type = in_type;
	this->attach_ref.attachment = 0;
	this->attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
	with( this->type )
	{
		is( frame_layer_type_rgba )
		{
			this->attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			skip;
		}

		is( frame_layer_type_depth )
			is( frame_layer_type_stencil )
		{
			this->attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			skip;
		}
	default: skip;
	}
	this->format = in_format;
	//
	#ifdef hept_trace
	print_trace( "new form_frame_layer: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global form_frame_layer default_form_frame_layer_rgba = null;
global form_frame_layer default_form_frame_layer_depth = null;
global form_frame_layer default_form_frame_layer_stencil = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_frame
// -------
// holds a blueprint for frame creation

make_enum( frame_type ){
	frame_type_null,
	frame_type_present = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	frame_type_shader_read = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	frame_type_attachment = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	frame_type_general = VK_IMAGE_LAYOUT_GENERAL,
};

make_object(
	form_frame,
	enum( frame_type ) type;
	H_render_pass render_pass;
	list layers;
)( in enum( frame_type ) in_type, in list in_layers )
{
	#ifdef hept_debug
	print_error( in_type == frame_type_null, "form_frame: in_type is null" );
	#endif
	//
	form_frame this = assign_form_frame();
	//
	this->type = in_type;
	this->layers = in_layers;

	ptr( H_attachment_reference ) attach_ref_depth_stencil = null;
	u32 rgba_count = 0;

	iter( this->layers->size, s )
	{
		form_frame_layer this_layer = list_get( this->layers, form_frame_layer, s );
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
		default: skip;
		}
	}

	ptr( H_attachment_reference ) attach_ref_rgba = null;
	if( rgba_count )
	{
		attach_ref_rgba = new_mem( H_attachment_reference, rgba_count );

		iter( rgba_count, r )
		{
			form_frame_layer this_layer = list_get( this->layers, form_frame_layer, r );
			attach_ref_rgba[ r ] = this_layer->attach_ref;
		}
	}

	H_subpass_description subpass = { 0 };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = rgba_count;
	subpass.pColorAttachments = attach_ref_rgba;
	subpass.pDepthStencilAttachment = attach_ref_depth_stencil;

	ptr( H_attachment_description ) attachments = new_mem( H_attachment_description, this->layers->size );
	iter( this->layers->size, l )
	{
		form_frame_layer this_layer = list_get( this->layers, form_frame_layer, l );
		attachments[ l ].format = this_layer->format;
		attachments[ l ].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[ l ].loadOp = ( this->type == frame_type_attachment ) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[ l ].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[ l ].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[ l ].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[ l ].initialLayout = ( this->type == frame_type_attachment ) ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[ l ].finalLayout = to( H_image_layout, this->type );
		attachments[ l ].flags = 0;
	}

	H_info_render_pass render_pass_info = H_create_info_render_pass( this->layers->size, attachments, 1, ref( subpass ), 0, null );
	this->render_pass = H_new_render_pass( current_os_machine->device, render_pass_info );
	//
	#ifdef hept_trace
	print_trace( "new form_frame: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global form_frame default_form_frame = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_renderer
// -------
// holds a blueprint for renderer creation

make_object(
	form_renderer,
	H_command_pool command_pool;
	H_queue queue;
)()
{
	form_renderer this = assign_form_renderer();
	//
	this->queue = H_get_queue( current_os_machine->device, current_os_machine->queue_family_index, 0 );
	if( this->command_pool != null ) vkDestroyCommandPool( current_os_machine->device, this->command_pool, null );
	H_info_command_pool command_pool_info = H_create_info_command_pool( current_os_machine->queue_family_index );
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	this->command_pool = H_new_command_pool( current_os_machine->device, command_pool_info );
	//
	#ifdef hept_trace
	print_trace( "new form_renderer: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global form_renderer default_form_renderer = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_mesh_attrib
// -------
// attribute for form_mesh

make_object(
	form_mesh_attrib,
	H_format format;
	u32 type_size;
	u32 size;
	text type_glsl;
)( in H_format in_format, in u32 in_type_size, in u32 in_size, in text in_type_glsl )
{
	#ifdef hept_debug
	print_error( in_format == 0, "form_mesh_attrib: in_format is 0" );
	print_error( in_type_size == 0, "form_mesh_attrib: in_type_size is 0" );
	print_error( in_size == 0, "form_mesh_attrib: in_size is 0" );
	print_error( in_type_glsl == null, "form_mesh_attrib: in_type_glsl is null" );
	#endif
	//
	form_mesh_attrib this = assign_form_mesh_attrib();
	//
	this->format = in_format;
	this->type_size = in_type_size;
	this->size = in_size;
	this->type_glsl = in_type_glsl;
	//
	#ifdef hept_trace
	print_trace( "new form_mesh_attrib: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

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

	#define new_form_mesh_attrib( type, size ) new_form_mesh_attrib( $format_type_##type##_size_##size, size_( type ), size, $format_type_##type##_size_##size##_text )

global form_mesh_attrib default_form_mesh_attrib_pos2 = null;
global form_mesh_attrib default_form_mesh_attrib_pos3 = null;
global form_mesh_attrib default_form_mesh_attrib_uv = null;
global form_mesh_attrib default_form_mesh_attrib_rgb = null;
global form_mesh_attrib default_form_mesh_attrib_rgba = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_mesh
// -------
// holds a blueprint for mesh creation

make_object(
	form_mesh,
	u32 type_size;
	list attribs;
	text layout_glsl;
)( in list in_attribs, in flag generate_glsl )
{
	#ifdef hept_debug
	print_error( in_attribs == null, "form_mesh: in_attribs is null" );
	#endif
	//
	form_mesh this = assign_form_mesh();
	//
	this->type_size = 0;
	this->attribs = in_attribs;
	this->layout_glsl = new_text( "", 0 );
	//
	iter_list( this->attribs, a )
	{
		form_mesh_attrib this_attrib = list_get( this->attribs, form_mesh_attrib, a );

		this->type_size += this_attrib->type_size * this_attrib->size;
	}
	//
	#ifdef hept_trace
	print_trace( "new form_mesh: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global form_mesh default_form_mesh_2d_tri = null;
global form_mesh default_form_mesh_2d_tri_tex = null;
global form_mesh default_form_mesh_2d_line = null;
// global form_mesh default_form_mesh_3d = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_shader_stage
// -------
// stage for form_shader

make_enum( shader_stage_type ){
	shader_stage_type_null,
	shader_stage_type_vertex = H_shader_stage_vertex,
	shader_stage_type_geometry = H_shader_stage_geometry,
	shader_stage_type_fragment = H_shader_stage_fragment,
	shader_stage_type_compute = H_shader_stage_compute,
};

make_object(
	form_shader_stage,
	enum( shader_stage_type ) type;
)( in enum( shader_stage_type ) in_type )
{
	#ifdef hept_debug
	print_error( in_type == shader_stage_type_null, "form_shader_stage: in_type is null" );
	#endif
	//
	form_shader_stage this = assign_form_shader_stage();
	//
	this->type = in_type;
	//
	#ifdef hept_trace
	print_trace( "new form_shader_stage: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global form_shader_stage default_form_shader_stage_vert = null;
global form_shader_stage default_form_shader_stage_geom = null;
global form_shader_stage default_form_shader_stage_frag = null;
global form_shader_stage default_form_shader_stage_comp = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_module
// -------
// holds a blueprint for module creation

make_object(
	form_module,
	form_shader_stage shader_stage;
	form_mesh mesh_form;
)( in form_shader_stage in_shader_stage, in form_mesh in_mesh_form )
{
	#ifdef hept_debug
	print_error( in_shader_stage == null, "form_module: in_shader_stage is null" );
	print_error( in_mesh_form == null, "form_module: in_mesh_form is null" );
	#endif
	//
	form_module this = assign_form_module();
	//
	this->shader_stage = in_shader_stage;
	this->mesh_form = in_mesh_form;
	//
	#ifdef hept_trace
	print_trace( "new form_module: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global form_module default_form_module_2d_tri_vert = null;
global form_module default_form_module_2d_tri_frag = null;
global form_module default_form_module_2d_tri_tex_vert = null;
global form_module default_form_module_2d_tri_tex_frag = null;
global form_module default_form_module_2d_line_vert = null;
global form_module default_form_module_2d_line_frag = null;
// global form_module default_form_module_3d_tri_vert = null;
// global form_module default_form_module_3d_tri_frag = null;
// global form_module default_form_module_3d_tri_tex_vert = null;
// global form_module default_form_module_3d_tri_tex_frag = null;
// global form_module default_form_module_3d_line_vert = null;
// global form_module default_form_module_3d_line_frag = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_shader_input
// -------
// input for form_shader

make_enum( shader_input_type ){
	shader_input_type_null,
	shader_input_type_image = H_descriptor_type_image,
	shader_input_type_storage = H_descriptor_type_storage,
};

make_object(
	form_shader_input,
	enum( shader_input_type ) type;
	enum( shader_stage_type ) stage_type;
)( in enum( shader_input_type ) in_type, in enum( shader_stage_type ) in_stage )
{
	#ifdef hept_debug
	print_error( in_type == shader_input_type_null, "form_shader_input: in_type is null" );
	#endif
	//
	form_shader_input this = assign_form_shader_input();
	//
	this->type = in_type;
	this->stage_type = in_stage;
	//
	#ifdef hept_trace
	print_trace( "new form_shader_input: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global form_shader_input default_form_shader_input_image = null;
global form_shader_input default_form_shader_input_storage_vert = null;
global form_shader_input default_form_shader_input_storage_frag = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_shader
// -------
// input for form_shader

make_object(
	form_shader,
	H_topology topology;
	H_descriptor_set_layout descriptor_layout;
	list inputs;
)( in H_topology in_topology, in list in_inputs )
{
	form_shader this = assign_form_shader();
	//
	this->topology = in_topology;
	this->inputs = in_inputs;
	list bindings = new_list( H_descriptor_set_layout_binding );

	if( this->inputs != null )
	{
		iter_list( this->inputs, i )
		{
			form_shader_input this_form_shader_input = list_get( this->inputs, form_shader_input, i );
			list_add(
				bindings,
				H_descriptor_set_layout_binding,
				( ( H_descriptor_set_layout_binding ){
					.binding = i,
					.descriptorType = to( VkDescriptorType, this_form_shader_input->type ),
					.descriptorCount = 1,
					.stageFlags = this_form_shader_input->stage_type,
					.pImmutableSamplers = null,
				} )
			);
		}
	}

	H_info_descriptor_set_layout layout_info = H_create_info_descriptor_set_layout(
		bindings->size, ( const ptr( VkDescriptorSetLayoutBinding ) )bindings->data, 0
	);
	this->descriptor_layout = H_new_descriptor_set_layout( current_os_machine->device, layout_info );
	//
	#ifdef hept_trace
	print_trace( "new form_shader: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global form_shader default_form_shader_tri = null;
global form_shader default_form_shader_tri_tex = null;
global form_shader default_form_shader_line = null;
global form_shader default_form_shader_tri_storage = null;
global form_shader default_form_shader_tri_tex_storage = null;
global form_shader default_form_shader_line_storage = null;

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

// hept objects
// -------
// using the forms, these objects can be created

/*
//

/////// /////// /////// /////// /////// /////// ///////

// NAME
// -------
//

make_object(
	NAME,
	form_NAME form;
	ELEMENTS;
)( in PARAMS )
{
	#ifdef hept_debug
	print_error( in_PARAMS == null, "NAME: in_PARAMS is null" );
	#endif
	//
	NAME this = assign_NAME();
	//

	//
	#ifdef hept_trace
	print_trace( "new NAME: ID: %d",this->pile_id );
	#endif
	//
	out this;
}

global NAME default_NAME_ = null;

//
*/

//

/////// /////// /////// /////// /////// /////// ///////

// buffer
// -------
//

make_object(
	buffer,
	form_buffer form;
	u64 size;
	H_buffer buffer;
	H_memory memory;
)( in form_buffer in_form, in u64 in_size )
{
	#ifdef hept_debug
	print_error( in_form == null, "buffer: in_form is null" );
	print_error( in_size == 0, "buffer: in_size is 0" );
	#endif
	//
	buffer this = assign_buffer();
	//
	this->form = in_form;
	this->size = in_size;

	H_info_buffer buffer_info = H_create_info_buffer(
		this->size,
		this->form->usage,
		VK_SHARING_MODE_EXCLUSIVE
	);
	this->buffer = H_new_buffer( current_os_machine->device, buffer_info );

	H_memory_requirements mem_requirements = H_get_memory_requirements_buffer( current_os_machine->device, this->buffer );
	H_info_memory memory_info = H_create_info_memory(
		mem_requirements.size,
		H_find_mem(
			current_os_machine->physical_device,
			mem_requirements.memoryTypeBits,
			this->form->properties
		)
	);
	this->memory = H_new_memory_buffer( current_os_machine->device, memory_info, this->buffer );
	//
	#ifdef hept_trace
	print_trace( "new buffer: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

	#define delete_buffer2( BUFFER )                                        \
		DEF_START                                                            \
		vkFreeMemory( current_os_machine->device, BUFFER->memory, null );    \
		vkDestroyBuffer( current_os_machine->device, BUFFER->buffer, null ); \
		delete_buffer( BUFFER );                                             \
		DEF_END

fn update_buffer( buffer in_buffer, in u32 in_data_size, in ptr( pure ) in_data )
{
	if( in_data_size > in_buffer->size )
	{
		in_buffer->size = in_data_size;
		H_info_buffer buffer_info = H_create_info_buffer(
			in_buffer->size,
			in_buffer->form->usage,
			VK_SHARING_MODE_EXCLUSIVE
		);
		in_buffer->buffer = H_new_buffer( current_os_machine->device, buffer_info );

		H_memory_requirements mem_requirements = H_get_memory_requirements_buffer( current_os_machine->device, in_buffer->buffer );
		H_info_memory memory_info = H_create_info_memory(
			mem_requirements.size,
			H_find_mem(
				current_os_machine->physical_device,
				mem_requirements.memoryTypeBits,
				in_buffer->form->properties
			)
		);
		in_buffer->memory = H_new_memory_buffer( current_os_machine->device, memory_info, in_buffer->buffer );

	}
	ptr( pure ) mapped = null;
	vkMapMemory( current_os_machine->device, in_buffer->memory, 0, in_data_size, 0, ref( mapped ) );
	copy_mem( mapped, in_data, in_buffer->size );
	vkUnmapMemory( current_os_machine->device, in_buffer->memory );
}

//

/////// /////// /////// /////// /////// /////// ///////

// image
// -------
//

make_enum( image_state ){
	image_state_null,
	image_state_src,
	image_state_dst,
};

make_object(
	image,
	form_image form;
	enum( image_state ) state;
	H_image image;
	H_image_view view;
	H_image_layout layout;
	H_memory memory;
	u32 width;
	u32 height;
	list data;
)( in form_image in_form, in enum( image_state ) in_state, in u32 in_width, in u32 in_height )
{
	#ifdef hept_debug
	print_error( in_form == null, "image: in_form is null" );
	print_error( in_state == image_state_null, "image: in_state is null" );
	print_error( in_width == 0, "image: in_width is 0" );
	print_error( in_height == 0, "image: in_height is 0" );
	#endif
	//
	image this = assign_image();
	//
	this->form = in_form;
	this->state = in_state;
	this->layout = H_image_layout_undefined;
	this->width = in_width;
	this->height = in_height;
	this->data = new_list( rgba );

	//
	H_extent_3d temp_extent = {
		.width = this->width,
		.height = this->height,
		.depth = 1 };

	/*VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties( result->form->machine->physical_device, result->form->format, ref( format_properties ) );

	if( !( format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT ) )
	{
		print( "Format cannot be used as color attachment!\n" );
	}*/

	H_info_image image_info = H_create_info_image(
		VK_IMAGE_TYPE_2D,
		temp_extent,
		1,
		1,
		this->form->format,
		( ( this->state == image_state_src ) ? ( VK_IMAGE_TILING_LINEAR ) : ( VK_IMAGE_TILING_OPTIMAL ) ), // OPTIMAL REQUIRES MULTIPLES OF 32x32
		this->layout,
		H_image_usage_sampled | H_image_usage_color_attachment | H_image_usage_transfer_src | H_image_usage_transfer_dst,
		VK_SHARING_MODE_EXCLUSIVE,
		VK_SAMPLE_COUNT_1_BIT
	);
	this->image = H_new_image( current_os_machine->device, image_info );

	H_memory_requirements mem_requirements = H_get_memory_requirements_image( current_os_machine->device, this->image );
	H_info_memory memory_info = H_create_info_memory(
		mem_requirements.size,
		H_find_mem(
			current_os_machine->physical_device,
			mem_requirements.memoryTypeBits,
			( ( this->state == image_state_src ) ? ( H_memory_property_host_visible | H_memory_property_host_coherent ) : ( H_memory_property_device_local ) )
		)
	);
	this->memory = H_new_memory_image( current_os_machine->device, memory_info, this->image );
	//
	#ifdef hept_trace
	print_trace( "new image: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

fn update_image( in image in_image )
{
	ptr( pure ) mapped = null;
	vkMapMemory( current_os_machine->device, in_image->memory, 0, in_image->data->size_type * in_image->data->size, 0, ref( mapped ) );
	copy_mem( mapped, in_image->data->data, in_image->data->size_type * in_image->data->size );
	vkUnmapMemory( current_os_machine->device, in_image->memory );
}

//

/////// /////// /////// /////// /////// /////// ///////

// frame
// -------
//

make_object(
	frame,
	form_frame form;
	H_framebuffer framebuffer;
	list images;
	list views;
	u32 max_w;
	u32 max_h;
	H_info_begin_render_pass info_begin;
	VkClearValue clear_col;
	VkClearValue clear_dep;
)( in form_frame in_form, in list in_images )
{
	#ifdef hept_debug
	print_error( in_form == null, "frame: in_form is null" );
	print_error( in_images == null, "frame: in_images is null" );
	#endif
	//
	frame this = assign_frame();
	//
	this->form = in_form;
	this->images = in_images;
	this->views = new_list( H_image_view );
	this->max_w = 0;
	this->max_h = 0;
	//
	iter( this->images->size, i )
	{
		image temp_image = list_get( this->images, image, i );
		this->max_w = max( this->max_w, temp_image->width );
		this->max_h = max( this->max_h, temp_image->height );
	}

	//

	iter( this->form->layers->size, v )
	{
		image temp_image = list_get( this->images, image, v );
		H_info_image_view image_view_info = H_create_info_image_view(
			temp_image->image,
			VK_IMAGE_VIEW_TYPE_2D,
			temp_image->form->format,
			( ( H_component_mapping ){
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY } ),
			( ( H_image_subresource_range ){
				list_get( this->form->layers, form_frame_layer, v )->type,
				0,
				1,
				0,
				1 } )
		);
		if( temp_image->view == null ) temp_image->view = H_new_image_view( current_os_machine->device, image_view_info );
		list_add( this->views, H_image_view, temp_image->view );
	}

	H_info_framebuffer framebuffer_info = H_create_info_framebuffer(
		this->form->render_pass,
		this->views->size,
		( ptr( H_image_view ) )( this->views->data ),
		this->max_w,
		this->max_h,
		1
	);
	this->framebuffer = H_new_framebuffer( current_os_machine->device, framebuffer_info );

	this->clear_col = ( VkClearValue ){ 0., 0., 0., 0. };
	// result->clear_dep = (VkClearValue){ 0.,0.,0.,0. };
	this->info_begin = H_create_info_begin_render_pass(
		this->form->render_pass,
		this->framebuffer,
		( ( VkRect2D ){ 0, 0, this->max_w, this->max_h } ),
		1,
		ref( this->clear_col )
	);
	//
	#ifdef hept_trace
	print_trace( "new frame: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global frame default_frame_ = null;

//

/////// /////// /////// /////// /////// /////// ///////

// renderer
// -------
//

make_object(
	renderer,
	form_renderer form;
	os_window ref_window;
	flag changed;
	H_viewport viewport;
	form_image swapchain_form_image;
	H_swapchain swapchain;
	H_format swapchain_format;
	H_extent swapchain_extent;
	u32 current_frame;
	form_frame form_frame_window;
	frame frame_window;
	u32 frame_window_width;
	u32 frame_window_height;
	list frames;
	u32 fence_id;
	ptr( H_command_buffer ) command_buffers;
	ptr( H_semaphore ) image_ready;
	ptr( H_semaphore ) image_done;
	ptr( H_fence ) flight_fences;
)( in form_renderer in_form, in os_window in_window, in u32 in_frame_width, in u32 in_frame_height )
{
	#ifdef hept_debug
	print_error( in_form == null, "renderer: in_form is null" );
	print_error( in_window == null, "renderer: in_window is null" );
	print_error( in_frame_width == 0, "renderer: in_frame_width is 0" );
	print_error( in_frame_height == 0, "renderer: in_frame_height is 0" );
	#endif
	//
	renderer this = assign_renderer();
	//
	this->form = in_form;
	this->changed = yes;
	this->ref_window = in_window;
	this->swapchain = null;
	this->current_frame = 0;
	this->frame_window = null;
	this->frame_window_width = in_frame_width;
	this->frame_window_height = in_frame_height;
	this->frames = new_list( frame );
	this->fence_id = 0;
	//
	#ifdef hept_trace
	print_trace( "new renderer: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

fn update_renderer( in renderer in_renderer )
{
	if( hept_exit ) out;

	vkDeviceWaitIdle( current_os_machine->device );

	//

	if( in_renderer->swapchain != null ) vkDestroySwapchainKHR( current_os_machine->device, in_renderer->swapchain, null );

	if( in_renderer->frames->size != 0 )
	{
		iter( in_renderer->frames->size, i )
		{
			frame temp_frame = list_get( in_renderer->frames, frame, i );
			iter( temp_frame->images->size, j )
			{
				image temp_image = list_get( temp_frame->images, image, j );
				delete_image( temp_image );
			}
			delete_frame( temp_frame );
			vkDestroySemaphore( current_os_machine->device, in_renderer->image_ready[ i ], null );
			vkDestroySemaphore( current_os_machine->device, in_renderer->image_done[ i ], null );
			vkDestroyFence( current_os_machine->device, in_renderer->flight_fences[ i ], null );
		}
		in_renderer->frames->size = 0;
	}

	//

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( current_os_machine->physical_device, in_renderer->ref_window->surface, ref( in_renderer->ref_window->surface_capabilities ) );

	H_info_swapchain swapchain_info = H_create_info_swapchain(
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

	in_renderer->swapchain = H_new_swapchain( current_os_machine->device, swapchain_info );
	in_renderer->swapchain_format = in_renderer->ref_window->surface_format.format;
	in_renderer->swapchain_extent = in_renderer->ref_window->surface_capabilities.currentExtent;

	in_renderer->ref_window->width = in_renderer->swapchain_extent.width;
	in_renderer->ref_window->height = in_renderer->swapchain_extent.height;

	in_renderer->viewport = H_create_viewport(
		0.0,
		0.0,
		to_f32( in_renderer->swapchain_extent.width ),
		to_f32( in_renderer->swapchain_extent.height ),
		0.0,
		1.0
	);

	u32 temp_count = 0;
	ptr( H_image ) temp_images = null;
	vkGetSwapchainImagesKHR( current_os_machine->device, in_renderer->swapchain, ref( temp_count ), null );
	temp_images = new_mem( H_image, temp_count );
	vkGetSwapchainImagesKHR( current_os_machine->device, in_renderer->swapchain, ref( temp_count ), temp_images );

	if( in_renderer->form_frame_window == null )
	{
		list layers = new_list( form_frame_layer );
		form_frame_layer layer_rgba = new_form_frame_layer( frame_layer_type_rgba, in_renderer->swapchain_format );
		list_add( layers, form_frame_layer, layer_rgba );

		in_renderer->form_frame_window = new_form_frame(
			frame_type_present,
			layers
		);
	}

	if( in_renderer->swapchain_form_image == null ) in_renderer->swapchain_form_image = new_form_image( image_type_rgba, in_renderer->swapchain_format );

	iter( temp_count, i )
	{
		list temp_list_images = new_list( image );
		image temp_image = assign_image();
		temp_image->form = in_renderer->swapchain_form_image;
		temp_image->image = temp_images[ i ];
		temp_image->width = in_renderer->swapchain_extent.width;
		temp_image->height = in_renderer->swapchain_extent.height;
		list_add( temp_list_images, image, temp_image );
		frame temp_frame = new_frame( in_renderer->form_frame_window, temp_list_images );
		list_add( in_renderer->frames, frame, temp_frame );
	}

	free_mem( temp_images );

	//

	in_renderer->image_ready = new_mem( H_semaphore, in_renderer->frames->size );
	in_renderer->image_done = new_mem( H_semaphore, in_renderer->frames->size );
	in_renderer->flight_fences = new_mem( H_fence, in_renderer->frames->size );
	in_renderer->command_buffers = new_mem( H_command_buffer, in_renderer->frames->size );

	H_info_semaphore semaphore_info = H_create_info_semaphore();
	H_info_fence fence_info = H_create_info_fence();
	H_info_command_buffer command_buffers_info = H_create_info_command_buffer(
		in_renderer->form->command_pool,
		H_command_buffer_level_primary,
		in_renderer->frames->size
	);

	H_allocate_command_buffers( current_os_machine->device, command_buffers_info, in_renderer->command_buffers );

	iter( in_renderer->frames->size, i )
	{
		in_renderer->image_ready[ i ] = H_new_semaphore( current_os_machine->device, semaphore_info );
		in_renderer->image_done[ i ] = H_new_semaphore( current_os_machine->device, semaphore_info );
		in_renderer->flight_fences[ i ] = H_new_fence( current_os_machine->device, fence_info );
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
			delete_image( temp_image );
		}
		delete_frame( in_renderer->frame_window );
	}

	image temp_image = new_image(
		in_renderer->swapchain_form_image,
		image_state_dst,
		in_renderer->frame_window_width,
		in_renderer->frame_window_height
	);
	//
	list temp_list_images = new_list( image );
	list_add( temp_list_images, image, temp_image );
	in_renderer->frame_window = new_frame( current_form_frame, temp_list_images );

	in_renderer->changed = no;
}

//

/////// /////// /////// /////// /////// /////// ///////

// mesh
// -------
//

make_struct( vertex_2d_tri )
{
	struct( fvec2 ) pos;
	struct( fvec3 ) rgb;
};
	#define create_struct_vertex_2d_tri( x, y, r, g, b ) create( struct( vertex_2d_tri ), .pos = { x, y }, .rgb = { r, g, b } )

make_struct( vertex_2d_tri_tex )
{
	struct( fvec2 ) pos;
	struct( fvec2 ) uv;
	struct( fvec3 ) rgb;
};
	#define create_struct_vertex_2d_tri_tex( x, y, u, v, r, g, b ) create( struct( vertex_2d_tri_tex ), .pos = { x, y }, .uv = { u, v }, .rgb = { r, g, b } )

make_struct( vertex_2d_line )
{
	struct( fvec2 ) pos;
	struct( fvec3 ) rgb;
};
	#define create_struct_vertex_2d_line( x, y, r, g, b ) create( struct( vertex_2d_line ), .pos = { x, y }, .rgb = { r, g, b } )

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

make_object(
	mesh,
	form_mesh form;
	spinlock lock;
	flag update;
	buffer vertex_buffer;
	buffer index_buffer;
	list vertices;
	list indices;
	u32 vertex_n;
)( in form_mesh in_form )
{
	#ifdef hept_debug
	print_error( in_form == null, "mesh: in_form is null" );
	#endif
	//
	mesh this = assign_mesh();
	//
	this->form = in_form;
	this->update = yes;
	this->vertices = assign_list( 0, 1, this->form->type_size, assign_mem( this->form->type_size ) );
	this->indices = new_list( u32 );
	//
	#ifdef hept_trace
	print_trace( "new mesh: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global list list_update_mesh = null;

global mesh default_mesh_line = null;
global mesh default_mesh_square = null;
global mesh default_mesh_square_tex = null;
global mesh default_mesh_window_white = null;
global mesh default_mesh_window_black = null;
global mesh default_mesh_window_tex = null;

	#define mesh_add_line( var, vertex_struct, a, b )         \
		DEF_START                                               \
		list_add( var->indices, u32, var->vertices->size );     \
		list_add( var->indices, u32, var->vertices->size + 1 ); \
		list_add( var->vertices, vertex_struct, a );            \
		list_add( var->vertices, vertex_struct, b );            \
		DEF_END

	#define mesh_add_tri( var, vertex_struct, a, b, c )       \
		DEF_START                                               \
		list_add( var->indices, u32, var->vertices->size );     \
		list_add( var->indices, u32, var->vertices->size + 1 ); \
		list_add( var->indices, u32, var->vertices->size + 2 ); \
		list_add( var->vertices, vertex_struct, a );            \
		list_add( var->vertices, vertex_struct, b );            \
		list_add( var->vertices, vertex_struct, c );            \
		DEF_END

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

fn update_mesh( in mesh in_mesh )
{
	engage_spinlock( in_mesh->lock );
	if( in_mesh->update )
	{
		list_add( list_update_mesh, mesh, in_mesh );
		in_mesh->update = no;
	}
	vacate_spinlock( in_mesh->lock );
}

fn draw_instanced_mesh( in mesh in_mesh, in u32 in_count )
{
	engage_spinlock( in_mesh->lock );
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers( current_renderer->command_buffers[ current_renderer->current_frame ], 0, 1, ref( in_mesh->vertex_buffer->buffer ), offsets );
	vkCmdBindIndexBuffer( current_renderer->command_buffers[ current_renderer->current_frame ], in_mesh->index_buffer->buffer, 0, VK_INDEX_TYPE_UINT32 );
	vkCmdDrawIndexed( current_renderer->command_buffers[ current_renderer->current_frame ], in_mesh->indices->size, in_count, 0, 0, 0 );
	vacate_spinlock( in_mesh->lock );
}

fn draw_mesh( in mesh in_mesh )
{
	draw_instanced_mesh( in_mesh, 1 );
}

//

/////// /////// /////// /////// /////// /////// ///////

// module
// -------
//

global text default_glsl_vert =
	"#version 450\n"
	""
	"layout(location = 0) in vec2 in_pos;\n"
	"layout(location = 1) in vec3 in_rgb;\n"
	""
	"layout(location = 0) out vec2 vert_pos;\n"
	"layout(location = 1) out vec3 vert_rgb;\n"
	""
	"void main()\n"
	"{\n"
	"    vert_pos = in_pos;\n"
	"    vert_rgb = in_rgb;\n"
	"    gl_Position = vec4(vert_pos, 0.0, 1.0);\n"
	"}";

global text default_glsl_frag =
	"#version 450\n"
	""
	"layout(location = 0) in vec2 vert_pos;\n"
	"layout(location = 1) in vec3 vert_rgb;\n"
	""
	"layout(location = 0) out vec4 out_rgba;\n"
	""
	"//layout(binding = 0) uniform sampler2D in_tex;\n"
	""
	"void main()\n"
	"{\n"
	"    out_rgba = vec4(vert_rgb, 1.);\n"
	"}";

make_object(
	module,
	form_module form;
	text path;
	H_shader_module shader_module;
	os_file file;
	H_info_pipeline_shader_stage stage_info;
)( in form_module in_form, in text in_path )
{
	#ifdef hept_debug
	print_error( in_form == null, "module: in_form is null" );
	print_error( in_path == null, "module: in_path is null" );
	#endif
	//
	module this = assign_module();
	//
	this->form = in_form;

	text glsl_name = new_text( in_path, 5 );
	join_text( glsl_name, ( ( this->form->shader_stage->type == shader_stage_type_vertex ) ? ( ".vert" ) : ( ".frag" ) ) );

	text spirv_name = new_text( glsl_name, 4 );
	join_text( spirv_name, ".spv" );

	//

	// ifn( check_file( spirv_name ) )
	{
	#ifndef hept_release
		ifn( check_file( glsl_name ) )
		{
			write_file( glsl_name, ( ( this->form->shader_stage->type == shader_stage_type_vertex ) ? ( default_glsl_vert ) : ( default_glsl_frag ) ) );
		}
	#endif

		text command = format_text( "glslangValidator -V %s -o %s", glsl_name, spirv_name );
		s32 sys_result = system( command );
	#ifdef hept_debug
		print_error( sys_result != 0, "failed to compile GLSL to SPIR-V\n" );
	#endif
	}

	this->file = new_os_file( spirv_name );

	H_info_shader_module module_info = H_create_info_shader_module( this->file->data, this->file->size );
	this->shader_module = H_new_shader_module( current_os_machine->device, module_info );
	this->stage_info = H_create_info_pipeline_shader_stage( to( VkShaderStageFlagBits, this->form->shader_stage->type ), this->shader_module, "main" );
	//
	#ifdef hept_trace
	print_trace( "new module: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global module default_module_2d_tri_vert = null;
global module default_module_2d_tri_frag = null;
global module default_module_2d_tri_tex_vert = null;
global module default_module_2d_tri_tex_frag = null;
global module default_module_2d_line_vert = null;
global module default_module_2d_line_frag = null;
// global module default_module_3d_tri_vert = null;
// global module default_module_3d_tri_frag = null;
// global module default_module_3d_tri_tex_vert = null;
// global module default_module_3d_tri_tex_frag = null;
// global module default_module_3d_line_vert = null;
// global module default_module_3d_line_frag = null;

//

/////// /////// /////// /////// /////// /////// ///////

// shader_input
// -------
// does thing

make_object(
	shader_input,
	u32 binding;
	ptr(pure) data;
	H_descriptor_pool descriptor_pool;
	H_descriptor_set descriptor_set;
)( in form_shader in_form_shader )
{
	#ifdef hept_debug
	print_error( in_form_shader == null, "shader_input: in_form_shader is null" );
	#endif
	//
	shader_input this = assign_shader_input();
	//

	list sizes = new_list( H_descriptor_pool_size );
	iter_list( in_form_shader->inputs, i )
	{
		form_shader_input this_input = list_get( in_form_shader->inputs, form_shader_input, i );
		list_add( sizes, H_descriptor_pool_size, ( ( H_descriptor_pool_size ){ .type = to( VkDescriptorType, this_input->type ), .descriptorCount = 1 } ) );
	}

	H_info_descriptor_pool pool_info = H_create_info_descriptor_pool( 1, sizes->size, ( const ptr( VkDescriptorPoolSize ) )sizes->data, 0 );

	this->descriptor_pool = H_new_descriptor_pool( current_os_machine->device, pool_info );

	H_info_descriptor_set alloc_info = H_create_info_descriptor_set(
		this->descriptor_pool,
		1,
		ref( in_form_shader->descriptor_layout )
	);

	this->descriptor_set = H_new_descriptor_set( current_os_machine->device, alloc_info );

	//
	#ifdef hept_trace
	print_trace( "new shader_input: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global list list_update_shader_input_storage = null;
global list list_update_shader_input_image = null;

fn update_shader_input_storage( in s32 in_binding, in shader_input in_shader_input, in buffer in_buffer )
{
	/*lock_list(list_update_shader_input_storage);
	in_shader_input->binding = in_binding;
	in_shader_input->data = to(ptr(pure),in_buffer);
	list_add(list_update_shader_input_storage,shader_input,in_shader_input);
	unlock_list(list_update_shader_input_storage);*/
	H_update_descriptor_set_storage( in_binding, current_os_machine->device, in_shader_input->descriptor_set, in_buffer->buffer, in_buffer->size );
}

fn update_shader_input_image( in s32 in_binding, in shader_input in_shader_input, in image in_image )
{
	H_update_descriptor_set_image( in_binding, current_os_machine->device, in_shader_input->descriptor_set, in_image->form->sampler, in_image->view );
}

//

/////// /////// /////// /////// /////// /////// ///////

// shader
// -------
// does thing

make_object(
	shader,
	form_shader form;
	form_frame frame_form;

	H_vertex_binding info_vertex_binding;
	H_info_pipeline_vertex info_vertex;
	H_info_pipeline_assembly info_assembly;
	H_info_pipeline_raster info_raster;
	H_info_pipeline_multisample info_multisample;
	H_info_pipeline_depth_stencil info_depth_stencil;
	H_info_pipeline_blend info_blend;

	u32 constant_bytes;
	list modules;
	list stages;
	H_pipeline_layout pipeline_layout;
	H_pipeline pipeline;
)( in form_shader in_form, in form_frame in_form_frame, in list in_modules, in ptr( H_info_pipeline_blend ) in_blend, in u32 in_constant_bytes )
{
	#ifdef hept_debug
	print_error( in_form == null, "shader: in_form is null" );
	print_error( in_form_frame == null, "shader: in_form_frame is null" );
	#endif
	//
	shader this = assign_shader();
	//
	this->form = in_form;
	this->frame_form = in_form_frame;
	this->modules = in_modules;
	this->constant_bytes = in_constant_bytes;
	//
	form_mesh vert_form_mesh = null;
	this->stages = new_list( H_info_pipeline_shader_stage );
	iter_list( this->modules, m )
	{
		module this_module = list_get( this->modules, module, m );
		if( this_module->form->shader_stage->type == shader_stage_type_vertex )
		{
			vert_form_mesh = this_module->form->mesh_form;
		}

		list_add( this->stages, H_info_pipeline_shader_stage, this_module->stage_info );
	}

	//

	this->info_vertex_binding = H_create_vertex_binding_per_vertex( 0, vert_form_mesh->type_size );

	ptr( H_vertex_attribute ) vert_attributes = new_mem( H_vertex_attribute, vert_form_mesh->attribs->size );
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

	this->info_vertex = H_create_info_pipeline_vertex(
		1, ref( this->info_vertex_binding ), vert_form_mesh->attribs->size, vert_attributes
	);

	this->info_assembly = H_create_info_pipeline_assembly( this->form->topology, no );

	this->info_raster = H_create_info_pipeline_raster(
		no,
		no,
		VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_CLOCKWISE,
		no,
		0.0f,
		0.0f,
		0.0f,
		1.0f
	);

	this->info_multisample = H_create_info_pipeline_multisample(
		VK_SAMPLE_COUNT_1_BIT,
		no,
		1.0f,
		null,
		no,
		no
	);

	this->info_depth_stencil = H_create_info_pipeline_depth_stencil(
		yes,
		yes,
		VK_COMPARE_OP_LESS_OR_EQUAL,
		no,
		no,
		0,
		0,
		0.0f,
		1.0f
	);

	//

	if( in_blend == null )
		this->info_blend = H_create_info_pipeline_blend(
			no,
			0,
			1,
			ref( H_blend_mode_normal ),
			1.,
			1.,
			1.,
			1.
		);
	else
		this->info_blend = val( in_blend );

	//

	H_info_pipeline_layout info_pipeline_layout;
	H_info_push_constant_range pushconst_range = H_create_info_push_constant_range(
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		this->constant_bytes
	);
	info_pipeline_layout = H_create_info_pipeline_layout( 1, ref( this->form->descriptor_layout ), ( this->constant_bytes > 0 ), ref( pushconst_range ) );
	this->pipeline_layout = H_new_pipeline_layout( current_os_machine->device, info_pipeline_layout );

	//

	H_info_pipeline_viewport viewport_info = H_create_info_pipeline_viewport(
		1, null, 1, null
	);
	//

	H_info_pipeline pipeline_info = H_create_info_pipeline(
		VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
		this->modules->size,
		to( ptr( H_info_pipeline_shader_stage ), this->stages->data ),
		ref( this->info_vertex ),
		ref( this->info_assembly ),
		null,
		ref( viewport_info ),
		ref( this->info_raster ),
		ref( this->info_multisample ),
		ref( this->info_depth_stencil ),
		ref( this->info_blend ),
		ref( H_dynamic_state_normal ),
		this->pipeline_layout,
		this->frame_form->render_pass,
		0,
		null,
		0
	);

	this->pipeline = H_new_pipeline( current_os_machine->device, pipeline_info );

	#ifdef hept_trace
	print_trace( "new shader: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

inl shader copy_shader( in shader in_shader, in form_shader in_form, in form_frame in_form_frame, in list in_modules, in ptr( H_info_pipeline_blend ) in_blend, in u32 in_constant_bytes )
{
	shader this = assign_shader();

	if( in_form == null )
	{
		this->form = in_shader->form;
		this->info_assembly = in_shader->info_assembly;
	}
	else
	{
		this->form = in_form;
		this->info_assembly = H_create_info_pipeline_assembly( this->form->topology, no );
	}
	if( in_form_frame == null ) this->frame_form = in_shader->frame_form;
	else
		this->frame_form = in_form_frame;

	this->info_vertex = in_shader->info_vertex;
	this->info_raster = in_shader->info_raster;
	this->info_multisample = in_shader->info_multisample;
	this->info_depth_stencil = in_shader->info_depth_stencil;
	if( in_blend == null ) this->info_blend = in_shader->info_blend;
	else
		this->info_blend = val( in_blend );

	if( in_modules == null )
	{
		this->modules = in_shader->modules;
		this->stages = in_shader->stages;
	}
	else
	{
		this->modules = in_modules;
		this->stages = new_list( H_info_pipeline_shader_stage );
		iter_list( this->modules, m )
		{
			list_add( this->stages, H_info_pipeline_shader_stage, list_get( this->modules, module, m )->stage_info );
		}
	}

	this->constant_bytes = in_constant_bytes;
	if( ( this->constant_bytes != in_shader->constant_bytes ) or in_form != null )
	{
		H_info_pipeline_layout info_pipeline_layout;
		H_info_push_constant_range pushconst_range = H_create_info_push_constant_range(
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			this->constant_bytes
		);
		info_pipeline_layout = H_create_info_pipeline_layout( 1, ref( this->form->descriptor_layout ), ( this->constant_bytes > 0 ), ref( pushconst_range ) );
		this->pipeline_layout = H_new_pipeline_layout( current_os_machine->device, info_pipeline_layout );
	}
	else
		this->pipeline_layout = in_shader->pipeline_layout;

	H_info_pipeline_viewport viewport_info = H_create_info_pipeline_viewport(
		1, null, 1, null
	);

	H_info_pipeline pipeline_info = H_create_info_pipeline(
		VK_PIPELINE_CREATE_DERIVATIVE_BIT,
		this->modules->size,
		to( ptr( H_info_pipeline_shader_stage ), this->stages->data ),
		ref( this->info_vertex ),
		ref( this->info_assembly ),
		null,
		ref( viewport_info ),
		ref( this->info_raster ),
		ref( this->info_multisample ),
		ref( this->info_depth_stencil ),
		ref( this->info_blend ),
		ref( H_dynamic_state_normal ),
		this->pipeline_layout,
		this->frame_form->render_pass,
		0,
		in_shader->pipeline,
		-1
	);

	this->pipeline = H_new_pipeline( current_os_machine->device, pipeline_info );

	out this;
}

global shader default_shader_2d_tri = null;
global shader default_shader_2d_tri_tex = null;
global shader default_shader_2d_tri_tex_mul = null;
global shader default_shader_2d_line = null;
// global shader default_shader_3d = null;

//

global H_info_pipeline_blend H_default_blend_none = H_create_info_pipeline_blend( no, 0, 1, ref( H_blend_mode_none ), 1., 1., 1., 1. );
global ptr( H_info_pipeline_blend ) default_blend_none = ref( H_default_blend_none );

global H_info_pipeline_blend H_default_blend_normal = H_create_info_pipeline_blend( no, 0, 1, ref( H_blend_mode_normal ), 1., 1., 1., 1. );
global ptr( H_info_pipeline_blend ) default_blend_normal = ref( H_default_blend_normal );

global H_info_pipeline_blend H_default_blend_red = H_create_info_pipeline_blend( no, 0, 1, ref( H_blend_mode_constant ), 1., 0., 0., 1. );
global ptr( H_info_pipeline_blend ) default_blend_red = ref( H_default_blend_red );

global H_info_pipeline_blend H_default_blend_add = H_create_info_pipeline_blend( no, 0, 1, ref( H_blend_mode_add ), 1., 1., 1., 1. );
global ptr( H_info_pipeline_blend ) default_blend_add = ref( H_default_blend_add );

global H_info_pipeline_blend H_default_blend_multiply = H_create_info_pipeline_blend( no, 0, 1, ref( H_blend_mode_multiply ), 1., 1., 1., 1. );
global ptr( H_info_pipeline_blend ) default_blend_multiply = ref( H_default_blend_multiply );

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

// commands

global H_command_buffer current_command_buffer = null;

/////// /////// /////// /////// /////// /////// ///////

// image commands

fn use_image_src( in image in_image, in flag in_preserve_contents )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		in_image->image,
		( in_preserve_contents ) ? ( in_image->layout ) : H_image_layout_undefined,
		H_image_layout_shader_read_only_optimal,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		1,
		0,
		1
	);
	in_image->layout = temp_barrier.newLayout;

	vkCmdPipelineBarrier(
		current_command_buffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		ref( temp_barrier )
	);
}

fn use_image_dst( in image in_image, in flag in_preserve_contents )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		in_image->image,
		( in_preserve_contents ) ? ( in_image->layout ) : H_image_layout_undefined,
		H_image_layout_color_attachment_optimal,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		1,
		0,
		1
	);
	in_image->layout = temp_barrier.newLayout;

	vkCmdPipelineBarrier(
		current_command_buffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		ref( temp_barrier )
	);
}

fn use_image_blit_src( in image in_image, in flag in_preserve_contents )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		in_image->image,
		( in_preserve_contents ) ? ( in_image->layout ) : H_image_layout_undefined,
		H_image_layout_transfer_src_optimal,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		1,
		0,
		1
	);
	in_image->layout = temp_barrier.newLayout;

	vkCmdPipelineBarrier(
		current_command_buffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		ref( temp_barrier )
	);
}

fn use_image_blit_dst( in image in_image, in flag in_preserve_contents )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		in_image->image,
		( in_preserve_contents ) ? ( in_image->layout ) : H_image_layout_undefined,
		H_image_layout_transfer_dst_optimal,
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		1,
		0,
		1
	);
	in_image->layout = temp_barrier.newLayout;

	vkCmdPipelineBarrier(
		current_command_buffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		ref( temp_barrier )
	);
}

fn use_image_present( in image in_image, in flag in_preserve_contents )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		in_image->image,
		( in_preserve_contents ) ? ( in_image->layout ) : H_image_layout_undefined,
		H_image_layout_present_src_KHR,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		1,
		0,
		1
	);
	in_image->layout = temp_barrier.newLayout;

	vkCmdPipelineBarrier(
		current_command_buffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		ref( temp_barrier )
	);
}

/////// /////// /////// /////// /////// /////// ///////

// shader commands

fn start_shader( in shader in_shader, in u32 in_width, in u32 in_height )
{
	set_current_shader( in_shader );

	H_scissor scissor = { 0 };
	scissor.offset = ( VkOffset2D ){ 0, 0 };
	scissor.extent.width = in_width;
	scissor.extent.height = in_height;

	vkCmdSetScissor( current_renderer->command_buffers[ current_renderer->current_frame ], 0, 1, ref( scissor ) );
	H_viewport viewport = H_create_viewport(
		0.0,
		0.0,
		to_f32( in_width ),
		to_f32( in_height ),
		0.0,
		1.0
	);
	vkCmdSetViewport( current_command_buffer, 0, 1, ref( viewport ) );
	vkCmdBeginRenderPass( current_command_buffer, ref( current_frame->info_begin ), VK_SUBPASS_CONTENTS_INLINE );
	vkCmdBindPipeline( current_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_shader->pipeline );
}

fn use_shader_input( in shader_input in_shader_input )
{
	vkCmdBindDescriptorSets( current_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_shader->pipeline_layout, 0, 1, ref( in_shader_input->descriptor_set ), 0, NULL );
}

fn use_constants( in u32 in_size, in ptr( pure ) in_data )
{
	vkCmdPushConstants(
		current_command_buffer,
		current_shader->pipeline_layout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		in_size,
		in_data
	);
}

fn end_shader()
{
	vkCmdEndRenderPass( current_command_buffer );
}

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

// form-less objects

/////// /////// /////// /////// /////// /////// ///////

// event
// -------
// does thing

make_enum( event_type ){
	event_type_null,
	event_type_once,
	event_type_always,
	event_type_alarm,
	event_type_count,
};

make_enum( event_state ){
	event_state_inactive,
	event_state_active,
	event_state_waiting,
};

make_object(
	event,
	enum( event_type ) type;
	enum( event_state ) state;
	u32 data;
	fn_ptr( pure, call );
)( in enum( event_type ) in_type, in u32 in_data, fn_ptr( pure, in_call ) )
{
	#ifdef hept_debug
	print_error( in_type == event_type_null, "event: in_type is null" );
	#endif
	//
	event this = assign_event();
	//
	this->type = in_type;
	if( this->type == event_type_alarm )
		this->state = event_state_waiting;
	else
		this->state = event_state_active;
	this->call = in_call;
	//
	#ifdef hept_trace
	print_trace( "new event: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

	#define new_event_once( FN ) new_event( event_type_once, 0, FN )
	#define new_event_always( FN ) new_event( event_type_always, 0, FN )
	#define new_event_alarm( FN, TICKS ) new_event( event_type_alarm, TICKS, FN )
	#define new_event_count( FN, TICKS ) new_event( event_type_count, TICKS, FN )

fn perform_event( in event in_event )
{
	if( in_event->state == event_state_inactive ) out;
	with( in_event->type )
	{
		is( event_type_once )
		{
			in_event->call();
			in_event->state = event_state_inactive;
			skip;
		}
		is( event_type_always )
		{
			in_event->call();
			skip;
		}
		is( event_type_alarm )
		{
			if( in_event->data == 0 )
			{
				in_event->call();
				in_event->state = event_state_inactive;
			}
			else
				in_event->data--;
			skip;
		}
		is( event_type_count )
		{
			if( in_event->data != 0 )
			{
				in_event->call();
				in_event->data--;
			}
			else
				in_event->state = event_state_inactive;
			skip;
		}
	}
}
/*
//

/////// /////// /////// /////// /////// /////// ///////

// thing
// -------
//

make_object(
	thing,
	ptr(pure) reference;
	list events;
)( in list in_events )
{
	#ifdef hept_debug
	print_error( in_events == null, "thing: in_events is null" );
	#endif
	//
	thing this = assign_thing();
	//
	this->events = in_events;
	//
	#ifdef hept_trace
	print_trace( "new thing: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

global thing default_thing_ = null;

//

/////// /////// /////// /////// /////// /////// ///////

// scene
// -------
//

make_object(
	scene,
	form_scene form;
	list actors;
	ELEMENTS;
)( in PARAMS )
{
	#ifdef hept_debug
	print_error( in_PARAMS == null, "scene: in_PARAMS is null" );
	#endif
	//
	scene this = assign_scene();
	//

	//
	#ifdef hept_trace
	print_trace( "new scene: ID: %d",this->pile_id );
	#endif
	//
	out this;
}

global scene default_scene_ = null;

//

/////// /////// /////// /////// /////// /////// ///////

// world
// -------
//

make_object(
	world,
	form_world form;
	list scenes;
)( in PARAMS )
{
	#ifdef hept_debug
	print_error( in_PARAMS == null, "world: in_PARAMS is null" );
	#endif
	//
	world this = assign_world();
	//

	//
	#ifdef hept_trace
	print_trace( "new world: ID: %d",this->pile_id );
	#endif
	//
	out this;
}

global world default_world_ = null;
*/
//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

global s32 main_width = 0;
global s32 main_height = 0;
fn_ptr( pure, main_fn_command, pure ) = null;
global os_thread main_thread = null;
global os_pacer main_thread_pacer = null;

make_struct( input )
{
	flag pressed, held, released;
};

global ptr( struct( input ) ) inputs;
global ptr( u16 ) input_updates;
global u8 input_update_ptr = 0;

fn main_init();

//

fn main_update_os_machines()
{
	#ifdef hept_trace
	do_once print_trace( "updating machines" );
	#endif
	iter_pile( pile_os_machine, m )
	{
		maybe maybe_machine = pile_find( pile_os_machine, os_machine, m );
		ifn( maybe_machine.valid ) next;
		os_machine this_machine = to( os_machine, maybe_machine.value );
		if( this_machine->physical_device != null ) next;
		//
		this_machine->queue_family_index = u32_max;

		u32 physical_devices_count = H_get_physical_devices( current_os_core->instance, null );
		ptr( H_physical_device ) physical_devices = new_mem( H_physical_device, physical_devices_count );
		H_get_physical_devices( current_os_core->instance, physical_devices );

		H_physical_device integrated = null;
		iter( physical_devices_count, i )
		{
			H_physical_device_properties dev_prop;
			vkGetPhysicalDeviceProperties( physical_devices[ i ], ref( dev_prop ) );

			if( dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU or dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU )
			{
				u32 queue_family_count = H_get_physical_device_queue_properties( physical_devices[ i ], null );
				ptr( VkQueueFamilyProperties ) queue_family_properties = new_mem( VkQueueFamilyProperties, queue_family_count );
				H_get_physical_device_queue_properties( physical_devices[ i ], queue_family_properties );

				iter( queue_family_count, j )
				{
					this_machine->queue_family_index = j;
					u32 support_present;
					vkGetPhysicalDeviceSurfaceSupportKHR( physical_devices[ i ], j, current_os_window->surface, ref( support_present ) );

					if( ( queue_family_properties[ j ].queueFlags & VK_QUEUE_GRAPHICS_BIT ) and ( queue_family_properties[ j ].queueFlags & VK_QUEUE_COMPUTE_BIT ) and support_present )
					{
						if( dev_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
						{
							this_machine->physical_device = physical_devices[ i ];
	#ifdef hept_debug
							print_debug( "GPU name: %s", dev_prop.deviceName );
	#endif
							skip;
						}
						elif( integrated == null )
						{
							integrated = physical_devices[ i ];
						}
					}
				}
				free_mem( queue_family_properties );
			}

			if( this_machine->physical_device != null )
			{
				skip;
			}
		}
		free_mem( physical_devices );

		if( this_machine->physical_device == null )
		{
			this_machine->physical_device = integrated;
	#ifdef hept_debug
			print_error( this_machine->physical_device == null, "main_update_os_machines: could not find GPU" );
	#endif
		}

		f32 queue_priority = 1.0f;
		H_info_device_queue device_queue = H_create_info_device_queue( this_machine->queue_family_index, 1, ref( queue_priority ) );

		H_physical_device_features features = H_get_physical_device_features( this_machine->physical_device );

		//

		text extensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		u32 extension_count = 1;

		H_info_device info_device = H_create_info_device( 1, ref( device_queue ), 0, null, extension_count, ( ptr( const char ) ptr( const ) )extensions, ref( features ) );

		this_machine->device = H_new_device( this_machine->physical_device, info_device );
	}
}

//

fn main_update_os_windows()
{
	#ifdef hept_trace
	do_once print_trace( "updating windows" );
	#endif
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
			if( msg.message == 0x0012 )
			{
				hept_exit = yes;
				out;
			}
			else
			{
				TranslateMessage( ref( msg ) );
				DispatchMessage( ref( msg ) );
			}
		}

	#elif OS_LINUX
		process_os_window( this_window->link.xdis, this_window->link.xwin );
	#endif

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
			u32 format_n = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR( current_os_machine->physical_device, this_window->surface, ref( format_n ), null );
			ptr( H_surface_format ) formats = new_mem( H_surface_format, format_n );
			vkGetPhysicalDeviceSurfaceFormatsKHR( current_os_machine->physical_device, this_window->surface, ref( format_n ), formats );
			this_window->surface_format = formats[ 0 ];
			free_mem( formats );

			u32 present_mode_n = 0;
			vkGetPhysicalDeviceSurfacePresentModesKHR( current_os_machine->physical_device, this_window->surface, ref( present_mode_n ), null );
			ptr( H_present_mode ) present_modes = new_mem( H_present_mode, present_mode_n );
			vkGetPhysicalDeviceSurfacePresentModesKHR( current_os_machine->physical_device, this_window->surface, ref( present_mode_n ), present_modes );
			iter( present_mode_n, i )
			{
				if( present_modes[ i ] == H_present_mode_vsync_off )
				{
					this_window->present_mode = H_present_mode_vsync_off;
					skip;
				}
			}
			free_mem( present_modes );

			new_renderer( default_form_renderer, this_window, main_width, main_height );
			this_window->ready = yes;
		}
	}
}

//

fn main_update_renderers()
{
	#ifdef hept_trace
	do_once print_trace( "updating renderers" );
	#endif
	iter_pile( pile_renderer, r )
	{
		maybe maybe_renderer = pile_find( pile_renderer, renderer, r );
		ifn( maybe_renderer.valid ) next;

		renderer this_renderer = cast( maybe_renderer.value, renderer );
		set_current_renderer( this_renderer );

		//

		if( this_renderer->changed ) update_renderer( this_renderer );

		current_command_buffer = current_renderer->command_buffers[ current_renderer->current_frame ];

		//

		{
			vkWaitForFences( current_os_machine->device, 1, ref( this_renderer->flight_fences[ this_renderer->fence_id ] ), VK_TRUE, UINT64_MAX );

			lock_list(list_update_shader_input_storage);
			//vkDeviceWaitIdle(current_os_machine->device);
			iter_list(list_update_shader_input_storage,s)
			{
				shader_input this_input = list_get(list_update_shader_input_storage,shader_input,s);
				buffer this_input_buffer = to(buffer,this_input->data);
				H_update_descriptor_set_storage( this_input->binding, current_os_machine->device, this_input->descriptor_set, this_input_buffer->buffer, this_input_buffer->size );
			}
			unlock_list(list_update_shader_input_storage);

			VkResult aquire_result = vkAcquireNextImageKHR(
				current_os_machine->device, this_renderer->swapchain, UINT64_MAX, this_renderer->image_ready[ this_renderer->current_frame ], VK_NULL_HANDLE, ref( this_renderer->current_frame )
			);

			if( aquire_result == VK_ERROR_OUT_OF_DATE_KHR || aquire_result == VK_SUBOPTIMAL_KHR )
			{
				update_renderer( this_renderer );
				main_update_renderers();
				next;
			}

			vkResetFences( current_os_machine->device, 1, ref( this_renderer->flight_fences[ this_renderer->fence_id ] ) );

			this_renderer->fence_id = ( this_renderer->current_frame + 1 ) mod this_renderer->frames->size;
		}

		//

		image this_window_image = list_get( this_renderer->frame_window->images, image, 0 );

		frame this_frame = list_get( this_renderer->frames, frame, this_renderer->current_frame );
		image this_frame_image = list_get( this_frame->images, image, 0 );

		VkCommandBufferBeginInfo begin_info = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

		vkBeginCommandBuffer(
			current_command_buffer,
			ref( begin_info )
		);

		//

		{
	#ifdef hept_trace
			do_once print_trace( "updating meshes" );
	#endif
			iter_list( list_update_mesh, m )
			{
				mesh this_mesh = list_pop_front( list_update_mesh, mesh );
	#ifdef hept_trace
				print_trace( "updating mesh ID: %d, with %d vert and %d ind", this_mesh->pile_id, this_mesh->vertices->size, this_mesh->indices->size );
	#endif
				engage_spinlock( this_mesh->lock );
				if( this_mesh->vertex_buffer == null )
				{
					this_mesh->vertex_buffer = new_buffer( default_form_buffer_vertex, this_mesh->form->type_size * this_mesh->vertices->size );
				}

				if( this_mesh->index_buffer == null )
				{
					this_mesh->index_buffer = new_buffer( default_form_buffer_index, size_u32 * this_mesh->indices->size );
				}
				update_buffer( this_mesh->vertex_buffer, this_mesh->vertices->size * this_mesh->vertices->size_type, this_mesh->vertices->data );
				update_buffer( this_mesh->index_buffer, this_mesh->indices->size * this_mesh->indices->size_type, this_mesh->indices->data );
				vacate_spinlock( this_mesh->lock );
			}
		}

		//

		set_current_frame( current_renderer->frame_window );
		use_image_dst( this_window_image, no );

		//

		this_renderer->ref_window->call();

		//

		{
			use_image_blit_src( this_window_image, yes );
			use_image_blit_dst( this_frame_image, no );

			//

			f32 src_w = to_f32( main_width );
			f32 src_h = to_f32( main_height );
			f32 dst_h = this_renderer->swapchain_extent.height;
			f32 scale = round_f32( dst_h / src_h );

			f32 dst_w = src_w * scale;
			dst_h = src_h * scale;

			f32 src_offset_x = 0;
			f32 src_offset_y = 0;

			if( dst_w > this_renderer->swapchain_extent.width )
			{
				src_offset_x = ( dst_w - this_renderer->swapchain_extent.width ) / ( 2 * scale );
				src_w -= 2 * src_offset_x;
				dst_w = src_w * scale;
			}

			if( dst_h > this_renderer->swapchain_extent.height )
			{
				src_offset_y = ( dst_h - this_renderer->swapchain_extent.height ) / ( 2 * scale );
				src_h -= 2 * src_offset_y;
				dst_h = src_h * scale;
			}

			src_h = floor_f32( dst_h / scale );
			dst_h = floor_f32( src_h * scale );
			src_w = floor_f32( dst_w / scale );
			dst_w = floor_f32( src_w * scale );

			f32 dst_offset_x = ( this_renderer->swapchain_extent.width - dst_w ) / 2;
			f32 dst_offset_y = ( this_renderer->swapchain_extent.height - dst_h ) / 2;

			VkImageBlit blit = { 0 };
			blit.srcOffsets[ 0 ] = ( H_offset_3d ){ src_offset_x, src_offset_y, 0 };
			blit.srcOffsets[ 1 ] = ( H_offset_3d ){ src_w + src_offset_x, src_h + src_offset_y, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = 0;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;

			blit.dstOffsets[ 0 ] = ( H_offset_3d ){ dst_offset_x, dst_offset_y, 0 };
			blit.dstOffsets[ 1 ] = ( H_offset_3d ){ dst_offset_x + dst_w, dst_offset_y + dst_h, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = 0;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(
				current_command_buffer,
				this_window_image->image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				this_frame_image->image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&blit,
				VK_FILTER_NEAREST
			);
		}

		use_image_present( this_frame_image, yes );

		vkEndCommandBuffer( current_command_buffer );

		//

		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		H_info_submit submit_info = H_create_info_submit(
			1, ref( this_renderer->image_ready[ this_renderer->current_frame ] ), wait_stages, 1, ref( current_command_buffer ), 1, ref( this_renderer->image_done[ this_renderer->current_frame ] )
		);

		H_submit_queue(
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
			update_renderer( this_renderer );
			main_update_renderers();
			next;
		}

		this_renderer->current_frame = ( this_renderer->current_frame + 1 ) % this_renderer->frames->size;
	}
}

//

fn main_update_actors()
{
	/*iter_pile( pile_event, e )
	{
		maybe maybe_window = pile_find( pile_os_window, event, e );
		ifn( maybe_window.valid ) next;
		os_window this_window = to( os_window, maybe_window.value );

		//
	}*/
}

//

fn main_update()
{
	if( hept_exit ) out;
	main_update_os_windows();
	if( hept_exit ) out;
	main_update_renderers();
}

//

fn update_inputs()
{
	// engage_spinlock(input_lock);
	as( safe_u8_get( input_update_ptr ) > 0 )
	{
		safe_u8_dec( input_update_ptr );
		safe_flag_set( inputs[ safe_u16_get( input_updates[ safe_u8_get( input_update_ptr ) ] ) ].pressed, no );
		safe_flag_set( inputs[ safe_u16_get( input_updates[ safe_u8_get( input_update_ptr ) ] ) ].released, no );
	}
	// vacate_spinlock(input_lock);
}

//

inl ptr( pure ) main_thread_call( in ptr( pure ) in_ptr )
{
	main_thread_pacer = new_os_pacer( 120 );
	loop
	{
		if( hept_exit ) out null;
		start_os_pacer( main_thread_pacer );
		//
		if(pile_event != null)
		{
			lock_pile( pile_event );
			iter_pile( pile_event, e )
			{
				maybe maybe_event = pile_find( pile_event, event, e );
				ifn( maybe_event.valid ) next;
				event this_event = to( event, maybe_event.value );
				perform_event( this_event );
				//
			}
			unlock_pile( pile_event );
		}
		//
		update_inputs();
		//
		if( hept_exit ) out null;
		wait_os_pacer( main_thread_pacer );
	}
}

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

// os_window call-back function for window events + input

	#if OS_WINDOWS
inl LRESULT CALLBACK process_os_window( HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param )
{
	with( u_msg )
	{
		is( WM_DESTROY )
		{
			safe_flag_set( hept_exit, yes );
			out 0;
		}

		is( WM_KEYDOWN )
			is( WM_SYSKEYDOWN )
		{
			// engage_spinlock(input_lock);
			if( safe_flag_get( inputs[ w_param ].held ) == no )
			{
				safe_flag_set( inputs[ w_param ].pressed, yes );
				safe_flag_set( inputs[ w_param ].released, no );
				safe_u16_set( input_updates[ safe_u8_get( input_update_ptr ) ], w_param );
				safe_u8_inc( input_update_ptr );
			}
			safe_flag_set( inputs[ w_param ].held, yes );
			// vacate_spinlock(input_lock);
			skip;
		}

		is( WM_KEYUP )
			is( WM_SYSKEYUP )
		{
			// engage_spinlock(input_lock);
			if( safe_flag_get( inputs[ w_param ].held ) )
			{
				safe_flag_set( inputs[ w_param ].held, no );
				safe_flag_set( inputs[ w_param ].pressed, no );
				safe_flag_set( inputs[ w_param ].released, yes );
				safe_u16_set( input_updates[ safe_u8_get( input_update_ptr ) ], w_param );
				safe_u8_inc( input_update_ptr );
			}
			// vacate_spinlock(input_lock);
			skip;
		}

	default:
		out DefWindowProc( hwnd, u_msg, w_param, l_param );
	}

	out 0;
}
	#elif OS_LINUX
void process_os_window( ptr( Display ) in_disp, Window in_win )
{
	XEvent e;
	u32 custom_key;
	as( XPending( in_disp ) )
	{
		XNextEvent( in_disp, ref( e ) );

		with( e.type )
		{
			is( DestroyNotify )
			{
				hept_exit = yes;
				out;
			}
			/*is( ConfigureNotify )
			{
				update_renderer( current_renderer );
				hept_update();
				out;
			}*/
		default: skip;
		}
	}
}
	#endif

//

/////// /////// /////// /////// /////// /////// ///////

	#ifdef hept_release
		#define main_data_folder "./data"
	#else
		#define main_data_folder "../../src/data"
	#endif

	#define main_shader_folder main_data_folder "/shader"
	#define main_image_folder main_data_folder "/image"
	#define main_sound_folder main_data_folder "/sound"

fn main_defaults()
{
	#ifdef hept_trace
	do_once print_trace( "creating defaults" );
	#endif

	/////// /////// /////// /////// /////// /////// ///////
	// default folders

	#ifndef hept_release
	make_folder( main_data_folder );
	make_folder( main_shader_folder );
	make_folder( main_shader_folder "/default" );
	// make_folder( main_image_folder );
	// make_folder( main_sound_folder );
	#endif

	/////// /////// /////// /////// /////// /////// ///////
	// default forms

	default_form_buffer_vertex = new_form_buffer(
		H_buffer_usage_transfer_dst | H_buffer_usage_vertex,
		H_memory_property_device_local | H_memory_property_host_visible | H_memory_property_host_coherent
	);
	default_form_buffer_index = new_form_buffer(
		H_buffer_usage_transfer_dst | H_buffer_usage_index,
		H_memory_property_device_local | H_memory_property_host_visible | H_memory_property_host_coherent
	);
	default_form_buffer_storage = new_form_buffer(
		H_buffer_usage_transfer_dst | H_buffer_usage_storage,
		H_memory_property_device_local | H_memory_property_host_visible | H_memory_property_host_coherent
	);
	//
	default_form_image_rgba = new_form_image( image_type_rgba, VK_FORMAT_B8G8R8A8_UNORM );
	default_form_image_depth = new_form_image( image_type_depth, VK_FORMAT_D32_SFLOAT );
	default_form_image_stencil = new_form_image( image_type_stencil, VK_FORMAT_S8_UINT );
	//
	default_form_frame_layer_rgba = new_form_frame_layer( frame_layer_type_rgba, VK_FORMAT_B8G8R8A8_UNORM );
	default_form_frame_layer_depth = new_form_frame_layer( frame_layer_type_depth, VK_FORMAT_D32_SFLOAT );
	default_form_frame_layer_stencil = new_form_frame_layer( frame_layer_type_stencil, VK_FORMAT_S8_UINT );
	//
	list layers = new_list( form_frame_layer );
	list_add( layers, form_frame_layer, default_form_frame_layer_rgba );
	default_form_frame = new_form_frame( frame_type_attachment, layers );
	//
	default_form_renderer = new_form_renderer();
	//
	default_form_mesh_attrib_pos2 = new_form_mesh_attrib( f32, 2 );
	default_form_mesh_attrib_pos3 = new_form_mesh_attrib( f32, 3 );
	default_form_mesh_attrib_uv = new_form_mesh_attrib( f32, 2 );
	default_form_mesh_attrib_rgb = new_form_mesh_attrib( f32, 3 );
	default_form_mesh_attrib_rgba = new_form_mesh_attrib( f32, 4 );
	//
	list attribs_2d_tri = new_list( form_mesh_attrib );
	list_add( attribs_2d_tri, form_mesh_attrib, default_form_mesh_attrib_pos2 );
	list_add( attribs_2d_tri, form_mesh_attrib, default_form_mesh_attrib_rgb );
	default_form_mesh_2d_tri = new_form_mesh( attribs_2d_tri, yes );
	list attribs_2d_tri_tex = new_list( form_mesh_attrib );
	list_add( attribs_2d_tri_tex, form_mesh_attrib, default_form_mesh_attrib_pos2 );
	list_add( attribs_2d_tri_tex, form_mesh_attrib, default_form_mesh_attrib_uv );
	list_add( attribs_2d_tri_tex, form_mesh_attrib, default_form_mesh_attrib_rgb );
	default_form_mesh_2d_tri_tex = new_form_mesh( attribs_2d_tri_tex, yes );
	list attribs_2d_line = new_list( form_mesh_attrib );
	list_add( attribs_2d_line, form_mesh_attrib, default_form_mesh_attrib_pos2 );
	list_add( attribs_2d_line, form_mesh_attrib, default_form_mesh_attrib_rgb );
	default_form_mesh_2d_line = new_form_mesh( attribs_2d_line, yes );
	// list attribs_3d = new_list( form_mesh_attrib );
	// list_add( attribs_3d, form_mesh_attrib, default_form_mesh_attrib_pos3 );
	// list_add( attribs_3d, form_mesh_attrib, default_form_mesh_attrib_uv );
	// list_add( attribs_3d, form_mesh_attrib, default_form_mesh_attrib_rgb );
	// default_form_mesh_3d = new_form_mesh( attribs_3d, yes );
	//
	default_form_shader_stage_vert = new_form_shader_stage( shader_stage_type_vertex );
	default_form_shader_stage_geom = new_form_shader_stage( shader_stage_type_geometry );
	default_form_shader_stage_frag = new_form_shader_stage( shader_stage_type_fragment );
	default_form_shader_stage_comp = new_form_shader_stage( shader_stage_type_compute );
	//
	default_form_module_2d_tri_vert = new_form_module( default_form_shader_stage_vert, default_form_mesh_2d_tri );
	default_form_module_2d_tri_frag = new_form_module( default_form_shader_stage_frag, default_form_mesh_2d_tri );
	default_form_module_2d_tri_tex_vert = new_form_module( default_form_shader_stage_vert, default_form_mesh_2d_tri_tex );
	default_form_module_2d_tri_tex_frag = new_form_module( default_form_shader_stage_frag, default_form_mesh_2d_tri_tex );
	default_form_module_2d_line_vert = new_form_module( default_form_shader_stage_vert, default_form_mesh_2d_line );
	default_form_module_2d_line_frag = new_form_module( default_form_shader_stage_frag, default_form_mesh_2d_line );
	//
	default_form_shader_input_image = new_form_shader_input( shader_input_type_image, shader_stage_type_fragment );
	default_form_shader_input_storage_vert = new_form_shader_input( shader_input_type_storage, shader_stage_type_vertex );
	default_form_shader_input_storage_frag = new_form_shader_input( shader_input_type_storage, shader_stage_type_fragment );
	//

	default_form_shader_line = new_form_shader( H_topology_line, null );

	default_form_shader_tri = new_form_shader( H_topology_tri, null );

	list shader_inputs_tri_tex = new_list( form_shader_input );
	list_add( shader_inputs_tri_tex, form_shader_input, default_form_shader_input_image );
	default_form_shader_tri_tex = new_form_shader( H_topology_tri, shader_inputs_tri_tex );

	list shader_inputs_line_storage = new_list( form_shader_input );
	list_add( shader_inputs_line_storage, form_shader_input, default_form_shader_input_storage_vert );
	default_form_shader_line_storage = new_form_shader( H_topology_line, shader_inputs_line_storage );

	list shader_inputs_tri_storage = new_list( form_shader_input );
	list_add( shader_inputs_tri_storage, form_shader_input, default_form_shader_input_storage_vert );
	default_form_shader_tri_storage = new_form_shader( H_topology_tri, shader_inputs_tri_storage );

	list shader_inputs_tri_tex_storage = new_list( form_shader_input );
	list_add( shader_inputs_tri_tex_storage, form_shader_input, default_form_shader_input_storage_vert );
	list_add( shader_inputs_tri_tex_storage, form_shader_input, default_form_shader_input_image );
	default_form_shader_tri_tex_storage = new_form_shader( H_topology_tri, shader_inputs_tri_tex_storage );

	/////// /////// /////// /////// /////// /////// ///////
	// default objects

	default_mesh_line = new_mesh( default_form_mesh_2d_line );
	mesh_add_line(
		default_mesh_line,
		struct( vertex_2d_line ),
		create_struct_vertex_2d_line( 0, 0, 1, 1, 1 ),
		create_struct_vertex_2d_line( 1, 0, 1, 1, 1 )
	);
	update_mesh( default_mesh_line );

	default_mesh_square = new_mesh( default_form_mesh_2d_tri );
	mesh_add_quad(
		default_mesh_square,
		struct( vertex_2d_tri ),
		create_struct_vertex_2d_tri( -.5, -.5, 1, 1, 1 ),
		create_struct_vertex_2d_tri( .5, -.5, 1, 1, 1 ),
		create_struct_vertex_2d_tri( .5, .5, 1, 1, 1 ),
		create_struct_vertex_2d_tri( -.5, .5, 1, 1, 1 )
	);
	update_mesh( default_mesh_square );

	default_mesh_square_tex = new_mesh( default_form_mesh_2d_tri_tex );
	mesh_add_quad(
		default_mesh_square_tex,
		struct( vertex_2d_tri_tex ),
		create_struct_vertex_2d_tri_tex( -.5, -.5, 0, 0, 1, 1, 1 ),
		create_struct_vertex_2d_tri_tex( .5, -.5, 1, 0, 1, 1, 1 ),
		create_struct_vertex_2d_tri_tex( .5, .5, 1, 1, 1, 1, 1 ),
		create_struct_vertex_2d_tri_tex( -.5, .5, 0, 1, 1, 1, 1 )
	);
	update_mesh( default_mesh_square_tex );

	default_mesh_window_white = new_mesh( default_form_mesh_2d_tri );
	mesh_add_quad(
		default_mesh_window_white,
		struct( vertex_2d_tri ),
		create_struct_vertex_2d_tri( -1., -1., 1, 1, 1 ),
		create_struct_vertex_2d_tri( 1., -1., 1, 1, 1 ),
		create_struct_vertex_2d_tri( 1., 1., 1, 1, 1 ),
		create_struct_vertex_2d_tri( -1., 1., 1, 1, 1 )
	);
	update_mesh( default_mesh_window_white );

	default_mesh_window_black = new_mesh( default_form_mesh_2d_tri );
	mesh_add_quad(
		default_mesh_window_black,
		struct( vertex_2d_tri ),
		create_struct_vertex_2d_tri( -1., -1., 0, 0, 0 ),
		create_struct_vertex_2d_tri( 1., -1., 0, 0, 0 ),
		create_struct_vertex_2d_tri( 1., 1., 0, 0, 0 ),
		create_struct_vertex_2d_tri( -1., 1., 0, 0, 0 )
	);
	update_mesh( default_mesh_window_black );

	default_mesh_window_tex = new_mesh( default_form_mesh_2d_tri_tex );
	mesh_add_quad(
		default_mesh_window_tex,
		struct( vertex_2d_tri_tex ),
		create_struct_vertex_2d_tri_tex( -1., -1., 0, 0, 1, 1, 1 ),
		create_struct_vertex_2d_tri_tex( 1., -1., 1, 0, 1, 1, 1 ),
		create_struct_vertex_2d_tri_tex( 1., 1., 1, 1, 1, 1, 1 ),
		create_struct_vertex_2d_tri_tex( -1., 1., 0, 1, 1, 1, 1 )
	);
	update_mesh( default_mesh_window_tex );

	default_module_2d_tri_vert = new_module( default_form_module_2d_tri_vert, main_shader_folder "/default/default_2d_tri" );
	default_module_2d_tri_frag = new_module( default_form_module_2d_tri_frag, main_shader_folder "/default/default_2d_tri" );
	default_module_2d_tri_tex_vert = new_module( default_form_module_2d_tri_tex_vert, main_shader_folder "/default/default_2d_tri_tex" );
	default_module_2d_tri_tex_frag = new_module( default_form_module_2d_tri_tex_frag, main_shader_folder "/default/default_2d_tri_tex" );
	default_module_2d_line_vert = new_module( default_form_module_2d_line_vert, main_shader_folder "/default/default_2d_line" );
	default_module_2d_line_frag = new_module( default_form_module_2d_line_frag, main_shader_folder "/default/default_2d_line" );

	list tri_modules = new_list( module );
	list_add( tri_modules, module, default_module_2d_tri_vert );
	list_add( tri_modules, module, default_module_2d_tri_frag );
	default_shader_2d_tri = new_shader( default_form_shader_tri, default_form_frame, tri_modules, null, 0 );

	list tri_tex_modules = new_list( module );
	list_add( tri_tex_modules, module, default_module_2d_tri_tex_vert );
	list_add( tri_tex_modules, module, default_module_2d_tri_tex_frag );
	default_shader_2d_tri_tex = new_shader( default_form_shader_tri_tex, default_form_frame, tri_tex_modules, null, 0 );

	list line_modules = new_list( module );
	list_add( line_modules, module, default_module_2d_line_vert );
	list_add( line_modules, module, default_module_2d_line_frag );
	default_shader_2d_line = new_shader( default_form_shader_line, default_form_frame, line_modules, null, 0 );
}

//

	#define main( CREATOR_NAME, WINDOW_NAME, WIDTH, HEIGHT, FN_COMMAND )               \
		global text main_creator_name = CREATOR_NAME;                                    \
		global text main_window_name = WINDOW_NAME;                                      \
                                                                                     \
		int main()                                                                       \
		{                                                                                \
			inputs = new_mem( struct( input ), 512 );                                      \
			input_updates = new_mem( u16, 256 );                                           \
			list_update_mesh = new_list( mesh );                                           \
      list_update_shader_input_storage = new_list(shader_input);                                                                              \
			main_width = WIDTH;                                                            \
			main_height = HEIGHT;                                                          \
			main_fn_command = to( fn_ptr( pure, , pure ), FN_COMMAND );                    \
			{                                                                              \
				list_object_piles = new_list( pile );                                        \
				new_os_core( main_creator_name );                                            \
				new_os_machine();                                                            \
				new_os_window( main_window_name, main_width, main_height, main_fn_command ); \
			}                                                                              \
			main_update_os_machines();                                                     \
			main_defaults();                                                               \
			main_init();                                                                   \
      main_thread = new_os_thread(main_thread_call);                                                                              \
			loop                                                                           \
			{                                                                              \
				main_update();                                                               \
				if( hept_exit ) out 0;                                                       \
			}                                                                              \
			out 0;                                                                         \
		}                                                                                \
		fn main_init()

//

/////// /////// /////// /////// /////// /////// ///////

#endif

/////// /////// /////// /////// /////// /////// ///////