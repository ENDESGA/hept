// // // // // // //
// > hept _
// -------
// minimal game engine language and system framework
// requires: hephaestus.h, c7h16.h
// @ENDESGA 2023
// // // // // // //

#pragma once
#ifndef hept_included
	#define hept_included

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

global list hept_object_piles = null;

//

	#define make_object( NAME, ... )                                             \
		make_struct( NAME )                                                        \
		{                                                                          \
			u32 pile_id;                                                             \
			spinlock lock;                                                           \
			__VA_ARGS__                                                              \
		};                                                                         \
		make_ptr( struct( NAME ) ) NAME;                                           \
		global NAME current_##NAME = null;                                         \
		global pile pile_##NAME = null;                                            \
		fn lock_##NAME( in NAME in_##NAME )                                        \
		{                                                                          \
			engage_spinlock( in_##NAME->lock );                                      \
		}                                                                          \
		fn unlock_##NAME( in NAME in_##NAME )                                      \
		{                                                                          \
			vacate_spinlock( in_##NAME->lock );                                      \
		}                                                                          \
		fn set_current_##NAME( in NAME in_##NAME )                                 \
		{                                                                          \
			safe_ptr_set( current_##NAME, in_##NAME );                               \
		}                                                                          \
		inl NAME assign_##NAME()                                                   \
		{                                                                          \
			NAME this = new_ptr( struct( NAME ), 1 );                                \
			if( safe_ptr_get( pile_##NAME ) == null )                                \
			{                                                                        \
				safe_ptr_set( pile_##NAME, new_pile( NAME ) );                         \
				list_safe_add( hept_object_piles, pile, pile_##NAME );                 \
			}                                                                        \
			pile_safe_add( pile_##NAME, NAME, this );                                \
			safe_u32_set( this->pile_id, safe_u32_get( pile_##NAME->prev_pos ) );    \
			if( safe_ptr_get( current_##NAME ) == null ) set_current_##NAME( this ); \
			out this;                                                                \
		}                                                                          \
		fn unassign_##NAME( in NAME in_##NAME )                                    \
		{                                                                          \
			pile_delete( pile_##NAME, ptr( pure ), in_##NAME->pile_id );             \
			delete_ptr( in_##NAME );                                                 \
		}                                                                          \
		NAME new_##NAME

fn delete_hept_object_piles()
{
	iter_list( hept_object_piles, p )
	{
		pile this_pile = list_remove_front( hept_object_piles, pile );
		delete_pile( this_pile );
	}
	delete_list( hept_object_piles );
}

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

fn delete_NAME( in NAME in_NAME )
{
	delete_ptr( in_NAME->ELEMENTS );
	unassign_NAME( in_NAME );
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
	HANDLE file = CreateFileA( ( LPCSTR )this->path, GENERIC_READ, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null );

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

fn delete_os_file( in os_file in_os_file )
{
	unassign_os_file( in_os_file );
}

fn delete_all_os_files()
{
	if( pile_os_file != null )
	{
		os_file this_os_file = null;
		u32 os_file_n = 0;
		u32 pile_size = pile_os_file->size;

		iter_pile( pile_os_file, t )
		{
			if( os_file_n >= pile_size ) skip;
			this_os_file = pile_find( pile_os_file, os_file, t );
			if( this_os_file == null ) next;
			else
				os_file_n++;
			//
			delete_os_file( this_os_file );
		}
	}
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

	#if OS_WINDOWS
		#define folder_sep "\\"
	#else
		#define folder_sep "/"
	#endif

text parent_folder( in text in_text )
{
	if( in_text == null )
	{
		out null;
	}

	text cur = in_text + text_length( in_text ) - 1;

	as( cur >= in_text )
	{
		if( val( cur ) == '\\' || val( cur ) == '/' )
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

fn delete_os_pacer( in os_pacer in_os_pacer )
{
	unassign_os_pacer( in_os_pacer );
}

fn delete_all_os_pacers()
{
	if( pile_os_pacer != null )
	{
		os_pacer this_os_pacer = null;
		u32 os_pacer_n = 0;
		u32 pile_size = pile_os_pacer->size;

		iter_pile( pile_os_pacer, t )
		{
			if( os_pacer_n >= pile_size ) skip;
			this_os_pacer = pile_find( pile_os_pacer, os_pacer, t );
			if( this_os_pacer == null ) next;
			else
				os_pacer_n++;
			//
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

fn delete_os_thread( in os_thread in_os_thread )
{
	unassign_os_thread( in_os_thread );
}

fn delete_all_os_threads()
{
	if( pile_os_thread != null )
	{
		os_thread this_os_thread = null;
		u32 os_thread_n = 0;
		u32 pile_size = pile_os_thread->size;

		iter_pile( pile_os_thread, t )
		{
			if( os_thread_n >= pile_size ) skip;
			this_os_thread = pile_find( pile_os_thread, os_thread, t );
			if( this_os_thread == null ) next;
			else
				os_thread_n++;
			//
			delete_os_thread( this_os_thread );
		}
	}
}

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
	H_struct_app info_app = H_create_struct_app(
		in_name,
		H_create_version( os_core_version, 0, 0 ),
		"hept",
		H_create_version( 0, 0, 1 ),
		H_create_version( 1, 3, 0 )
	);

	text desired_debug_layers[] = {
		"VK_LAYER_KHRONOS_validation",
		"VK_LAYER_LUNARG_monitor" };
	u32 desired_debug_layers_count = 2;

	u32 debug_layer_count = H_get_debug_layers( null ), enabled_debug_layer_count = 0;
	ptr( H_debug_layer_properties ) available_layers = new_ptr( H_debug_layer_properties, debug_layer_count );
	H_get_debug_layers( available_layers );
	if( available_layers == null )
	{
	#ifdef hept_debug
		print_error( yes, "os_core: memory H_get_debug_layers() failed for available_layers" );
	#endif
		delete_ptr( available_layers );
		out null;
	}

	ptr( text ) debug_layers = new_ptr( text, desired_debug_layers_count );

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

	delete_ptr( available_layers );

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

	H_struct_instance instance_info = H_create_struct_instance(
		ref( info_app ),
		enabled_debug_layer_count,
		( const char* const* )debug_layers,
		extension_count,
		( const char* const* )extensions
	);
	this->instance = H_new_instance( instance_info );
	#ifdef hept_debug
	if( this->instance == null )
	{
		print_error( yes, "os_core: instance could not be created" );
		delete_ptr( debug_layers );
		out null;
	}
	#endif

	#ifdef hept_debug
	print_error( this->instance == null, "os_core: instance could not be created" );
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
	vkDestroyInstance( in_os_core->instance, null );
	unassign_os_core( in_os_core );
}

fn delete_all_os_cores()
{
	if( pile_os_core != null )
	{
		os_core this_os_core = null;
		u32 os_core_n = 0;
		u32 pile_size = pile_os_core->size;

		iter_pile( pile_os_core, t )
		{
			if( os_core_n >= pile_size ) skip;
			this_os_core = pile_find( pile_os_core, os_core, t );
			if( this_os_core == null ) next;
			else
				os_core_n++;
			//
			delete_os_core( this_os_core );
		}
	}
}

//

/////// /////// /////// /////// /////// /////// ///////

// os_machine
// -------
// links the CPU to the GPU

make_object(
	os_machine,
	u32 queue_index;
	H_gpu gpu;
	H_device device;
	H_gpu_memory_properties memory_properties;
)()
{
	os_machine this = assign_os_machine();
	//
	this->queue_index = 0;
	this->gpu = null;
	this->device = null;
	//
	#ifdef hept_trace
	print_trace( "new os_machine: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

fn delete_os_machine( in os_machine in_os_machine )
{
	vkDestroyDevice( in_os_machine->device, null );
	unassign_os_machine( in_os_machine );
}

fn delete_all_os_machines()
{
	if( pile_os_machine != null )
	{
		os_machine this_os_machine = null;
		u32 os_machine_n = 0;
		u32 pile_size = pile_os_machine->size;

		iter_pile( pile_os_machine, t )
		{
			if( os_machine_n >= pile_size ) skip;
			this_os_machine = pile_find( pile_os_machine, os_machine, t );
			if( this_os_machine == null ) next;
			else
				os_machine_n++;
			//
			delete_os_machine( this_os_machine );
		}
	}
}

//

/////// /////// /////// /////// /////// /////// ///////

// os_window
// -------
// holds the window

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
		.lpszClassName = "hept" };

	flag rc = RegisterClass( ref( wc ) );

		#ifdef hept_debug
	print_error( rc == 0, "os_window: cannot create win32 window" );
		#endif
	DWORD style = WS_EX_APPWINDOW;
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

	VkWin32SurfaceCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hwnd = this->link.hwnd;
	create_info.hinstance = this->link.inst;

	vkCreateWin32SurfaceKHR( current_os_core->instance, ref( create_info ), null, ref( this->surface ) );

	SetWindowText( this->link.hwnd, this->name );

	HANDLE consoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );

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

fn delete_os_window( in os_window in_os_window )
{
	delete_text( in_os_window->name );
	vkDestroySurfaceKHR( current_os_core->instance, in_os_window->surface, null );
	#if OS_WINDOWS
	DestroyWindow( in_os_window->link.hwnd );
	#elif OS_LINUX
	XDestroyWindow( in_os_window->link.xdis, in_os_window->link.xwin );
	#endif
	unassign_os_window( in_os_window );
}

fn delete_all_os_windows()
{
	if( pile_os_window != null )
	{
		os_window this_os_window = null;
		u32 os_window_n = 0;
		u32 pile_size = pile_os_window->size;

		iter_pile( pile_os_window, t )
		{
			if( os_window_n >= pile_size ) skip;
			this_os_window = pile_find( pile_os_window, os_window, t );
			if( this_os_window == null ) next;
			else
				os_window_n++;
			//
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

fn delete_form_buffer( in form_buffer in_form_buffer )
{
	unassign_form_buffer( in_form_buffer );
}

fn delete_all_form_buffers()
{
	if( pile_form_buffer != null )
	{
		form_buffer this_form_buffer = null;
		u32 form_buffer_n = 0;
		u32 pile_size = pile_form_buffer->size;

		iter_pile( pile_form_buffer, t )
		{
			if( form_buffer_n >= pile_size ) skip;
			this_form_buffer = pile_find( pile_form_buffer, form_buffer, t );
			if( this_form_buffer == null ) next;
			else
				form_buffer_n++;
			//
			delete_form_buffer( this_form_buffer );
		}
	}
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

	H_struct_sampler info = H_create_struct_sampler(
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

fn delete_form_image( in form_image in_form_image )
{
	vkDestroySampler( current_os_machine->device, in_form_image->sampler, null );
	unassign_form_image( in_form_image );
}

fn delete_all_form_images()
{
	if( pile_form_image != null )
	{
		form_image this_form_image = null;
		u32 form_image_n = 0;
		u32 pile_size = pile_form_image->size;

		iter_pile( pile_form_image, t )
		{
			if( form_image_n >= pile_size ) skip;
			this_form_image = pile_find( pile_form_image, form_image, t );
			if( this_form_image == null ) next;
			else
				form_image_n++;
			//
			delete_form_image( this_form_image );
		}
	}
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

fn delete_form_frame_layer( in form_frame_layer in_form_frame_layer )
{
	unassign_form_frame_layer( in_form_frame_layer );
}

fn delete_all_form_frame_layers()
{
	if( pile_form_frame_layer != null )
	{
		form_frame_layer this_form_frame_layer = null;
		u32 form_frame_layer_n = 0;
		u32 pile_size = pile_form_frame_layer->size;

		iter_pile( pile_form_frame_layer, t )
		{
			if( form_frame_layer_n >= pile_size ) skip;
			this_form_frame_layer = pile_find( pile_form_frame_layer, form_frame_layer, t );
			if( this_form_frame_layer == null ) next;
			else
				form_frame_layer_n++;
			//
			delete_form_frame_layer( this_form_frame_layer );
		}
	}
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
	frame_type_depth = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
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
		attach_ref_rgba = new_ptr( H_attachment_reference, rgba_count );

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

	ptr( H_attachment_description ) attachments = new_ptr( H_attachment_description, this->layers->size );
	iter( this->layers->size, l )
	{
		form_frame_layer this_layer = list_get( this->layers, form_frame_layer, l );
		attachments[ l ].format = this_layer->format;
		attachments[ l ].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[ l ].loadOp = ( this->type == frame_type_attachment or this->type == frame_type_depth ) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[ l ].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[ l ].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[ l ].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[ l ].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[ l ].finalLayout = to( H_image_layout, this->type );
		attachments[ l ].flags = 0;

		if( this->type == frame_type_attachment or this->type == frame_type_depth )
		{
			with( this_layer->type )
			{
				is( frame_layer_type_rgba )
				{
					attachments[ l ].initialLayout = to( H_image_layout, frame_type_attachment );
					attachments[ l ].finalLayout = to( H_image_layout, frame_type_attachment );
					skip;
				}
				is( frame_layer_type_depth )
				{
					attachments[ l ].initialLayout = to( H_image_layout, frame_type_depth );
					attachments[ l ].finalLayout = to( H_image_layout, frame_type_depth );
					skip;
				}
			}
		}
	}

	H_struct_render_pass render_pass_info = H_create_struct_render_pass( this->layers->size, attachments, 1, ref( subpass ), 0, null );
	this->render_pass = H_new_render_pass( current_os_machine->device, render_pass_info );
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
	if( pile_form_frame != null )
	{
		form_frame this_form_frame = null;
		u32 form_frame_n = 0;
		u32 pile_size = pile_form_frame->size;

		iter_pile( pile_form_frame, t )
		{
			if( form_frame_n >= pile_size ) skip;
			this_form_frame = pile_find( pile_form_frame, form_frame, t );
			if( this_form_frame == null ) next;
			else
				form_frame_n++;
			//
			delete_form_frame( this_form_frame );
		}
	}
}

global form_frame default_form_frame = null;
global form_frame default_form_frame_depth = null;

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
	this->queue = H_get_queue( current_os_machine->device, current_os_machine->queue_index, 0 );
	if( this->command_pool != null ) vkDestroyCommandPool( current_os_machine->device, this->command_pool, null );
	H_struct_command_pool command_pool_info = H_create_struct_command_pool( current_os_machine->queue_index );
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	this->command_pool = H_new_command_pool( current_os_machine->device, command_pool_info );
	//
	#ifdef hept_trace
	print_trace( "new form_renderer: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

fn delete_form_renderer( in form_renderer in_form_renderer )
{
	vkDestroyCommandPool( current_os_machine->device, in_form_renderer->command_pool, null );
	unassign_form_renderer( in_form_renderer );
}

fn delete_all_form_renderers()
{
	if( pile_form_renderer != null )
	{
		form_renderer this_form_renderer = null;
		u32 form_renderer_n = 0;
		u32 pile_size = pile_form_renderer->size;

		iter_pile( pile_form_renderer, t )
		{
			if( form_renderer_n >= pile_size ) skip;
			this_form_renderer = pile_find( pile_form_renderer, form_renderer, t );
			if( this_form_renderer == null ) next;
			else
				form_renderer_n++;
			//
			delete_form_renderer( this_form_renderer );
		}
	}
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

fn delete_form_mesh_attrib( in form_mesh_attrib in_form_mesh_attrib )
{
	unassign_form_mesh_attrib( in_form_mesh_attrib );
}

fn delete_all_form_mesh_attribs()
{
	if( pile_form_mesh_attrib != null )
	{
		form_mesh_attrib this_form_mesh_attrib = null;
		u32 form_mesh_attrib_n = 0;
		u32 pile_size = pile_form_mesh_attrib->size;

		iter_pile( pile_form_mesh_attrib, t )
		{
			if( form_mesh_attrib_n >= pile_size ) skip;
			this_form_mesh_attrib = pile_find( pile_form_mesh_attrib, form_mesh_attrib, t );
			if( this_form_mesh_attrib == null ) next;
			else
				form_mesh_attrib_n++;
			//
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

fn delete_form_mesh( in form_mesh in_form_mesh )
{
	delete_list( in_form_mesh->attribs );
	delete_text( in_form_mesh->layout_glsl );
	unassign_form_mesh( in_form_mesh );
}

fn delete_all_form_meshes()
{
	if( pile_form_mesh != null )
	{
		form_mesh this_form_mesh = null;
		u32 form_mesh_n = 0;
		u32 pile_size = pile_form_mesh->size;

		iter_pile( pile_form_mesh, t )
		{
			if( form_mesh_n >= pile_size ) skip;
			this_form_mesh = pile_find( pile_form_mesh, form_mesh, t );
			if( this_form_mesh == null ) next;
			else
				form_mesh_n++;
			//
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

fn delete_form_shader_stage( in form_shader_stage in_form_shader_stage )
{
	unassign_form_shader_stage( in_form_shader_stage );
}

fn delete_all_form_shader_stages()
{
	if( pile_form_shader_stage != null )
	{
		form_shader_stage this_form_shader_stage = null;
		u32 form_shader_stage_n = 0;
		u32 pile_size = pile_form_shader_stage->size;

		iter_pile( pile_form_shader_stage, t )
		{
			if( form_shader_stage_n >= pile_size ) skip;
			this_form_shader_stage = pile_find( pile_form_shader_stage, form_shader_stage, t );
			if( this_form_shader_stage == null ) next;
			else
				form_shader_stage_n++;
			//
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

fn delete_form_module( in form_module in_form_module )
{
	unassign_form_module( in_form_module );
}

fn delete_all_form_modules()
{
	if( pile_form_module != null )
	{
		form_module this_form_module = null;
		u32 form_module_n = 0;
		u32 pile_size = pile_form_module->size;

		iter_pile( pile_form_module, t )
		{
			if( form_module_n >= pile_size ) skip;
			this_form_module = pile_find( pile_form_module, form_module, t );
			if( this_form_module == null ) next;
			else
				form_module_n++;
			//
			delete_form_module( this_form_module );
		}
	}
}

global form_module default_form_module_2d_tri_vert = null;
global form_module default_form_module_2d_tri_frag = null;
global form_module default_form_module_2d_tri_tex_vert = null;
global form_module default_form_module_2d_tri_tex_frag = null;
global form_module default_form_module_2d_line_vert = null;
global form_module default_form_module_2d_line_frag = null;
global form_module default_form_module_3d_tri_vert = null;
global form_module default_form_module_3d_tri_frag = null;
global form_module default_form_module_3d_tri_tex_vert = null;
global form_module default_form_module_3d_tri_tex_frag = null;
global form_module default_form_module_3d_line_vert = null;
global form_module default_form_module_3d_line_frag = null;

//

/////// /////// /////// /////// /////// /////// ///////

// form_shader_binding
// -------
// input for form_shader

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
	print_error( in_type == shader_binding_type_null, "form_shader_binding: in_type is null" );
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
	if( pile_form_shader_binding != null )
	{
		form_shader_binding this_form_shader_binding = null;
		u32 form_shader_binding_n = 0;
		u32 pile_size = pile_form_shader_binding->size;

		iter_pile( pile_form_shader_binding, t )
		{
			if( form_shader_binding_n >= pile_size ) skip;
			this_form_shader_binding = pile_find( pile_form_shader_binding, form_shader_binding, t );
			if( this_form_shader_binding == null ) next;
			else
				form_shader_binding_n++;
			//
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
// -------
// holds a blueprint for shader

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

	if( this->bindings != null )
	{
		iter_list( this->bindings, i )
		{
			form_shader_binding this_form_shader_binding = list_get( this->bindings, form_shader_binding, i );
			list_add(
				bindings,
				H_descriptor_layout_binding,
				( ( H_descriptor_layout_binding ){
					.binding = i,
					.descriptorType = to( VkDescriptorType, this_form_shader_binding->type ),
					.descriptorCount = 1,
					.stageFlags = this_form_shader_binding->stage_type | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
					.pImmutableSamplers = null,
				} )
			);
		}
	}

	H_struct_descriptor_layout layout_info = H_create_struct_descriptor_layout(
		bindings->size, ( const ptr( VkDescriptorSetLayoutBinding ) )bindings->data
	);
	this->descriptor_layout = H_new_descriptor_layout( current_os_machine->device, layout_info );
	//
	#ifdef hept_trace
	print_trace( "new form_shader: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

fn delete_form_shader( in form_shader in_form_shader )
{
	if( in_form_shader->descriptor_layout != null ) vkDestroyDescriptorSetLayout( current_os_machine->device, in_form_shader->descriptor_layout, null );
	delete_list( in_form_shader->bindings );
	unassign_form_shader( in_form_shader );
}

fn delete_all_form_shaders()
{
	if( pile_form_shader != null )
	{
		form_shader this_form_shader = null;
		u32 form_shader_n = 0;
		u32 pile_size = pile_form_shader->size;

		iter_pile( pile_form_shader, t )
		{
			if( form_shader_n >= pile_size ) skip;
			this_form_shader = pile_find( pile_form_shader, form_shader, t );
			if( this_form_shader == null ) next;
			else
				form_shader_n++;
			//
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

// buffer
// -------
//

// global pile update_buffers = null;

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
	print_error( in_form == null, "buffer: in_form is null" );
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

make_struct( buffer_ticket )
{
	buffer buffer;
	u64 size;
	ptr( pure ) data;
};
global pile buffer_tickets = null;

make_struct( buffer_delete_ticket )
{
	H_buffer buffer;
	H_memory memory;
	u8 count;
};
global list buffer_delete_tickets = null;

inl flag update_buffer( in buffer in_buffer, in u64 in_update_size, ptr( pure ) in_data )
{
	if( in_update_size == 0 ) out no;
	// lock_pile(buffer_tickets);
	// lock_buffer(in_buffer);
	if( in_buffer->ticket_id == -1 )
	{
		pile_add(
			buffer_tickets,
			struct( buffer_ticket ),
			create(
				struct( buffer_ticket ),
				in_buffer,
				in_update_size,
				in_data
			)
		);
		in_buffer->ticket_id = buffer_tickets->prev_pos;
	}
	else
	{
		ptr( struct( buffer_ticket ) ) ticket_ptr = ref( pile_find( buffer_tickets, struct( buffer_ticket ), in_buffer->ticket_id ) );
		ticket_ptr->size = in_update_size;
		ticket_ptr->data = in_data;
	}

	u64 temp_size = in_buffer->size;
	// unlock_buffer(in_buffer);
	// unlock_pile(buffer_tickets);
	out in_update_size > temp_size;
}

fn process_buffer_ticket( ptr( struct( buffer_ticket ) ) in_ticket )
{
	buffer this_buffer = in_ticket->buffer;
	// lock_buffer(this_buffer);
	if( in_ticket->size > this_buffer->size )
	{
		if( this_buffer->buffer != null )
			list_add(
				buffer_delete_tickets,
				struct( buffer_delete_ticket ),
				create(
					struct( buffer_delete_ticket ),
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
		this_buffer->buffer = H_new_buffer( current_os_machine->device, buffer_info );

		H_memory_requirements mem_requirements = H_get_memory_requirements_buffer( current_os_machine->device, this_buffer->buffer );
		H_struct_memory memory_info = H_create_struct_memory(
			mem_requirements.size,
			H_find_mem(
				current_os_machine->memory_properties,
				mem_requirements.memoryTypeBits,
				this_buffer->form->properties
			)
		);
		this_buffer->memory = H_new_memory_buffer( current_os_machine->device, memory_info, this_buffer->buffer );
	}

	//

	if( in_ticket->data != null )
	{
		once ptr( pure ) mapped = null;
		vkMapMemory( current_os_machine->device, this_buffer->memory, 0, in_ticket->size, 0, ref( mapped ) );
		copy_ptr( mapped, in_ticket->data, in_ticket->size );
		vkUnmapMemory( current_os_machine->device, this_buffer->memory );
		in_ticket->data = null;
	}

	pile_delete( buffer_tickets, struct( buffer_ticket ), this_buffer->ticket_id );
	this_buffer->ticket_id = -1;
	// unlock_buffer(this_buffer);
}

fn delete_all_buffers()
{
	if( pile_buffer != null )
	{
		buffer this_buffer = null;
		u32 buffer_n = 0;
		u32 pile_size = pile_buffer->size;

		iter_pile( pile_buffer, t )
		{
			if( buffer_n >= pile_size ) skip;
			this_buffer = pile_find( pile_buffer, buffer, t );
			if( this_buffer == null ) next;
			else
				buffer_n++;
			//
			delete_buffer( this_buffer );
		}
	}

	iter_list( buffer_delete_tickets, b )
	{
		ptr( struct( buffer_delete_ticket ) ) this_ticket = ref( list_remove_front( buffer_delete_tickets, struct( buffer_delete_ticket ) ) );
		vkFreeMemory( current_os_machine->device, this_ticket->memory, null );
		vkDestroyBuffer( current_os_machine->device, this_ticket->buffer, null );
	}
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

	H_struct_image image_info = H_create_struct_image(
		H_image_type_2d,
		this->form->format,
		temp_extent,
		1,
		1,
		H_image_sample_1,
		( ( this->state == image_state_src ) ? ( VK_IMAGE_TILING_LINEAR ) : ( VK_IMAGE_TILING_OPTIMAL ) ),
		( ( this->form->type == image_type_depth ) ? ( H_image_usage_depth_stencil_attachment ) : ( H_image_usage_sampled | H_image_usage_color_attachment ) ) | H_image_usage_transfer_src | H_image_usage_transfer_dst,
		H_sharing_mode_exclusive,
		0,
		null,
		this->layout
	);
	this->image = H_new_image( current_os_machine->device, image_info );

	H_memory_requirements mem_requirements = H_get_memory_requirements_image( current_os_machine->device, this->image );
	H_struct_memory memory_info = H_create_struct_memory(
		mem_requirements.size,
		H_find_mem(
			current_os_machine->memory_properties,
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

fn delete_image( in image in_image )
{
	if( in_image->memory != null ) vkFreeMemory( current_os_machine->device, in_image->memory, null );
	if( in_image->view != null ) vkDestroyImageView( current_os_machine->device, in_image->view, null );
	if( in_image->image != null and in_image->state != image_state_swap ) vkDestroyImage( current_os_machine->device, in_image->image, null );
	delete_list( in_image->data );
	unassign_image( in_image );
}

fn delete_all_images()
{
	if( pile_image != null )
	{
		image this_image = null;
		u32 image_n = 0;
		u32 pile_size = pile_image->size;

		iter_pile( pile_image, t )
		{
			if( image_n >= pile_size ) skip;
			this_image = pile_find( pile_image, image, t );
			if( this_image == null ) next;
			else
				image_n++;
			//
			delete_image( this_image );
		}
	}
}

//

/////// /////// /////// /////// /////// /////// ///////

// frame
// -------
//

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
				list_get( this->form->layers, form_frame_layer, v )->type,
				0,
				1,
				0,
				1 } )
		);
		if( temp_image->view == null ) temp_image->view = H_new_image_view( current_os_machine->device, image_view_info );
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
	this->frame = H_new_frame( current_os_machine->device, frame_info );

	this->clear_col[ 0 ] = ( VkClearValue ){ 0., 0., 0., 0. };
	this->clear_col[ 1 ] = ( VkClearValue ){ 0., 0., 0., 0. };
	this->info_begin = H_create_struct_begin_render_pass(
		this->form->render_pass,
		this->frame,
		( ( H_rect_2d ){ 0, 0, this->max_w, this->max_h } ),
		this->form->layers->size,
		ref( this->clear_col )
	);
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
	delete_list( in_frame->images );
	delete_list( in_frame->views );
	unassign_frame( in_frame );
}

fn delete_all_frames()
{
	if( pile_frame != null )
	{
		frame this_frame = null;
		u32 frame_n = 0;
		u32 pile_size = pile_frame->size;

		iter_pile( pile_frame, t )
		{
			if( frame_n >= pile_size ) skip;
			this_frame = pile_find( pile_frame, frame, t );
			if( this_frame == null ) next;
			else
				frame_n++;
			//
			delete_frame( this_frame );
		}
	}
}

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
	H_extent_2d swapchain_extent;
	u32 current_frame;
	form_frame form_frame_window;
	frame frame_window;
	u32 frame_window_width;
	u32 frame_window_height;
	list frames;
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
	//
	#ifdef hept_trace
	print_trace( "new renderer: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

fn delete_renderer( in renderer in_renderer )
{
	vkDestroySwapchainKHR( current_os_machine->device, in_renderer->swapchain, null );
	vkFreeCommandBuffers( current_os_machine->device, in_renderer->form->command_pool, in_renderer->frames->size, in_renderer->command_buffers );
	iter( in_renderer->frames->size, i )
	{
		vkDestroySemaphore( current_os_machine->device, in_renderer->image_ready[ i ], null );
		vkDestroySemaphore( current_os_machine->device, in_renderer->image_done[ i ], null );
		vkDestroyFence( current_os_machine->device, in_renderer->flight_fences[ i ], null );
	}
	delete_list( in_renderer->frames );
	unassign_renderer( in_renderer );
}

fn delete_all_renderers()
{
	if( pile_renderer != null )
	{
		renderer this_renderer = null;
		u32 renderer_n = 0;
		u32 pile_size = pile_renderer->size;

		iter_pile( pile_renderer, t )
		{
			if( renderer_n >= pile_size ) skip;
			this_renderer = pile_find( pile_renderer, renderer, t );
			if( this_renderer == null ) next;
			else
				renderer_n++;
			//
			delete_renderer( this_renderer );
		}
	}
}

fn refresh_renderer( in renderer in_renderer )
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

	in_renderer->ref_window->surface_capabilities = H_get_surface_capabilities( current_os_machine->gpu, in_renderer->ref_window->surface );

	u32 image_count_max = in_renderer->ref_window->surface_capabilities.maxImageCount;
	u32 image_count_min = in_renderer->ref_window->surface_capabilities.minImageCount + 1;
	if( ( image_count_max > 0 ) and ( image_count_min > image_count_max ) )
	{
		image_count_min = image_count_max;
	}

	H_struct_swapchain swapchain_info = H_create_struct_swapchain(
		in_renderer->ref_window->surface,
		image_count_min,
		in_renderer->ref_window->surface_format.format,
		in_renderer->ref_window->surface_format.colorSpace,
		in_renderer->ref_window->surface_capabilities.currentExtent,
		1,
		H_image_usage_transfer_src | H_image_usage_transfer_dst | H_image_usage_color_attachment,
		H_sharing_mode_exclusive,
		0,
		null,
		in_renderer->ref_window->surface_capabilities.currentTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		in_renderer->ref_window->present_mode,
		yes,
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

	u32 image_count = H_get_swapchain_images( current_os_machine->device, in_renderer->swapchain, null );
	ptr( H_image ) temp_images = new_ptr( H_image, image_count );
	H_get_swapchain_images( current_os_machine->device, in_renderer->swapchain, temp_images );

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

	iter( image_count, i )
	{
		list temp_list_images = new_list( image );
		image temp_image = assign_image();
		temp_image->form = in_renderer->swapchain_form_image;
		temp_image->image = temp_images[ i ];
		temp_image->width = in_renderer->swapchain_extent.width;
		temp_image->height = in_renderer->swapchain_extent.height;
		temp_image->state = image_state_swap;
		list_add( temp_list_images, image, temp_image );
		frame temp_frame = new_frame( in_renderer->form_frame_window, temp_list_images );
		list_add( in_renderer->frames, frame, temp_frame );
	}

	delete_ptr( temp_images );

	//

	in_renderer->image_ready = new_ptr( H_semaphore, in_renderer->frames->size );
	in_renderer->image_done = new_ptr( H_semaphore, in_renderer->frames->size );
	in_renderer->flight_fences = new_ptr( H_fence, in_renderer->frames->size );
	in_renderer->command_buffers = new_ptr( H_command_buffer, in_renderer->frames->size );

	H_struct_semaphore semaphore_info = H_create_struct_semaphore();
	H_struct_fence fence_info = H_create_struct_fence();
	H_struct_command_buffer command_buffers_info = H_create_struct_command_buffer(
		in_renderer->form->command_pool,
		H_command_buffer_level_primary,
		in_renderer->frames->size
	);

	H_new_command_buffers( current_os_machine->device, command_buffers_info, in_renderer->command_buffers );

	iter( in_renderer->frames->size, i )
	{
		in_renderer->image_ready[ i ] = H_new_semaphore( current_os_machine->device, semaphore_info );
		in_renderer->image_done[ i ] = H_new_semaphore( current_os_machine->device, semaphore_info );
		in_renderer->flight_fences[ i ] = H_new_fence( current_os_machine->device, fence_info );
	}
	//

	in_renderer->current_frame = 0;

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

	form_image form_image_depth = new_form_image( image_type_depth, VK_FORMAT_D32_SFLOAT );

	image depth_image = new_image(
		form_image_depth,
		image_state_dst,
		in_renderer->frame_window_width,
		in_renderer->frame_window_height
	);

	form_frame temp_form_frame;
	{
		list layers = new_list( form_frame_layer );
		form_frame_layer layer_rgba = new_form_frame_layer( frame_layer_type_rgba, in_renderer->swapchain_format );
		form_frame_layer layer_depth = new_form_frame_layer( frame_layer_type_depth, VK_FORMAT_D32_SFLOAT );
		list_add( layers, form_frame_layer, layer_rgba );
		list_add( layers, form_frame_layer, layer_depth );

		temp_form_frame = new_form_frame(
			frame_type_depth,
			layers
		);
	}

	list temp_list_images = new_list( image );
	list_add( temp_list_images, image, temp_image );
	list_add( temp_list_images, image, depth_image );
	in_renderer->frame_window = new_frame( temp_form_frame, temp_list_images );

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

make_struct( vertex_3d_tri )
{
	struct( fvec3 ) pos;
	struct( fvec3 ) rgb;
};
	#define create_struct_vertex_3d_tri( x, y, z, r, g, b ) create( struct( vertex_3d_tri ), .pos = { x, y, z }, .rgb = { r, g, b } )

make_struct( vertex_3d_tri_tex )
{
	struct( fvec3 ) pos;
	struct( fvec2 ) uv;
	struct( fvec3 ) rgb;
};
	#define create_struct_vertex_3d_tri_tex( x, y, z, u, v, r, g, b ) create( struct( vertex_3d_tri_tex ), .pos = { x, y, z }, .uv = { u, v }, .rgb = { r, g, b } )

make_struct( vertex_3d_line )
{
	struct( fvec3 ) pos;
	struct( fvec3 ) rgb;
};
	#define create_struct_vertex_3d_line( x, y, z, r, g, b ) create( struct( vertex_3d_line ), .pos = { x, y, z }, .rgb = { r, g, b } )

//

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
	flag update;
	u32 update_id;
	buffer vertex_buffer;
	// buffer index_buffer;
	list vertices;
	// list indices;
	// u32 vertex_n;
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
	this->vertices = assign_list( 0, 1, this->form->type_size, assign_ptr( this->form->type_size ) );
	// this->indices = new_list( u32 );
	this->vertex_buffer = new_buffer( default_form_buffer_vertex );
	// this->index_buffer = new_buffer( default_form_buffer_index );
	//
	#ifdef hept_trace
	print_trace( "new mesh: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

fn delete_mesh( in mesh in_mesh )
{
	delete_list( in_mesh->vertices );
	// delete_list( in_mesh->indices );
	unassign_mesh( in_mesh );
}

fn delete_all_meshes()
{
	if( pile_mesh != null )
	{
		mesh this_mesh = null;
		u32 mesh_n = 0;
		u32 pile_size = pile_mesh->size;

		iter_pile( pile_mesh, t )
		{
			if( mesh_n >= pile_size ) skip;
			this_mesh = pile_find( pile_mesh, mesh, t );
			if( this_mesh == null ) next;
			else
				mesh_n++;
			//
			delete_mesh( this_mesh );
		}
	}
}

global list list_update_mesh = null;

global mesh default_mesh_line = null;
global mesh default_mesh_square = null;
global mesh default_mesh_square_tex = null;
global mesh default_mesh_window_white = null;
global mesh default_mesh_window_black = null;
global mesh default_mesh_window_tex = null;

	#define mesh_add_point( var, vertex_struct, p ) \
		DEF_START                                     \
		list_add( var->vertices, vertex_struct, p );  \
		DEF_END

	#define mesh_add_line( var, vertex_struct, a, b ) \
		DEF_START                                       \
		mesh_add_point( var, vertex_struct, a );        \
		mesh_add_point( var, vertex_struct, b );        \
		DEF_END

	#define mesh_add_tri( var, vertex_struct, a, b, c ) \
		DEF_START                                         \
		mesh_add_point( var, vertex_struct, a );          \
		mesh_add_point( var, vertex_struct, b );          \
		mesh_add_point( var, vertex_struct, c );          \
		DEF_END

	#define mesh_add_quad( var, vertex_struct, tl, tr, br, bl ) \
		DEF_START                                                 \
		mesh_add_tri( var, vertex_struct, tl, tr, br );           \
		mesh_add_tri( var, vertex_struct, tl, br, bl );           \
		DEF_END

fn mesh_add_mesh_3d( in mesh a, in mesh b, in struct( fvec3 ) offset )
{
	u32 vs = a->vertices->size;

	/*ter_list( b->indices, i )
	{
		list_add(
			a->indices,
			u32,
			vs + list_get( b->indices, u32, i )
		);
	}*/

	iter_list( b->vertices, v )
	{
		struct( vertex_3d_tri ) temp = list_get( b->vertices, struct( vertex_3d_tri ), v );
		temp.pos = fvec3_add( temp.pos, offset );
		list_add(
			a->vertices,
			struct( vertex_3d_tri ),
			temp
		);
	}
}

fn update_mesh( in mesh in_mesh )
{
	update_buffer( in_mesh->vertex_buffer, in_mesh->vertices->size * in_mesh->vertices->size_type, in_mesh->vertices->data );
	// update_buffer( in_mesh->index_buffer, in_mesh->indices->size * in_mesh->indices->size_type, in_mesh->indices->data );
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
	H_struct_pipeline_shader_stage stage_info;
)( in form_module in_form, in text in_path, in text in_name )
{
	#ifdef hept_debug
	print_error( in_form == null, "module: in_form is null" );
	print_error( in_path == null, "module: in_path is null" );
	print_error( in_name == null, "module: in_name is null" );
	#endif
	//
	module this = assign_module();
	//
	this->form = in_form;

	text temp_path = new_text( in_path, text_length( in_name ) + 1 );
	join_text( temp_path, folder_sep );
	join_text( temp_path, in_name );

	text glsl_name = new_text( temp_path, 5 );
	join_text( glsl_name, ( ( this->form->shader_stage->type == shader_stage_type_vertex ) ? ( ".vert" ) : ( ".frag" ) ) );
	delete_text( temp_path );

	text spirv_name = new_text( glsl_name, 4 );
	join_text( spirv_name, ".spv" );

	//

	#ifndef hept_release
	ifn( check_file( glsl_name ) )
	{
		write_file( glsl_name, ( ( this->form->shader_stage->type == shader_stage_type_vertex ) ? ( default_glsl_vert ) : ( default_glsl_frag ) ) );
	}
	#endif

	if( check_file( glsl_name ) )
	{
		text command = format_text( "glslangValidator -V %s -o %s", glsl_name, spirv_name );
		s32 sys_result = system( command );
	#ifdef hept_debug
		print_error( sys_result != 0, "failed to compile GLSL to SPIR-V\n" );
	#endif
	}

	this->file = new_os_file( spirv_name );

	H_struct_shader_module module_info = H_create_struct_shader_module( this->file->size, to( ptr( u32 ), this->file->data ) );
	this->shader_module = H_new_shader_module( current_os_machine->device, module_info );
	this->stage_info = H_create_struct_pipeline_shader_stage(
		to( VkShaderStageFlagBits, this->form->shader_stage->type ),
		this->shader_module,
		"main",
		null // special constants
	);
	//
	#ifdef hept_trace
	print_trace( "new module: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

fn delete_module( in module in_module )
{
	vkDestroyShaderModule( current_os_machine->device, in_module->shader_module, null );
	unassign_module( in_module );
}

fn delete_all_modules()
{
	if( pile_module != null )
	{
		module this_module = null;
		u32 module_n = 0;
		u32 pile_size = pile_module->size;

		iter_pile( pile_module, t )
		{
			if( module_n >= pile_size ) skip;
			this_module = pile_find( pile_module, module, t );
			if( this_module == null ) next;
			else
				module_n++;
			//
			delete_module( this_module );
		}
	}
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
	print_error( in_form_shader == null, "shader_input: in_form_shader is null" );
	#endif
	//
	shader_input this = assign_shader_input();
	//

	list sizes = new_list( H_descriptor_pool_size );
	iter_list( in_form_shader->bindings, i )
	{
		form_shader_binding this_input = list_get( in_form_shader->bindings, form_shader_binding, i );
		list_add( sizes, H_descriptor_pool_size, create( H_descriptor_pool_size, .type = to( H_descriptor_type, this_input->type ), .descriptorCount = 3 ) );

		with( this_input->type )
		{
			is( shader_binding_type_storage )
			{
				/*if( this->storages == null )
				{
					this->storages = new_list( buffer );
					this->update_storages = new_list( struct( shader_input_update_storage ) );
				}
				list_add( this->storages, buffer, null );*/
				skip;
			}
			is( shader_binding_type_image )
			{
				/*if( this->images == null )
				{
					this->images = new_list( image );
					this->update_images = new_list( struct( shader_input_update_image ) );
				}
				list_add( this->images, image, null );*/
				skip;
			}
		}
	}

	H_struct_descriptor_pool pool_info = H_create_struct_descriptor_pool( 3, sizes->size, ( const ptr( VkDescriptorPoolSize ) )sizes->data );
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	this->descriptor_pool = H_new_descriptor_pool( current_os_machine->device, pool_info );

	H_struct_descriptor alloc_info = H_create_struct_descriptor(
		this->descriptor_pool,
		1,
		ref( in_form_shader->descriptor_layout )
	);

	this->descriptors[ 0 ] = H_new_descriptor( current_os_machine->device, alloc_info );
	this->descriptors[ 1 ] = H_new_descriptor( current_os_machine->device, alloc_info );
	this->descriptors[ 2 ] = H_new_descriptor( current_os_machine->device, alloc_info );
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
	if( pile_shader_input != null )
	{
		shader_input this_shader_input = null;
		u32 shader_input_n = 0;
		u32 pile_size = pile_shader_input->size;

		iter_pile( pile_shader_input, t )
		{
			if( shader_input_n >= pile_size ) skip;
			this_shader_input = pile_find( pile_shader_input, shader_input, t );
			if( this_shader_input == null ) next;
			else
				shader_input_n++;
			//
			delete_shader_input( this_shader_input );
		}
	}
}

global pile shader_input_storage_tickets = null;
global pile shader_input_image_tickets = null;

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
	if( in_shader_input->ticket_storage_id == -1 )
	{
		pile_add(
			shader_input_storage_tickets,
			struct( shader_input_storage_ticket ),
			create(
				struct( shader_input_storage_ticket ),
				in_shader_input,
				in_binding,
				in_buffer,
				0
			)
		);
		in_shader_input->ticket_storage_id = shader_input_storage_tickets->prev_pos;
	}
	else
	{
		ptr( struct( shader_input_storage_ticket ) ) ticket_ptr = ref( pile_find( shader_input_storage_tickets, struct( shader_input_storage_ticket ), in_shader_input->ticket_storage_id ) );
		ticket_ptr->binding = in_binding;
		ticket_ptr->buffer = in_buffer;
	}
	// unlock_shader_input(in_shader_input);
	// unlock_pile(shader_input_storage_tickets);
}

fn process_shader_input_storage_ticket( ptr( struct( shader_input_storage_ticket ) ) in_ticket )
{
	shader_input this_shader_input = in_ticket->input;

	// lock_shader_input(this_shader_input);

	if(
		( this_shader_input->descriptor_updates[ 0 ] == no ) and
		( this_shader_input->descriptor_updates[ 1 ] == no ) and
		( this_shader_input->descriptor_updates[ 2 ] == no )
	)
	{
		pile_delete( shader_input_storage_tickets, struct( shader_input_storage_ticket ), this_shader_input->ticket_storage_id );
		this_shader_input->ticket_storage_id = -1;
		// unlock_shader_input(this_shader_input);
		out;
	}

	ifn( this_shader_input->descriptor_updates[ in_ticket->frame ] )
	{
		// unlock_shader_input(this_shader_input);
		out;
	}
	if( in_ticket->buffer == null or in_ticket->buffer->buffer == null )
	{
		// unlock_shader_input(this_shader_input);
		out;
	}

	H_update_descriptor_set_storage( in_ticket->binding, current_os_machine->device, this_shader_input->descriptors[ in_ticket->frame ], in_ticket->buffer->buffer, in_ticket->buffer->size );
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
	if( in_shader_input->ticket_image_id == -1 )
	{
		pile_add(
			shader_input_image_tickets,
			struct( shader_input_image_ticket ),
			create(
				struct( shader_input_image_ticket ),
				in_shader_input,
				in_binding,
				in_image,
				0
			)
		);
		in_shader_input->ticket_image_id = shader_input_image_tickets->prev_pos;
	}
	else
	{
		ptr( struct( shader_input_image_ticket ) ) ticket_ptr = ref( pile_find( shader_input_image_tickets, struct( shader_input_image_ticket ), in_shader_input->ticket_image_id ) );
		ticket_ptr->binding = in_binding;
		ticket_ptr->image = in_image;
	}
	// unlock_shader_input(in_shader_input);
	// unlock_pile(shader_input_image_tickets);
}

fn process_shader_input_image_ticket( ptr( struct( shader_input_image_ticket ) ) in_ticket )
{
	shader_input this_shader_input = in_ticket->input;

	// lock_shader_input(this_shader_input);

	if(
		( this_shader_input->descriptor_updates[ 0 ] == no ) and
		( this_shader_input->descriptor_updates[ 1 ] == no ) and
		( this_shader_input->descriptor_updates[ 2 ] == no )
	)
	{
		pile_delete( shader_input_image_tickets, struct( shader_input_image_ticket ), this_shader_input->ticket_image_id );
		this_shader_input->ticket_image_id = -1;
		// unlock_shader_input(this_shader_input);
		out;
	}

	ifn( this_shader_input->descriptor_updates[ in_ticket->frame ] )
	{
		// unlock_shader_input(this_shader_input);
		out;
	}
	if( in_ticket->image == null or in_ticket->image->image == null )
	{
		// unlock_shader_input(this_shader_input);
		out;
	}

	H_update_descriptor_set_image( in_ticket->binding, current_os_machine->device, this_shader_input->descriptors[ in_ticket->frame ], in_ticket->image->form->sampler, in_ticket->image->view );
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
	this->stages = new_list( H_struct_pipeline_shader_stage );
	iter_list( this->modules, m )
	{
		module this_module = list_get( this->modules, module, m );
		if( this_module->form->shader_stage->type == shader_stage_type_vertex )
		{
			vert_form_mesh = this_module->form->mesh_form;
		}

		list_add( this->stages, H_struct_pipeline_shader_stage, this_module->stage_info );
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

	if( in_blend == null )
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
	this->pipeline_layout = H_new_pipeline_layout( current_os_machine->device, info_pipeline_layout );

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

	this->pipeline = H_new_render_pipeline( current_os_machine->device, pipeline_info );

	#ifdef hept_trace
	print_trace( "new shader: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

fn delete_shader( in shader in_shader )
{
	// copied stuff!!
	// if(in_shader->pipeline_layout != null) vkDestroyPipelineLayout(current_os_machine->device,in_shader->pipeline_layout,null);
	if( in_shader->pipeline != null ) vkDestroyPipeline( current_os_machine->device, in_shader->pipeline, null );
	// delete_list( in_shader->modules );
	// delete_list( in_shader->stages );
	unassign_shader( in_shader );
}

fn delete_all_shaders()
{
	if( pile_shader != null )
	{
		shader this_shader = null;
		u32 shader_n = 0;
		u32 pile_size = pile_shader->size;

		iter_pile( pile_shader, t )
		{
			if( shader_n >= pile_size ) skip;
			this_shader = pile_find( pile_shader, shader, t );
			if( this_shader == null ) next;
			else
				shader_n++;
			//
			delete_shader( this_shader );
		}
	}
}

inl shader copy_shader( in shader in_shader, in form_shader in_form, in form_frame in_form_frame, in list in_modules, in ptr( H_struct_pipeline_blend ) in_blend, in u32 in_constant_bytes )
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
		this->info_assembly = H_create_struct_pipeline_assembly( this->form->topology, no );
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
		this->stages = new_list( H_struct_pipeline_shader_stage );
		iter_list( this->modules, m )
		{
			list_add( this->stages, H_struct_pipeline_shader_stage, list_get( this->modules, module, m )->stage_info );
		}
	}

	this->constant_bytes = in_constant_bytes;
	if( ( this->constant_bytes != in_shader->constant_bytes ) or in_form != null )
	{
		H_struct_pipeline_layout info_pipeline_layout;
		H_struct_push_constant_range pushconst_range = H_create_struct_push_constant_range(
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			this->constant_bytes
		);
		info_pipeline_layout = H_create_struct_pipeline_layout( 1, ref( this->form->descriptor_layout ), ( this->constant_bytes > 0 ), ref( pushconst_range ) );
		this->pipeline_layout = H_new_pipeline_layout( current_os_machine->device, info_pipeline_layout );
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

	this->pipeline = H_new_render_pipeline( current_os_machine->device, pipeline_info );

	out this;
}

global shader default_shader_2d_tri = null;
global shader default_shader_2d_tri_tex = null;
global shader default_shader_2d_tri_tex_mul = null;
global shader default_shader_2d_line = null;
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

// commands

global H_command_buffer current_command_buffer = null;

/////// /////// /////// /////// /////// /////// ///////

// image commands

fn use_image_src( in image in_image, in flag in_preserve_contents )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		( in_preserve_contents ) ? ( in_image->layout ) : H_image_layout_undefined,
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

fn use_image_dst( in image in_image, in flag in_preserve_contents )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		0,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		( in_preserve_contents ) ? ( in_image->layout ) : H_image_layout_undefined,
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

fn use_image_dst_depth( in image in_image, in flag in_preserve_contents )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		0,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		( in_preserve_contents ) ? ( in_image->layout ) : H_image_layout_undefined,
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

fn use_image_blit_src( in image in_image, in flag in_preserve_contents )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		( in_preserve_contents ) ? ( in_image->layout ) : H_image_layout_undefined,
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

fn use_image_blit_dst( in image in_image, in flag in_preserve_contents )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		( in_preserve_contents ) ? ( in_image->layout ) : H_image_layout_undefined,
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

fn use_image_blit_dst_depth( in image in_image, in flag in_preserve_contents )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		( in_preserve_contents ) ? ( in_image->layout ) : H_image_layout_undefined,
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

fn use_image_present( in image in_image, in flag in_preserve_contents )
{
	H_image_barrier temp_barrier = H_create_image_barrier(
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		( in_preserve_contents ) ? ( in_image->layout ) : H_image_layout_undefined,
		H_image_layout_present_src_KHR,
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
	use_image_blit_dst( in_image, no );

	VkClearColorValue clearColor;
	clearColor.float32[ 0 ] = 0.0f;
	clearColor.float32[ 1 ] = 0.0f;
	clearColor.float32[ 2 ] = 0.0f;
	clearColor.float32[ 3 ] = 0.0f;

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

	set_current_shader_input( null );
}

fn use_shader_input( in shader_input in_shader_input )
{
	set_current_shader_input( in_shader_input );
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
	if( in_count == 0 ) out;
	if( in_mesh->vertex_buffer->buffer == null ) out;
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers( current_renderer->command_buffers[ current_renderer->current_frame ], 0, 1, ref( in_mesh->vertex_buffer->buffer ), offsets );
	// vkCmdBindIndexBuffer( current_renderer->command_buffers[ current_renderer->current_frame ], in_mesh->index_buffer->buffer, 0, VK_INDEX_TYPE_UINT32 );
	// vkCmdDrawIndexed( current_renderer->command_buffers[ current_renderer->current_frame ], in_mesh->indices->size, in_count, 0, 0, 0 );
	vkCmdDraw( current_renderer->command_buffers[ current_renderer->current_frame ], in_mesh->vertices->size, in_count, 0, 0 );
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

/////// /////// /////// /////// /////// /////// ///////

//

// audio
// -------
// does thing

HWAVEOUT hWaveOut; // Audio device handle
WAVEFORMATEX wfx;  // Audio format
global list sounds = null; // Linked list of playing sounds

void init_audio() {
	// Initialize WAVE format descriptor
	wfx.nSamplesPerSec = 44100; // sample rate
	wfx.wBitsPerSample = 16;   // bits per sample
	wfx.nChannels = 2;         // number of channels
	wfx.cbSize = 0;            // size of extra info
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	// Open audio device
	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
		fprintf(stderr, "Error opening audio device.\n");
		exit(EXIT_FAILURE);
	}
}

void cleanup_audio() {
	if (waveOutClose(hWaveOut) != MMSYSERR_NOERROR) {
		fprintf(stderr, "Error closing audio device.\n");
		// You might want to exit or handle the error here
	}
}


make_object(
	sound,
	WAVEHDR header;
	//struct Sound *next;
)(  )
{
	#ifdef hept_debug
	//print_error( in_type == event_type_null, "event: in_type is null" );
	#endif
	//
	sound this = assign_sound();
	//

	//
	#ifdef hept_trace
	print_trace( "new sound: ID: %d", this->pile_id );
	#endif
	//
	out this;
}

BOOL load_audio(const char *filename, BYTE **data, DWORD *length) {
	FILE *file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Error opening audio file: %s\n", filename);
		return FALSE;
	}

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Skip the WAV header (44 bytes)
	fseek(file, 44, SEEK_SET);
	*length = fileSize - 44;

	*data = (BYTE *)malloc(*length);
	if (!*data) {
		fprintf(stderr, "Error allocating memory for audio data.\n");
		fclose(file);
		return FALSE;
	}

	fread(*data, 1, *length, file);
	fclose(file);

	return TRUE;
}

void play_sound(const BYTE *data, DWORD length) {
	sound sound = new_ptr(struct(sound),1);

	// Set up the WAVEHDR
	sound->header.lpData = (LPSTR)data;
	sound->header.dwBufferLength = length;
	sound->header.dwFlags = 0;
	sound->header.dwLoops = 0;

	// Prepare the header
	if (waveOutPrepareHeader(hWaveOut, &sound->header, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
		fprintf(stderr, "Error preparing header.\n");
		free(sound);
		return;
	}

	// Play the sound
	if (waveOutWrite(hWaveOut, &sound->header, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
		fprintf(stderr, "Error playing sound.\n");
		waveOutUnprepareHeader(hWaveOut, &sound->header, sizeof(WAVEHDR));
		free(sound);
		return;
	}

	// Add the sound to the list of playing sounds
	// (You need to implement sound list management)
}

void stop_sound(in sound sound) {
	// Stop the sound
	if (waveOutReset(hWaveOut) != MMSYSERR_NOERROR) {
		fprintf(stderr, "Error stopping sound.\n");
	}

	// Unprepare the header
	if (waveOutUnprepareHeader(hWaveOut, &sound->header, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
		fprintf(stderr, "Error unpreparing header.\n");
	}

	// Remove the sound from the list of playing sounds
	// (You need to implement sound list management)

	// Free the sound structure
	free(sound);
}

/////// /////// /////// /////// /////// /////// ///////

//

global text main_path = null;
global text main_data_path = null;
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

global flag main_thread_ready = no;
global flag main_thread_go = no;
global flag main_thread_done = no;
global spinlock main_thread_lock = 0;
global os_thread main_thread = null;
global os_pacer main_thread_pacer = null;
global flag main_thread_exit = no;

make_struct( input )
{
	flag pressed, held, released;
};

global ptr( struct( input ) ) inputs;
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

inl s32 os_get_mouse_button( UINT u_msg )
{
	switch( u_msg )
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		return 511;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		return 510;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		return 509;
	default:
		return -1;
	}
}

inl LRESULT CALLBACK process_os_window( HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param )
{
	s32 button = os_get_mouse_button( u_msg );
	if( button == -1 ) button = w_param;

	with( u_msg )
	{
		is( WM_DESTROY )
		{
			safe_flag_set( hept_exit, yes );
			out 0;
		}

		//

		is( WM_SETCURSOR )
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

		is( WM_LBUTTONDOWN )
			is( WM_RBUTTONDOWN )
				is( WM_MBUTTONDOWN )
					is( WM_KEYDOWN )
						is( WM_SYSKEYDOWN )
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

		is( WM_LBUTTONUP )
			is( WM_RBUTTONUP )
				is( WM_MBUTTONUP )
					is( WM_KEYUP )
						is( WM_SYSKEYUP )
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

		is( WM_MOUSEMOVE )
		{
			safe_s32_set(
				main_mouse_window_x,
				( ( int )( short )LOWORD( l_param ) ) -
					( ( current_renderer->swapchain_extent.width - (main_window_width ) ) / 2 )
			);
			safe_s32_set(
				main_mouse_window_y,
				( ( int )( short )HIWORD( l_param ) ) -
					( ( current_renderer->swapchain_extent.height - (main_window_height ) ) / 2 )
			);
			out 1;
		}

		is( WM_MOUSEHWHEEL )
		{
			if(GET_WHEEL_DELTA_WPARAM(w_param) > 0)
			{
				safe_s32_set(main_mouse_wheel_x, 1);
			}
			else
			{
				safe_s32_set(main_mouse_wheel_x, -1);
			}
			out 1;
		}

		is( WM_MOUSEWHEEL )
		{
			if(GET_WHEEL_DELTA_WPARAM(w_param) > 0)
			{
				safe_s32_set(main_mouse_wheel_y, 1);
			}
			else
			{
				safe_s32_set(main_mouse_wheel_y, -1);
			}
			out 1;
		}

	default:
		out DefWindowProc( hwnd, u_msg, w_param, l_param );
	}

	out 0;
}
	#elif OS_LINUX

s32 map_key_for_x11( Display* in_disp, KeyCode keycode )
{
	// Convert the keycode to a keysym using XkbKeycodeToKeysym
	KeySym keysym = XkbKeycodeToKeysym( in_disp, keycode, 0, 0 );

	switch( keysym )
	{
	// Map alphanumeric keys
	case XK_A:
	case XK_a: return 'A';
	case XK_B:
	case XK_b: return 'B';
	case XK_C:
	case XK_c: return 'C';
	case XK_D:
	case XK_d: return 'D';
	case XK_E:
	case XK_e: return 'E';
	case XK_F:
	case XK_f: return 'F';
	case XK_G:
	case XK_g: return 'G';
	case XK_H:
	case XK_h: return 'H';
	case XK_I:
	case XK_i: return 'I';
	case XK_J:
	case XK_j: return 'J';
	case XK_K:
	case XK_k: return 'K';
	case XK_L:
	case XK_l: return 'L';
	case XK_M:
	case XK_m: return 'M';
	case XK_N:
	case XK_n: return 'N';
	case XK_O:
	case XK_o: return 'O';
	case XK_P:
	case XK_p: return 'P';
	case XK_Q:
	case XK_q: return 'Q';
	case XK_R:
	case XK_r: return 'R';
	case XK_S:
	case XK_s: return 'S';
	case XK_T:
	case XK_t: return 'T';
	case XK_U:
	case XK_u: return 'U';
	case XK_V:
	case XK_v: return 'V';
	case XK_W:
	case XK_w: return 'W';
	case XK_X:
	case XK_x: return 'X';
	case XK_Y:
	case XK_y: return 'Y';
	case XK_Z:
	case XK_z: return 'Z';

	// Map number keys
	case XK_0: return '0';
	case XK_1: return '1';
	case XK_2: return '2';
	case XK_3: return '3';
	case XK_4: return '4';
	case XK_5: return '5';
	case XK_6: return '6';
	case XK_7: return '7';
	case XK_8: return '8';
	case XK_9: return '9';

	default: return -1; // Unmapped key
	}
}

void process_os_window( Display* in_disp, Window in_win )
{
	XEvent e;
	u32 custom_key;

	while( XPending( in_disp ) )
	{
		XNextEvent( in_disp, &e );

		with( e.type )
		{
			is( DestroyNotify )
			{
				safe_flag_set( hept_exit, yes );
				out;
			}

			is( ButtonPress )
				is( ButtonRelease )
			{
				s32 button = -1;
				switch( e.xbutton.button )
				{
				case Button1: button = 511; break; // Left mouse button
				case Button2: button = 509; break; // Middle mouse button
				case Button3: button = 510; break; // Right mouse button
				}

				if( button != -1 )
				{
					if( e.type == ButtonPress )
					{
						if( safe_flag_get( inputs[ button ].held ) == no )
						{
							safe_flag_set( inputs[ button ].pressed, yes );
							safe_flag_set( inputs[ button ].released, no );
							safe_u16_set( input_updates[ safe_u8_get( input_update_ptr ) ], button );
							safe_u8_inc( input_update_ptr );
						}
						safe_flag_set( inputs[ button ].held, yes );
					}
					else if( e.type == ButtonRelease )
					{
						if( safe_flag_get( inputs[ button ].held ) )
						{
							safe_flag_set( inputs[ button ].held, no );
							safe_flag_set( inputs[ button ].pressed, no );
							safe_flag_set( inputs[ button ].released, yes );
							safe_u16_set( input_updates[ safe_u8_get( input_update_ptr ) ], button );
							safe_u8_inc( input_update_ptr );
						}
					}
				}
			}

			is( KeyPress )
				is( KeyRelease )
			{
				// Here you'll need a way to map X11 key codes to your custom keys.
				// Let's assume you have a function map_key_for_x11 that does this.
				s32 button = map_key_for_x11( in_disp, e.xkey.keycode ); // e.xkey.keycode;
				if( e.type == KeyPress )
				{
					if( safe_flag_get( inputs[ button ].held ) == no )
					{
						safe_flag_set( inputs[ button ].pressed, yes );
						safe_flag_set( inputs[ button ].released, no );
						safe_u16_set( input_updates[ safe_u8_get( input_update_ptr ) ], button );
						safe_u8_inc( input_update_ptr );
					}
					safe_flag_set( inputs[ button ].held, yes );
				}
				else if( e.type == KeyRelease )
				{
					if( safe_flag_get( inputs[ button ].held ) )
					{
						safe_flag_set( inputs[ button ].held, no );
						safe_flag_set( inputs[ button ].pressed, no );
						safe_flag_set( inputs[ button ].released, yes );
						safe_u16_set( input_updates[ safe_u8_get( input_update_ptr ) ], button );
						safe_u8_inc( input_update_ptr );
					}
				}
			}

			is( MotionNotify )
			{
				// Assuming you have similar variables for main_mouse_window_x and main_mouse_window_y
				safe_s32_set( main_mouse_window_x, e.xmotion.x );
				safe_s32_set( main_mouse_window_y, e.xmotion.y );
			}

		default: skip;
		}
	}
}

	#endif

//

/////// /////// /////// /////// /////// /////// ///////

/// update functions

//

fn update_os_machines()
{
	#ifdef hept_trace
	do_once print_trace( "updating machines" );
	#endif
	os_machine this_machine = null;
	u32 machine_n = 0;
	u32 pile_size = pile_os_machine->size;
	iter_pile( pile_os_machine, m )
	{
		if( machine_n >= pile_size ) skip;
		this_machine = pile_find( pile_os_machine, os_machine, m );
		if( this_machine == null ) next;
		else
			machine_n++;

		if( this_machine->gpu != null ) next;
		//
		this_machine->queue_index = u32_max;

		u32 gpu_count = H_get_gpus( current_os_core->instance, null );
		ptr( H_gpu ) gpus = new_ptr( H_gpu, gpu_count );
		H_get_gpus( current_os_core->instance, gpus );

		H_gpu integrated = null;
		iter( gpu_count, g )
		{
			H_gpu_properties gpu_prop = H_get_gpu_properties( gpus[ g ] );

			if( gpu_prop.deviceType == H_gpu_type_discrete or gpu_prop.deviceType == H_gpu_type_integrated )
			{
				u32 queue_count = H_get_gpu_queues( gpus[ g ], null );
				ptr( H_gpu_queue ) queues = new_ptr( H_gpu_queue, queue_count );
				H_get_gpu_queues( gpus[ g ], queues );

				iter( queue_count, q )
				{
					this_machine->queue_index = q;
					flag supports_present = H_get_gpu_supports_present( gpus[ g ], q, current_os_window->surface );

					if( supports_present and H_check_gpu_queue_render( queues[ q ] ) )
					{
						if( gpu_prop.deviceType == H_gpu_type_discrete )
						{
							this_machine->gpu = gpus[ g ];
	#ifdef hept_debug
							print_debug( "GPU name: %s", gpu_prop.deviceName );
	#endif
							skip;
						}
						elif( integrated == null )
						{
							integrated = gpus[ g ];
						}
					}
				}
				delete_ptr( queues );
			}

			if( this_machine->gpu != null )
			{
				skip;
			}
		}
		delete_ptr( gpus );

		if( this_machine->gpu == null )
		{
			this_machine->gpu = integrated;
	#ifdef hept_debug
			print_error( this_machine->gpu == null, "main_update_os_machines: could not find GPU" );
	#endif
		}

		f32 queue_priority = 1.0f;
		H_struct_device_queue device_queue = H_create_struct_device_queue( this_machine->queue_index, 1, ref( queue_priority ) );

		H_gpu_features features = H_get_gpu_features( this_machine->gpu );

		//

		text extensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_MEMORY_BUDGET_EXTENSION_NAME };
		u32 extension_count = 2;

		H_struct_device info_device = H_create_struct_device( 1, ref( device_queue ), extension_count, ( ptr( const char ) ptr( const ) )extensions, ref( features ) );

		this_machine->device = H_new_device( this_machine->gpu, info_device );

		this_machine->memory_properties = H_get_gpu_memory_properties( this_machine->gpu );

		//

		VkPhysicalDeviceMemoryBudgetPropertiesEXT memoryBudget = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT };
		VkPhysicalDeviceMemoryProperties2 memProperties2 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
			.pNext = &memoryBudget };
		vkGetPhysicalDeviceMemoryProperties2( this_machine->gpu, &memProperties2 );

		for( uint32_t i = 0; i < memProperties2.memoryProperties.memoryHeapCount; i++ )
		{
			printf( "Heap %d: Budget = %llu, Usage = %llu\n", i, memoryBudget.heapBudget[ i ], memoryBudget.heapUsage[ i ] );
		}
	}
}

//

fn update_os_windows()
{
	#ifdef hept_trace
	do_once print_trace( "updating windows" );
	#endif
	os_window this_window = null;
	u32 window_n = 0;
	u32 pile_size = pile_os_window->size;
	iter_pile( pile_os_window, w )
	{
		if( window_n >= pile_size ) skip;
		this_window = pile_find( pile_os_window, os_window, w );
		if( this_window == null ) next;
		else
			window_n++;

			//

			//

	#if OS_WINDOWS
		once MSG msg = { 0 };

		as( PeekMessage( ref( msg ), NULL, 0, 0, PM_REMOVE ) )
		{
			if( msg.message == 0x0012 )
			{
				safe_flag_set( hept_exit, yes );
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
			u32 formats_count = H_get_surface_formats( current_os_machine->gpu, this_window->surface, null );
			ptr( H_surface_format ) formats = new_ptr( H_surface_format, formats_count );
			H_get_surface_formats( current_os_machine->gpu, this_window->surface, formats );
			if( formats_count == 1 and formats[ 0 ].format == VK_FORMAT_UNDEFINED )
			{
				this_window->surface_format.format = H_format_bgra_u8_to_norm_f32;
				this_window->surface_format.colorSpace = H_colorspace_srgb_nonlinear;
			}
			else
			{
				this_window->surface_format = formats[ 0 ];
			}
			delete_ptr( formats );

			u32 modes_count = H_get_present_modes( current_os_machine->gpu, this_window->surface, null );
			ptr( H_present_mode ) present_modes = new_ptr( H_present_mode, modes_count );
			H_get_present_modes( current_os_machine->gpu, this_window->surface, present_modes );
			iter( modes_count, m )
			{
				if( present_modes[ m ] == main_vsync )
				{
					this_window->present_mode = main_vsync;
					skip;
				}
			}
			delete_ptr( present_modes );

			new_renderer( default_form_renderer, this_window, this_window->width, this_window->height );
			this_window->ready = yes;
		}
	}
}

//

fn update_renderers()
{
	#ifdef hept_trace
	do_once print_trace( "updating renderers" );
	#endif
	renderer this_renderer = null;
	u32 renderer_n = 0;
	u32 renderer_pile_size = pile_renderer->size;
	iter_pile( pile_renderer, r )
	{
		if( renderer_n >= renderer_pile_size ) skip;
		this_renderer = pile_find( pile_renderer, renderer, r );
		if( this_renderer == null ) next;
		else
			renderer_n++;

		set_current_renderer( this_renderer );

		//

		if( this_renderer->changed ) refresh_renderer( this_renderer );

		current_command_buffer = this_renderer->command_buffers[ this_renderer->current_frame ];

		//

		H_result aquire_result = vkAcquireNextImageKHR(
			current_os_machine->device,
			this_renderer->swapchain,
			UINT64_MAX,
			this_renderer->image_ready[ this_renderer->current_frame ],
			VK_NULL_HANDLE,
			ref( this_renderer->current_frame )
		);

		if( aquire_result == VK_ERROR_OUT_OF_DATE_KHR || aquire_result == VK_SUBOPTIMAL_KHR )
		{
			refresh_renderer( this_renderer );
			update_renderers();
			out;
		}

		//

		// lock_renderer( this_renderer );

		H_wait_fence( current_os_machine->device, this_renderer->flight_fences[ this_renderer->current_frame ] );

		//

		/*if( pile_event != null )
		{
			event this_event = null;
			u32 event_n = 0;
			u32 pile_size = pile_event->size;
			iter_pile( pile_event, e )
			{
				if( event_n >= pile_size ) skip;
				this_event = pile_find( pile_event, event, e );
				if( this_event == null ) next;
				else
					event_n++;
				//
				perform_event( this_event );
			}
		}*/

		//

		vacate_spinlock( main_thread_lock );
		sleep_ns( 1 );
		engage_spinlock( main_thread_lock );
		{
			// lock_pile(buffer_tickets);
			ptr( struct( buffer_ticket ) ) this_ticket = null;
			u32 ticket_n = 0;
			u32 pile_size = buffer_tickets->size;

			iter_pile( buffer_tickets, t )
			{
				if( ticket_n >= pile_size ) skip;
				this_ticket = ref( pile_find( buffer_tickets, struct( buffer_ticket ), t ) );
				if( this_ticket->buffer == null ) next;
				else
					ticket_n++;
				//
				process_buffer_ticket( this_ticket );
			}
			// unlock_pile(buffer_tickets);
		}

		//

		{
			// lock_pile(shader_input_storage_tickets);
			ptr( struct( shader_input_storage_ticket ) ) this_ticket = null;
			u32 ticket_n = 0;
			u32 pile_size = shader_input_storage_tickets->size;

			iter_pile( shader_input_storage_tickets, t )
			{
				if( ticket_n >= pile_size ) skip;
				this_ticket = ref( pile_find( shader_input_storage_tickets, struct( shader_input_storage_ticket ), t ) );
				if( this_ticket->input == null ) next;
				else
					ticket_n++;
				//
				this_ticket->frame = current_renderer->current_frame;
				process_shader_input_storage_ticket( this_ticket );
			}
			// unlock_pile(shader_input_storage_tickets);
		}

		{
			// lock_pile(shader_input_image_tickets);
			ptr( struct( shader_input_image_ticket ) ) this_ticket = null;
			u32 ticket_n = 0;
			u32 pile_size = shader_input_image_tickets->size;

			iter_pile( shader_input_image_tickets, t )
			{
				if( ticket_n >= pile_size ) skip;
				this_ticket = ref( pile_find( shader_input_image_tickets, struct( shader_input_image_ticket ), t ) );
				if( this_ticket->input == null ) next;
				else
					ticket_n++;
				//
				this_ticket->frame = current_renderer->current_frame;
				process_shader_input_image_ticket( this_ticket );
			}
			// unlock_pile(shader_input_image_tickets);
		}

		//

		iter_list( buffer_delete_tickets, b )
		{
			if( b >= buffer_delete_tickets->size ) skip;
			ptr( struct( buffer_delete_ticket ) ) this_ticket = ref( list_get( buffer_delete_tickets, struct( buffer_delete_ticket ), b ) );
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

		H_reset_fence( current_os_machine->device, this_renderer->flight_fences[ this_renderer->current_frame ] );

		//

		image this_window_image = list_get( this_renderer->frame_window->images, image, 0 );
		image this_window_depth = list_get( this_renderer->frame_window->images, image, 1 );

		frame this_frame = list_get( this_renderer->frames, frame, this_renderer->current_frame );
		image this_frame_image = list_get( this_frame->images, image, 0 );

		H_struct_command_buffer_start command_buffer_start_info = H_create_struct_command_buffer_start( null );
		command_buffer_start_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		H_start_command_buffer( current_command_buffer, command_buffer_start_info );

		set_current_frame( current_renderer->frame_window );
		// use_image_dst( this_window_image, no );
		// use_image_dst( this_window_depth, no );

		use_image_blit_dst_depth( this_window_depth, no );

		clear_image( this_window_image );
		clear_depth( this_window_depth );

		use_image_dst( this_window_image, no );
		use_image_dst_depth( this_window_depth, no );

		//
		/*if( safe_flag_get( main_thread_ready ) )
		{
			once u8 F = 0;
			if(F>=255) this_renderer->ref_window->call(); else F++;
		}*/

		this_renderer->ref_window->call();
		//

		{
			use_image_blit_src( this_window_image, no );
			use_image_blit_dst( this_frame_image, no );

			//

			f32 src_w = to_f32( main_width );
			f32 src_h = to_f32( main_height );
			main_window_height = this_renderer->swapchain_extent.height;
			main_scale = round_f32( main_window_height / src_h );

			main_window_width = src_w * main_scale;
			main_window_height = src_h * main_scale;

			f32 src_offset_x = 0;
			f32 src_offset_y = 0;

			if( main_window_width > this_renderer->swapchain_extent.width )
			{
				src_offset_x = ( main_window_width - this_renderer->swapchain_extent.width ) / ( 2 * main_scale );
				src_w -= 2 * src_offset_x;
				main_window_width = src_w * main_scale;
			}

			if( main_window_height > this_renderer->swapchain_extent.height )
			{
				src_offset_y = ( main_window_height - this_renderer->swapchain_extent.height ) / ( 2 * main_scale );
				src_h -= 2 * src_offset_y;
				main_window_height = src_h * main_scale;
			}

			src_h = floor_f32( main_window_height / main_scale );
			main_window_height = floor_f32( src_h * main_scale );
			src_w = floor_f32( main_window_width / main_scale );
			main_window_width = floor_f32( src_w * main_scale );

			f32 dst_offset_x = ( this_renderer->swapchain_extent.width - main_window_width ) / 2;
			f32 dst_offset_y = ( this_renderer->swapchain_extent.height - main_window_height ) / 2;

			VkImageBlit blit = { 0 };
			blit.srcOffsets[ 0 ] = ( H_offset_3d ){ src_offset_x, src_offset_y, 0 };
			blit.srcOffsets[ 1 ] = ( H_offset_3d ){ src_w + src_offset_x, src_h + src_offset_y, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = 0;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;

			blit.dstOffsets[ 0 ] = ( H_offset_3d ){ dst_offset_x, dst_offset_y, 0 };
			blit.dstOffsets[ 1 ] = ( H_offset_3d ){ dst_offset_x + main_window_width, dst_offset_y + main_window_height, 1 };
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

		H_end_command_buffer( current_command_buffer );

		//
		H_pipeline_stage wait_stages[] = { H_pipeline_stage_color_attachment_output };

		H_struct_submit submit_info = H_create_struct_submit(
			1,
			ref( this_renderer->image_ready[ this_renderer->current_frame ] ),
			wait_stages,
			1,
			ref( current_command_buffer ),
			1,
			ref( this_renderer->image_done[ this_renderer->current_frame ] )
		);

		H_submit_queue(
			this_renderer->form->queue,
			submit_info,
			this_renderer->flight_fences[ this_renderer->current_frame ]
		);

		//

		H_struct_present present_info = H_create_struct_present(
			1,
			ref( this_renderer->image_done[ this_renderer->current_frame ] ),
			1,
			ref( this_renderer->swapchain ),
			ref( this_renderer->current_frame )
		);

		H_result present_result = H_present( this_renderer->form->queue, present_info );

		// unlock_renderer( this_renderer );

		if( present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR )
		{
			refresh_renderer( this_renderer );
			update_renderers();
			out;
		}

		//

		this_renderer->current_frame = ( this_renderer->current_frame + 1 ) mod this_renderer->frames->size;
	}
}

//

fn main_core()
{
	H_init();
	//
	hept_object_piles = new_list( pile );
	//
	inputs = new_ptr( struct( input ), 512 );
	input_updates = new_ptr( u16, 256 );
	//
	buffer_tickets = new_pile( struct( buffer_ticket ) );
	buffer_delete_tickets = new_list( struct( buffer_delete_ticket ) );
	shader_input_storage_tickets = new_pile( struct( shader_input_storage_ticket ) );
	shader_input_image_tickets = new_pile( struct( shader_input_image_ticket ) );
	//
	new_os_core( main_creator_name );
	new_os_machine();
	new_os_window( main_app_name, main_width, main_height, main_fn_command );
	update_os_machines();
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
	// make_folder( main_sound_folder );
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
	default_form_image_rgba = new_form_image( image_type_rgba, H_format_bgra_u8_to_norm_f32 );
	default_form_image_depth = new_form_image( image_type_depth, VK_FORMAT_D32_SFLOAT );
	default_form_image_stencil = new_form_image( image_type_stencil, VK_FORMAT_S8_UINT );
	//
	default_form_frame_layer_rgba = new_form_frame_layer( frame_layer_type_rgba, H_format_bgra_u8_to_norm_f32 );
	default_form_frame_layer_depth = new_form_frame_layer( frame_layer_type_depth, VK_FORMAT_D32_SFLOAT );
	default_form_frame_layer_stencil = new_form_frame_layer( frame_layer_type_stencil, VK_FORMAT_S8_UINT );
	//
	{
		list layers = new_list( form_frame_layer );
		list_add( layers, form_frame_layer, default_form_frame_layer_rgba );
		default_form_frame = new_form_frame( frame_type_attachment, layers );
	}
	//
	{
		list layers = new_list( form_frame_layer );
		list_add( layers, form_frame_layer, default_form_frame_layer_rgba );
		list_add( layers, form_frame_layer, default_form_frame_layer_depth );
		default_form_frame_depth = new_form_frame( frame_type_depth, layers );
	}
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
	default_form_module_2d_tri_vert = new_form_module( default_form_shader_stage_vert, default_form_mesh_2d_tri );
	default_form_module_2d_tri_frag = new_form_module( default_form_shader_stage_frag, default_form_mesh_2d_tri );
	default_form_module_2d_tri_tex_vert = new_form_module( default_form_shader_stage_vert, default_form_mesh_2d_tri_tex );
	default_form_module_2d_tri_tex_frag = new_form_module( default_form_shader_stage_frag, default_form_mesh_2d_tri_tex );
	default_form_module_2d_line_vert = new_form_module( default_form_shader_stage_vert, default_form_mesh_2d_line );
	default_form_module_2d_line_frag = new_form_module( default_form_shader_stage_frag, default_form_mesh_2d_line );

	default_form_module_3d_tri_vert = new_form_module( default_form_shader_stage_vert, default_form_mesh_3d_tri );
	default_form_module_3d_tri_frag = new_form_module( default_form_shader_stage_frag, default_form_mesh_3d_tri );
	default_form_module_3d_tri_tex_vert = new_form_module( default_form_shader_stage_vert, default_form_mesh_3d_tri_tex );
	default_form_module_3d_tri_tex_frag = new_form_module( default_form_shader_stage_frag, default_form_mesh_3d_tri_tex );
	default_form_module_3d_line_vert = new_form_module( default_form_shader_stage_vert, default_form_mesh_3d_line );
	default_form_module_3d_line_frag = new_form_module( default_form_shader_stage_frag, default_form_mesh_3d_line );
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

	default_module_2d_tri_vert = new_module( default_form_module_2d_tri_vert, main_shader_default_path, "default_2d_tri" );
	default_module_2d_tri_frag = new_module( default_form_module_2d_tri_frag, main_shader_default_path, "default_2d_tri" );
	default_module_2d_tri_tex_vert = new_module( default_form_module_2d_tri_tex_vert, main_shader_default_path, "default_2d_tri_tex" );
	default_module_2d_tri_tex_frag = new_module( default_form_module_2d_tri_tex_frag, main_shader_default_path, "default_2d_tri_tex" );
	default_module_2d_line_vert = new_module( default_form_module_2d_line_vert, main_shader_default_path, "default_2d_line" );
	default_module_2d_line_frag = new_module( default_form_module_2d_line_frag, main_shader_default_path, "default_2d_line" );

	list tri_modules = new_list( module );
	list_add( tri_modules, module, default_module_2d_tri_vert );
	list_add( tri_modules, module, default_module_2d_tri_frag );
	default_shader_2d_tri = new_shader( default_form_shader_tri, default_form_frame_depth, tri_modules, null, 0 );

	list tri_tex_modules = new_list( module );
	list_add( tri_tex_modules, module, default_module_2d_tri_tex_vert );
	list_add( tri_tex_modules, module, default_module_2d_tri_tex_frag );
	default_shader_2d_tri_tex = new_shader( default_form_shader_tri_tex, default_form_frame_depth, tri_tex_modules, null, 0 );

	list line_modules = new_list( module );
	list_add( line_modules, module, default_module_2d_line_vert );
	list_add( line_modules, module, default_module_2d_line_frag );
	default_shader_2d_line = new_shader( default_form_shader_line, default_form_frame_depth, line_modules, null, 0 );
}

fn main_init();

fn update_inputs()
{
	// engage_spinlock(input_lock);

	main_mouse_wheel_x = 0;
	main_mouse_wheel_y = 0;

	as( safe_u8_get( input_update_ptr ) > 0 )
	{
		safe_u8_dec( input_update_ptr );
		safe_flag_set( inputs[ safe_u16_get( input_updates[ safe_u8_get( input_update_ptr ) ] ) ].pressed, no );
		safe_flag_set( inputs[ safe_u16_get( input_updates[ safe_u8_get( input_update_ptr ) ] ) ].released, no );
	}
	// vacate_spinlock(input_lock);
}

inl ptr( pure ) main_thread_loop( in ptr( pure ) in_ptr )
{
	main_thread_pacer = new_os_pacer( to_u32(main_fps) );
	loop
	{
		if( safe_flag_get( hept_exit ) ) skip;
		start_os_pacer( main_thread_pacer );


		once f32 T = 0;
		//T += 1./main_fps;

		//print_f32(T);


		//
		// if( safe_flag_get( main_thread_ready ) )
		//{
		engage_spinlock( main_thread_lock );
		//
		mouse_window_prev_x = mouse_window_x;
		mouse_window_prev_y = mouse_window_y;
		mouse_window_x = main_mouse_window_x;
		mouse_window_y = main_mouse_window_y;
		//
		//u64 T = get_ns();
		if( pile_event != null )
		{
			event this_event = null;
			u32 event_n = 0;
			u32 pile_size = pile_event->size;
			iter_pile( pile_event, e )
			{
				if( event_n >= pile_size ) skip;
				this_event = pile_find( pile_event, event, e );
				if( this_event == null ) next;
				else
					event_n++;
				//
				perform_event( this_event );
			}
		}
		//print( "%d\n", to_u32( 1000. / ( to_f64( get_ns() - T ) / to_f64( nano_per_milli ) ) ) );
		//
		update_inputs();
		//
		vacate_spinlock( main_thread_lock );
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
	engage_spinlock( main_thread_lock );
	main_thread = new_os_thread( main_thread_loop );
	loop
	{
		if( safe_flag_get( hept_exit ) ) out;
		update_os_windows();
		if( safe_flag_get( hept_exit ) ) out;
		update_renderers();
	}
}

fn main_exit()
{
	as( safe_flag_get( main_thread_exit ) == no ) sleep_ns( nano_per_milli );
	vkDeviceWaitIdle( current_os_machine->device );

	delete_all_shaders();
	delete_all_shader_inputs();
	delete_all_modules();
	delete_all_meshes();
	delete_all_renderers();
	delete_all_frames();
	delete_all_images();
	delete_all_buffers();

	delete_all_form_shaders();
	delete_all_form_shader_bindings();
	delete_all_form_modules();
	delete_all_form_shader_stages();
	delete_all_form_meshes();
	delete_all_form_mesh_attribs();
	delete_all_form_renderers();
	delete_all_form_frames();
	delete_all_form_frame_layers();
	delete_all_form_images();
	delete_all_form_buffers();

	delete_all_os_windows();
	delete_all_os_machines();
	delete_all_os_cores();
	delete_all_os_threads();
	delete_all_os_pacers();
	delete_all_os_files();

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