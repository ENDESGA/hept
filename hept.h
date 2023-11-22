// // // // // // //
// > hept _
// -------
// minimal game engine language and system framework
// requires: hephaestus.h, c7h16.h
// @ENDESGA 2023
// // // // // // //

#ifndef hept_included
#define hept_included

#ifndef hept_no_audio
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#endif

#include <c7h16.h>

#if OS_WINDOWS
	#define VK_USE_PLATFORM_WIN32_KHR
#elif OS_LINUX
	#define VK_USE_PLATFORM_XLIB_KHR
#elif OS_MACOS

	#define VK_USE_PLATFORM_MACOS_MVK
#endif

#include <Hephaestus.h>
#include <Hephaestus.h>

//

global flag hept_exit = no;

global list object_pile_list = null;

fn delete_hept_object_piles()
{
	iter_list( object_pile_list, p )
	{
		constant pile this_pile = list_remove_back( object_pile_list, pile );
		delete_pile( this_pile );
	}
	delete_list( object_pile_list );
}

//

#define make_object( NAME, ... )                          \
	make_struct( NAME##_struct )                            \
	{                                                       \
		u32 pile_id;                                          \
		__VA_ARGS__                                           \
	};                                                      \
	make_ptr( NAME##_struct ) NAME;                         \
                                                          \
	global NAME current_##NAME = null;                      \
	global pile NAME##_pile = null;                         \
	global list NAME##_update_list = null;                  \
                                                          \
	inl NAME assign_##NAME()                                \
	{                                                       \
		NAME this = new_ptr( NAME##_struct, 1 );              \
		if( NAME##_pile is null )                             \
		{                                                     \
			NAME##_pile = new_pile( NAME );                     \
			list_add( object_pile_list, pile, NAME##_pile );    \
		}                                                     \
		pile_add( NAME##_pile, NAME, this );                  \
		this->pile_id = NAME##_pile->prev_pos;                \
		current_##NAME = this;                                \
		out this;                                             \
	}                                                       \
                                                          \
	fn unassign_##NAME( NAME in_##NAME )                    \
	{                                                       \
		pile_delete( NAME##_pile, NAME, in_##NAME->pile_id ); \
		delete_ptr( in_##NAME );                              \
	}                                                       \
                                                          \
	NAME new_##NAME

//

/*
//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

//

/////// /////// /////// /////// /////// /////// ///////

/// NAME

make_object(
	NAME,
	ELEMENTS;
	ELEMENTS;
	ELEMENTS;
)( in PARAMS, in PARAMS )
{
	#ifdef hept_debug
	print_error( PARAMS is null, "NAME: PARAMS is null" );
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

fn delete_NAME( in NAME in_NAME )
{
	delete_ptr( in_NAME->ELEMENTS );
	unassign_NAME( in_NAME );
}

fn delete_all_NAMEs()
{
 if( pile_NAME isnt null )
 {
	 iter_pile( pile_NAME, NAME )
	 {
		 pile_find_iter( pile_NAME, NAME );
		 delete_NAME( this_NAME );
	 }
 }
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

//

global text main_path = null;
global text main_data_path = null;
global text main_audio_path = null;
global text main_shader_path = null;
global text main_shader_default_path = null;

global text main_creator_name = null;
global text main_app_name = null;
global s32 main_width = 0;
global s32 main_height = 0;
global f32 main_window_width = 0;
global f32 main_window_height = 0;
global f32 main_window_scale = 1;
global f32 main_scale = 1;
global f32 main_fps = 120;
global flag main_cursor_visible = yes;

fn_ptr( pure, main_fn_command, pure ) = null;

global flag main_vsync = no;

make_struct( input )
{
	flag pressed, held, released;
};

global ptr( input ) inputs;
global ptr( u16 ) input_updates;
global u8 input_update_ptr = 0;

global s32 mouse_prev_x = 0;
global s32 mouse_prev_y = 0;
global s32 mouse_x = 0;
global s32 mouse_y = 0;

global s32 main_mouse_window_x = 0;
global s32 main_mouse_window_y = 0;

global s32 main_mouse_wheel_x = 0;
global s32 main_mouse_wheel_y = 0;

global s32 mouse_window_prev_x = 0;
global s32 mouse_window_prev_y = 0;
global s32 mouse_window_x = 0;
global s32 mouse_window_y = 0;

//global flag main_thread_ready = no;
//global flag main_thread_go = no;
//global flag main_thread_done = no;
global spinlock main_thread_lock = 0;
global flag main_thread_exit = no;

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

//

/////// /////// /////// /////// /////// /////// ///////

// os_file

make_object(
	os_file,
	text path;
	text data;
	u32 size;
)( in text in_path )
{
#ifdef hept_debug
	print_error( in_path is null, "os_file: in_path is null" );
#endif
	//
	os_file this = assign_os_file();
	//
	this->path = in_path;
#if OS_WINDOWS
	HANDLE file = CreateFileA( ( LPCSTR )this->path, GENERIC_READ, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null );

	#ifdef hept_debug
	print_error( file is INVALID_HANDLE_VALUE, "failed to open: %s", this->path );
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
	print_error( fd is -1, "os_file: failed to open: %s", this->path );
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

fn delete_os_file( in os_file in_os_file )
{
	unassign_os_file( in_os_file );
}

fn delete_all_os_files()
{
	if( os_file_pile isnt null )
	{
		iter_pile( os_file_pile, os_file )
		{
			pile_find_iter( os_file_pile, os_file );
			delete_os_file( this_os_file );
		}
	}
}

fn write_file( in text in_path, in text in_contents )
{
#if OS_WINDOWS
	HANDLE hFile = CreateFileA( in_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile isnt INVALID_HANDLE_VALUE )
	{
		DWORD bytesWritten;
		WriteFile( hFile, in_contents, text_length( in_contents ), &bytesWritten, NULL );
		CloseHandle( hFile );
	}
#elif OS_LINUX
	s32 fd = open( in_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );
	if( fd isnt -1 )
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
	out( file_attr isnt INVALID_FILE_ATTRIBUTES ) and !( file_attr & FILE_ATTRIBUTE_DIRECTORY );
#elif OS_LINUX
	out access( in_path, F_OK ) isnt -1;
#endif
}

// folder

#if OS_WINDOWS
	#define folder_sep "\\"
#else
	#define folder_sep "/"
#endif

text parent_folder( in text in_text )
{
	if( in_text is null )
	{
		out null;
	}

	text cur = in_text + text_length( in_text ) - 1;

	as( cur >= in_text )
	{
		if( val( cur ) is '\\' || val( cur ) is '/' )
		{
			val( cur ) = '\0';
			skip;
		}
		val( cur ) = '\0';
		cur--;
	}

	out in_text;
}

text get_folder()
{
	s8 temp_text[ 257 ];
#if OS_WINDOWS
	GetModuleFileNameA( NULL, temp_text, 256 );
#elif OS_LINUX
	readlink( "/proc/self/exe", temp_text, 256 );
#endif
	text out_text = parent_folder( new_text( temp_text, 0 ) );
	out out_text;
}

flag check_folder( in text in_path )
{
#if OS_WINDOWS
	DWORD file_attr = GetFileAttributesA( in_path );
	out( ( file_attr isnt INVALID_FILE_ATTRIBUTES ) and ( file_attr & FILE_ATTRIBUTE_DIRECTORY ) );
#elif OS_LINUX
	struct stat st;
	out( stat( in_path, &st ) is 0 ) and S_ISDIR( st.st_mode );
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
		/*SHFILEOPSTRUCTA shFileOp = { 0 };
	shFileOp.wFunc = FO_COPY;
	shFileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;
	shFileOp.pFrom = in_src_path;
	shFileOp.pTo = in_dest_path;
	SHFileOperationA( &shFileOp );*/
#elif OS_LINUX
	text command = format_text( "cp -r %s %s", in_src_path, in_dest_path );
	system( command );
#endif
}

fn delete_folder( in text in_path )
{
	ifn( check_folder( in_path ) ) out;
#if OS_WINDOWS
		/*SHFILEOPSTRUCTA shFileOp = { 0 };
	shFileOp.wFunc = FO_DELETE;
	shFileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;
	shFileOp.pFrom = in_path;
	SHFileOperationA( &shFileOp );*/
#elif OS_LINUX
	text command = format_text( "rm -r %s", in_path );
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
	u64 time_ns;
)( in u32 in_fps )
{
#ifdef hept_debug
	print_error( in_fps is 0, "os_pacer: in_fps is 0" );
#endif
	//
	os_pacer this = assign_os_pacer();
	//
	this->fps = in_fps;
	this->time_ns = to_u64( to_f64( nano_per_sec ) / to_f64( this->fps ) );
//
#ifdef hept_trace
	print_trace( "new os_pacer: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_os_pacer( in os_pacer in_os_pacer )
{
	unassign_os_pacer( in_os_pacer );
}

fn delete_all_os_pacers()
{
	if( os_pacer_pile isnt null )
	{
		iter_pile( os_pacer_pile, os_pacer )
		{
			pile_find_iter( os_pacer_pile, os_pacer );
			delete_os_pacer( this_os_pacer );
		}
	}
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
	in_pacer->time_ns = to_u64( to_f64( nano_per_sec ) / to_f64( in_pacer->fps ) );
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
	print_error( in_function is null, "os_thread: in_function is null" );
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
	print_error( this->id is 0, "os_thread: could not create thread" );
#endif
//
#ifdef hept_trace
	print_trace( "new os_thread: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_os_thread( in os_thread in_os_thread )
{
	unassign_os_thread( in_os_thread );
}

fn delete_all_os_threads()
{
	if( os_thread_pile isnt null )
	{
		iter_pile( os_thread_pile, os_thread )
		{
			pile_find_iter( os_thread_pile, os_thread );
			delete_os_thread( this_os_thread );
		}
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

//

/////// /////// /////// /////// /////// /////// ///////

/// os_core

global u32 os_core_version = 1;

make_object(
	os_core,
	H_instance instance;
)( in text in_name )
{
#ifdef hept_debug
	print_error( in_name is null, "os_core: in_name is null" );
#endif
	//
	os_core this = assign_os_core();
	//

	//
	constant H_struct_app info_app = H_create_struct_app(
		in_name,
		H_create_version( os_core_version, 0, 0 ),
		"hept",
		H_create_version( 0, 0, 1 ),
		H_create_version( 1, 0, 0 )
	);

	//

	text desired_extensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
#if OS_WINDOWS
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif OS_LINUX
		VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
	};
	constant u32 desired_extensions_count = 2;
	u32 enabled_extensions_count = 0;

	// enumerate_instance_extension_properties();

	ptr( text ) extensions = null;
	u32 extensions_count = H_get_instance_extensions_count();
	if( extensions_count isnt 0 )
	{
		ptr( H_extension_properties ) available_extensions = new_ptr( H_extension_properties, ( extensions_count + 1 ) * 2 );
		H_get_instance_extensions( extensions_count, available_extensions );
		if( available_extensions is null )
		{
#ifdef hept_debug
			print_error( yes, "os_core: memory H_get_instance_extensions() failed for available_extensions" );
#endif
			delete_ptr( available_extensions );
			out null;
		}

		extensions = new_ptr( text, desired_extensions_count );

		iter( desired_extensions_count, i )
		{
			iter( extensions_count, j )
			{
				if( compare_text( desired_extensions[ i ], available_extensions[ j ].extensionName ) )
				{
#ifdef hept_debug
					print_debug( "activated extension: %s", available_extensions[ j ].extensionName );
#endif
					extensions[ enabled_extensions_count++ ] = desired_extensions[ i ];
					skip;
				}
			}
		}

#ifdef hept_debug
		print_error( enabled_extensions_count < desired_extensions_count, "os_core: all extensions not found" );
#endif

		delete_ptr( available_extensions );
	}

	//
	u32 enabled_debug_layers_count = 0;

#ifdef hept_debug
	text desired_debug_layers[] = { "VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor" };
	u32 desired_debug_layers_count = 2;
#else
	text desired_debug_layers[] = {};
	u32 desired_debug_layers_count = 0;
#endif

	ptr( text ) debug_layers = null;
	constant u32 debug_layer_count = H_get_instance_layer_count();
	if( debug_layer_count isnt 0 )
	{
		ptr( H_debug_layer_properties ) available_layers = new_ptr( H_debug_layer_properties, debug_layer_count );
		H_get_instance_layers( debug_layer_count, available_layers );
		if( available_layers is null )
		{
#ifdef hept_debug
			print_error( yes, "os_core: memory H_get_instance_layers() failed for available_layers" );
#endif
			delete_ptr( available_layers );
			out null;
		}

		debug_layers = new_ptr( text, desired_debug_layers_count );
		iter( desired_debug_layers_count, i )
		{
			iter( debug_layer_count, j )
			{
				if( compare_text( desired_debug_layers[ i ], available_layers[ j ].layerName ) )
				{
#ifdef hept_debug
					print_debug( "activated layer: %s", available_layers[ j ].layerName );
#endif
					( debug_layers )[ enabled_debug_layers_count++ ] = desired_debug_layers[ i ];
					skip;
				}
			}
		}

#ifdef hept_debug
		if( enabled_debug_layers_count < desired_debug_layers_count )
			print( "os_core: all layers not found\n" );
#endif

		delete_ptr( available_layers );
	}

	//

	this->instance = H_new_instance(
		ref( info_app ),
		enabled_debug_layers_count,
		debug_layers,
		enabled_extensions_count,
		extensions
	);

#ifdef hept_debug
	if( this->instance is null )
	{
		print_error( yes, "os_core: instance could not be created" );
		delete_ptr( debug_layers );
		out null;
	}
#endif

#ifdef hept_debug
	print_error( this->instance is null, "os_core: instance could not be created" );
#endif

	delete_ptr( debug_layers );
	//
#ifdef hept_trace
	print_trace( "new os_core: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_os_core( in os_core in_os_core )
{
	H_delete_instance( in_os_core->instance );
	unassign_os_core( in_os_core );
}

fn delete_all_os_cores()
{
	if( os_core_pile isnt null )
	{
		iter_pile( os_core_pile, os_core )
		{
			pile_find_iter( os_core_pile, os_core );
			delete_os_core( this_os_core );
		}
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

//

/////// /////// /////// /////// /////// /////// ///////

/// os_window

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
	u32 width, height, display_fps;
	H_surface surface;
	H_surface_capabilities surface_capabilities;
	H_surface_format surface_format;
	H_present_mode present_mode;
	os_window_link link;
)( in text in_name, in u32 in_width, in u32 in_height, fn_ptr( pure, in_call ) )
{
#ifdef hept_debug
	print_error( in_call is null, "os_window: in_call is null" );
	print_error( in_width is 0, "os_window: in_width is 0" );
	print_error( in_height is 0, "os_window: in_height is 0" );
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
	//
#if OS_WINDOWS
	constant DWORD window_style = WS_OVERLAPPEDWINDOW;
	RECT rect = { 0, 0, this->width, this->height };
	AdjustWindowRect( ref( rect ), window_style, no );
	constant s32 this_width = rect.right - rect.left, this_height = rect.bottom - rect.top;

	constant WNDCLASS wc = {
		.lpfnWndProc = process_os_window,
		.hInstance = GetModuleHandle( null ),
		.hbrBackground = CreateSolidBrush( RGB( 0, 0, 0 ) ),
		.lpszClassName = "hept" };

	flag rc = RegisterClass( ref( wc ) );

	#ifdef hept_debug
	print_error( rc is 0, "os_window: cannot create win32 window" );
	#endif
	constant DWORD style = WS_EX_APPWINDOW;
	this->link.hwnd = CreateWindowEx(
		style,
		"hept",
		"hept",
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

	this->link.inst = wc.hInstance;

	SetWindowText( this->link.hwnd, this->name );

#elif OS_LINUX
	this->link.xdis = XOpenDisplay( NULL );

	#ifdef hept_debug
	print_error( this->link.xdis is null, "os_window: cannot open X11 display" );
	#endif

	s32 screen_num = DefaultScreen( this->link.xdis );
	s32 screen_width = DisplayWidth( this->link.xdis, screen_num );
	s32 screen_height = DisplayHeight( this->link.xdis, screen_num );

	Window root_win = RootWindow( this->link.xdis, screen_num );

	this->link.xwin = XCreateSimpleWindow( this->link.xdis, root_win, ( screen_width - this->width ) / 2, ( screen_height - this->height ) / 2, this->width, this->height, 1, BlackPixel( this->link.xdis, screen_num ), BlackPixel( this->link.xdis, screen_num ) );

	#ifdef hept_debug
	print_error( this->link.xwin is 0, "os_window: cannot create X11 window" );
	#endif

	XStoreName( this->link.xdis, this->link.xwin, this->name );

	XSelectInput( this->link.xdis, this->link.xwin, StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask );

#endif

	//

#if OS_WINDOWS
	VkWin32SurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hwnd = this->link.hwnd;
	create_info.hinstance = this->link.inst;
	vkCreateWin32SurfaceKHR( current_os_core->instance, ref( create_info ), null, ref( this->surface ) );
#elif OS_LINUX
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

fn delete_os_window( in os_window in_os_window )
{
	delete_text( in_os_window->name );
#if OS_WINDOWS
	DestroyWindow( in_os_window->link.hwnd );
#elif OS_LINUX
	XDestroyWindow( in_os_window->link.xdis, in_os_window->link.xwin );
#endif
	unassign_os_window( in_os_window );
}

fn delete_all_os_windows()
{
	if( os_window_pile isnt null )
	{
		iter_pile( os_window_pile, os_window )
		{
			pile_find_iter( os_window_pile, os_window );
			delete_os_window( this_os_window );
		}
	}
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

//

/////// /////// /////// /////// /////// /////// ///////

//

/// allocator

make_struct( memory_heap )
{
	u64 size, used;
};

//

/// os_machine

make_object(
	os_machine,
	H_gpu gpu;
	u32 queue_index;
	H_device device;
	H_gpu_memory_properties memory_properties;
	list heaps;
)()
{
	//
	os_machine this = assign_os_machine();
	//
	this->queue_index = u32_max;

	constant u32 gpu_count = H_get_gpu_count( current_os_core->instance );
	ptr( H_gpu ) gpus = new_ptr( H_gpu, gpu_count );
	H_get_gpus( current_os_core->instance, gpu_count, gpus );

	iter( gpu_count, g )
	{
		H_gpu_properties gpu_prop = H_get_gpu_properties( gpus[ g ] );

		if( gpu_prop.deviceType is H_gpu_type_discrete or gpu_prop.deviceType is H_gpu_type_integrated )
		{
			constant u32 queue_count = H_get_gpu_queue_count( gpus[ g ] );
			ptr( H_gpu_queue ) queues = new_ptr( H_gpu_queue, queue_count );
			H_get_gpu_queues( gpus[ g ], queue_count, queues );

			iter( queue_count, q )
			{
				if(
					H_check_gpu_queue_render( queues[ q ] ) and
					H_check_gpu_queue_compute( queues[ q ] ) and
					H_check_gpu_queue_transfer( queues[ q ] ) and
					H_get_gpu_supports_present( gpus[ g ], q, current_os_window->surface )
				)
				{
					this->gpu = gpus[ g ];
					this->queue_index = q;
					this->memory_properties = H_get_gpu_memory_properties( this->gpu );
					this->heaps = new_list( memory_heap );
					iter( this->memory_properties.memoryHeapCount, h )
					{
						list_add(
							this->heaps,
							memory_heap,
							create(
								memory_heap,
								.size = this->memory_properties.memoryHeaps[ h ].size,
								.used = 0
							)
						);
					}
#ifdef hept_debug
					print_debug( "GPU name: %s, QUEUE_INDEX: %d", gpu_prop.deviceName, this->queue_index );
#endif
				}
			}
			delete_ptr( queues );

			if( this->gpu isnt null and gpu_prop.deviceType is H_gpu_type_discrete )
			{
				skip;
			}
		}
	}

	if( this->gpu is null )
	{
#ifdef hept_debug
		print_error( yes, "Could not find a suitable GPU" );
#endif
	}

	delete_ptr( gpus );

	constant f32 queue_priority = 1.0f;
	constant H_struct_device_queue device_queue = H_create_struct_device_queue(
		this->queue_index, 1, ref( queue_priority )
	);

	constant H_gpu_features features = H_get_gpu_features( this->gpu );

	//

	constant text extensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
		// ,VK_EXT_MEMORY_BUDGET_EXTENSION_NAME
	};
	constant u32 extension_count = 1;

	this->device = H_new_device(
		this->gpu,
		1,
		ref( device_queue ),
		extension_count,
		extensions,
		ref( features )
	);

	// this->memory_properties = H_get_gpu_memory_properties(this->gpu);

	//
#ifdef hept_trace
	print_trace( "new os_machine: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_os_machine( in os_machine in_os_machine )
{
	delete_device( in_os_machine->device );
	unassign_os_machine( in_os_machine );
}

fn delete_all_os_machines()
{
	if( os_machine_pile isnt null )
	{
		iter_pile( os_machine_pile, os_machine )
		{
			pile_find_iter( os_machine_pile, os_machine );
			delete_os_machine( this_os_machine );
		}
	}
}

H_result allocate_memory( VkMemoryRequirements* mem_reqs, VkMemoryPropertyFlags properties, H_memory* memory )
{
	iter( current_os_machine->memory_properties.memoryTypeCount, i )
	{
		if( ( mem_reqs->memoryTypeBits & ( 1 << i ) ) && ( current_os_machine->memory_properties.memoryTypes[ i ].propertyFlags & properties ) is properties )
		{
			ptr( memory_heap ) heap = ref(
				list_get( current_os_machine->heaps, memory_heap, current_os_machine->memory_properties.memoryTypes[ i ].heapIndex )
			);
			if( heap->used + mem_reqs->size > heap->size )
			{
				print_error( yes, "FAILED TO ALLOCATE MEMORY" );
			}

			VkMemoryAllocateInfo alloc_info = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.allocationSize = mem_reqs->size,
				.memoryTypeIndex = i };

			H_result result = vkAllocateMemory( current_os_machine->device, &alloc_info, NULL, memory );
			if( result isnt VK_SUCCESS )
			{
				return result;
			}

			heap->used += mem_reqs->size;
			return VK_SUCCESS;
		}
	}

	return VK_ERROR_OUT_OF_DEVICE_MEMORY;
}

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

//

/////// /////// /////// /////// /////// /////// ///////

/// form_buffer

make_object(
	form_buffer,
	H_buffer_usage usage;
	H_memory_properties properties;
)( in H_buffer_usage in_usage, in H_memory_properties in_properties )
{
#ifdef hept_debug
	print_error( in_usage is 0, "form_buffer: in_usage is 0" );
	print_error( in_properties is 0, "form_buffer: in_properties is 0" );
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

fn delete_form_buffer( in form_buffer in_form_buffer )
{
	unassign_form_buffer( in_form_buffer );
}

fn delete_all_form_buffers()
{
	if( form_buffer_pile isnt null )
	{
		iter_pile( form_buffer_pile, form_buffer )
		{
			pile_find_iter( form_buffer_pile, form_buffer );
			delete_form_buffer( this_form_buffer );
		}
	}
}

global form_buffer default_form_buffer_vertex = null;
global form_buffer default_form_buffer_index = null;
global form_buffer default_form_buffer_storage = null;

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

//

/////// /////// /////// /////// /////// /////// ///////

/// form_image

make_enum( image_type ){
	image_type_null,
	image_type_rgba = VK_IMAGE_ASPECT_COLOR_BIT,
	image_type_depth_stencil = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
	image_type_depth = VK_IMAGE_ASPECT_DEPTH_BIT,
	image_type_stencil = VK_IMAGE_ASPECT_STENCIL_BIT,
};

make_object(
	form_image,
	enum( image_type ) type;
	H_format format;
	H_sampler sampler;
)( in enum( image_type ) in_type )
{
#ifdef hept_debug
	print_error( in_type is image_type_null, "form_image: in_type is null" );
#endif
	//
	form_image this = assign_form_image();
	//
	this->type = in_type;

	check( this->type )
	{
		with( image_type_rgba ) this->format = current_os_window->surface_format.format;
		skip;
		with( image_type_depth ) this->format = H_find_depth_format( current_os_machine->gpu );
		skip;
		with( image_type_depth_stencil ) this->format = H_find_depth_stencil_format( current_os_machine->gpu );
		skip;
		with_other skip;
	}

	this->sampler = H_new_sampler(
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
#ifdef hept_debug
	print_error( this->sampler is null, "form_image: could not make sampler" );
#endif
	//
#ifdef hept_trace
	print_trace( "new form_image: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_form_image( in form_image in_form_image )
{
	vkDestroySampler( current_os_machine->device, in_form_image->sampler, null );
	unassign_form_image( in_form_image );
}

fn delete_all_form_images()
{
	if( form_image_pile isnt null )
	{
		iter_pile( form_image_pile, form_image )
		{
			pile_find_iter( form_image_pile, form_image );
			delete_form_image( this_form_image );
		}
	}
}

global form_image default_form_image_rgba = null;
global form_image default_form_image_depth = null;
global form_image default_form_image_stencil = null;
global form_image default_form_image_depth_stencil = null;

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

//

/////// /////// /////// /////// /////// /////// ///////

/// form_frame_layer

make_object(
	form_frame_layer,
	// enum( image_type ) type;
	form_image form;
	H_attachment_reference attach_ref;
	// H_format format;
)( in form_image in_form )
{
#ifdef hept_debug
	print_error( in_form is null, "form_frame_layer: in_form is null" );
#endif
	//
	form_frame_layer this = assign_form_frame_layer();
	//
	this->form = in_form;
	this->attach_ref.attachment = 0;
	this->attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
	check( this->form->type )
	{
		with( image_type_rgba )
		{
			this->attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			skip;
		}

		with( image_type_depth )
		with( image_type_stencil )
		with( image_type_depth_stencil )
		{
			this->attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			skip;
		}

		with_other skip;
	}
	//
#ifdef hept_trace
	print_trace( "new form_frame_layer: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_form_frame_layer( in form_frame_layer in_form_frame_layer )
{
	unassign_form_frame_layer( in_form_frame_layer );
}

fn delete_all_form_frame_layers()
{
	if( form_frame_layer_pile isnt null )
	{
		iter_pile( form_frame_layer_pile, form_frame_layer )
		{
			pile_find_iter( form_frame_layer_pile, form_frame_layer );
			delete_form_frame_layer( this_form_frame_layer );
		}
	}
}

global form_frame_layer default_form_frame_layer_rgba = null;
global form_frame_layer default_form_frame_layer_depth = null;
global form_frame_layer default_form_frame_layer_stencil = null;
global form_frame_layer default_form_frame_layer_depth_stencil = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_frame

make_object(
	form_frame,
	H_render_pass render_pass;
	list layers;
	list attach_ref_rgba_list;
	H_attachment_reference attach_ref_depth_stencil;
)( in list in_layers )
{
#ifdef hept_debug
	print_error( in_layers is null, "form_frame: in_layers is null" );
#endif
	//
	form_frame this = assign_form_frame();
	//
	this->layers = in_layers;
	this->attach_ref_rgba_list = new_list( H_attachment_reference );
	flag has_depth_stencil = no;
	ptr( H_attachment_description ) attachments = new_ptr( H_attachment_description, this->layers->size );
	iter( this->layers->size, l )
	{
		constant form_frame_layer this_layer = list_get( this->layers, form_frame_layer, l );
		this_layer->attach_ref.attachment = l;

		check( this_layer->form->type )
		{
			with( image_type_rgba )
			{
				list_add( this->attach_ref_rgba_list, H_attachment_reference, this_layer->attach_ref );
				skip;
			}
			with( image_type_depth )
				with( image_type_stencil )
					with( image_type_depth_stencil )
			{
				this->attach_ref_depth_stencil = this_layer->attach_ref;
				has_depth_stencil = yes;
				skip;
			}
			with_other skip;
		}

		attachments[ l ].format = this_layer->form->format;
		attachments[ l ].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[ l ].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[ l ].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[ l ].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[ l ].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[ l ].initialLayout = this_layer->attach_ref.layout;
		attachments[ l ].finalLayout = this_layer->attach_ref.layout;
		attachments[ l ].flags = 0;
	}

	H_subpass_description subpass = { 0 };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = this->attach_ref_rgba_list->size;
	subpass.pColorAttachments = to( ptr( H_attachment_reference ), this->attach_ref_rgba_list->data );
	subpass.pDepthStencilAttachment = has_depth_stencil ? ref( this->attach_ref_depth_stencil ) : null;

	this->render_pass = H_new_render_pass(
		this->layers->size,
		attachments,
		1,
		ref( subpass ),
		0,
		null
	);
	//
#ifdef hept_trace
	print_trace( "new form_frame: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_form_frame( in form_frame in_form_frame )
{
	vkDestroyRenderPass( current_os_machine->device, in_form_frame->render_pass, null );
	delete_list( in_form_frame->layers );
	unassign_form_frame( in_form_frame );
}

fn delete_all_form_frames()
{
	if( form_frame_pile isnt null )
	{
		iter_pile( form_frame_pile, form_frame )
		{
			pile_find_iter( form_frame_pile, form_frame );
			delete_form_frame( this_form_frame );
		}
	}
}

global form_frame default_form_frame = null;
global form_frame default_form_frame_depth = null;

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

//

/////// /////// /////// /////// /////// /////// ///////

/// form_renderer

make_object(
	form_renderer,
	H_queue queue;
	H_command_pool command_pool;
)()
{
	form_renderer this = assign_form_renderer();
	//
	this->queue = H_get_queue( current_os_machine->queue_index, 0 );
	this->command_pool = H_new_command_pool( current_os_machine->queue_index );
	//
#ifdef hept_trace
	print_trace( "new form_renderer: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_form_renderer( in form_renderer in_form_renderer )
{
	delete_command_pool( in_form_renderer->command_pool );
	unassign_form_renderer( in_form_renderer );
}

fn delete_all_form_renderers()
{
	if( form_renderer_pile isnt null )
	{
		iter_pile( form_renderer_pile, form_renderer )
		{
			pile_find_iter( form_renderer_pile, form_renderer );
			delete_form_renderer( this_form_renderer );
		}
	}
}

global form_renderer default_form_renderer = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_mesh_attrib

make_object(
	form_mesh_attrib,
	H_format format;
	u32 type_size;
	u32 size;
	text type_glsl;
)( in H_format in_format, in u32 in_type_size, in u32 in_size, in text in_type_glsl )
{
#ifdef hept_debug
	print_error( in_format is 0, "form_mesh_attrib: in_format is 0" );
	print_error( in_type_size is 0, "form_mesh_attrib: in_type_size is 0" );
	print_error( in_size is 0, "form_mesh_attrib: in_size is 0" );
	print_error( in_type_glsl is null, "form_mesh_attrib: in_type_glsl is null" );
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

fn delete_form_mesh_attrib( in form_mesh_attrib in_form_mesh_attrib )
{
	unassign_form_mesh_attrib( in_form_mesh_attrib );
}

fn delete_all_form_mesh_attribs()
{
	if( form_mesh_attrib_pile isnt null )
	{
		iter_pile( form_mesh_attrib_pile, form_mesh_attrib )
		{
			pile_find_iter( form_mesh_attrib_pile, form_mesh_attrib );
			delete_form_mesh_attrib( this_form_mesh_attrib );
		}
	}
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

make_object(
	form_mesh,
	u32 type_size;
	list attribs;
	text layout_glsl;
)( in list in_attribs, in flag generate_glsl )
{
#ifdef hept_debug
	print_error( in_attribs is null, "form_mesh: in_attribs is null" );
#endif
	//
	form_mesh this = assign_form_mesh();
	//
	this->type_size = 0;
	this->attribs = in_attribs;
	this->layout_glsl = new_text( "", 0 );
	//
	iter_list( this->attribs, form_mesh_attrib )
	{
		list_get_iter( this->attribs, form_mesh_attrib );
		this->type_size += this_form_mesh_attrib->type_size * this_form_mesh_attrib->size;
	}
	//
#ifdef hept_trace
	print_trace( "new form_mesh: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_form_mesh( in form_mesh in_form_mesh )
{
	delete_list( in_form_mesh->attribs );
	delete_text( in_form_mesh->layout_glsl );
	unassign_form_mesh( in_form_mesh );
}

fn delete_all_form_meshes()
{
	if( form_mesh_pile isnt null )
	{
		iter_pile( form_mesh_pile, form_mesh )
		{
			pile_find_iter( form_mesh_pile, form_mesh );
			delete_form_mesh( this_form_mesh );
		}
	}
}

global form_mesh default_form_mesh_2d_tri = null;
global form_mesh default_form_mesh_2d_tri_tex = null;
global form_mesh default_form_mesh_2d_line = null;
global form_mesh default_form_mesh_3d_tri = null;
global form_mesh default_form_mesh_3d_tri_tex = null;
global form_mesh default_form_mesh_3d_line = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_shader_stage

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
	print_error( in_type is shader_stage_type_null, "form_shader_stage: in_type is null" );
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

fn delete_form_shader_stage( in form_shader_stage in_form_shader_stage )
{
	unassign_form_shader_stage( in_form_shader_stage );
}

fn delete_all_form_shader_stages()
{
	if( form_shader_stage_pile isnt null )
	{
		iter_pile( form_shader_stage_pile, form_shader_stage )
		{
			pile_find_iter( form_shader_stage_pile, form_shader_stage );
			delete_form_shader_stage( this_form_shader_stage );
		}
	}
}

global form_shader_stage default_form_shader_stage_vert = null;
global form_shader_stage default_form_shader_stage_geom = null;
global form_shader_stage default_form_shader_stage_frag = null;
global form_shader_stage default_form_shader_stage_comp = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_shader_module

make_object(
	form_shader_module,
	form_shader_stage shader_stage;
	form_mesh mesh_form;
)( in form_shader_stage in_shader_stage, in form_mesh in_mesh_form )
{
#ifdef hept_debug
	print_error( in_shader_stage is null, "form_shader_module: in_shader_stage is null" );
	print_error( in_mesh_form is null, "form_shader_module: in_mesh_form is null" );
#endif
	//
	form_shader_module this = assign_form_shader_module();
	//
	this->shader_stage = in_shader_stage;
	this->mesh_form = in_mesh_form;
//
#ifdef hept_trace
	print_trace( "new form_shader_module: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_form_shader_module( in form_shader_module in_form_shader_module )
{
	unassign_form_shader_module( in_form_shader_module );
}

fn delete_all_form_shader_modules()
{
	if( form_shader_module_pile isnt null )
	{
		iter_pile( form_shader_module_pile, form_shader_module )
		{
			pile_find_iter( form_shader_module_pile, form_shader_module );
			delete_form_shader_module( this_form_shader_module );
		}
	}
}

global form_shader_module default_form_shader_module_2d_tri_vert = null;
global form_shader_module default_form_shader_module_2d_tri_frag = null;
global form_shader_module default_form_shader_module_2d_tri_tex_vert = null;
global form_shader_module default_form_shader_module_2d_tri_tex_frag = null;
global form_shader_module default_form_shader_module_2d_line_vert = null;
global form_shader_module default_form_shader_module_2d_line_frag = null;
global form_shader_module default_form_shader_module_3d_tri_vert = null;
global form_shader_module default_form_shader_module_3d_tri_frag = null;
global form_shader_module default_form_shader_module_3d_tri_tex_vert = null;
global form_shader_module default_form_shader_module_3d_tri_tex_frag = null;
global form_shader_module default_form_shader_module_3d_line_vert = null;
global form_shader_module default_form_shader_module_3d_line_frag = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_shader_binding

make_enum( shader_binding_type ){
	shader_binding_type_null,
	shader_binding_type_image = H_descriptor_type_image,
	shader_binding_type_storage = H_descriptor_type_storage,
};

make_object(
	form_shader_binding,
	enum( shader_binding_type ) type;
	enum( shader_stage_type ) stage_type;
)( in enum( shader_binding_type ) in_type, in enum( shader_stage_type ) in_stage )
{
#ifdef hept_debug
	print_error( in_type is shader_binding_type_null, "form_shader_binding: in_type is null" );
#endif
	//
	form_shader_binding this = assign_form_shader_binding();
	//
	this->type = in_type;
	this->stage_type = in_stage;
//
#ifdef hept_trace
	print_trace( "new form_shader_binding: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_form_shader_binding( in form_shader_binding in_form_shader_binding )
{
	unassign_form_shader_binding( in_form_shader_binding );
}

fn delete_all_form_shader_bindings()
{
	if( form_shader_binding_pile isnt null )
	{
		iter_pile( form_shader_binding_pile, form_shader_binding )
		{
			pile_find_iter( form_shader_binding_pile, form_shader_binding );
			delete_form_shader_binding( this_form_shader_binding );
		}
	}
}

global form_shader_binding default_form_shader_binding_image = null;
global form_shader_binding default_form_shader_binding_storage_vert = null;
global form_shader_binding default_form_shader_binding_storage_frag = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_shader

make_object(
	form_shader,
	H_topology topology;
	H_descriptor_layout descriptor_layout;
	list bindings;
)( in H_topology in_topology, in list in_bindings )
{
	form_shader this = assign_form_shader();
	//
	this->topology = in_topology;
	this->bindings = in_bindings;
	list bindings = new_list( H_descriptor_layout_binding );

	if( this->bindings isnt null )
	{
		iter_list( this->bindings, form_shader_binding )
		{
			list_get_iter( this->bindings, form_shader_binding );
			list_add(
				bindings,
				H_descriptor_layout_binding,
				( ( H_descriptor_layout_binding ){
					.binding = _iter_form_shader_binding,
					.descriptorType = to( VkDescriptorType, this_form_shader_binding->type ),
					.descriptorCount = 1,
					.stageFlags = this_form_shader_binding->stage_type |
						VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
					.pImmutableSamplers = null,
				} )
			);
		}
	}

	H_struct_descriptor_layout layout_info = H_create_struct_descriptor_layout(
		bindings->size, ( const ptr( VkDescriptorSetLayoutBinding ) )bindings->data
	);
	this->descriptor_layout = H_new_descriptor_layout( layout_info );
//
#ifdef hept_trace
	print_trace( "new form_shader: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_form_shader( in form_shader in_form_shader )
{
	if( in_form_shader->descriptor_layout isnt null )
		vkDestroyDescriptorSetLayout( current_os_machine->device, in_form_shader->descriptor_layout, null );
	delete_list( in_form_shader->bindings );
	unassign_form_shader( in_form_shader );
}

fn delete_all_form_shaders()
{
	if( form_shader_pile isnt null )
	{
		iter_pile( form_shader_pile, form_shader )
		{
			pile_find_iter( form_shader_pile, form_shader );
			delete_form_shader( this_form_shader );
		}
	}
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

//

/////// /////// /////// /////// /////// /////// ///////

/// buffer

make_object(
	buffer,
	form_buffer form;
	u64 size;
	s32 ticket_id;
	H_buffer buffer;
	H_memory memory;
)( in form_buffer in_form )
{
#ifdef hept_debug
	print_error( in_form is null, "buffer: in_form is null" );
#endif
	//
	buffer this = assign_buffer();
	//
	this->form = in_form;
	this->size = 0;
	this->ticket_id = -1;
	//
#ifdef hept_trace
	print_trace( "new buffer: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_buffer( in buffer in_buffer )
{
	vkFreeMemory( current_os_machine->device, in_buffer->memory, null );
	vkDestroyBuffer( current_os_machine->device, in_buffer->buffer, null );
	unassign_buffer( in_buffer );
}

fn delete_all_buffers()
{
	if( buffer_pile isnt null )
	{
		iter_pile( buffer_pile, buffer )
		{
			pile_find_iter( buffer_pile, buffer );
			delete_buffer( this_buffer );
		}
	}
}

make_struct( buffer_ticket )
{
	buffer buffer;
	u64 size;
	ptr( pure ) data;
};
global pile buffer_ticket_pile = null;

make_struct( buffer_delete_ticket )
{
	H_buffer buffer;
	H_memory memory;
	u8 count;
};
global list buffer_delete_tickets = null;

inl flag update_buffer( in buffer in_buffer, in u64 in_update_size, ptr( pure ) in_data )
{
	if( in_update_size is 0 ) out no;

	if( in_buffer->ticket_id is -1 )
	{
		pile_add(
			buffer_ticket_pile,
			buffer_ticket,
			create(
				buffer_ticket,
				in_buffer,
				in_update_size,
				in_data
			)
		);
		in_buffer->ticket_id = buffer_ticket_pile->prev_pos;
	}
	else
	{
		ptr( buffer_ticket ) ticket_ptr = ref(
			pile_find( buffer_ticket_pile, buffer_ticket, in_buffer->ticket_id )
		);
		ticket_ptr->size = in_update_size;
		ticket_ptr->data = in_data;
	}

	u64 temp_size = in_buffer->size;
	out in_update_size > temp_size;
}

fn process_buffer_ticket( ptr( buffer_ticket ) in_ticket )
{
	buffer this_buffer = in_ticket->buffer;
	if( in_ticket->size > this_buffer->size )
	{
		if( this_buffer->buffer isnt null )
			list_add(
				buffer_delete_tickets,
				buffer_delete_ticket,
				create(
					buffer_delete_ticket,
					this_buffer->buffer,
					this_buffer->memory,
					0
				)
			);

		this_buffer->size = in_ticket->size;
		H_struct_buffer buffer_info = H_create_struct_buffer(
			this_buffer->size,
			this_buffer->form->usage,
			H_sharing_mode_exclusive,
			0,
			null
		);
		this_buffer->buffer = H_new_buffer( buffer_info );

		H_memory_requirements mem_requirements = H_get_memory_requirements_buffer( this_buffer->buffer );
		/*H_struct_memory memory_info = H_create_struct_memory(
			mem_requirements.size,
			H_find_mem(
				H_get_gpu_memory_properties( current_os_machine->gpu ),
				mem_requirements.memoryTypeBits,
				this_buffer->form->properties
			)
		);
		this_buffer->memory = H_new_memory_buffer( memory_info, this_buffer->buffer );*/
		allocate_memory(ref(mem_requirements),H_memory_property_device_local | H_memory_property_host_visible | H_memory_property_host_coherent, ref(this_buffer->memory));

		H_CHECK(vkBindBufferMemory( H_current_device, this_buffer->buffer, this_buffer->memory, 0 ));
	}

	//

	if( in_ticket->data isnt null )
	{
		once ptr( pure ) mapped = null;
		vkMapMemory( current_os_machine->device, this_buffer->memory, 0, in_ticket->size, 0, ref( mapped ) );
		copy_ptr( mapped, in_ticket->data, in_ticket->size );
		vkUnmapMemory( current_os_machine->device, this_buffer->memory );
		in_ticket->data = null;
	}

	pile_delete( buffer_ticket_pile, buffer_ticket, this_buffer->ticket_id );
	this_buffer->ticket_id = -1;
}

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

//

/////// /////// /////// /////// /////// /////// ///////

/// image

make_enum( image_state ){
	image_state_null,
	image_state_cpu,
	image_state_gpu,
	image_state_swap,
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
	list pixel_list;
)( in form_image in_form, in enum( image_state ) in_state, in u32 in_width, in u32 in_height )
{
#ifdef hept_debug
	print_error( in_form is null, "image: in_form is null" );
	print_error( in_state is image_state_null, "image: in_state is null" );
	print_error( in_width is 0, "image: in_width is 0" );
	print_error( in_height is 0, "image: in_height is 0" );
#endif
	//
	image this = assign_image();
	//
	this->form = in_form;
	this->state = in_state;
	this->layout = H_image_layout_undefined;
	this->width = in_width;
	this->height = in_height;
	this->pixel_list = new_list( rgba );

	//
	constant H_extent_3d temp_extent = {
		.width = this->width,
		.height = this->height,
		.depth = 1 };

	this->image = H_new_image(
		H_image_type_2d,
		this->form->format,
		temp_extent,
		1,
		1,
		H_image_sample_1,
		H_image_tiling_optimal,
		( ( this->form->type is image_type_rgba )
				? ( H_image_usage_sampled | H_image_usage_color_attachment )
				: ( H_image_usage_depth_stencil_attachment ) ) |
			H_image_usage_transfer_src | H_image_usage_transfer_dst,
		H_sharing_mode_exclusive,
		0,
		null,
		this->layout
	);

	H_memory_requirements mem_reqs = H_get_memory_requirements_image( this->image );

	H_result result = allocate_memory( &mem_reqs, H_memory_property_device_local, &this->memory );
	if( result isnt VK_SUCCESS )
	{
		print_error( yes, "MEMORY ALLOC FAILURE" );
	}

	result = vkBindImageMemory( current_os_machine->device, this->image, this->memory, 0 );
	if( result isnt VK_SUCCESS )
	{
		vkDestroyImage( current_os_machine->device, this->image, NULL );
		print_error( yes, "MEMORY BIND FAILURE" );
	}

	//
#ifdef hept_trace
	print_trace( "new image: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_image( in image in_image )
{
	if( in_image->memory isnt null ) vkFreeMemory( current_os_machine->device, in_image->memory, null );
	if( in_image->view isnt null ) vkDestroyImageView( current_os_machine->device, in_image->view, null );
	/*if (in_image->image isnt null and in_image->state isnt image_state_swap)
			vkDestroyImage(current_os_machine->device, in_image->image, null);*/
	delete_list( in_image->pixel_list );
	unassign_image( in_image );
}

fn delete_all_images()
{
	if( image_pile isnt null )
	{
		iter_pile( image_pile, image )
		{
			pile_find_iter( image_pile, image );
			delete_image( this_image );
		}
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

//

/////// /////// /////// /////// /////// /////// ///////

/// frame

make_object(
	frame,
	form_frame form;
	H_frame frame;
	list images;
	list views;
	u32 max_w;
	u32 max_h;
	H_struct_begin_render_pass info_begin;
	VkClearValue clear_col[ 2 ];
	H_semaphore ready;
	H_semaphore done;
	H_fence wait;
)( in form_frame in_form, in list in_images )
{
#ifdef hept_debug
	print_error( in_form is null, "frame: in_form is null" );
	print_error( in_images is null, "frame: in_images is null" );
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
		H_struct_image_view image_view_info = H_create_struct_image_view(
			temp_image->image,
			H_image_view_type_2d,
			temp_image->form->format,
			( ( H_component_mapping ){
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY } ),
			( ( H_image_subresource_range ){
				list_get( this->form->layers, form_frame_layer, v )->form->type,
				0,
				1,
				0,
				1 } )
		);
		if( temp_image->view is null )
			temp_image->view = H_new_image_view( image_view_info );
		list_add( this->views, H_image_view, temp_image->view );
	}

	H_struct_frame frame_info = H_create_struct_frame(
		this->form->render_pass,
		this->views->size,
		( ptr( H_image_view ) )( this->views->data ),
		this->max_w,
		this->max_h,
		1
	);
	this->frame = H_new_frame( frame_info );

	this->clear_col[ 0 ] = ( VkClearValue ){ 1., 1., 1., 1. };
	this->clear_col[ 1 ] = ( VkClearValue ){ 1., 1., 1., 1. };
	this->info_begin = H_create_struct_begin_render_pass(
		this->form->render_pass,
		this->frame,
		( ( H_rect_2d ){ 0, 0, this->max_w, this->max_h } ),
		this->form->layers->size,
		ref( this->clear_col )
	);

	this->ready = H_new_semaphore();
	this->done = H_new_semaphore();
	this->wait = H_new_fence();

	//
#ifdef hept_trace
	print_trace( "new frame: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_frame( in frame in_frame )
{
	vkDestroyFramebuffer( current_os_machine->device, in_frame->frame, null );
	vkDestroySemaphore( current_os_machine->device, in_frame->ready, null );
	vkDestroySemaphore( current_os_machine->device, in_frame->done, null );
	vkDestroyFence( current_os_machine->device, in_frame->wait, null );
	delete_list( in_frame->images );
	delete_list( in_frame->views );
	unassign_frame( in_frame );
}

fn delete_all_frames()
{
	if( frame_pile isnt null )
	{
		iter_pile( frame_pile, frame )
		{
			pile_find_iter( frame_pile, frame );
			delete_frame( this_frame );
		}
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

//

/////// /////// /////// /////// /////// /////// ///////

/// renderer

global H_command_buffer current_command_buffer = null;

make_object(
	renderer,
	form_renderer form;
	// os_window window;
	H_viewport viewport;
	//form_image form_image_swapchain;
	H_swapchain swapchain;
	u32 current_frame;
	//form_frame form_frame_window;
	frame frame_window;
	list frame_present_list;
	ptr( H_command_buffer ) command_buffers;
	list recycled_semaphores;
	// H_surface_format window_surface_format;
	// H_surface_capabilities window_surface_capabilities;
	// H_present_mode window_present_mode;
)( in form_renderer in_form )
{
#ifdef hept_debug
	print_error( in_form is null, "renderer: in_form is null" );
		// print_error(in_window is null, "renderer: in_window is null");
#endif
	//
	renderer this = assign_renderer();
	//
	this->form = in_form;
	this->swapchain = null;
	this->current_frame = 0;
	this->frame_window = null;
	this->frame_present_list = new_list( frame );
	this->recycled_semaphores = new_list( H_semaphore );
	//

	//
#ifdef hept_trace
	print_trace( "new renderer: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn clear_renderer( in renderer in_renderer )
{
	vkDeviceWaitIdle( current_os_machine->device );
	if( in_renderer->swapchain isnt null ) vkDestroySwapchainKHR( current_os_machine->device, in_renderer->swapchain, null );
	iter_list( in_renderer->frame_present_list, frame )
	{
		list_get_iter( in_renderer->frame_present_list, frame );
		iter_list( this_frame->images, image )
		{
			list_get_iter( this_frame->images, image );
			delete_image( this_image );
		}
		delete_frame( this_frame );
	}
	empty_list( in_renderer->frame_present_list );
}

fn delete_renderer( in renderer in_renderer )
{
	clear_renderer( in_renderer );
	unassign_renderer( in_renderer );
}

fn delete_all_renderers()
{
	if( renderer_pile isnt null )
	{
		iter_pile( renderer_pile, renderer )
		{
			pile_find_iter( renderer_pile, renderer );
			delete_renderer( this_renderer );
		}
	}
}

fn refresh_renderer( in renderer in_renderer )
{

	clear_renderer(in_renderer);

	current_os_window->surface_capabilities = H_get_surface_capabilities( current_os_machine->gpu, current_os_window->surface );

	u32 image_count_max = current_os_window->surface_capabilities.maxImageCount;
	u32 image_count_min = current_os_window->surface_capabilities.minImageCount + 1;
	if( ( image_count_max > 0 ) and ( image_count_min > image_count_max ) )
	{
		image_count_min = image_count_max;
	}

	current_os_window->width = current_os_window->surface_capabilities.currentExtent.width;
	current_os_window->height = current_os_window->surface_capabilities.currentExtent.height;

	H_struct_swapchain swapchain_info = H_create_struct_swapchain(
		current_os_window->surface,
		image_count_min,
		current_os_window->surface_format.format,
		current_os_window->surface_format.colorSpace,
		current_os_window->surface_capabilities.currentExtent,
		1,
		H_image_usage_transfer_src | H_image_usage_transfer_dst | H_image_usage_color_attachment,
		H_sharing_mode_exclusive,
		0,
		null,
		current_os_window->surface_capabilities.currentTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		current_os_window->present_mode,
		yes,
		null
	);

	in_renderer->swapchain = H_new_swapchain( swapchain_info );

	in_renderer->viewport = H_create_viewport(
		0.0,
		0.0,
		to_f32( current_os_window->width ),
		to_f32( current_os_window->height ),
		0.0,
		1.0
	);

	u32 image_count = H_get_swapchain_images( in_renderer->swapchain, null );
	ptr( H_image ) temp_images = new_ptr( H_image, image_count );
	H_get_swapchain_images( in_renderer->swapchain, temp_images );

	/*if( in_renderer->form_frame_window is null )
	{
		list layers = new_list( form_frame_layer );
		form_frame_layer layer_rgba = new_form_frame_layer( default_form_image_rgba );

		list_add( layers, form_frame_layer, layer_rgba );

		in_renderer->form_frame_window = new_form_frame(
			layers
		);
	}*/

	//if( in_renderer->form_image_swapchain is null ) in_renderer->form_image_swapchain = new_form_image( image_type_rgba );

	iter( image_count, i )
	{
		list temp_list_images = new_list( image );
		image temp_image = assign_image();
		temp_image->form = default_form_image_rgba;
		temp_image->image = temp_images[ i ];
		temp_image->width = current_os_window->width;
		temp_image->height = current_os_window->height;
		temp_image->state = image_state_swap;
		list_add( temp_list_images, image, temp_image );
		frame temp_frame = new_frame( default_form_frame, temp_list_images );
		list_add( in_renderer->frame_present_list, frame, temp_frame );
	}

	delete_ptr( temp_images );

	//

	//if(in_renderer->frame_window isnt null) out;

		if (in_renderer->command_buffers is null)
		{
			in_renderer->command_buffers = new_ptr(H_command_buffer, image_count);
			H_struct_command_buffer command_buffers_info = H_create_struct_command_buffer(
				in_renderer->form->command_pool,
				H_command_buffer_level_primary,
				in_renderer->frame_present_list->size
			);

			H_new_command_buffers(command_buffers_info, in_renderer->command_buffers);
		}

		//
		if(in_renderer->frame_window == null)
		{
			list frame_window_images = new_list(image);

			image temp_image = new_image(
				default_form_image_rgba,
				image_state_gpu,
				main_width,
				main_height
			);

			image depth_image = new_image(
				default_form_image_depth_stencil,
				image_state_gpu,
				main_width,
				main_height
			);

			list_add(frame_window_images, image, temp_image);
			list_add(frame_window_images, image, depth_image);
			in_renderer->frame_window = new_frame(default_form_frame_depth, frame_window_images);
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

//

/////// /////// /////// /////// /////// /////// ///////

/// mesh

make_struct( vertex_2d_tri )
{
	fvec2 pos;
	fvec3 rgb;
};
#define create_vertex_2d_tri( x, y, r, g, b ) create( vertex_2d_tri, .pos = { x, y }, .rgb = { r, g, b } )

make_struct( vertex_2d_tri_tex )
{
	fvec2 pos;
	fvec2 uv;
	fvec3 rgb;
};
#define create_vertex_2d_tri_tex( x, y, u, v, r, g, b ) create( vertex_2d_tri_tex, .pos = { x, y }, .uv = { u, v }, .rgb = { r, g, b } )

make_struct( vertex_2d_line )
{
	fvec2 pos;
	fvec3 rgb;
};
#define create_vertex_2d_line( x, y, r, g, b ) create( vertex_2d_line, .pos = { x, y }, .rgb = { r, g, b } )

make_struct( vertex_3d_tri )
{
	fvec3 pos;
	fvec3 rgb;
};
#define create_vertex_3d_tri( x, y, z, r, g, b ) create( vertex_3d_tri, .pos = { x, y, z }, .rgb = { r, g, b } )

make_struct( vertex_3d_tri_tex )
{
	fvec3 pos;
	fvec2 uv;
	fvec3 rgb;
};
#define create_vertex_3d_tri_tex( x, y, z, u, v, r, g, b ) create( vertex_3d_tri_tex, .pos = { x, y, z }, .uv = { u, v }, .rgb = { r, g, b } )

make_struct( vertex_3d_line )
{
	fvec3 pos;
	fvec3 rgb;
};
#define create_vertex_3d_line( x, y, z, r, g, b ) create( vertex_3d_line, .pos = { x, y, z }, .rgb = { r, g, b } )

//

make_object(
	mesh,
	form_mesh form;
	flag update;
	u32 update_id;
	buffer vertex_buffer;
	list vertex_list;
)( in form_mesh in_form )
{
#ifdef hept_debug
	print_error( in_form is null, "mesh: in_form is null" );
#endif
	//
	mesh this = assign_mesh();
	//
	this->form = in_form;
	this->update = yes;
	this->vertex_list = assign_list( 0, 1, this->form->type_size, assign_ptr( this->form->type_size ) );
	this->vertex_buffer = new_buffer( default_form_buffer_vertex );
	//
#ifdef hept_trace
	print_trace( "new mesh: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_mesh( in mesh in_mesh )
{
	delete_list( in_mesh->vertex_list );
	unassign_mesh( in_mesh );
}

fn delete_all_meshes()
{
	if( mesh_pile isnt null )
	{
		iter_pile( mesh_pile, mesh )
		{
			pile_find_iter( mesh_pile, mesh );
			delete_mesh( this_mesh );
		}
	}
}

global mesh default_mesh_line = null;
global mesh default_mesh_square = null;
global mesh default_mesh_square_tex = null;
global mesh default_mesh_window_white = null;
global mesh default_mesh_window_black = null;
global mesh default_mesh_window_tex = null;

#define mesh_add_point( var, vertex_struct, p )       \
	DEF_START                                           \
	list_add( ( var )->vertex_list, vertex_struct, p ); \
	DEF_END

#define mesh_add_line( var, vertex_struct, a, b ) \
	DEF_START                                       \
	mesh_add_point( ( var ), vertex_struct, a );    \
	mesh_add_point( ( var ), vertex_struct, b );    \
	DEF_END

#define mesh_add_tri( var, vertex_struct, a, b, c ) \
	DEF_START                                         \
	mesh_add_point( ( var ), vertex_struct, a );      \
	mesh_add_point( ( var ), vertex_struct, b );      \
	mesh_add_point( ( var ), vertex_struct, c );      \
	DEF_END

#define mesh_add_quad( var, vertex_struct, tl, tr, br, bl ) \
	DEF_START                                                 \
	mesh_add_tri( ( var ), vertex_struct, tl, tr, br );       \
	mesh_add_tri( ( var ), vertex_struct, tl, br, bl );       \
	DEF_END

fn update_mesh( in mesh in_mesh )
{
	update_buffer( in_mesh->vertex_buffer, in_mesh->vertex_list->size * in_mesh->vertex_list->size_type, in_mesh->vertex_list->data );
}

//

/////// /////// /////// /////// /////// /////// ///////

// shader_module
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
	shader_module,
	form_shader_module form;
	text path;
	H_shader_module shader_module;
	os_file file;
	H_struct_pipeline_shader_stage stage_info;
)( in form_shader_module in_form, in text in_path, in text in_name )
{
#ifdef hept_debug
	print_error( in_form is null, "shader_module: in_form is null" );
	print_error( in_path is null, "shader_module: in_path is null" );
	print_error( in_name is null, "shader_module: in_name is null" );
#endif
	//
	shader_module this = assign_shader_module();
	//
	this->form = in_form;

	text temp_path = new_text( in_path, text_length( in_name ) + 1 );
	join_text( temp_path, folder_sep );
	join_text( temp_path, in_name );

	text glsl_name = new_text( temp_path, 5 );
	join_text( glsl_name, ( ( this->form->shader_stage->type is shader_stage_type_vertex ) ? ( ".vert" ) : ( ".frag" ) ) );
	delete_text( temp_path );

	text spirv_name = new_text( glsl_name, 4 );
	join_text( spirv_name, ".spv" );

	//

#ifndef hept_release
	ifn( check_file( glsl_name ) )
	{
		write_file( glsl_name, ( ( this->form->shader_stage->type is shader_stage_type_vertex ) ? ( default_glsl_vert ) : ( default_glsl_frag ) ) );
	}
#endif

	if( check_file( glsl_name ) )
	{
		text command = format_text( "glslangValidator -V \"%s\" -o \"%s\"", glsl_name, spirv_name );
		print( "COMPILING with command '%s'\n", command );
		s32 sys_result = system( command );
#ifdef hept_debug
		print_error( sys_result isnt 0, "failed to compile GLSL to SPIR-V\n" );
#endif
	}

	this->file = new_os_file( spirv_name );

	H_struct_shader_module module_info = H_create_struct_shader_module( this->file->size, to( ptr( u32 ), this->file->data ) );
	this->shader_module = H_new_shader_module( module_info );
	this->stage_info = H_create_struct_pipeline_shader_stage(
		to( VkShaderStageFlagBits, this->form->shader_stage->type ),
		this->shader_module,
		"main",
		null // special constants
	);
//
#ifdef hept_trace
	print_trace( "new shader_module: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_module( in shader_module in_module )
{
	vkDestroyShaderModule( current_os_machine->device, in_module->shader_module, null );
	unassign_shader_module( in_module );
}

fn delete_all_modules()
{
	if( shader_module_pile isnt null )
	{
		iter_pile( shader_module_pile, shader_module )
		{
			pile_find_iter( shader_module_pile, shader_module );
			delete_module( this_shader_module );
		}
	}
}

global shader_module default_shader_module_2d_tri_vert = null;
global shader_module default_shader_module_2d_tri_frag = null;
global shader_module default_shader_module_2d_tri_tex_vert = null;
global shader_module default_shader_module_2d_tri_tex_frag = null;
global shader_module default_shader_module_2d_line_vert = null;
global shader_module default_shader_module_2d_line_frag = null;
// global shader_module default_module_3d_tri_vert = null;
// global shader_module default_module_3d_tri_frag = null;
// global shader_module default_module_3d_tri_tex_vert = null;
// global shader_module default_module_3d_tri_tex_frag = null;
// global shader_module default_module_3d_line_vert = null;
// global shader_module default_module_3d_line_frag = null;

//

/////// /////// /////// /////// /////// /////// ///////

// shader_input
// -------
// data input for shaders

make_object(
	shader_input,
	H_descriptor_pool descriptor_pool;
	H_descriptor descriptors[ 3 ];
	flag descriptor_updates[ 3 ];
	s32 ticket_storage_id;
	s32 ticket_image_id;
)( in form_shader in_form_shader )
{
#ifdef hept_debug
	print_error( in_form_shader is null, "shader_input: in_form_shader is null" );
#endif
	//
	shader_input this = assign_shader_input();
	//

	list sizes = new_list( H_descriptor_pool_size );
	iter_list( in_form_shader->bindings, form_shader_binding )
	{
		list_get_iter( in_form_shader->bindings, form_shader_binding );
		list_add( sizes, H_descriptor_pool_size, create( H_descriptor_pool_size, .type = to( H_descriptor_type, this_form_shader_binding->type ), .descriptorCount = 3 ) );
	}

	H_struct_descriptor_pool pool_info = H_create_struct_descriptor_pool( 3, sizes->size, ( const ptr( VkDescriptorPoolSize ) )sizes->data );
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	this->descriptor_pool = H_new_descriptor_pool( pool_info );

	H_struct_descriptor alloc_info = H_create_struct_descriptor(
		this->descriptor_pool,
		1,
		ref( in_form_shader->descriptor_layout )
	);

	this->descriptors[ 0 ] = H_new_descriptor( alloc_info );
	this->descriptors[ 1 ] = H_new_descriptor( alloc_info );
	this->descriptors[ 2 ] = H_new_descriptor( alloc_info );
	this->descriptor_updates[ 0 ] = yes;
	this->descriptor_updates[ 1 ] = yes;
	this->descriptor_updates[ 2 ] = yes;

	this->ticket_storage_id = -1;
	this->ticket_image_id = -1;

//
#ifdef hept_trace
	print_trace( "new shader_input: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_shader_input( in shader_input in_shader_input )
{
	vkFreeDescriptorSets( current_os_machine->device, in_shader_input->descriptor_pool, 3, in_shader_input->descriptors );
	vkDestroyDescriptorPool( current_os_machine->device, in_shader_input->descriptor_pool, null );
	unassign_shader_input( in_shader_input );
}

fn delete_all_shader_inputs()
{
	if( shader_input_pile isnt null )
	{
		iter_pile( shader_input_pile, shader_input )
		{
			pile_find_iter( shader_input_pile, shader_input );
			delete_shader_input( this_shader_input );
		}
	}
}

global pile shader_input_storage_ticket_pile = null;
global pile shader_input_image_ticket_pile = null;

make_struct( shader_input_storage_ticket )
{
	shader_input input;
	u8 binding;
	buffer buffer;
	u8 frame;
};

make_struct( shader_input_image_ticket )
{
	shader_input input;
	u8 binding;
	image image;
	u8 frame;
};

fn update_shader_input_storage( in shader_input in_shader_input, in u8 in_binding, in buffer in_buffer )
{
	// lock_pile(shader_input_storage_tickets);
	// lock_shader_input(in_shader_input);
	in_shader_input->descriptor_updates[ 0 ] = yes;
	in_shader_input->descriptor_updates[ 1 ] = yes;
	in_shader_input->descriptor_updates[ 2 ] = yes;
	if( in_shader_input->ticket_storage_id is -1 )
	{
		pile_add(
			shader_input_storage_ticket_pile,
			shader_input_storage_ticket,
			create(
				shader_input_storage_ticket,
				in_shader_input,
				in_binding,
				in_buffer,
				0
			)
		);
		in_shader_input->ticket_storage_id = shader_input_storage_ticket_pile->prev_pos;
	}
	else
	{
		ptr( shader_input_storage_ticket ) ticket_ptr = ref( pile_find(
			shader_input_storage_ticket_pile, shader_input_storage_ticket, in_shader_input->ticket_storage_id
		) );
		ticket_ptr->binding = in_binding;
		ticket_ptr->buffer = in_buffer;
	}
	// unlock_shader_input(in_shader_input);
	// unlock_pile(shader_input_storage_tickets);
}

fn process_shader_input_storage_ticket( ptr( shader_input_storage_ticket ) in_ticket )
{
	shader_input this_shader_input = in_ticket->input;

	// lock_shader_input(this_shader_input);

	if(
		( this_shader_input->descriptor_updates[ 0 ] is no ) and
		( this_shader_input->descriptor_updates[ 1 ] is no ) and
		( this_shader_input->descriptor_updates[ 2 ] is no )
	)
	{
		pile_delete( shader_input_storage_ticket_pile, shader_input_storage_ticket, this_shader_input->ticket_storage_id );
		this_shader_input->ticket_storage_id = -1;
		// unlock_shader_input(this_shader_input);
		out;
	}

	ifn( this_shader_input->descriptor_updates[ in_ticket->frame ] )
	{
		// unlock_shader_input(this_shader_input);
		out;
	}
	if( in_ticket->buffer is null or in_ticket->buffer->buffer is null )
	{
		// unlock_shader_input(this_shader_input);
		out;
	}

	H_update_descriptor_set_storage( in_ticket->binding, this_shader_input->descriptors[ in_ticket->frame ], in_ticket->buffer->buffer, in_ticket->buffer->size );
	this_shader_input->descriptor_updates[ in_ticket->frame ] = no;
	// unlock_shader_input(this_shader_input);
}

fn update_shader_input_image( in shader_input in_shader_input, in u8 in_binding, in image in_image )
{
	// lock_pile(shader_input_image_tickets);
	// lock_shader_input(in_shader_input);
	in_shader_input->descriptor_updates[ 0 ] = yes;
	in_shader_input->descriptor_updates[ 1 ] = yes;
	in_shader_input->descriptor_updates[ 2 ] = yes;
	if( in_shader_input->ticket_image_id is -1 )
	{
		pile_add(
			shader_input_image_ticket_pile,
			shader_input_image_ticket,
			create(
				shader_input_image_ticket,
				in_shader_input,
				in_binding,
				in_image,
				0
			)
		);
		in_shader_input->ticket_image_id = shader_input_image_ticket_pile->prev_pos;
	}
	else
	{
		ptr( shader_input_image_ticket ) ticket_ptr = ref( pile_find(
			shader_input_image_ticket_pile, shader_input_image_ticket, in_shader_input->ticket_image_id
		) );
		ticket_ptr->binding = in_binding;
		ticket_ptr->image = in_image;
	}
	// unlock_shader_input(in_shader_input);
	// unlock_pile(shader_input_image_tickets);
}

fn process_shader_input_image_ticket( ptr( shader_input_image_ticket ) in_ticket )
{
	shader_input this_shader_input = in_ticket->input;

	// lock_shader_input(this_shader_input);

	if(
		( this_shader_input->descriptor_updates[ 0 ] is no ) and
		( this_shader_input->descriptor_updates[ 1 ] is no ) and
		( this_shader_input->descriptor_updates[ 2 ] is no )
	)
	{
		pile_delete( shader_input_image_ticket_pile, shader_input_image_ticket, this_shader_input->ticket_image_id );
		this_shader_input->ticket_image_id = -1;
		// unlock_shader_input(this_shader_input);
		out;
	}

	ifn( this_shader_input->descriptor_updates[ in_ticket->frame ] )
	{
		// unlock_shader_input(this_shader_input);
		out;
	}
	if( in_ticket->image is null or in_ticket->image->image is null )
	{
		// unlock_shader_input(this_shader_input);
		out;
	}

	H_update_descriptor_set_image( in_ticket->binding, this_shader_input->descriptors[ in_ticket->frame ], in_ticket->image->form->sampler, in_ticket->image->view );
	this_shader_input->descriptor_updates[ in_ticket->frame ] = no;
	// unlock_shader_input(this_shader_input);
}

//

/////// /////// /////// /////// /////// /////// ///////

// shader
// -------
// renders via GLSL modules

make_object(
	shader,
	form_shader form;
	form_frame frame_form;

	H_vertex_binding info_vertex_binding;
	H_struct_pipeline_vertex info_vertex;
	H_struct_pipeline_assembly info_assembly;
	H_struct_pipeline_raster info_raster;
	H_struct_pipeline_multisample info_multisample;
	H_struct_pipeline_depth_stencil info_depth_stencil;
	H_struct_pipeline_blend info_blend;

	u32 constant_bytes;
	list modules;
	list stages;
	H_pipeline_layout pipeline_layout;
	H_pipeline pipeline;
)( in form_shader in_form, in form_frame in_form_frame, in list in_modules, in ptr( H_struct_pipeline_blend ) in_blend, in u32 in_constant_bytes )
{
#ifdef hept_debug
	print_error( in_form is null, "shader: in_form is null" );
	print_error( in_form_frame is null, "shader: in_form_frame is null" );
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
	this->stages = new_list( H_struct_pipeline_shader_stage );
	iter_list( this->modules, shader_module )
	{
		list_get_iter( this->modules, shader_module );
		if( this_shader_module->form->shader_stage->type is shader_stage_type_vertex )
		{
			vert_form_mesh = this_shader_module->form->mesh_form;
		}

		list_add( this->stages, H_struct_pipeline_shader_stage, this_shader_module->stage_info );
	}

	//

	this->info_vertex_binding = H_create_vertex_binding_per_vertex( 0, vert_form_mesh->type_size );

	ptr( H_vertex_attribute ) vert_attributes = new_ptr( H_vertex_attribute, vert_form_mesh->attribs->size );
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

	this->info_vertex = H_create_struct_pipeline_vertex(
		1, ref( this->info_vertex_binding ), vert_form_mesh->attribs->size, vert_attributes
	);

	this->info_assembly = H_create_struct_pipeline_assembly( this->form->topology, no );

	this->info_raster = H_create_struct_pipeline_raster(
		yes,
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

	this->info_multisample = H_create_struct_pipeline_multisample(
		H_image_sample_1,
		no,
		1.0f,
		null,
		no,
		no
	);

	this->info_depth_stencil = H_create_struct_pipeline_depth_stencil(
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

	if( in_blend is null )
		this->info_blend = H_create_struct_pipeline_blend(
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

	H_struct_pipeline_layout info_pipeline_layout;
	H_struct_push_constant_range pushconst_range = H_create_struct_push_constant_range(
		H_shader_stage_vertex,
		0,
		this->constant_bytes
	);
	info_pipeline_layout = H_create_struct_pipeline_layout( 1, ref( this->form->descriptor_layout ), ( this->constant_bytes > 0 ), ref( pushconst_range ) );
	this->pipeline_layout = H_new_pipeline_layout( info_pipeline_layout );

	//

	H_struct_pipeline_viewport viewport_info = H_create_struct_pipeline_viewport(
		1, null, 1, null
	);
	//

	H_struct_render_pipeline pipeline_info = H_create_struct_render_pipeline(
		this->modules->size,
		to( ptr( H_struct_pipeline_shader_stage ), this->stages->data ),
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
	pipeline_info.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

	this->pipeline = H_new_render_pipeline( pipeline_info );

#ifdef hept_trace
	print_trace( "new shader: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn delete_shader( in shader in_shader )
{
	// copied stuff!!
	// if(in_shader->pipeline_layout isnt null) vkDestroyPipelineLayout(current_os_machine->device,in_shader->pipeline_layout,null);
	if( in_shader->pipeline isnt null ) vkDestroyPipeline( current_os_machine->device, in_shader->pipeline, null );
	// delete_list( in_shader->modules );
	// delete_list( in_shader->stages );
	unassign_shader( in_shader );
}

fn delete_all_shaders()
{
	if( shader_pile isnt null )
	{
		iter_pile( shader_pile, shader )
		{
			pile_find_iter( shader_pile, shader );
			delete_shader( this_shader );
		}
	}
}

inl shader copy_shader( in shader in_shader, in form_shader in_form, in form_frame in_form_frame, in list in_modules, in ptr( H_struct_pipeline_blend ) in_blend, in u32 in_constant_bytes )
{
	shader this = assign_shader();

	if( in_form is null )
	{
		this->form = in_shader->form;
		this->info_assembly = in_shader->info_assembly;
	}
	else
	{
		this->form = in_form;
		this->info_assembly = H_create_struct_pipeline_assembly( this->form->topology, no );
	}
	if( in_form_frame is null ) this->frame_form = in_shader->frame_form;
	else
		this->frame_form = in_form_frame;

	this->info_vertex = in_shader->info_vertex;
	this->info_raster = in_shader->info_raster;
	this->info_multisample = in_shader->info_multisample;
	this->info_depth_stencil = in_shader->info_depth_stencil;
	if( in_blend is null ) this->info_blend = in_shader->info_blend;
	else
		this->info_blend = val( in_blend );

	if( in_modules is null )
	{
		this->modules = in_shader->modules;
		this->stages = in_shader->stages;
	}
	else
	{
		this->modules = in_modules;
		this->stages = new_list( H_struct_pipeline_shader_stage );
		iter_list( this->modules, shader_module )
		{
			list_get_iter( this->modules, shader_module );
			list_add( this->stages, H_struct_pipeline_shader_stage, this_shader_module->stage_info );
		}
	}

	this->constant_bytes = in_constant_bytes;
	if( ( this->constant_bytes isnt in_shader->constant_bytes ) or in_form isnt null )
	{
		H_struct_pipeline_layout info_pipeline_layout;
		H_struct_push_constant_range pushconst_range = H_create_struct_push_constant_range(
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			this->constant_bytes
		);
		info_pipeline_layout = H_create_struct_pipeline_layout( 1, ref( this->form->descriptor_layout ), ( this->constant_bytes > 0 ), ref( pushconst_range ) );
		this->pipeline_layout = H_new_pipeline_layout( info_pipeline_layout );
	}
	else
		this->pipeline_layout = in_shader->pipeline_layout;

	H_struct_pipeline_viewport viewport_info = H_create_struct_pipeline_viewport(
		1, null, 1, null
	);

	H_struct_render_pipeline pipeline_info = H_create_struct_render_pipeline(
		this->modules->size,
		to( ptr( H_struct_pipeline_shader_stage ), this->stages->data ),
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
	pipeline_info.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;

	this->pipeline = H_new_render_pipeline( pipeline_info );

	out this;
}

global shader default_shader_2d_tri = null;
global shader default_shader_2d_tri_tex = null;
global shader default_shader_2d_line = null;
global shader default_shader_2d_tri_depth = null;
global shader default_shader_2d_tri_tex_depth = null;
global shader default_shader_2d_line_depth = null;
// global shader default_shader_3d = null;

global ptr( H_struct_pipeline_blend ) default_blend_none = null;
global ptr( H_struct_pipeline_blend ) default_blend_normal = null;
global ptr( H_struct_pipeline_blend ) default_blend_red = null;
global ptr( H_struct_pipeline_blend ) default_blend_add = null;
global ptr( H_struct_pipeline_blend ) default_blend_multiply = null;


//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

//

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
	print_error( in_type is event_type_null, "event: in_type is null" );
#endif
	//
	event this = assign_event();
	//
	this->type = in_type;
	if( this->type is event_type_alarm )
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
	if( in_event->state is event_state_inactive ) out;
	check( in_event->type )
	{
		with( event_type_once )
		{
			in_event->call();
			in_event->state = event_state_inactive;
			skip;
		}
		with( event_type_always )
		{
			in_event->call();
			skip;
		}
		with( event_type_alarm )
		{
			if( in_event->data is 0 )
			{
				in_event->call();
				in_event->state = event_state_inactive;
			}
			else
				in_event->data--;
			skip;
		}
		with( event_type_count )
		{
			if( in_event->data isnt 0 )
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

fn delete_event( in event in_event )
{
	unassign_event( in_event );
}

fn delete_all_events()
{
	if( event_pile isnt null )
	{
		iter_pile( event_pile, event )
		{
			pile_find_iter( event_pile, event );
			delete_event( this_event );
		}
	}
}

/////// /////// /////// /////// /////// /////// ///////

//

/// scene

make_object(
	scene,
	list events;
)()
{
	scene this = assign_scene();
//
#ifdef hept_trace
	print_trace( "new scene: ID: %d", this->pile_id );
#endif
	//
	out this;
}

fn start_scene( in scene in_scene )
{
}

fn end_scene( in scene in_scene )
{
}

fn delete_scene( in scene in_scene )
{
	unassign_scene( in_scene );
}

fn delete_all_scenes()
{
	if( scene_pile isnt null )
	{
		iter_pile( scene_pile, scene )
		{
			pile_find_iter( scene_pile, scene );
			delete_scene( this_scene );
		}
	}
}

/////// /////// /////// /////// /////// /////// ///////

//

/// audio

global u32 listener_index = 0;

#ifndef hept_no_audio
global ma_result _audio_result;
global ma_engine _audio_engine;
#endif

make_object(
	audio,
	text path;
)( in text in_path, in text in_name )
{
#ifdef hept_debug
		// print_error( in_type is event_type_null, "event: in_type is null" );
#endif
	//
	audio this = assign_audio();
	//
	this->path = new_text( in_path, text_length( in_name ) + 1 );
	join_text( this->path, folder_sep );
	join_text( this->path, in_name );
//
#ifdef hept_trace
	print_trace( "new sound: ID: %d", this->pile_id );
#endif
	//
	out this;
}

	#ifndef hept_no_audio
make_object(
	sound,
	ma_sound sound;
)
	#else
make_object(
	sound,
	ptr(pure) sound;
)
	#endif
	( in audio in_audio )
{
#ifdef hept_debug
		print_error( in_audio is null, "event: in_audio is null" );
#endif
	//
	sound this = assign_sound();
#ifndef hept_no_audio
	//
	ma_sound_init_from_file( &_audio_engine, in_audio->path, 0, NULL, NULL, ref( this->sound ) );
#endif
//
#ifdef hept_trace
	print_trace( "new sound: ID: %d", this->pile_id );
#endif
	//
	out this;
}

inl sound play_sound( audio a, in fvec3 in_pos, in f32 in_volume, in f32 in_pitch )
{
	#ifndef hept_no_audio
	sound s = new_sound( a );

	ma_sound_set_pitch( ref( s->sound ), in_pitch );
	ma_sound_set_position( ref( s->sound ), in_pos.x, in_pos.y, in_pos.z );
	ma_sound_set_volume( ref( s->sound ), in_volume );
	ma_sound_set_pinned_listener_index( ref( s->sound ), listener_index );

	ma_sound_start( ref( s->sound ) );

	out s;
#endif
}

void stop_audio( audio a )
{
	//
}

/////// /////// /////// /////// /////// /////// ///////

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

//

/// commands

fn use_image_src( in image in_image )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		in_image->layout,
		H_image_layout_shader_read_only_optimal,
		in_image->image,
		in_image->form->type,
		0,
		1,
		0,
		1
	);
	in_image->layout = temp_barrier.newLayout;

	vkCmdPipelineBarrier(
		current_command_buffer,
		H_pipeline_stage_color_attachment_output,
		H_pipeline_stage_fragment_shader,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		ref( temp_barrier )
	);
}

fn use_image_dst( in image in_image )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		0,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		in_image->layout,
		H_image_layout_color_attachment_optimal,
		in_image->image,
		in_image->form->type,
		0,
		1,
		0,
		1
	);
	in_image->layout = temp_barrier.newLayout;

	vkCmdPipelineBarrier(
		current_command_buffer,
		H_pipeline_stage_top_of_pipe,
		H_pipeline_stage_color_attachment_output,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		ref( temp_barrier )
	);
}

fn use_image_dst_depth( in image in_image )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		0,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		in_image->layout,
		H_image_layout_depth_stencil_attachment_optimal,
		in_image->image,
		in_image->form->type,
		0,
		1,
		0,
		1
	);
	in_image->layout = temp_barrier.newLayout;

	vkCmdPipelineBarrier(
		current_command_buffer,
		H_pipeline_stage_top_of_pipe,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		ref( temp_barrier )
	);
}

fn use_image_blit_src( in image in_image )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		in_image->layout,
		H_image_layout_transfer_src_optimal,
		in_image->image,
		in_image->form->type,
		0,
		1,
		0,
		1
	);
	in_image->layout = temp_barrier.newLayout;

	vkCmdPipelineBarrier(
		current_command_buffer,
		H_pipeline_stage_color_attachment_output,
		H_pipeline_stage_transfer,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		ref( temp_barrier )
	);
}

fn use_image_blit_dst( in image in_image )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		in_image->layout,
		H_image_layout_transfer_dst_optimal,
		in_image->image,
		in_image->form->type,
		0,
		1,
		0,
		1
	);
	in_image->layout = temp_barrier.newLayout;

	vkCmdPipelineBarrier(
		current_command_buffer,
		H_pipeline_stage_top_of_pipe,
		H_pipeline_stage_transfer,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		ref( temp_barrier )
	);
}

fn use_image_blit_dst_depth( in image in_image )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		in_image->layout,
		H_image_layout_transfer_dst_optimal,
		in_image->image,
		in_image->form->type,
		0,
		1,
		0,
		1
	);
	in_image->layout = temp_barrier.newLayout;

	vkCmdPipelineBarrier(
		current_command_buffer,
		H_pipeline_stage_top_of_pipe,
		H_pipeline_stage_transfer,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		ref( temp_barrier )
	);
}

fn use_image_present( in image in_image )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		in_image->layout,
		H_image_layout_present,
		in_image->image,
		in_image->form->type,
		0,
		1,
		0,
		1
	);
	in_image->layout = temp_barrier.newLayout;

	vkCmdPipelineBarrier(
		current_command_buffer,
		H_pipeline_stage_color_attachment_output,
		H_pipeline_stage_bottom_of_pipe,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		ref( temp_barrier )
	);
}

//

fn clear_image( in image in_image )
{
	use_image_blit_dst( in_image );

	VkClearColorValue clearColor;
	clearColor.float32[ 0 ] = 0.0f;
	clearColor.float32[ 1 ] = 0.0f;
	clearColor.float32[ 2 ] = 0.0f;
	clearColor.float32[ 3 ] = 1.0f;

	VkImageSubresourceRange imageRange;
	imageRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageRange.baseMipLevel = 0;
	imageRange.levelCount = 1;
	imageRange.baseArrayLayer = 0;
	imageRange.layerCount = 1;

	vkCmdClearColorImage(
		current_command_buffer,
		in_image->image,
		in_image->layout,
		&clearColor,
		1,
		&imageRange
	);
}

fn clear_depth( in image in_image )
{
	use_image_blit_dst_depth( in_image );

	VkClearDepthStencilValue clearDepth;
	clearDepth.depth = 1.0f;
	clearDepth.stencil = 0;

	VkImageSubresourceRange imageRange;
	imageRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imageRange.baseMipLevel = 0;
	imageRange.levelCount = 1;
	imageRange.baseArrayLayer = 0;
	imageRange.layerCount = 1;

	vkCmdClearDepthStencilImage(
		current_command_buffer,
		in_image->image,
		in_image->layout,
		&clearDepth,
		1,
		&imageRange
	);
}

/////// /////// /////// /////// /////// /////// ///////

// shader commands

fn start_shader( in shader in_shader, in u32 in_width, in u32 in_height )
{
	current_shader = in_shader;
	current_shader_input = null;

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
	current_shader_input = in_shader_input;
	// lock_shader_input(in_shader_input);
	vkCmdBindDescriptorSets( current_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_shader->pipeline_layout, 0, 1, ref( in_shader_input->descriptors[ current_renderer->current_frame ] ), 0, NULL );
	// unlock_shader_input(in_shader_input);
}

fn use_constants( in u32 in_size, in ptr( pure ) in_data )
{
	vkCmdPushConstants(
		current_command_buffer,
		current_shader->pipeline_layout,
		H_shader_stage_vertex,
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

// mesh commands

fn draw_instanced_mesh( in mesh in_mesh, in u32 in_count )
{
	if( in_count is 0 ) out;
	if( in_mesh->vertex_buffer->buffer is null ) out;
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers( current_renderer->command_buffers[ current_renderer->current_frame ], 0, 1, ref( in_mesh->vertex_buffer->buffer ), offsets );
	vkCmdDraw( current_renderer->command_buffers[ current_renderer->current_frame ], in_mesh->vertex_list->size, in_count, 0, 0 );
}

fn draw_mesh( in mesh in_mesh )
{
	draw_instanced_mesh( in_mesh, 1 );
}

//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

//

/// update functions

fn update_os_window_surface( in os_window in_window )
{
	constant u32 formats_count = H_get_surface_format_count( current_os_machine->gpu, in_window->surface );
	ptr( H_surface_format ) formats = new_ptr( H_surface_format, formats_count );
	H_get_surface_formats( current_os_machine->gpu, in_window->surface, formats_count, formats );
	if( formats_count is 0 or ( formats_count is 1 and formats[ 0 ].format is VK_FORMAT_UNDEFINED ) )
	{
		in_window->surface_format.format = H_format_bgra_u8_to_norm_f32;
		in_window->surface_format.colorSpace = H_colorspace_srgb_nonlinear;
	}
	else
	{
		in_window->surface_format = formats[ 0 ];
	}
	delete_ptr( formats );

	constant u32 modes_count = H_get_present_mode_count( current_os_machine->gpu, in_window->surface );
	ptr( H_present_mode ) present_modes = new_ptr( H_present_mode, modes_count );
	H_get_present_modes( current_os_machine->gpu, in_window->surface, modes_count, present_modes );
	iter( modes_count, m )
	{
		if( present_modes[ m ] is main_vsync )
		{
			in_window->present_mode = main_vsync;
			skip;
		}
	}
	delete_ptr( present_modes );

	in_window->surface_capabilities = H_get_surface_capabilities(
		current_os_machine->gpu, in_window->surface
	);

	in_window->width = in_window->surface_capabilities.currentExtent.width;
	in_window->height = in_window->surface_capabilities.currentExtent.height;
}

fn update_os_windows()
{
#ifdef hept_trace
	do_once print_trace( "updating windows" );
#endif
	iter_pile( os_window_pile, os_window )
	{
		pile_find_iter( os_window_pile, os_window );

		current_os_window = this_os_window;

		//

#if OS_WINDOWS
		once MSG msg = { 0 };

		as( PeekMessage( ref( msg ), NULL, 0, 0, PM_REMOVE ) )
		{
			if( msg.message is 0x0012 )
			{
				safe_flag_set( hept_exit, yes );
				out;
			}

			TranslateMessage( ref( msg ) );
			DispatchMessage( ref( msg ) );
		}

#elif OS_LINUX
		process_os_window( this_os_window->link.xdis, this_os_window->link.xwin );
#endif

		//

		if( this_os_window->ready )
		{
			ifn( this_os_window->visible )
			{
				show_os_window( this_os_window );
			}
		}
		else
		{
			update_os_window_surface( this_os_window );
			this_os_window->ready = yes;
		}
	}
}

//

fn update_inputs()
{
	main_mouse_wheel_x = 0;
	main_mouse_wheel_y = 0;

	as( safe_u8_get( input_update_ptr ) > 0 )
	{
		safe_u8_dec( input_update_ptr );
		safe_flag_set( inputs[ safe_u16_get( input_updates[ safe_u8_get( input_update_ptr ) ] ) ].pressed, no );
		safe_flag_set( inputs[ safe_u16_get( input_updates[ safe_u8_get( input_update_ptr ) ] ) ].released, no );
	}
}

//

fn update_events()
{
	mouse_window_prev_x = mouse_window_x;
	mouse_window_prev_y = mouse_window_y;
	mouse_window_x = main_mouse_window_x;
	mouse_window_y = main_mouse_window_y;
	//
	u64 T = get_ns();
	if( event_pile != null )
	{
		iter_pile( event_pile, event )
		{
			pile_find_iter( event_pile, event );
			//
			perform_event( this_event );
		}
	}
	once f64 oTf = 0;
	f64 oT = 1000. / ( to_f64( get_ns() - T ) / to_f64( nano_per_milli ) );
	oTf = ( oTf * .95 ) + ( oT * .05 );
	// print( "%d\n", to_u32( oTf ) );
}

//

fn update_renderers()
{
#ifdef hept_trace
	do_once print_trace( "updating renderers" );
#endif
	iter_pile( renderer_pile, renderer )
	{
		pile_find_iter( renderer_pile, renderer );

		current_renderer = this_renderer;

		if( this_renderer->swapchain is null ) refresh_renderer( this_renderer );

		H_semaphore image_semaphore = null;

		if( this_renderer->recycled_semaphores->size <= 0 )
			image_semaphore = H_new_semaphore();
		else
			image_semaphore = list_remove_back( this_renderer->recycled_semaphores, H_semaphore );

		H_result aquire_result = vkAcquireNextImageKHR(
			current_os_machine->device,
			this_renderer->swapchain,
			UINT64_MAX,
			image_semaphore,
			VK_NULL_HANDLE,
			ref( this_renderer->current_frame )
		);

		if( aquire_result is VK_ERROR_OUT_OF_DATE_KHR || aquire_result is VK_SUBOPTIMAL_KHR )
		{
			refresh_renderer( this_renderer );
			update_renderers();
			out;
		}

		frame this_frame = list_get( this_renderer->frame_present_list, frame, this_renderer->current_frame );

		H_wait_fence( this_frame->wait );
		//
		H_reset_fence( this_frame->wait );

		current_command_buffer = this_renderer->command_buffers[ this_renderer->current_frame ];
		vkResetCommandBuffer( current_command_buffer, 0 );

		H_semaphore old_semaphore = this_frame->ready;

		if( old_semaphore isnt null )
		{
			list_add( this_renderer->recycled_semaphores, H_semaphore, old_semaphore );
		}

		this_frame->ready = image_semaphore;

		//

		vacate_spinlock( main_thread_lock );
		sleep_ns( 1 );

		//update_events();

		engage_spinlock( main_thread_lock );

		//

		image this_window_image = list_get( this_renderer->frame_window->images, image, 0 );
		image this_window_depth = list_get( this_renderer->frame_window->images, image, 1 );

		once shader_input present_input = null;
		once shader_module present_shader_module_vert = null;
		once shader_module present_shader_module_frag = null;
		once shader present_shader = null;
		once fvec2 present_resolution;
		do_once
		{
			present_input = new_shader_input(default_form_shader_tri_tex);
			update_shader_input_image(present_input,0,this_window_image);

			present_shader_module_vert = new_shader_module( default_form_shader_module_2d_tri_tex_vert, main_shader_path, "present" );
			present_shader_module_frag = new_shader_module( default_form_shader_module_2d_tri_tex_frag, main_shader_path, "present" );
			list present_modules = new_list( shader_module );
			list_add( present_modules, shader_module, present_shader_module_vert );
			list_add( present_modules, shader_module, present_shader_module_frag );
			present_shader = copy_shader( default_shader_2d_tri_tex, null, null, present_modules, null, size_( fvec2 ) );
		}

		{
			iter_pile_ptr( buffer_ticket_pile, buffer_ticket )
			{
				pile_find_iter_ptr( buffer_ticket_pile, buffer_ticket );
				//
				process_buffer_ticket( this_buffer_ticket );
			}
		}
		//
		{
			iter_pile_ptr( shader_input_storage_ticket_pile, shader_input_storage_ticket )
			{
				pile_find_iter_ptr( shader_input_storage_ticket_pile, shader_input_storage_ticket );
				//
				this_shader_input_storage_ticket->frame = current_renderer->current_frame;
				process_shader_input_storage_ticket( this_shader_input_storage_ticket );
			}
		}
		//
		{
			iter_pile_ptr( shader_input_image_ticket_pile, shader_input_image_ticket )
			{
				pile_find_iter_ptr( shader_input_image_ticket_pile, shader_input_image_ticket );
				//
				this_shader_input_image_ticket->frame = current_renderer->current_frame;
				process_shader_input_image_ticket( this_shader_input_image_ticket );
			}
		}
		//
		iter( buffer_delete_tickets->size, b )
		{
			if( b >= buffer_delete_tickets->size ) skip;
			ptr( buffer_delete_ticket ) this_ticket = ref(
				list_get( buffer_delete_tickets, buffer_delete_ticket, b )
			);
			this_ticket->count++;
			if( this_ticket->count > 3 )
			{
				vkFreeMemory( current_os_machine->device, this_ticket->memory, null );
				vkDestroyBuffer( current_os_machine->device, this_ticket->buffer, null );
				list_delete( buffer_delete_tickets, b );
				b--;
			}
		}

		//

		H_struct_command_buffer_start command_buffer_start_info = H_create_struct_command_buffer_start( null );
		command_buffer_start_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		H_start_command_buffer( current_command_buffer, command_buffer_start_info );

		image this_image = list_get( this_frame->images, image, 0 );

		clear_image( this_window_image );
		clear_depth( this_window_depth );

		use_image_dst( this_window_image );
		use_image_dst_depth( this_window_depth );

		current_frame = current_renderer->frame_window;

		///

		current_os_window->call();

		///

		current_frame = this_frame;

		clear_image( this_image );

		use_image_src( this_window_image );
		use_image_dst( this_image );

		start_shader( present_shader, current_os_window->width, current_os_window->height );
		use_shader_input(present_input);

		present_resolution.w = to_f32(current_os_window->width) / to_f32(main_width);
		present_resolution.h = to_f32(current_os_window->height) / to_f32(main_height);

		main_scale = ceil_f32(max(
			present_resolution.w,
			present_resolution.h
		));

		present_resolution.w /= main_scale;
		present_resolution.h /= main_scale;

		//print("%d, %d\n", current_os_window->width,main_width);
		use_constants(size_(fvec2),ref(present_resolution));
		draw_mesh( default_mesh_window_tex );
		end_shader();

		use_image_present( this_image );

		H_end_command_buffer( current_command_buffer );

		//

		H_pipeline_stage wait_stages[] = { H_pipeline_stage_color_attachment_output };

		H_struct_submit submit_info = H_create_struct_submit(
			1,
			ref( this_frame->ready ),
			wait_stages,
			1,
			ref( current_command_buffer ),
			1,
			ref( this_frame->done )
		);

		H_submit_queue(
			this_renderer->form->queue,
			submit_info,
			this_frame->wait
		);

		//

		H_struct_present present_info = H_create_struct_present(
			1,
			ref( this_frame->done ),
			1,
			ref( this_renderer->swapchain ),
			ref( this_renderer->current_frame )
		);

		H_result present_result = H_present( this_renderer->form->queue, present_info );

		if( present_result is VK_ERROR_OUT_OF_DATE_KHR || present_result is VK_SUBOPTIMAL_KHR )
		{
			refresh_renderer( this_renderer );
			update_renderers();
			out;
		}
	}
}

//



//

/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////
/////// /////// /////// /////// /////// /////// ///////

//

#if OS_WINDOWS

inl s32 os_get_mouse_button( UINT u_msg )
{
	check( u_msg )
	{
		with(WM_LBUTTONDOWN)
			with( WM_LBUTTONUP)
				out 511;
		with( WM_RBUTTONDOWN)
			with( WM_RBUTTONUP)
				out 510;
		with( WM_MBUTTONDOWN)
			with( WM_MBUTTONUP)
				out 509;
		with_other
				out -1;
	}
}

inl LRESULT CALLBACK process_os_window( HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param )
{
	s32 button = os_get_mouse_button( u_msg );
	if( button == -1 ) button = w_param;

	check( u_msg )
	{
		with( WM_DESTROY )
		{
			safe_flag_set( hept_exit, yes );
			out 0;
		}

		//

		with( WM_SETCURSOR )
		{
			if( LOWORD( l_param ) == HTCLIENT )
			{
				if( main_cursor_visible )
					SetCursor( LoadCursor( NULL, IDC_ARROW ) );
				else
					SetCursor( NULL );
				out TRUE;
			}
			else
			{
				out DefWindowProc( hwnd, u_msg, w_param, l_param );
			}
			skip;
		}

		with( WM_LBUTTONDOWN )
			with( WM_RBUTTONDOWN )
				with( WM_MBUTTONDOWN )
					with( WM_KEYDOWN )
						with( WM_SYSKEYDOWN )
		{
			if( safe_flag_get( inputs[ button ].held ) == no )
			{
				safe_flag_set( inputs[ button ].pressed, yes );
				safe_flag_set( inputs[ button ].released, no );
				safe_u16_set( input_updates[ safe_u8_get( input_update_ptr ) ], button );
				safe_u8_inc( input_update_ptr );
			}
			safe_flag_set( inputs[ button ].held, yes );
			out 1;
		}

		with( WM_LBUTTONUP )
			with( WM_RBUTTONUP )
				with( WM_MBUTTONUP )
					with( WM_KEYUP )
						with( WM_SYSKEYUP )
		{
			if( safe_flag_get( inputs[ button ].held ) )
			{
				safe_flag_set( inputs[ button ].held, no );
				safe_flag_set( inputs[ button ].pressed, no );
				safe_flag_set( inputs[ button ].released, yes );
				safe_u16_set( input_updates[ safe_u8_get( input_update_ptr ) ], button );
				safe_u8_inc( input_update_ptr );
			}
			out 1;
		}

		with( WM_MOUSEMOVE )
		{
			safe_s32_set(
				main_mouse_window_x,
				( ( int )( short )LOWORD( l_param ) ) -
					( ( current_os_window->width - ( main_window_width ) ) / 2 )
			);
			safe_s32_set(
				main_mouse_window_y,
				( ( int )( short )HIWORD( l_param ) ) -
					( ( current_os_window->height - ( main_window_height ) ) / 2 )
			);
			out 1;
		}

		with( WM_MOUSEHWHEEL )
		{
			if( GET_WHEEL_DELTA_WPARAM( w_param ) > 0 )
			{
				safe_s32_set( main_mouse_wheel_x, 1 );
			}
			else
			{
				safe_s32_set( main_mouse_wheel_x, -1 );
			}
			out 1;
		}

		with( WM_MOUSEWHEEL )
		{
			if( GET_WHEEL_DELTA_WPARAM( w_param ) > 0 )
			{
				safe_s32_set( main_mouse_wheel_y, 1 );
			}
			else
			{
				safe_s32_set( main_mouse_wheel_y, -1 );
			}
			out 1;
		}

		with_other
			out DefWindowProc( hwnd, u_msg, w_param, l_param );
	}

	out 0;
}

#elif OS_LINUX

void process_os_window( Display* in_disp, Window in_win )
{
	XEvent e;
	u32 custom_key;

	as( XPending( in_disp ) )
	{
		XNextEvent( in_disp, &e );

		check( e.type )
		{
			with( DestroyNotify )
			{
				safe_flag_set( hept_exit, yes );
				out;
			}
			with_other skip;
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

//

fn main_core()
{
	H_init();
	//
	object_pile_list = new_list( pile );
	//
	inputs = new_ptr( input, 512 );
	input_updates = new_ptr( u16, 256 );
	//
	buffer_ticket_pile = new_pile( buffer_ticket );
	buffer_delete_tickets = new_list( buffer_delete_ticket );
	shader_input_storage_ticket_pile = new_pile( shader_input_storage_ticket );
	shader_input_image_ticket_pile = new_pile( shader_input_image_ticket );
	//
	new_os_core( main_creator_name );
	new_os_window( main_app_name, main_width, main_height, main_fn_command );
	new_os_machine();
	//
	H_current_device = current_os_machine->device;
	//
	default_form_renderer = new_form_renderer();
	new_renderer( default_form_renderer );
	update_os_window_surface( current_os_window );
	//
#ifndef hept_no_audio
	_audio_result = ma_engine_init( null, &_audio_engine );
	if( _audio_result != MA_SUCCESS )
	{
		print( "AUDIO ERROR\n" );
	}

	listener_index = ma_engine_find_closest_listener( &_audio_engine, 0, 0, 0 );
	ma_engine_listener_set_world_up( &_audio_engine, listener_index, 0, 0, 1 );
	ma_engine_listener_set_direction( &_audio_engine, listener_index, 0, 0, -1 );
#endif
}

fn main_defaults()
{
	/////// /////// /////// /////// /////// /////// ///////
	// default folders

	main_path = get_folder();

#ifdef hept_release
	main_data_path = new_text( main_path, 5 );
	join_text( main_data_path, folder_sep "data" );
#else
	main_data_path = new_text( main_path, 0 );
	text temp_src = null;
	loop
	{
		temp_src = new_text( main_data_path, 4 );
		join_text( temp_src, folder_sep "src" );
		if( check_folder( temp_src ) )
		{
			delete_text( main_data_path );
			main_data_path = new_text( temp_src, 5 );
			join_text( main_data_path, folder_sep "data" );
			skip;
		}
		else
			main_data_path = parent_folder( main_data_path );
		delete_text( temp_src );
	}
	delete_text( temp_src );
#endif

	print( "executed path: %s\n", main_path );
	print( "data path: %s\n", main_data_path );

	main_audio_path = new_text( main_data_path, 6 );
	join_text( main_audio_path, folder_sep "audio" );

	main_shader_path = new_text( main_data_path, 7 );
	join_text( main_shader_path, folder_sep "shader" );

	main_shader_default_path = new_text( main_shader_path, 8 );
	join_text( main_shader_default_path, folder_sep "default" );

	print( "shader path: %s\n", main_shader_path );
	print( "shader defaults path: %s\n", main_shader_default_path );

#ifndef hept_release
	make_folder( main_data_path );
	make_folder( main_shader_path );
	make_folder( main_shader_default_path );
	// make_folder( main_image_folder );
	make_folder( main_audio_path );
#endif

	/////// /////// /////// /////// /////// /////// ///////
	// default values

	default_blend_none = ref( H_default_blend_none );
	default_blend_normal = ref( H_default_blend_normal );
	default_blend_red = ref( H_default_blend_red );
	default_blend_add = ref( H_default_blend_add );
	default_blend_multiply = ref( H_default_blend_multiply );

	/////// /////// /////// /////// /////// /////// ///////
	// default forms

	default_form_buffer_vertex = new_form_buffer(
		H_buffer_usage_vertex,
		H_memory_property_device_local | H_memory_property_host_visible | H_memory_property_host_coherent
	);
	default_form_buffer_index = new_form_buffer(
		H_buffer_usage_index,
		H_memory_property_device_local | H_memory_property_host_visible | H_memory_property_host_coherent
	);
	default_form_buffer_storage = new_form_buffer(
		H_buffer_usage_storage,
		H_memory_property_device_local | H_memory_property_host_visible | H_memory_property_host_coherent
	);
	//

	default_form_image_rgba = new_form_image( image_type_rgba );
	default_form_image_depth = new_form_image( image_type_depth );
	default_form_image_stencil = new_form_image( image_type_stencil );
	default_form_image_depth_stencil = new_form_image( image_type_depth_stencil );
	//
	default_form_frame_layer_rgba = new_form_frame_layer( default_form_image_rgba );
	default_form_frame_layer_depth = new_form_frame_layer( default_form_image_depth );
	default_form_frame_layer_stencil = new_form_frame_layer( default_form_image_stencil );
	default_form_frame_layer_depth_stencil = new_form_frame_layer( default_form_image_depth_stencil );
	//
	{
		list layers = new_list( form_frame_layer );
		list_add( layers, form_frame_layer, default_form_frame_layer_rgba );
		default_form_frame = new_form_frame( layers );
	}
	//
	//if( no )
	{
		list layers = new_list( form_frame_layer );
		list_add( layers, form_frame_layer, default_form_frame_layer_rgba );
		list_add( layers, form_frame_layer, default_form_frame_layer_depth_stencil );
		default_form_frame_depth = new_form_frame( layers );
	}

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

	list attribs_3d_tri = new_list( form_mesh_attrib );
	list_add( attribs_3d_tri, form_mesh_attrib, default_form_mesh_attrib_pos3 );
	list_add( attribs_3d_tri, form_mesh_attrib, default_form_mesh_attrib_rgb );
	default_form_mesh_3d_tri = new_form_mesh( attribs_3d_tri, yes );
	list attribs_3d_tri_tex = new_list( form_mesh_attrib );
	list_add( attribs_3d_tri_tex, form_mesh_attrib, default_form_mesh_attrib_pos3 );
	list_add( attribs_3d_tri_tex, form_mesh_attrib, default_form_mesh_attrib_uv );
	list_add( attribs_3d_tri_tex, form_mesh_attrib, default_form_mesh_attrib_rgb );
	default_form_mesh_3d_tri_tex = new_form_mesh( attribs_3d_tri_tex, yes );
	list attribs_3d_line = new_list( form_mesh_attrib );
	list_add( attribs_3d_line, form_mesh_attrib, default_form_mesh_attrib_pos3 );
	list_add( attribs_3d_line, form_mesh_attrib, default_form_mesh_attrib_rgb );
	default_form_mesh_3d_line = new_form_mesh( attribs_3d_line, yes );
	//
	default_form_shader_stage_vert = new_form_shader_stage( shader_stage_type_vertex );
	default_form_shader_stage_geom = new_form_shader_stage( shader_stage_type_geometry );
	default_form_shader_stage_frag = new_form_shader_stage( shader_stage_type_fragment );
	default_form_shader_stage_comp = new_form_shader_stage( shader_stage_type_compute );
	//
	default_form_shader_module_2d_tri_vert = new_form_shader_module( default_form_shader_stage_vert, default_form_mesh_2d_tri );
	default_form_shader_module_2d_tri_frag = new_form_shader_module( default_form_shader_stage_frag, default_form_mesh_2d_tri );
	default_form_shader_module_2d_tri_tex_vert = new_form_shader_module( default_form_shader_stage_vert, default_form_mesh_2d_tri_tex );
	default_form_shader_module_2d_tri_tex_frag = new_form_shader_module( default_form_shader_stage_frag, default_form_mesh_2d_tri_tex );
	default_form_shader_module_2d_line_vert = new_form_shader_module( default_form_shader_stage_vert, default_form_mesh_2d_line );
	default_form_shader_module_2d_line_frag = new_form_shader_module( default_form_shader_stage_frag, default_form_mesh_2d_line );

	default_form_shader_module_3d_tri_vert = new_form_shader_module( default_form_shader_stage_vert, default_form_mesh_3d_tri );
	default_form_shader_module_3d_tri_frag = new_form_shader_module( default_form_shader_stage_frag, default_form_mesh_3d_tri );
	default_form_shader_module_3d_tri_tex_vert = new_form_shader_module( default_form_shader_stage_vert, default_form_mesh_3d_tri_tex );
	default_form_shader_module_3d_tri_tex_frag = new_form_shader_module( default_form_shader_stage_frag, default_form_mesh_3d_tri_tex );
	default_form_shader_module_3d_line_vert = new_form_shader_module( default_form_shader_stage_vert, default_form_mesh_3d_line );
	default_form_shader_module_3d_line_frag = new_form_shader_module( default_form_shader_stage_frag, default_form_mesh_3d_line );
	//
	default_form_shader_binding_image = new_form_shader_binding( shader_binding_type_image, shader_stage_type_fragment );
	default_form_shader_binding_storage_vert = new_form_shader_binding( shader_binding_type_storage, shader_stage_type_vertex );
	default_form_shader_binding_storage_frag = new_form_shader_binding( shader_binding_type_storage, shader_stage_type_fragment );
	//

	default_form_shader_line = new_form_shader( H_topology_line, null );

	default_form_shader_tri = new_form_shader( H_topology_tri, null );

	list shader_inputs_tri_tex = new_list( form_shader_binding );
	list_add( shader_inputs_tri_tex, form_shader_binding, default_form_shader_binding_image );
	default_form_shader_tri_tex = new_form_shader( H_topology_tri, shader_inputs_tri_tex );

	list shader_inputs_line_storage = new_list( form_shader_binding );
	list_add( shader_inputs_line_storage, form_shader_binding, default_form_shader_binding_storage_vert );
	default_form_shader_line_storage = new_form_shader( H_topology_line, shader_inputs_line_storage );

	list shader_inputs_tri_storage = new_list( form_shader_binding );
	list_add( shader_inputs_tri_storage, form_shader_binding, default_form_shader_binding_storage_vert );
	default_form_shader_tri_storage = new_form_shader( H_topology_tri, shader_inputs_tri_storage );

	list shader_inputs_tri_tex_storage = new_list( form_shader_binding );
	list_add( shader_inputs_tri_tex_storage, form_shader_binding, default_form_shader_binding_storage_vert );
	list_add( shader_inputs_tri_tex_storage, form_shader_binding, default_form_shader_binding_image );
	default_form_shader_tri_tex_storage = new_form_shader( H_topology_tri, shader_inputs_tri_tex_storage );

	//

	/////// /////// /////// /////// /////// /////// ///////
	// default objects

	default_mesh_line = new_mesh( default_form_mesh_2d_line );
	mesh_add_line(
		default_mesh_line,
		vertex_2d_line,
		create_vertex_2d_line( 0, 0, 1, 1, 1 ),
		create_vertex_2d_line( 1, 0, 1, 1, 1 )
	);
	update_mesh( default_mesh_line );

	default_mesh_square = new_mesh( default_form_mesh_2d_tri );
	mesh_add_quad(
		default_mesh_square,
		vertex_2d_tri,
		create_vertex_2d_tri( -.5, -.5, 1, 1, 1 ),
		create_vertex_2d_tri( .5, -.5, 1, 1, 1 ),
		create_vertex_2d_tri( .5, .5, 1, 1, 1 ),
		create_vertex_2d_tri( -.5, .5, 1, 1, 1 )
	);
	update_mesh( default_mesh_square );

	default_mesh_square_tex = new_mesh( default_form_mesh_2d_tri_tex );
	mesh_add_quad(
		default_mesh_square_tex,
		vertex_2d_tri_tex,
		create_vertex_2d_tri_tex( -.5, -.5, 0, 0, 1, 1, 1 ),
		create_vertex_2d_tri_tex( .5, -.5, 1, 0, 1, 1, 1 ),
		create_vertex_2d_tri_tex( .5, .5, 1, 1, 1, 1, 1 ),
		create_vertex_2d_tri_tex( -.5, .5, 0, 1, 1, 1, 1 )
	);
	update_mesh( default_mesh_square_tex );

	default_mesh_window_white = new_mesh( default_form_mesh_2d_tri );
	mesh_add_quad(
		default_mesh_window_white,
		vertex_2d_tri,
		create_vertex_2d_tri( -1., -1., 1, 1, 1 ),
		create_vertex_2d_tri( 1., -1., 1, 1, 1 ),
		create_vertex_2d_tri( 1., 1., 1, 1, 1 ),
		create_vertex_2d_tri( -1., 1., 1, 1, 1 )
	);
	update_mesh( default_mesh_window_white );

	default_mesh_window_black = new_mesh( default_form_mesh_2d_tri );
	mesh_add_quad(
		default_mesh_window_black,
		vertex_2d_tri,
		create_vertex_2d_tri( -1., -1., 0, 0, 0 ),
		create_vertex_2d_tri( 1., -1., 0, 0, 0 ),
		create_vertex_2d_tri( 1., 1., 0, 0, 0 ),
		create_vertex_2d_tri( -1., 1., 0, 0, 0 )
	);
	update_mesh( default_mesh_window_black );

	default_mesh_window_tex = new_mesh( default_form_mesh_2d_tri_tex );
	mesh_add_quad(
		default_mesh_window_tex,
		vertex_2d_tri_tex,
		create_vertex_2d_tri_tex( -1., -1., 0, 0, 1, 1, 1 ),
		create_vertex_2d_tri_tex( 1., -1., 1, 0, 1, 1, 1 ),
		create_vertex_2d_tri_tex( 1., 1., 1, 1, 1, 1, 1 ),
		create_vertex_2d_tri_tex( -1., 1., 0, 1, 1, 1, 1 )
	);
	update_mesh( default_mesh_window_tex );

	default_shader_module_2d_tri_vert = new_shader_module( default_form_shader_module_2d_tri_vert, main_shader_default_path, "default_2d_tri" );
	default_shader_module_2d_tri_frag = new_shader_module( default_form_shader_module_2d_tri_frag, main_shader_default_path, "default_2d_tri" );
	default_shader_module_2d_tri_tex_vert = new_shader_module( default_form_shader_module_2d_tri_tex_vert, main_shader_default_path, "default_2d_tri_tex" );
	default_shader_module_2d_tri_tex_frag = new_shader_module( default_form_shader_module_2d_tri_tex_frag, main_shader_default_path, "default_2d_tri_tex" );
	default_shader_module_2d_line_vert = new_shader_module( default_form_shader_module_2d_line_vert, main_shader_default_path, "default_2d_line" );
	default_shader_module_2d_line_frag = new_shader_module( default_form_shader_module_2d_line_frag, main_shader_default_path, "default_2d_line" );

	list tri_modules = new_list( shader_module );
	list_add( tri_modules, shader_module, default_shader_module_2d_tri_vert );
	list_add( tri_modules, shader_module, default_shader_module_2d_tri_frag );
	default_shader_2d_tri = new_shader( default_form_shader_tri, default_form_frame, tri_modules, null, 0 );
	default_shader_2d_tri_depth = new_shader( default_form_shader_tri, default_form_frame_depth, tri_modules, null, 0 );

	list tri_tex_modules = new_list( shader_module );
	list_add( tri_tex_modules, shader_module, default_shader_module_2d_tri_tex_vert );
	list_add( tri_tex_modules, shader_module, default_shader_module_2d_tri_tex_frag );
	default_shader_2d_tri_tex = new_shader( default_form_shader_tri_tex, default_form_frame, tri_tex_modules, null, 0 );
	default_shader_2d_tri_tex_depth = new_shader( default_form_shader_tri_tex, default_form_frame_depth, tri_tex_modules, null, 0 );

	list line_modules = new_list( shader_module );
	list_add( line_modules, shader_module, default_shader_module_2d_line_vert );
	list_add( line_modules, shader_module, default_shader_module_2d_line_frag );
	default_shader_2d_line = new_shader( default_form_shader_line, default_form_frame, line_modules, null, 0 );
	default_shader_2d_line_depth = new_shader( default_form_shader_line, default_form_frame_depth, line_modules, null, 0 );
}

fn main_init();

global os_thread main_thread = null;
global os_pacer main_thread_pacer = null;

inl ptr( pure ) main_thread_loop( in ptr( pure ) in_ptr )
{
	main_thread_pacer = new_os_pacer( to_u32( main_fps ) );
	loop
	{
		if( safe_flag_get( hept_exit ) ) skip;
		start_os_pacer( main_thread_pacer );

		// once f32 T = 0;
		//  T += 1./main_fps;

		// print_f32(T);

		//
		// if( safe_flag_get( main_thread_ready ) )
		//{
		engage_spinlock( main_thread_lock );
		//
		update_events();
		//
		sleep_ns( 1 );
		//
		vacate_spinlock( main_thread_lock );
		//
		update_inputs();
		//}
		//
		if( safe_flag_get( hept_exit ) ) skip;
		wait_os_pacer( main_thread_pacer );
	}
	safe_flag_set( main_thread_exit, yes );
	out null;
}

fn main_loop()
{
	engage_spinlock(main_thread_lock);
	main_thread = new_os_thread(main_thread_loop);
	loop
	{
		if( safe_flag_get( hept_exit ) )
			out;
		update_os_windows();
		if( safe_flag_get( hept_exit ) )
			out;
		update_renderers();
	}
}

fn main_exit()
{
	// as(safe_flag_get( main_thread_exit ) is no) sleep_ns(nano_per_milli);
	// vkDeviceWaitIdle( current_os_machine->device );

	// delete_all_shaders();
	// delete_all_shader_inputs();
	// delete_all_shader_modules();
	// delete_all_meshes();
	delete_all_renderers();
	// delete_all_frames();
	// delete_all_images();
	// delete_all_buffers();

	// delete_all_form_shaders();
	// delete_all_form_shader_bindings();
	// delete_all_form_shader_modules();
	// delete_all_form_shader_stages();
	// delete_all_form_meshes();
	// delete_all_form_mesh_attribs();
	delete_all_form_renderers();
	// delete_all_form_frames();
	// delete_all_form_frame_layers();
	// delete_all_form_images();
	// delete_all_form_buffers();

	delete_all_os_machines();
	delete_all_os_windows();
	delete_all_os_cores();
	// delete_all_os_threads();
	// delete_all_os_pacers();
	// delete_all_os_files();

	delete_hept_object_piles();
}

//

#if OS_WINDOWS & COMPILER_MSVC
int main();
inl s32 WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	out main();
}
#endif

#define main( CREATOR_NAME, APP_NAME, WIDTH, HEIGHT, FN_COMMAND, VSYNC, FPS )            \
	int main()                                                                             \
	{                                                                                      \
		main_creator_name = new_text( CREATOR_NAME, 0 );                                     \
		main_app_name = new_text( APP_NAME, 0 );                                             \
		main_width = WIDTH;                                                                  \
		main_height = HEIGHT;                                                                \
		main_fn_command = to( fn_ptr( pure, , pure ), FN_COMMAND );                          \
		main_vsync = ( VSYNC ) ? ( H_present_mode_vsync_on ) : ( H_present_mode_vsync_off ); \
		main_fps = FPS;                                                                      \
		main_core();                                                                         \
		main_defaults();                                                                     \
		main_init();                                                                         \
		main_loop();                                                                         \
		main_exit();                                                                         \
		out 0;                                                                               \
	}                                                                                      \
	fn main_init()

#endif
