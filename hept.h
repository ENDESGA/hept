#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>

#include <glm/glm.hpp>

#include <GL/glew.h>

using namespace glm;

//

// MACRO MAGIC
#pragma region /////// /////// /////// /////// /////// /////// /////// MACRO MAGIC

#define EVAL0( ... ) __VA_ARGS__
#define EVAL1( ... ) EVAL0( EVAL0( EVAL0( __VA_ARGS__ ) ) )
#define EVAL2( ... ) EVAL1( EVAL1( EVAL1( __VA_ARGS__ ) ) )
#define EVAL3( ... ) EVAL2( EVAL2( EVAL2( __VA_ARGS__ ) ) )
#define EVAL4( ... ) EVAL3( EVAL3( EVAL3( __VA_ARGS__ ) ) )
#define EVAL( ... ) EVAL4( EVAL4( EVAL4( __VA_ARGS__ ) ) )

#define EACH_END( ... )
#define EACH_OUT
#define EACH_COMMA ,

#define EACH_GET_END2() 0, EACH_END
#define EACH_GET_END1( ... ) EACH_GET_END2
#define EACH_GET_END( ... ) EACH_GET_END1
#define EACH_NEXT0( test, next, ... ) next EACH_OUT

#define EACH_NEXT1( test, next ) EACH_NEXT0( test, next, 0 )
#define EACH_NEXT( test, next ) EACH_NEXT1( EACH_GET_END test, next )
#define EACH0( f, x, peek, ... ) f( x ) EACH_NEXT( peek, EACH1 )( f, peek, __VA_ARGS__ )
#define EACH1( f, x, peek, ... ) f( x ) EACH_NEXT( peek, EACH0 )( f, peek, __VA_ARGS__ )
#define EACH( f, ... ) EVAL( EACH1( f, __VA_ARGS__, ()()(), ()()(), ()()(), 0 ) )

#define EACH_LIST_NEXT1( test, next ) EACH_NEXT0( test, EACH_COMMA next, 0 )
#define EACH_LIST_NEXT( test, next ) EACH_LIST_NEXT1( EACH_GET_END test, next )
#define EACH_LIST0( f, x, peek, ... ) f( x ) EACH_LIST_NEXT( peek, EACH_LIST1 )( f, peek, __VA_ARGS__ )
#define EACH_LIST1( f, x, peek, ... ) f( x ) EACH_LIST_NEXT( peek, EACH_LIST0 )( f, peek, __VA_ARGS__ )
#define EACH_LIST( f, ... ) EVAL( EACH_LIST1( f, __VA_ARGS__, ()()(), ()()(), ()()(), 0 ) )

#define EACH_SYMBOL_NEXT1( test, symbol, next ) EACH_NEXT0( test, symbol next, 0 )
#define EACH_SYMBOL_NEXT( test, symbol, next ) EACH_SYMBOL_NEXT1( EACH_GET_END test, symbol, next )
#define EACH_SYMBOL0( f, symbol, x, peek, ... ) f( x ) EACH_SYMBOL_NEXT( peek, symbol, EACH_SYMBOL1 )( f, symbol, peek, __VA_ARGS__ )
#define EACH_SYMBOL1( f, symbol, x, peek, ... ) f( x ) EACH_SYMBOL_NEXT( peek, symbol, EACH_SYMBOL0 )( f, symbol, peek, __VA_ARGS__ )
#define EACH_SYMBOL( f, symbol, ... ) EVAL( EACH_SYMBOL1( f, symbol, __VA_ARGS__, ()()(), ()()(), ()()(), 0 ) )

//

#define ELEVENTH_ARGUMENT( a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, ... ) a11
#define ARG_N( ... ) ELEVENTH_ARGUMENT( dummy, ##__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 )

#pragma endregion

//

#define global inline static
#define null nullptr
#define obj struct

#define _PRINT( x ) << x
#define print( ... ) std::cout __VA_OPT__( EACH( _PRINT, __VA_ARGS__ ) ) << std::endl

#define to( n, v ) for( u32 v = 0; v++ <= n; )
#define from( n, v ) for( u32 v = n; v-- > 0; )
#define iter( n, v ) for( auto n: v )

