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

/// HEPT template

	#define HEPT( NAME, ... )                                            \
		make_struct{                                                       \
				__VA_ARGS__ } struct_hept_##NAME;                              \
		make_ptr( struct_hept_##NAME ) hept_##NAME;                        \
		global pile pile_hept_##NAME = null;                               \
                                                                       \
		hept_##NAME new_hept_##NAME()                                      \
		{                                                                  \
			hept_##NAME temp_hept_##NAME = new_mem( struct_hept_##NAME, 1 ); \
			pile_add( pile_hept_##NAME, hept_##NAME, temp_hept_##NAME );     \
			out temp_hept_##NAME;                                            \
		}                                                                  \
		hept_##NAME make_hept_##NAME

//

fn LRESULT CALLBACK process_hept_window( in HWND hwnd, in UINT u_msg, in WPARAM w_param, in LPARAM l_param );

HEPT(
		window,
		text name;
		u32 width, height;
		HWND hwnd;
		HINSTANCE inst;
		h_surface surface;
		h_surface_capabilities surface_capabilities;
		h_surface_format surface_format;
		h_present_mode present_mode; )
( in text in_name, in u32 in_width, in u32 in_height, in flag is_borderless )
{
	hept_window temp_window = new_hept_window();
	temp_window->name = in_name;
	temp_window->width = in_width;
	temp_window->height = in_height;
	//
	#if OS_WINDOWS
	HWND hwnd = GetConsoleWindow();
	DWORD window_style = is_borderless ? WS_POPUP : WS_OVERLAPPEDWINDOW;
	RECT rect = { 0, 0, temp_window->width, temp_window->height };
	AdjustWindowRect( &rect, window_style, no );
	s32 temp_width = rect.right - rect.left, temp_height = rect.bottom - rect.top;

	WNDCLASS wc = {
			.lpfnWndProc = process_hept_window,
			.hInstance = GetModuleHandle( null ),
			.hbrBackground = CreateSolidBrush( RGB( 0, 0, 0 ) ),
			.lpszClassName = temp_window->name };

	ifn( RegisterClass( &wc ) )
	{
		print( "COULD NOT MAKE WINDOW\n" );
		out null;
	}

	temp_window->hwnd = CreateWindowEx(
			0,
			wc.lpszClassName,
			wc.lpszClassName,
			window_style,
			( GetSystemMetrics( SM_CXSCREEN ) - temp_width ) / 2,
			( GetSystemMetrics( SM_CYSCREEN ) - temp_height ) / 2,
			temp_width,
			temp_height,
			null,
			null,
			wc.hInstance,
			null );
	#elif OS_LINUX
	//
	#endif
	//
	out temp_window;
}

fn pure show_hept_window( in hept_window in_window )
{
	#if OS_WINDOWS
	ShowWindow( in_window->hwnd, SW_SHOWDEFAULT );
	UpdateWindow( in_window->hwnd );
	#elif OS_LINUX
	//
	#elif OS_MACOS
	//
	#endif
}

fn flag update_hept_window( in hept_window in_window )
{
	#if OS_WINDOWS
	once MSG msg = { 0 };
	once flag visible = no, reveal = no;
	while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
	{
		if( msg.message == WM_QUIT )
		{
			out no;
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
	ifn( visible )
	{
		if( reveal )
		{
			show_hept_window( in_window );
			visible = yes;
		}
		else reveal = yes;
	}
	out yes;
}

//

HEPT(
		machine,
		text name;
		u32 queue_family_index;
		list windows;
		h_instance instance;
		h_physical_device physical_device;
		h_device device; )
( in text name, in list windows )
{
	hept_machine temp_machine = new_hept_machine();
	temp_machine->name = name;
	temp_machine->windows = windows;
	//

	//
	out temp_machine;
}

//

/////// /////// /////// /////// /////// /////// ///////

/// FORM

	#define FORM( NAME, ... )                                            \
		make_struct                                                        \
		{                                                                  \
			hept_machine machine;                                            \
			__VA_ARGS__                                                      \
		}                                                                  \
		struct_form_##NAME;                                                \
		make_ptr( struct_form_##NAME ) form_##NAME;                        \
		global pile pile_form_##NAME = null;                               \
                                                                       \
		form_##NAME new_form_##NAME( in hept_machine in_machine )          \
		{                                                                  \
			form_##NAME temp_form_##NAME = new_mem( struct_form_##NAME, 1 ); \
			temp_form_##NAME->machine = in_machine;                          \
			pile_add( pile_form_##NAME, form_##NAME, temp_form_##NAME );     \
			out temp_form_##NAME;                                            \
		}

//

/////// /////// /////// /////// /////// /////// ///////

/// OBJECT

	#define OBJECT( NAME, IN_FORM, ... )                \
		make_struct                                       \
		{                                                 \
			IN_FORM form;                                   \
			__VA_ARGS__                                     \
		}                                                 \
		struct_##NAME;                                    \
		make_ptr( struct_##NAME ) NAME;                   \
		global pile pile_##NAME = null;                   \
                                                      \
		NAME new_##NAME( in IN_FORM in_form )             \
		{                                                 \
			NAME temp_##NAME = new_mem( struct_##NAME, 1 ); \
			temp_##NAME->form = in_form;                    \
			pile_add( pile_##NAME, NAME, temp_##NAME );     \
			out temp_##NAME;                                \
		}

//

/////// /////// /////// /////// /////// /////// ///////

/// FN

	#define FN( OBJECT, NAME, ... )                                                      \
		make_fn( ptr_##OBJECT##_##NAME, ( OBJECT OBJECT_##OBJECT, __VA_ARGS__ ), OBJECT ); \
		OBJECT fn_##OBJECT##_##NAME( OBJECT OBJECT_##OBJECT, __VA_ARGS__ );                \
		ptr_##OBJECT##_##NAME OBJECT##_##NAME = fn_##OBJECT##_##NAME;                      \
		OBJECT fn_##OBJECT##_##NAME( OBJECT OBJECT_##OBJECT, __VA_ARGS__ )

//

/////// /////// /////// /////// /////// /////// ///////

/// main

fn pure main_init();
fn pure main_draw();
fn pure main_step();

fn LRESULT CALLBACK process_hept_window( in HWND hwnd, in UINT u_msg, in WPARAM w_param, in LPARAM l_param )
{
	if( u_msg == WM_DESTROY )
	{
		PostQuitMessage( 0 );
		out 0;
	}
	else
	{
		/*switch( u_msg )
		{
			case WM_SIZE:
				//renderer_update(default_renderer); // recreate swapchain
				//this_fence = 0;
				//main_draw();
				//resizing = false;
				return 1;
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				//key_state[ w_param ] = true;
				break;
			case WM_KEYUP:
			case WM_SYSKEYUP:
				//key_state[ w_param ] = false;
				break;
		}*/
	}
	out DefWindowProc( hwnd, u_msg, w_param, l_param );
}

//

global hept_window main_hept_window = null;
global hept_machine main_hept_machine = null;

fn pure main_piles()
{
	pile_hept_window = new_pile( hept_window );
	pile_hept_machine = new_pile( hept_machine );
	//
	//pile_form_buffer = new_pile( form_buffer );
}

fn pure main_core()
{
	list windows = new_list( hept_window );
	main_hept_window = make_hept_window( "hept_app", 512, 256, no );
	list_add( windows, hept_window, main_hept_window );

	main_hept_machine = make_hept_machine( "hept", windows );

	//
	loop
	{
		flag continue_loop = update_hept_window( main_hept_window );
	}
}

//

	#undef init
	#define init      \
		int main()      \
		{               \
			main_piles(); \
			main_init();  \
			main_core();  \
			out 0;        \
		}               \
		fn pure main_init()

	#define draw

#endif
