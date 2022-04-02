#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

//#define to( v, n ) for( auto v = 0; v++ <= n; )
#define to( v, n ) for( uint v = 0; v < n; v++ )
#define from( n, v ) for( auto v = n; v-- > 0; )
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

#define fixed static

/*
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
*/

//using uint = uint32_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using str = std::string;

using winptr = SDL_Window*;
using renptr = SDL_Renderer*;
using texptr = SDL_Texture*;

#define fs std::filesystem

#define list std::vector
#define duo std::pair

using timer = std::chrono::system_clock::time_point;

#define timer_now() std::chrono::high_resolution_clock::now()
#define _timer_type( size ) size##seconds
#define timer_get( t, size ) std::chrono::duration_cast<std::chrono::_timer_type( size )>( timer_now() - ( t ) ).count()
#define timer_delta( t, d, size ) std::chrono::duration_cast<std::chrono::_timer_type( size )>( d - t ).count()

#define TIMER_START TIMER_TICK = timer_now();
#define TIMER_END ( timer_get( TIMER_TICK ) / 1000000.00 )

#define unique( ptr ) std::unique_ptr<ptr>
#define unique_new( ptr, ... ) std::make_unique<ptr>( __VA_ARGS__ )

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

global u16 win_x = 0;
global u16 win_y = 0;
global u16 win_w = 1;
global u16 win_h = 1;

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

#define file_watch( id, file )                              \
                                                            \
	static fs::file_time_type ftv##id;                        \
	fs::file_time_type lwt##id = fs::last_write_time( file ); \
	bool ftv_go##id = false;                                  \
	if( ftv##id != lwt##id )                                  \
	{                                                         \
		ftv##id = lwt##id;                                      \
		ftv_go##id = true;                                      \
	}                                                         \
	if( ftv_go##id )

str file_read( const fs::path& p )
{
	file_create_in( p, f );
	const long sz = (long)fs::file_size( p );
	str result( sz, '\0' );
	f.read( result.data(), sz );
	return result;
}

void file_write( const fs::path& p, const str& s )
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

// rand

uint _HEPT32_X = 0x77777777u, _HEPT32_Y = 0x77777777u, _HEPT32_Z = 0x77777777u;
uint _HEPT32_X_ACCUM = 0x77777777u, _HEPT32_Y_ACCUM = 0x77777777u, _HEPT32_Z_ACCUM = 0x77777777u;

fn hept32_seeded( uint x = 7, uint y = 7, uint z = 7 )
{
	x = ( x * _HEPT32_X ) - ( ~x * 0x77777777u ) - ~( x * _HEPT32_Y );
	y = ( y * _HEPT32_Y ) - ( ~y * 0x77777777u ) - ~( y * _HEPT32_Z );
	z = ( z * _HEPT32_Z ) - ( ~z * 0x77777777u ) - ~( z * _HEPT32_X );
	z = ~( ~( ~x * y * z ) * ~( x * ~y * z ) * ~( x * y * ~z ) );
	return z ^ ( z >> 16 );
}

fn hept32_seed( uint seed )
{
	// multi-layer seed expansion and extraction
	seed = ( hept32_seeded( seed, seed, seed ) * 0x77777777u ) - 0x77777777u;
	_HEPT32_X = ( ( hept32_seeded( 0x77777777u, seed, seed ) * seed ) - seed );
	_HEPT32_Y = ( ( hept32_seeded( seed, 0x77777777u, seed ) * seed ) - seed );
	_HEPT32_Z = ( ( hept32_seeded( seed, seed, 0x77777777u ) * seed ) - seed );

	_HEPT32_X_ACCUM = _HEPT32_X;
	_HEPT32_Y_ACCUM = _HEPT32_Y;
	_HEPT32_Z_ACCUM = _HEPT32_Z;
}

fn hept32()
{
	return hept32_seeded( _HEPT32_X_ACCUM++, _HEPT32_Y_ACCUM++, _HEPT32_Z_ACCUM++ );
}

#define rand_seed( x ) hept32_seed( x )
#define rand( ... ) hept32##__VA_OPT__( _seeded )( __VA_ARGS__ )

// GL

#define glsl GLuint
#define glsl_vert GL_VERTEX_SHADER
#define glsl_frag GL_FRAGMENT_SHADER
#define glsl_comp GL_COMPUTE_SHADER

#define glsl_comp_tex 32, 32
#define glsl_comp_max 1024

#define gpu GLuint

struct vert
{
		GLfloat x, y, z, u, v;

		vert( GLfloat x, GLfloat y, GLfloat z, GLfloat u, GLfloat v ) :
				x{ x }, y{ y }, z{ z }, u{ u }, v{ v } {};
};
#define vert_list list<vert>

struct vert_data
{
		GLuint id, size;

		vert_data( GLuint id = 0, GLuint size = 0 ) :
				id{ id }, size{ size } {};
};

#define vert_tri_list list<GLuint>

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
	vert( -1., -1., 0., 0., 1. ),
	vert( -1., 1., 0., 0., 0. ),
	vert( 1., 1., 0., 1., 0. ),
	vert( 1., -1., 0., 1., 1. ),
};
global vert_tri_list win_tris = {
	0, 1, 2,
	0, 2, 3 };
global tex win_tex = 0;
global tex win_tex_depth = 0;
global u16 win_tex_w = 0;
global u16 win_tex_h = 0;
global vert_data win_vert_data;

global gpu obj_gpu = 0;
global glsl obj_glsl_comp = 0;

//

global const str _glsl_comp_define_pre = R"(
#version 460
)";