#define _IF( x ) ( x )
#define if( ... ) if( EACH_SYMBOL( _IF, &&, __VA_ARGS__ ) )

#define fn inline auto
#define ref( ... ) const __VA_ARGS__&

#define operate( symbol, t ) fn operator symbol( ref( t ) other )
#define operate_self( symbol ) fn operator symbol( int )
#define convert( type ) operator type()

#define loop( ... ) for( __VA_OPT__( auto loop_x = 0 ); __VA_OPT__( loop_x++ > __VA_ARGS__ ); )

#define _CASE( x ) case x:
#define case( ... ) EACH( _CASE, __VA_ARGS__ )

//
/*
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
*/
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using str = std::string;

using winptr = SDL_Window*;
using renptr = SDL_Renderer*;
using texptr = SDL_Texture*;

#define list std::vector

using timer = std::chrono::system_clock::time_point;

#define timer_now() std::chrono::high_resolution_clock::now()
#define _timer_type( size ) size##seconds
#define timer_get( t, size ) std::chrono::duration_cast<std::chrono::_timer_type( size )>( timer_now() - ( t ) ).count()

#define TIMER_START TIMER_TICK = timer_now();
#define TIMER_END ( timer_get( TIMER_TICK ) / 1000000.00 )

//

template<typename T>
fn constexpr mix( const T& a, const T& b, const T& s )
{
	return ( ( 1 - s ) * a + s * b );
}

template<typename T>
fn constexpr mixinv( const T& a, const T& b, const T& v )
{
	return ( ( v - a ) / ( b - a ) );
}

template<typename T>
fn constexpr mixex( const T& a, const T& b, const T& s )
{
	return ( pow( a, 1 - s ) * pow( b, s ) );
}

template<typename T>
fn constexpr mixinvex( const T& a, const T& b, const T& s )
{
	return ( log( a / s ) / log( a / b ) );
}

template<typename T>
fn constexpr remap( const T& a1, const T& b1, const T& a2, const T& b2, const T& v )
{
	return ( mix( a2, b2, mixinv( a1, b1, v ) ) );
}

template<typename T>
fn constexpr abs( const T& v )
{
	return ( v < 0 ? -v : v );
}

template<typename... T>
fn constexpr avg( T&... args )
{
	return ( ... + args ) / ( sizeof...( args ) );
}

template<typename T>
fn constexpr sign( const T& v )
{
	return ( v < 0 ? -1 : ( v > 0 ? 1 : 0 ) );
}

typedef union rgba
{
		struct
		{
				u8 r = 0, g = 0, b = 0, a = 0;
		};
		struct
		{
				u32 col;
		};

		auto operator<=>( const rgba& ) const = default;

		rgba( const u8& r, const u8& g, const u8& b, const u8& a ) :
				r{ r }, g{ g }, b{ b }, a{ a } {}
		rgba( const u32& col ) :
				col{ col } {}
		rgba( const u8& n ) :
				r{ n }, g{ n }, b{ n }, a{ n } {}
		rgba() :
				r{ 0 }, g{ 0 }, b{ 0 }, a{ 0 } {}
		~rgba() {}
} rgba;
rgba rgba_current = { 255, 255, 255, 255 };

struct quad
{
		vec2 a, b;

		fn middle()
		{
			//return a + b;
		}
};

//

global winptr win_current = null;
global renptr ren_current = null;
global texptr tex_current = null;

global u16 win_x = 0;
global u16 win_y = 0;
global u16 win_w = 640;
global u16 win_h = 360;

global u16 dis_w = 0;
global u16 dis_h = 0;
global u16 dis_fps = 120;

global s32 MOUSE_X = 0, MOUSE_Y = 0,
					 MOUSE_X_PREV = 0, MOUSE_Y_PREV = 0,
					 MOUSE_X_DELTA = 0, MOUSE_Y_DELTA = 0;
global s8 MOUSE_WHEEL_V = 0, MOUSE_WHEEL_H = 0;

global list<u32> KEY( 256, 0 );
global list<u8> KEY_LIST;

