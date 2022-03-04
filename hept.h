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
fn constexpr lerp( const T& a, const T& b, const T& s )
{
	return ( ( 1 - s ) * a + s * b );
}

template<typename T>
fn constexpr lerpinv( const T& a, const T& b, const T& v )
{
	return ( ( v - a ) / ( b - a ) );
}

template<typename T>
fn constexpr lerpex( const T& a, const T& b, const T& s )
{
	return ( pow( a, 1 - s ) * pow( b, s ) );
}

template<typename T>
fn constexpr lerpinvex( const T& a, const T& b, const T& s )
{
	return ( log( a / s ) / log( a / b ) );
}

template<typename T>
fn constexpr remap( const T& a1, const T& b1, const T& a2, const T& b2, const T& v )
{
	return ( lerp( a2, b2, lerpinv( a1, b1, v ) ) );
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
//global renptr ren_current = null;
//global texptr tex_current = null;

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

global GLuint prog_comp;
global const char* comp_src;
global GLuint comp_tex;
global list<rgba> comp_data;

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

// GL

#define glsl GLuint
#define glsl_vert GL_VERTEX_SHADER
#define glsl_frag GL_FRAGMENT_SHADER
#define glsl_comp GL_COMPUTE_SHADER

#define gpu GLuint

struct vert
{
		GLfloat x, y, u, v;

		vert( GLfloat x, GLfloat y, GLfloat u, GLfloat v ) :
				x{ x }, y{ y }, u{ u }, v{ v } {};
};
#define vert_list list<vert>

struct vert_data
{
		GLuint id, size;

		vert_data( GLuint id = 0, GLuint size = 0 ) :
				id{ id }, size{ size } {};
};

#define tri_list list<GLuint>

#define tex GLuint
#define tex_id GLuint

//

global gpu gpu_current = 0;
global vert_data vert_data_current;

//

global gpu win_gpu = 0;
global glsl win_glsl_vert = 0;
global glsl win_glsl_frag = 0;
global vert_list win_verts = {
	vert( -1., -1., 0., 1. ),
	vert( -1., 1., 0., 0. ),
	vert( 1., 1., 1., 0. ),
	vert( 1., -1., 1., 1. ),
};
global tri_list win_tris = {
	0, 1, 2,
	0, 2, 3 };
global tex win_tex = 0;
global u16 win_tex_w = 0;
global u16 win_tex_h = 0;
global vert_data win_vert_data;

global gpu obj_gpu = 0;
global glsl obj_glsl_comp = 0;

//

global const str _glsl_comp_define = "#version 460 core\n"
																		 "#define get imageLoad\n"
																		 "#define set imageStore\n"
																		 "precision lowp float;"
																		 "const int N = int(gl_GlobalInvocationID.x);";

fn _glsl_new_raw( GLenum type, const char* sc )
{
	glsl g = glCreateShader( type );

	glShaderSource( g, 1, &sc, null );
	glCompileShader( g );

	int sc_n = 1000;
	char sc_t[ 1000 ];
	glGetShaderInfoLog( g, 1000, &sc_n, sc_t );
	print( sc_t );

	return g;
}
fn _glsl_new( GLenum type, const str& file )
{
	str s = file_load( file );

	switch( type )
	{
		case( glsl_comp ) {
			s = _glsl_comp_define + s;
			break;
		} default: break;
	}

	const char* sc = s.c_str();
	return _glsl_new_raw( type, sc );
}
#define glsl_new( type, file ) _glsl_new( type, file )
#define glsl_new_raw( type, src ) _glsl_new_raw( type, src )

fn _gpu_new( glsl in_glsl1 = 0, glsl in_glsl2 = 0, glsl in_glsl3 = 0, glsl in_glsl4 = 0, glsl in_glsl5 = 0, glsl in_glsl6 = 0, glsl in_glsl7 = 0 )
{
	gpu g = glCreateProgram();

	if( in_glsl1 != 0 ) glAttachShader( g, in_glsl1 );
	if( in_glsl2 != 0 ) glAttachShader( g, in_glsl2 );
	if( in_glsl3 != 0 ) glAttachShader( g, in_glsl3 );
	if( in_glsl4 != 0 ) glAttachShader( g, in_glsl4 );
	if( in_glsl5 != 0 ) glAttachShader( g, in_glsl5 );
	if( in_glsl6 != 0 ) glAttachShader( g, in_glsl6 );
	if( in_glsl7 != 0 ) glAttachShader( g, in_glsl7 );
	glLinkProgram( g );

	return g;
}
#define gpu_new( ... ) _gpu_new( __VA_ARGS__ )

#define gpu_set( gpu )   \
	do {                   \
		glUseProgram( gpu ); \
		gpu_current = gpu;   \
	} while( false )

#define gpu_uni_int( name, val ) glUniform1i( glGetUniformLocation( gpu_current, name ), val )
#define gpu_uni_float( name, val ) glUniform1f( glGetUniformLocation( gpu_current, name ), val )

fn gpu_print_errors()
{
	GLenum err( glGetError() );
	while( err != GL_NO_ERROR )
	{
		switch( err )
		{
			default:
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
}

fn _vert_data_new( const vert_list& v, const tri_list& t )
{
	GLuint va, vb, eb;
	glCreateVertexArrays( 1, &va );
	glCreateBuffers( 1, &vb );
	glCreateBuffers( 1, &eb );

	glNamedBufferData( vb, v.size() * sizeof( vert ), v.data(), GL_STATIC_DRAW );
	glNamedBufferData( eb, t.size() * sizeof( GLuint ), t.data(), GL_STATIC_DRAW );

	glEnableVertexArrayAttrib( va, 0 );
	glVertexArrayAttribBinding( va, 0, 0 );
	glVertexArrayAttribFormat( va, 0, 2, GL_FLOAT, GL_FALSE, 0 );

	glEnableVertexArrayAttrib( va, 1 );
	glVertexArrayAttribBinding( va, 1, 0 );
	glVertexArrayAttribFormat( va, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ) );

	glVertexArrayVertexBuffer( va, 0, vb, 0, 4 * sizeof( GLfloat ) );
	glVertexArrayElementBuffer( va, eb );

	return vert_data{ va, GLuint( t.size() ) };
}
#define vert_data_new( v, t ) _vert_data_new( v, t )

#define gpu_bind_vert_data( vd ) \
	do {                           \
		vert_data_current = vd;      \
		glBindVertexArray( vd.id );  \
	} while( false )

#define gpu_draw_tris() glDrawElements( GL_TRIANGLES, vert_data_current.size, GL_UNSIGNED_INT, 0 )

#define gpu_compute( x, y, z )                                  \
	do {                                                          \
		glDispatchCompute( max( 1, x ), max( 1, y ), max( 1, z ) ); \
		glMemoryBarrier( GL_ALL_BARRIER_BITS );                     \
	} while( false )

fn _tex_new( u16 w = 1, u16 h = 1 )
{
	tex t;
	glCreateTextures( GL_TEXTURE_2D, 1, &t );
	glTextureParameteri( t, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTextureParameteri( t, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTextureParameteri( t, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTextureParameteri( t, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTextureParameteri( t, GL_TEXTURE_COMPARE_MODE, GL_NONE );
	glTextureParameteri( t, GL_TEXTURE_COMPARE_FUNC, GL_NEVER );
	glTextureStorage2D( t, 1, GL_RGBA8, GLsizei( w ), GLsizei( h ) );

	std::vector<rgba> pixels;
	for( u32 y = 0; y < h; y++ )
	{
		for( u32 x = 0; x < w; x++ )
		{
			pixels.push_back( rgba{} );
		}
	}
	glTextureSubImage2D( t, 0, 0, 0, GLsizei( w ), GLsizei( h ), GL_RGBA, GL_UNSIGNED_BYTE, pixels.data() );
	glBindImageTexture( 0, t, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI );
	return t;
}
#define tex_new( ... ) _tex_new( __VA_ARGS__ )

#define tex_bind( t, id )       \
	do {                          \
		glBindTextureUnit( id, t ); \
	} while( false )

#define tex_set( t, x, y, w, h, p ) glTextureSubImage2D( t, 0, GLint( x ), GLint( y ), GLsizei( w ), GLsizei( h ), GL_RGBA, GL_UNSIGNED_BYTE, p )

//

fn _win_update()
{
	int get_win_x, get_win_y;
	int get_win_w, get_win_h;
	SDL_GetWindowPosition( win_current, &get_win_x, &get_win_y );
	SDL_GetWindowSize( win_current, &get_win_w, &get_win_h );
	win_x = get_win_x;
	win_y = get_win_y;
	win_w = get_win_w;
	win_h = get_win_h;
}

fn _dis_update()
{
	SDL_DisplayMode get_dis_mode;
	SDL_GetCurrentDisplayMode( SDL_GetWindowDisplayIndex( win_current ), &get_dis_mode );
	dis_w = get_dis_mode.w;
	dis_h = get_dis_mode.h;
	dis_fps = get_dis_mode.refresh_rate;
}

//

fn ren_rgba( ref( rgba ) c = {} )
{
	rgba_current = c;
	//SDL_SetRenderDrawColor( ren_current, rgba_current.r, rgba_current.g, rgba_current.b, rgba_current.a );
}

fn ren_fill( ref( rgba ) c = {} )
{
	//ren_rgba( c );
	//SDL_RenderClear( ren_current );
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
		//characterRGBA( ren_current, X += 8, Y, c, rgba_current.r, rgba_current.g, rgba_current.b, rgba_current.a );
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

	//
	const int win_scale = 3;
	win_tex_w = w;
win_tex_h = h;

	win_current = SDL_CreateWindow( name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, win_tex_w * win_scale, win_tex_h * win_scale, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN );

	_win_update();
	_dis_update();

	//

	SDL_GL_CreateContext( win_current );
	SDL_GL_SetSwapInterval( -1 );

	glewExperimental = GL_TRUE;
	glewInit();
	glViewport( 0, 0, win_tex_w * win_scale, win_tex_h * win_scale );

	// window gpu

	win_vert_data = vert_data_new( win_verts, win_tris );

	win_tex = tex_new( win_tex_w, win_tex_h );
	tex_bind(win_tex,0);

	win_glsl_vert = glsl_new( glsl_vert, "glsl/win_vert.glsl" );
	win_glsl_frag = glsl_new( glsl_frag, "glsl/win_frag.glsl" );
	win_gpu = gpu_new( win_glsl_vert, win_glsl_frag );
/*
	obj_glsl_comp = glsl_new(glsl_comp, obj_comp.glsl);
	obj_gpu = gpu_new( obj_glsl_comp );*/

	gpu_print_errors();
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
	static int main_loop_n = 0;
	static timer TICK = timer_now();

	//

	std::vector<rgba> pixels;
	for( u32 y = 0; y < win_h; y++ )
	{
		for( u32 x = 0; x < win_w; x++ )
		{
			pixels.push_back( rgba{} );
		}
	}

	tex_set( win_tex, 0, 0, win_tex_w, win_tex_h, pixels.data() );

	loop()
	{
		_win_update();
		_dis_update();

		main_input();

		if( KEY[ SDL_SCANCODE_ESCAPE ] > 0 ) break;

		//

		if( draw_fnptr != null ) draw_fnptr();

		//

		gpu_set( win_gpu );
		gpu_uni_int( "in_tex", 0 );

		gpu_bind_vert_data( win_vert_data );
		gpu_draw_tris();

		//

		SDL_GL_SwapWindow( win_current );
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