global const str _glsl_comp_define = R"(
) in;

#define ZERO min(F,0)
#define get imageLoad
#define set imageStore
#define rgba uvec4
#define obj struct
#define in_tex( id ) layout(rgba8ui, binding = id) uniform uimage2D
#define in_data( id ) layout(std430, binding = id) buffer
#define avg( x, y ) ((x+y)/2.)
#define to( var, n ) for( uint var = ZERO; var < n; var++ )
#define norm normalize
#define fn void
#define draw() void main()

const ivec3 ID = ivec3(gl_GlobalInvocationID);
const int N = int(
	gl_GlobalInvocationID.x
);

uniform float T;
uniform uint F;
uniform ivec2 R;

#define PI 3.1415926535897932384626433832795028841971693993751058209749445923

// special quotients (was custom fixed-point LUT)
#define q5d3   .1666666
#define q5d6   .8333333
#define q1d12  .0833333
#define q1d3   .3333333
#define q1d2   .5

vec3 spectra( float x, float l ) {
    
    // (optional) rectangular expand
    x = mix((x * .5)+.25,x,1. - l);
    
    return vec3(
    // RED + VIOLET-FALLOFF
    -q1d12 * ( l - 1. ) * (
    cos( PI * max( 0., min( 1., 12. * abs( ( q1d12 * l + x - q5d6 ) / ( l + 2. ) ) ) ) ) + 1. )
    + q1d2 * cos( PI * min( 1., ( l + 3. ) * abs( -q5d3 * l + x - q1d3 ) ) ) + q1d2,
    // GREEN, BLUE
    q1d2 + q1d2 * cos( PI * min(
    vec2( 1. ), abs( vec2( x ) - vec2( q1d2, ( 1.0 - ( ( 2. + l ) / 3. ) * q1d2 ) ) )
    * vec2( 3. + l ) ) ) );
}

)";