global const char* comp_src;

//

#define file_create_in( p, f ) std::ifstream f( p, std::ios::in | std::ios::binary )
#define file_create_out( p, f ) std::ofstream f( p, std::ios::out | std::ios::binary )

str file_read( const std::filesystem::path& p )
{
	file_create_in( p, f );
	const long sz = (long)std::filesystem::file_size( p );
	str result( sz, '\0' );
	f.read( result.data(), sz );
	return result;
}

void file_write( const std::filesystem::path& p, const str& s )
{
	file_create_out( p, f );
	f.write( s.data(), (long)s.size() );
}

str file_load( str const& file )
{
	using it = std::istreambuf_iterator<char>;
	std::ifstream in( file );
	return { it( in.rdbuf() ), it() };
}

//

fn ren_rgba( ref( rgba ) c = {} )
{
	rgba_current = c;
	SDL_SetRenderDrawColor( ren_current, rgba_current.r, rgba_current.g, rgba_current.b, rgba_current.a );
}

fn ren_fill( ref( rgba ) c = {} )
{
	ren_rgba( c );
	SDL_RenderClear( ren_current );
}

fn ren_text( ref( int ) x = 0, ref( int ) y = 0, ref( str ) text = "" )
{
	int X = x;
	int Y = y;

	for( u8 c: text )
	{
		switch( c )
		{
			default: break;

			case('\n')
			{
				X = x;
				Y += 8;
				continue;
			}

			case('\t')
			{
				X += 16;
				continue;
			}
		}
		characterRGBA( ren_current, X += 8, Y, c, rgba_current.r, rgba_current.g, rgba_current.b, rgba_current.a );
	}
}

//

fn quit()
{
	SDL_Quit();
	exit(0);
}

//

obj obj_default
{
	virtual void step() {}
	virtual void draw() {}
};

//

fn main_init( ref( str ) name = "hept", ref( u16 ) w = 640, ref( u16 ) h = 360 )
{
	SDL_Init( SDL_INIT_EVERYTHING );

	SDL_SetHint( SDL_HINT_RENDER_BATCHING, "1" );
	SDL_SetHint( SDL_HINT_RENDER_LINE_METHOD, "3" );
	SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "0" );
	SDL_SetHint( SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING, "1" );
	SDL_SetHint( SDL_HINT_FRAMEBUFFER_ACCELERATION, "opengl" );
	SDL_SetHint( SDL_HINT_RENDER_DRIVER, "opengl" );
	SDL_SetHint( SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT, "1" );
	SDL_SetHint( SDL_HINT_VIDEO_DOUBLE_BUFFER, "1" );
	SDL_SetHint( SDL_HINT_RENDER_VSYNC, "1" );

	//
