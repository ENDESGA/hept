//#define vsync
#include "hept.h"

global glsl test_glsl;
global gpu test_gpu;

global glsl test2_glsl;
global gpu test2_gpu;

/*struct data_type
{
		ivec2 pos, size;
		u32 col;
		u32 id;
};*/
/*
struct data_type
{
		vec3 pos;
};*/

struct star
{
		vec3 pos;
};
list<star> stars;

const uint obj_n = 1000000;

main()
{
	test_glsl = glsl_new( glsl_comp, "comp.glsl", 27 );
	test_gpu = gpu_new( test_glsl );

	test2_glsl = glsl_new( glsl_comp, "comp2.glsl", glsl_max );
	test2_gpu = gpu_new( test2_glsl );

	for( uint n = 0; n < obj_n; n++ )
	{
		/*in_data.push_back( {
			ivec2( int(( rand() % ( win_tex_w / 1 ) ) + ( win_tex_w / 4000 )), int(( rand() % ( win_tex_h / 1 ) ) + ( win_tex_h / 4000 )) )
			,ivec2(((rand()%16)+2)*2,( ( rand() % 16 ) + 2)*2)
			, rgba{rand()%256, rand() % 256, rand() % 256,255}.col, n
		} );*/
		/*in_data.push_back( { vec3(
			float( rand() % 200 ) - 0.,
			float( rand() % 200 ) - 0.,
			0.//( float( rand() % 20000 ) / 100. ) - 1.
			) } );*/

		stars.push_back( { vec3(
			( ( pow( float( rand() % 77777 ) / 77777.f, 1.f ) * 2.f ) - 1.f ) * 200.f,
			( ( pow( float( rand() % 77777 ) / 77777.f, 1.f ) * 2.f ) - 1.f ) * 200.f,
			( ( pow( float( rand() % 77777 ) / 77777.f, 1.f ) * 2.f ) - 1.f ) * 200.f ) } );
		/*
		stars.push_back( { vec3(
			float( int(rand())-(INT_MAX/2)  )/200000.f,
			float( int(rand())-(INT_MAX/2)  )/200000.f,
			float( int(rand())-(INT_MAX/2)  )/200000.f) } );*/
	}
	gpu_data in_gpu_data = gpu_data_new( stars.size() * sizeof( star ), stars.data() );
	gpu_data_bind( in_gpu_data, 1 );
}

draw()
{
	static vec3 eye = vec3( 0, 0, 0 ), focus = vec3( 0, 0, 0 );
	static vec3 eye_an = eye, focus_an = focus;

	if( KEY[ SDL_SCANCODE_SPACE ] > 0 )
	{
		eye = mix( eye, stars[ floor( TIME * .25 ) ].pos, .01 );
	} else
	{
		eye = vec3( sin( TIME * .125 ) * 20., cos( TIME * .125 ) * 20., -sin( TIME * .125 ) * 20. );
	}

	focus = vec3( 0 );

	eye_an = mix( eye_an, eye, .01 );
	focus_an = mix( focus_an, focus, .01 );

	mat4 view = lookAt( eye_an, focus_an, vec3( 0, 0, 1 ) );
	//mat4 proj = perspective( 90., double( win_tex_w ) / double( win_tex_h ), .1, 2000. );
	mat4 proj = infinitePerspective( 90., double( win_tex_w ) / double( win_tex_h ), .1 );

	static bool init = true;
	if( init )
	{
		to( p, win_tex_w * win_tex_h )
		{
			pixels[ p ] = rgba{};
		}
		init = false;
	}

	gpu_set( test_gpu );
	gpu_compute();

	//tex_set( win_tex, 0, 0, win_tex_w, win_tex_h, pixels.data() );
	gpu_set( test2_gpu );
	gpu_uni_mat4( "in_view", view );
	gpu_uni_mat4( "in_proj", proj );
	gpu_compute( int( ceil( float( obj_n ) / 1024. ) ) );
	//
}