fn _glsl_new_raw( GLenum type, const char* sc )
{
	glsl g = glCreateShader( type );

	glShaderSource( g, 1, &sc, null );
	glCompileShader( g );

	int sc_n = 1000;
	char sc_t[ 1000 ];
	sc_t[ 0 ] = 0xffu;
	glGetShaderInfoLog( g, 1000, &sc_n, sc_t );
	if( sc_t[ 0 ] != 0xffu ) print( sc_t );

	return g;
}
fn _glsl_new( GLenum type, const str& file, const int& size_x = 1, const int& size_y = 1, const int& size_z = 1 )
{
	str s = file_load( "glsl/" + file );

	switch( type )
	{
		case( glsl_comp ) {
			s = _glsl_comp_define_pre + "layout(local_size_x = " + std::to_string( size_x ) +
					", local_size_y = " + std::to_string( size_y ) +
					", local_size_z = " + std::to_string( size_z ) + _glsl_comp_define + s;
			break;
		} default: break;
	}

	const char* sc = s.c_str();
	return _glsl_new_raw( type, sc );
}
#define glsl_new( type, file, ... ) _glsl_new( type, file __VA_OPT__(, __VA_ARGS__ ) )
#define glsl_new_raw( type, src ) _glsl_new_raw( type, src )

#define glsl_delete( ... ) glDeleteShader( __VA_ARGS__ )

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

#define gpu_delete( ... ) glDeleteProgram( __VA_ARGS__ )

#define gpu_set( gpu )                                   \
	do {                                                   \
		glUseProgram( gpu );                                 \
		gpu_current = gpu;                                   \
		gpu_uni_float( "T", TIME );                          \
		gpu_uni_int( "F", FRAME );                           \
		gpu_uni_ivec2( "R", ivec2( win_tex_w, win_tex_h ) ); \
	} while( false )

#define gpu_uni_int( name, val ) glUniform1i( glGetUniformLocation( gpu_current, name ), val )
#define gpu_uni_float( name, val ) glUniform1f( glGetUniformLocation( gpu_current, name ), val )
#define gpu_uni_ivec2( name, val ) glUniform2i( glGetUniformLocation( gpu_current, name ), val.x, val.y )
#define gpu_uni_vec3( name, val ) glUniform3f( glGetUniformLocation( gpu_current, name ), val.x, val.y, val.z )
#define gpu_uni_mat4( name, val ) glUniformMatrix4fv( glGetUniformLocation( gpu_current, name ), 1, false, &( val )[ 0 ][ 0 ] )

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

// vert_data

fn _vert_data_new( const vert_list& v, const vert_tri_list& t )
{
	GLuint va, vb, eb;
	glCreateVertexArrays( 1, &va );
	glCreateBuffers( 1, &vb );
	glCreateBuffers( 1, &eb );

	glNamedBufferData( vb, v.size() * sizeof( vert ), v.data(), GL_STATIC_DRAW );
	glNamedBufferData( eb, t.size() * sizeof( GLuint ), t.data(), GL_STATIC_DRAW );

	glEnableVertexArrayAttrib( va, 0 );
	glVertexArrayAttribBinding( va, 0, 0 );
	glVertexArrayAttribFormat( va, 0, 3, GL_FLOAT, GL_FALSE, 0 );

	glEnableVertexArrayAttrib( va, 1 );
	glVertexArrayAttribBinding( va, 1, 0 );
	glVertexArrayAttribFormat( va, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof( GLfloat ) );

	glVertexArrayVertexBuffer( va, 0, vb, 0, 5 * sizeof( GLfloat ) );
	glVertexArrayElementBuffer( va, eb );

	return vert_data{ va, GLuint( t.size() ) };
}
#define vert_data_new( v, t ) _vert_data_new( v, t )

#define vert_data_bind( vd )    \
	do {                          \
		vert_data_current = vd;     \
		glBindVertexArray( vd.id ); \
	} while( false )

// gpu_data

#define gpu_data GLuint

fn _gpu_data_new( const uint& size_bytes, void* data_ptr )
{
	gpu_data g;
	glGenBuffers( 1, &g );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, g );
	glBufferData( GL_SHADER_STORAGE_BUFFER, size_bytes, data_ptr, GL_STREAM_DRAW );
	glBindBuffer( 0, 0 );
	return g;
}
#define gpu_data_new( size_bytes, data_ptr ) _gpu_data_new( size_bytes, data_ptr )