/*
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 1 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 1 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 1 );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0 );
	SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, 0 );
	
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 6 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	
	SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 0 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );*/

	//

	win_w = w;
	win_h = h;

	win_current = SDL_CreateWindow( name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, win_w, win_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN );

	ren_current = SDL_CreateRenderer( win_current, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_ACCELERATED );
	SDL_SetRenderDrawBlendMode( ren_current, SDL_BLENDMODE_BLEND );
	SDL_SetRenderDrawColor( ren_current, 255, 255, 255, 255 );

	tex_current = SDL_CreateTexture( ren_current, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, win_w, win_h );
	SDL_SetTextureBlendMode( tex_current, SDL_BLENDMODE_BLEND );
	SDL_SetRenderTarget( ren_current, tex_current );

	//
	
	SDL_GL_CreateContext( win_current );
	SDL_GL_SetSwapInterval( 0 );
	glewExperimental = GL_TRUE;
	glewInit();
	glViewport( 0, 0, win_w, win_h );

	GLuint tex;
	glCreateTextures( GL_TEXTURE_2D, 1, &tex );
	glTextureParameteri( tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTextureParameteri( tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTextureParameteri( tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTextureParameteri( tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTextureStorage2D( tex, 1, GL_RGBA8, win_w, win_h );
	std::vector<rgba> pixels, pixels2;
	for( u32 y = 0; y < win_h; y++ )
	{
	for( u32 x = 0; x < win_w; x++ )
	{
		pixels2.emplace_back(255,255,64,255 );//rgba{ u8( rand() ), u8( rand() ), u8( rand() ), 255 } );
	}
	}
	
	//pixels[7 + (7 * W)] = rgba{255,255,255,255};
	
	const void* pixels_ptr = pixels.data();
	glTextureSubImage2D( tex, 0, 0, 0, win_w, win_h, GL_RGBA, GL_UNSIGNED_BYTE, &(pixels2)[0] );
	glBindImageTexture( 0, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI );
	glBindTextureUnit( 0, tex );

for( u32 y = 0; y < win_h; y++ )
	{
	for( u32 x = 0; x < win_w; x++ )
	{
		pixels.emplace_back();
	}
	}

//

GLuint sh_comp = glCreateShader( GL_COMPUTE_SHADER );
str comp_src_load = file_load("comp.glsl");
comp_src = comp_src_load.c_str();

print(comp_src);

	glShaderSource( sh_comp, 1, &comp_src, null );
	glCompileShader( sh_comp );

int sh_compn = 1000;
	char sh_comp_text[ 1000 ];
	glGetShaderInfoLog( sh_comp, 1000, &sh_compn, sh_comp_text );
	print( sh_comp_text );

	GLint sh_comp_status;
	glGetShaderiv( sh_comp, GL_COMPILE_STATUS, &sh_comp_status );
	if( sh_comp_status == GL_FALSE ) { quit(); }

GLuint prog_comp = glCreateProgram();
	glAttachShader( prog_comp, sh_comp );
	glLinkProgram( prog_comp );

glUseProgram( prog_comp );
		glUniform1f( glGetUniformLocation( prog_comp, "in_uni" ), 7. );
		glUniform1i( glGetUniformLocation( prog_comp, "in_frame" ), 7 );
		glDispatchCompute( ceil( win_w / 32. ), ceil( win_h / 32. ), 1. );
		glMemoryBarrier( GL_ALL_BARRIER_BITS );

//

auto * data = new unsigned char[(win_w * win_h) * 4];
	glGetTextureImage(tex,0,GL_RGBA,GL_UNSIGNED_BYTE,win_w * win_h * 4,&(data)[0]);

SDL_UpdateTexture(tex_current,null,&(data)[0],win_w * 4);

///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GLenum err( glGetError() );
while( err != GL_NO_ERROR )
	{
		switch( err )
		{
			case GL_INVALID_OPERATION:
				print( "INVALID_OPERATION" );
				break;
			case GL_INVALID_ENUM:
				print( "INVALID_ENUM" );
				break;
			case GL_INVALID_VALUE:
				print( "INVALID_VALUE" );
				break;
			case GL_OUT_OF_MEMORY:
				print( "OUT_OF_MEMORY" );
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				print( "INVALID_FRAMEBUFFER_OPERATION" );
				break;
		}
		err = glGetError();
	}

	print( glGetError() );
	print( glewGetErrorString( glGetError() ) );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
}

global void ( *draw_fnptr )() = null;
#define draw( ... )             \
	global void draw_fn();        \
	auto _draw_fn_set = [] {draw_fnptr = &draw_fn; return null; }(); \
	global void draw_fn()

fn main_input()
{
	int get_mouse_x, get_mouse_y;
	SDL_GetGlobalMouseState( &get_mouse_x, &get_mouse_y );

	MOUSE_X_PREV = MOUSE_X;
	MOUSE_Y_PREV = MOUSE_Y;

	MOUSE_X = s32( get_mouse_x - win_x );
	MOUSE_Y = s32( get_mouse_y - win_y );

	MOUSE_X_DELTA = MOUSE_X_PREV - MOUSE_X;
	MOUSE_Y_DELTA = MOUSE_Y_PREV - MOUSE_Y;

	// reset wheel input
	MOUSE_WHEEL_V = 0;
	MOUSE_WHEEL_H = 0;

	iter( k, KEY_LIST )
	{
		KEY[ k ]++;
	}

	SDL_Event EVENT{};
	while( SDL_PollEvent( &EVENT ) )
	{
		const auto& in_k = u32( EVENT.key.keysym.scancode );
		const auto& in_m = u32( EVENT.button.button );
		const bool win_close = ( EVENT.window.event == SDL_WINDOWEVENT_CLOSE );

		switch( EVENT.type )
		{
			case SDL_WINDOWEVENT:
				{
					if( win_close ) quit();
				}
				break;

			case SDL_QUIT:
				{
					quit();
				}
				break;

			case SDL_KEYDOWN:
				{
					if( in_k < 256 )
					{
						KEY[ in_k ] = 1;
						KEY_LIST.push_back( in_k );
					}
				}
				break;

			case SDL_KEYUP:
				{
					if( in_k < 256 )
					{
						KEY[ in_k ] = 0;
						std::erase( KEY_LIST, in_k );
					}
				}
				break;

			case SDL_MOUSEWHEEL:
				{
					MOUSE_WHEEL_H = s8( EVENT.wheel.x );
					MOUSE_WHEEL_V = s8( EVENT.wheel.y );
				}
				break;
		}
	}
}

fn main_loop()
{
	loop()
	{
		int get_win_x, get_win_y;
		int get_win_w, get_win_h;
		SDL_GetWindowPosition( win_current, &get_win_x, &get_win_y );
		SDL_GetWindowSize( win_current, &get_win_w, &get_win_h );
		win_x = get_win_x;
		win_y = get_win_y;
		win_w = get_win_w;
		win_h = get_win_h;

		SDL_DisplayMode get_dis_mode;
		SDL_GetCurrentDisplayMode( SDL_GetWindowDisplayIndex( win_current ), &get_dis_mode );
		dis_w = get_dis_mode.w;
		dis_h = get_dis_mode.h;
		dis_fps = get_dis_mode.refresh_rate;

		main_input();

		if( KEY[ SDL_SCANCODE_ESCAPE ] > 0 ) break;

		SDL_SetRenderTarget( ren_current, tex_current );

		if( draw_fnptr != null ) draw_fnptr();

		SDL_SetRenderTarget( ren_current, null );
		SDL_RenderCopy( ren_current, tex_current, null, null );
		SDL_RenderPresent( ren_current );
	}
	quit();
}

void main_fn();

#define main( ... )                                                                      \
	int main()                                                                             \
	{                                                                                      \
                                                                                         \
		main_init( __VA_OPT__( __VA_ARGS__ ) );                                              \
                                                                                         \
		print( "| hept :::. v0b0a1" );                                                       \
		print( "|-------" );                                                                 \
		print( "- CPU:" );                                                                   \
		print( "|\tTHREADS: ", SDL_GetCPUCount() );                                          \
		print( "|\tCACHE: ", SDL_GetCPUCacheLineSize(), "MB" );                              \
		print( "|-------" );                                                                 \
		print( "- GPU:" );                                                                   \
		print( "|\tDRIVER: ", SDL_GetCurrentVideoDriver() );                                 \
		print( "|-------" );                                                                 \
		print( "- RAM:" );                                                                   \
		print( "|\tTOTAL: ", floor( ( float( SDL_GetSystemRAM() ) / 1000.f ) + .5 ), "GB" ); \
		print( "|-------" );                                                                 \
		print( "- OS:" );                                                                    \
		print( "|\tNAME: ", SDL_GetPlatform() );                                             \
		print( "|-------" );                                                                 \
		print( "- AUDIO:" );                                                                 \
		print( "|\tDRIVER: ", SDL_GetCurrentAudioDriver() );                                 \
		print( "|-------" );                                                                 \
		print( "- DISPLAY:" );                                                               \
		print( "|\tWIDTH: ", dis_w );                                                        \
		print( "|\tHEIGHT: ", dis_h );                                                       \
		print( "|\tFPS: ", dis_fps );                                                        \
		print( "|-------" );                                                                 \
		print( "- PRINT OUTPUT:" );                                                          \
                                                                                         \
		main_fn();                                                                           \
		main_loop();                                                                         \
                                                                                         \
		return 0;                                                                            \
	}                                                                                      \
	void main_fn()

//