#define gpu_data_bind( data, id ) glBindBufferBase( GL_SHADER_STORAGE_BUFFER, id, data )

#define gpu_data_new_bind( size_bytes, data_ptr, id ) glBindBufferBase( GL_SHADER_STORAGE_BUFFER, id, _gpu_data_new( size_bytes, data_ptr ) )

// gpu invoke

#define gpu_draw_tris() glDrawElements( GL_TRIANGLES, vert_data_current.size, GL_UNSIGNED_INT, 0 )

fn _gpu_comp( int x = 1, int y = 1, int z = 1 )
{
	glDispatchCompute( max( 1, x ), max( 1, y ), max( 1, z ) );
}
#define gpu_comp( ... )                     \
	do {                                      \
		_gpu_comp( __VA_ARGS__ );               \
		glMemoryBarrier( GL_ALL_BARRIER_BITS ); \
	} while( false )

#define gpu_comp_tex( w, h )                                         \
	do {                                                               \
		_gpu_comp( ceil( float( w ) / 32. ), ceil( float( h ) / 32. ) ); \
		glMemoryBarrier( GL_ALL_BARRIER_BITS );                          \
	} while( false )

fn _gpu_load( GLenum type, const str& file, const int& size_x = 1, const int& size_y = 1, const int& size_z = 1 )
{
	return gpu_new( glsl_new( type, file, size_x, size_y, size_z ) );
}
#define gpu_load( type, file, ... ) _gpu_load( type, file, __VA_ARGS__ )

//

fn _tex_new( u16 w = 1, u16 h = 1, void* p = null )
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

	//void* _pixels = p;
	list<rgba> _plist;
	//if( p == null )
	//{
	for( u32 y = 0; y < h; y++ )
	{
		for( u32 x = 0; x < w; x++ )
		{
			_plist.push_back( rgba{ 255, 255, 255, 255 } );
		}
	}
	//_pixels = _plist.data();
	//}
	glTextureSubImage2D( t, 0, 0, 0, GLsizei( w ), GLsizei( h ), GL_RGBA, GL_UNSIGNED_BYTE, _plist.data() );
	return t;
}
#define tex_new( ... ) _tex_new( __VA_ARGS__ )

#define tex_bind( t, id )                                                   \
	do {                                                                      \
		glBindTextureUnit( id, t );                                             \
		glBindImageTexture( id, t, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI ); \
	} while( false )

#define tex_set( t, x, y, w, h, p ) glTextureSubImage2D( t, 0, GLint( x ), GLint( y ), GLsizei( w ), GLsizei( h ), GL_RGBA, GL_UNSIGNED_BYTE, p )

fn tex_load( str filename )
{
	auto tp = IMG_Load( filename.c_str() );

	return tp;
}

//

struct spr
{
		str file;
		u16 w = 1, h = 1;
		list<rgba> pixels;
};

list<unique( spr )> SPRITES;
list<str> SPR_LOADLIST;

#define spr_new( name, filename )           \
	uint name = []() {                        \
		SPR_LOADLIST.emplace_back( filename );  \
		return uint( SPR_LOADLIST.size() - 1 ); \
	}();

#define spr_get( name ) ( SPRITES[ name ] )
#define spr_pixels( name ) ( SPRITES[ name ]->pixels )
#define spr_w( name ) ( SPRITES[ name ]->w )
#define spr_h( name ) ( SPRITES[ name ]->h )

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

global list<rgba> pixels;

const u16 win_w_default = 1920, win_h_default = 1080;
fn main_init( str name = "hept", u16 w = win_w_default, u16 h = win_h_default, u16 scale = 1 )
{
	SDL_Init( SDL_INIT_EVERYTHING );

//

rand_seed ( 7 );

	//
	const int win_scale = scale;
	win_tex_w = w;
win_tex_h = h;

	win_current = SDL_CreateWindow( name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, win_tex_w * win_scale, win_tex_h * win_scale, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

	_win_update();
	_dis_update();

	//

	SDL_GL_CreateContext( win_current );
#ifdef vsync
SDL_GL_SetSwapInterval( 1 );
#else
SDL_GL_SetSwapInterval( 0 );
#endif

	glewExperimental = GL_TRUE;
	glewInit();
	glViewport( 0, 0, win_tex_w * win_scale, win_tex_h * win_scale );

	for( u32 y = 0; y < win_tex_h; y++ )
		{
			for( u32 x = 0; x < win_tex_w; x++ )
			{
				pixels.push_back( rgba{0,0,0,0} );
			}
		}

	// window gpu

	win_vert_data = vert_data_new( win_verts, win_tris );

	win_tex = tex_new( win_tex_w, win_tex_h, null );
	tex_bind(win_tex,0);

	win_glsl_vert = glsl_new( glsl_vert, "win_vert.glsl" );
	win_glsl_frag = glsl_new( glsl_frag, "win_frag.glsl" );
	win_gpu = gpu_new( win_glsl_vert, win_glsl_frag );
/*
	obj_glsl_comp = glsl_new(glsl_comp, obj_comp.glsl);
	obj_gpu = gpu_new( obj_glsl_comp );*/

	gpu_print_errors();

//

if( !SPR_LOADLIST.empty() ) {
		iter( s, SPR_LOADLIST ) {
			unique( spr ) tspr = unique_new( spr );
			auto lspr = IMG_Load(("spr/" + s).c_str());
if(lspr == NULL) continue; else print("spr loaded: " + s);
			u8 r=0,g=0,b=0,a=0;
			to(p,lspr->w * lspr->h)
			{
				SDL_GetRGBA(((uint32_t*)lspr->pixels)[p],lspr->format,&r,&g,&b,&a);
				tspr->pixels.emplace_back(r,g,b,a);
			}
tspr->w = lspr->w;
tspr->h = lspr->h;

			SPRITES.push_back( move( tspr ) );
		}
	}
print();
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

global timer TIME_TIMER = timer_now();
global float TIME = 0;
global uint FRAME = 0;
fn main_loop()
{
	static int main_loop_n = 0;
	static timer TICK = timer_now();

	//

	//vec3 up = vec3( 0., 0., 1. );
	//vec3 pos = vec3( 1.0f, 1.0f, 1.0f );
	//vec3 look = vec3( 0. );

	//mat4 view = lookAt( pos, look, up );
	//mat4 proj = perspective( 90., double( win_tex_w ) / double( win_tex_h ), .1, 2000. );
	//mat4 proj = tweakedInfinitePerspective(90., double( win_tex_w ) / double( win_tex_h ),.01);

	static timer T = timer_now();
	double time = 0;
	loop()
	{
		time = ( time + ( timer_get( T, nano ) / 1000000. ) ) / 2.;
		std::cout << '\r' << time << std::flush;
		T = timer_now();
		TIME = timer_get( TIME_TIMER, nano ) / 1'000'000'000.;

		_win_update();
		_dis_update();

		main_input();

		if( KEY[ SDL_SCANCODE_ESCAPE ] > 0 ) break;

		//

		//tex_set( win_tex, 0, 0, win_tex_w, win_tex_h, pixels.data() );

		//pos = vec3( sin( TIME * .25 ), cos( TIME * .25 ), 1.0f );
		//view = lookAt( pos, look, up );
		//glClearColor(0,0,0,0);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		if( draw_fnptr != null ) draw_fnptr();

		//

		//glActiveTexture( GL_TEXTURE_BASE_LEVEL );
		//glBindTexture( GL_TEXTURE_2D, win_tex );

		gpu_set( win_gpu );
		gpu_uni_int( "in_tex", 0 );
		//gpu_uni_mat4( "in_view", view );
		//gpu_uni_mat4( "in_proj", proj );

		vert_data_bind( win_vert_data );
		gpu_draw_tris();

		//

		SDL_GL_SwapWindow( win_current );

		FRAME++;
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