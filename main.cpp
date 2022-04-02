//#define vsync
#include "hept.h"

global glsl world_bg_glsl;
global gpu world_bg_gpu;

global spr_new( tiles_spr, "scene.png" );
global tex tiles_tex;
obj tile
{
	int x, y;
	uint idx, idy;
};
global list<tile> tiles;
global gpu_data tiles_data;

global glsl tiles_glsl;
global gpu tiles_gpu;

global tex lighting_tex;
global glsl lighting_rays_glsl;
global gpu lighting_rays_gpu;
global glsl lighting_glsl;
global gpu lighting_gpu;

global glsl bloom_get_glsl;
global gpu bloom_get_gpu;
global glsl bloom_set_glsl;
global gpu bloom_set_gpu;
obj bloom_pixel
{
	int x, y;
};
global list<bloom_pixel> bloom_pixels;
global gpu_data bloom_pixels_data;

global const uint W = 640, H = 360;
global const uint TEX_W = W - 64, TEX_H = H + 64;

main( "hept", W, H, 1 )
{
	tiles_tex = tex_new( win_tex_w, win_tex_h, null );
	tex_bind( tiles_tex, 1 );
	tex_set( tiles_tex, 0, 0, spr_w( tiles_spr ), spr_h( tiles_spr ), spr_pixels( tiles_spr ).data() );

	tiles.push_back( { 64, 64, 0, 0 } );
	tiles.push_back( { 16, 32, 1, 0 } );
	tiles.push_back( { 32, 48, 2, 0 } );
	tiles.push_back( { 64, 16, 3, 0 } );
	tiles_data = gpu_data_new( sizeof( tile ) * tiles.size(), tiles.data() );
	//gpu_data_bind( tiles_data, 3 );

	//

	lighting_tex = tex_new( TEX_W, TEX_H, null );
	tex_bind( lighting_tex, 2 );
	tex_set( lighting_tex, 0, 0, TEX_W, TEX_H, pixels.data() );

	//

	/*loop( W * H )
	{
		bloom_pixels.push_back({int(rand() % win_tex_w), int(rand() % win_tex_h)});
	}*/
	bloom_pixels.push_back( { 64, 64 } );
	bloom_pixels.push_back( { 16, 32 } );
	bloom_pixels.push_back( { 32, 48 } );
	bloom_pixels.push_back( { 64, 16 } );
	bloom_pixels_data = gpu_data_new( sizeof( bloom_pixel ) * bloom_pixels.size(), bloom_pixels.data() );
	gpu_data_bind( bloom_pixels_data, 4 );
}

draw()
{
	file_watch( 0, "glsl/world_bg.glsl" )
	{
		glsl_delete( world_bg_glsl );
		gpu_delete( world_bg_gpu );
		world_bg_glsl = glsl_new( glsl_comp, "world_bg.glsl", glsl_comp_tex );
		world_bg_gpu = gpu_new( world_bg_glsl );
	}

	file_watch( 1, "glsl/tiles.glsl" )
	{
		glsl_delete( tiles_glsl );
		gpu_delete( tiles_gpu );
		tiles_glsl = glsl_new( glsl_comp, "tiles.glsl", glsl_comp_max );
		tiles_gpu = gpu_new( tiles_glsl );
	}

	file_watch( 2, "glsl/lighting_rays.glsl" )
	{
		glsl_delete( lighting_rays_glsl );
		gpu_delete( lighting_rays_gpu );
		lighting_rays_glsl = glsl_new( glsl_comp, "lighting_rays.glsl", glsl_comp_tex );
		lighting_rays_gpu = gpu_new( lighting_rays_glsl );
	}

	file_watch( 3, "glsl/lighting.glsl" )
	{
		glsl_delete( lighting_glsl );
		gpu_delete( lighting_gpu );
		lighting_glsl = glsl_new( glsl_comp, "lighting.glsl", glsl_comp_tex );
		lighting_gpu = gpu_new( lighting_glsl );
	}

	file_watch( 4, "glsl/bloom_get.glsl" )
	{
		glsl_delete( bloom_get_glsl );
		gpu_delete( bloom_get_gpu );
		bloom_get_glsl = glsl_new( glsl_comp, "bloom_get.glsl", glsl_comp_tex );
		bloom_get_gpu = gpu_new( bloom_get_glsl );
	}

	file_watch( 5, "glsl/bloom_set.glsl" )
	{
		glsl_delete( bloom_set_glsl );
		gpu_delete( bloom_set_gpu );
		bloom_set_glsl = glsl_new( glsl_comp, "bloom_set.glsl", glsl_comp_max );
		bloom_set_gpu = gpu_new( bloom_set_glsl );
	}

	//

	tex_set( win_tex, 0, 0, TEX_W, TEX_H, pixels.data() );
	//tex_set( lighting_tex, 0, 0, W, H, pixels.data() );

	gpu_set( world_bg_gpu );
	gpu_comp_tex( TEX_W, TEX_H );

	//gpu_set( tiles_gpu );
	//gpu_comp( ceil(float(W * H)/glsl_comp_max) );

	gpu_set( lighting_rays_gpu );
	gpu_comp_tex( TEX_W, TEX_H );

	//
	/*gpu_set( bloom_get_gpu );
	gpu_comp_tex( W, H );

	gpu_set( bloom_set_gpu );
	gpu_comp( bloom_pixels.size() );*/
	//

	gpu_set( lighting_gpu );
	gpu_comp_tex( TEX_W, TEX_H );
}

//

/*
global glsl comp_glsl, rast_glsl;
global gpu comp_gpu, rast_gpu;

mat4 eye_view, eye_proj;

#define vert_rand vec3(                                                  \
	( ( ( float( rand() ) / float( UINT_MAX - 1 ) ) * 2.f ) - 1.f ) * SCL, \
	( ( ( float( rand() ) / float( UINT_MAX - 1 ) ) * 2.f ) - 1.f ) * SCL, \
	( ( ( float( rand() ) / float( UINT_MAX - 1 ) ) * 2.f ) - 1.f ) * SCL )

//

obj pnt
{
	float x, y, z;
};

obj tri
{
	pnt a, b, c;
};

obj tri_bnd
{
	pnt a, b, c;
	uint w, h;
};

GLuint dispatch_args_buffer;
global list<tri> pnt_list;
global gpu_data in_data;
main( "", 640, 360, 2 )
{
	//comp_glsl = glsl_new( glsl_comp, "vert.glsl", glsl_max );
	//rast_glsl = glsl_new( glsl_comp, "rast.glsl", glsl_max );
	//comp_gpu = gpu_new( comp_glsl );
	//rast_gpu = gpu_new( rast_glsl );

	glCreateBuffers( 1, &dispatch_args_buffer );
	glNamedBufferStorage(
		dispatch_args_buffer,
		4 * sizeof( uint32_t ),
		nullptr,
		0 );

	

	// plane
	//pnt_list.push_back( tri{ pnt{ -30, -30, -1 }, pnt{ 30, -30, -1 }, pnt{ 30, 30, -1 } } );
	//pnt_list.push_back( tri{ pnt{ -30, -30, -1 }, pnt{ 30, 30, -1 }, pnt{ -30, 30, -1 } } );

	to( yy, 32 )
		to( xx, 32 )
	{
		float XX = float( xx) - 16 , YY = float( yy) - 16 ;
		pnt_list.push_back( tri{ pnt{ XX, YY, -1 }, pnt{ XX+1, YY, -1 }, pnt{ XX+1, YY+1, -1 } } );
		pnt_list.push_back( tri{ pnt{ XX, YY, -1 }, pnt{ XX+1, YY+1, -1 }, pnt{ XX, YY+1, -1 } } );
	}

	// top
	pnt_list.push_back( tri{ pnt{ -1, -1, 1 }, pnt{ 1, -1, 1 }, pnt{ 1, 1, 1 } } );
	pnt_list.push_back( tri{ pnt{ -1, -1, 1 }, pnt{ 1, 1, 1 }, pnt{ -1, 1, 1 } } );

	// bottom
	pnt_list.push_back( tri{ pnt{ -1, 1, -1 }, pnt{ 1, 1, -1 }, pnt{ 1, -1, -1 } } );
	pnt_list.push_back( tri{ pnt{ -1, 1, -1 }, pnt{ 1, -1, -1 }, pnt{ -1, -1, -1 } } );

	// back
	pnt_list.push_back( tri{ pnt{ 1, -1, 1 }, pnt{ -1, -1, 1 }, pnt{ -1, -1, -1 } } );
	pnt_list.push_back( tri{ pnt{ 1, -1, 1 }, pnt{ -1, -1, -1 }, pnt{ 1, -1, -1 } } );

	// front
	pnt_list.push_back( tri{ pnt{ -1, 1, 1 }, pnt{ 1, 1, 1 }, pnt{ 1, 1, -1 } } );
	pnt_list.push_back( tri{ pnt{ -1, 1, 1 }, pnt{ 1, 1, -1 }, pnt{ -1, 1, -1 } } );

	// right
	pnt_list.push_back( tri{ pnt{ 1, 1, 1 }, pnt{ 1, -1, 1 }, pnt{ 1, -1, -1 } } );
	pnt_list.push_back( tri{ pnt{ 1, 1, 1 }, pnt{ 1, -1, -1 }, pnt{ 1, 1, -1 } } );

	// left
	pnt_list.push_back( tri{ pnt{ -1, -1, 1 }, pnt{ -1, 1, 1 }, pnt{ -1, 1, -1 } } );
	pnt_list.push_back( tri{ pnt{ -1, -1, 1 }, pnt{ -1, 1, -1 }, pnt{ -1, -1, -1 } } );

	in_data = gpu_data_new( sizeof( tri ) * pnt_list.size(), pnt_list.data() );
	gpu_data_bind( in_data, 2 );

	gpu_data_new_bind( sizeof( tri_bnd ) * pnt_list.size(), nullptr, 3 );
}

draw()
{
	static fs::file_time_type ftv;
	fs::file_time_type lwt = fs::last_write_time( "glsl/vert.glsl" );
	if( ftv != lwt )
	{
		glDeleteShader( comp_glsl );
		glDeleteProgram( comp_gpu );

		comp_glsl = glsl_new( glsl_comp, "vert.glsl", glsl_max );
		comp_gpu = gpu_new( comp_glsl );

		ftv = lwt;
	}

	static fs::file_time_type ftr;
	lwt = fs::last_write_time( "glsl/rast.glsl" );
	if( ftr != lwt )
	{
		glDeleteShader( rast_glsl );
		glDeleteProgram( rast_gpu );

		rast_glsl = glsl_new( glsl_comp, "rast.glsl", glsl_max );
		rast_gpu = gpu_new( rast_glsl );

		ftr = lwt;
	}

	//

	static vec3 eye = vec3( 0 ), focus = vec3( 0 );
	static vec3 eye_an = eye, focus_an = focus;

	vec3 look = vec3( sin( TIME ) * 7., cos( TIME ) * 7., ( -sin( TIME ) ) + 2.5 ); //vec3( sin( TIME * .25 ) * 3., cos( TIME * .25 ) * 3., -sin( TIME * .25 ) * 3. );

	if( KEY[ SDL_SCANCODE_SPACE ] > 0 )
	{
		eye = vec3( 0 );
		focus = mix( focus, look, .01 );
	} else
	{
		eye = look;
		focus = vec3( 0 );
	}

	eye = mix( eye, ( KEY[ SDL_SCANCODE_SPACE ] > 0 ) ? ( vec3( 0 ) ) : ( look ), .01 );
	focus = vec3( 0 );

	eye_an = mix( eye_an, eye, .01 );
	focus_an = mix( focus_an, focus, .01 );

	eye_view = lookAt( eye_an, focus_an, vec3( 0, 0, 1 ) );
	eye_proj = perspective( ( 2. * M_PI ) / 5., double( win_tex_w ) / double( win_tex_h ), .1, 100. );

	//	mat4 eye_mat = eye_view * eye_proj;

	tex_set( win_tex, 0, 0, win_tex_w, win_tex_h, pixels.data() );
	tex_set( win_tex_depth, 0, 0, win_tex_w, win_tex_h, pixels.data() );

	gpu_set( comp_gpu );
	gpu_uni_vec3( "eye_pos", eye );
	gpu_uni_mat4( "eye_view", eye_view );
	gpu_uni_mat4( "eye_proj", eye_proj );
	//glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, dispatch_args_buffer );
	gpu_compute( ceil(float(pnt_list.size()) / 1024.) );

	//

	/*glUseProgram( rast_gpu );
	glMemoryBarrier( GL_COMMAND_BARRIER_BIT );
	glBindBuffer( GL_DISPATCH_INDIRECT_BUFFER, dispatch_args_buffer );
	glDispatchComputeIndirect( 0 );* /

	gpu_set( rast_gpu );
	gpu_compute( ceil( ( win_tex_w * win_tex_h ) / 1024. ) );
}

/*
global glsl shader;
global gpu main_gpu;

global uint W = 640, H = 320;
global uint AREA = W * H;

main( "hept", W, H, 2 )
{
	//
}

draw()
{
	static fs::file_time_type ft;
	fs::file_time_type lwt = fs::last_write_time( "glsl/comp.glsl" );
	if( ft != lwt )
	{
		glDeleteShader( shader );
		glDeleteProgram( main_gpu );

		shader = glsl_new( glsl_comp, "comp.glsl", 32, 32 );
		main_gpu = gpu_new( shader );

		ft = lwt;

		TIME_TIMER = timer_now();
		TIME = 0;
	}
	
	gpu_set( main_gpu );
	gpu_compute(ceil( W / 32), ceil( H / 32 ));
}*/
/*

GLuint dispatch_args_buffer;

global glsl vert_glsl, rast_glsl;
global gpu vert_gpu, rast_gpu;
global gpu_data in_data;

obj pnt
{
	float x, y, z;
};

obj tri
{
	pnt a, b, c;
};

main( "hept", 640, 320, 2 )
{
	//main_tex = tex_new( win_tex_w, win_tex_h, null );
	//tex_bind( main_tex, 1 );

	vert_glsl = glsl_new( glsl_comp, "vert.glsl", glsl_max );
	vert_gpu = gpu_new( vert_glsl );

	rast_glsl = glsl_new( glsl_comp, "rast.glsl", 1 );
	rast_gpu = gpu_new( rast_glsl );
	
	glCreateBuffers( 1, &dispatch_args_buffer );
	glNamedBufferStorage(
		dispatch_args_buffer,
		4 * sizeof( uint32_t ),
		nullptr,
		0
	);
	
	//

	list<tri> pnt_list;

	// plane
	pnt_list.push_back( tri{ pnt{ -3, -3, -1 }, pnt{ 3, -3, -1 }, pnt{ 3, 3, -1 } } );
	pnt_list.push_back( tri{ pnt{ -3, -3, -1 }, pnt{ 3, -3, -1 }, pnt{ 3, 3, -1 } } );
	pnt_list.push_back( tri{ pnt{ -3, -3, -1 }, pnt{ 3, -3, -1 }, pnt{ 3, 3, -1 } } );
	pnt_list.push_back( tri{ pnt{ -3, -3, -1 }, pnt{ 3, -3, -1 }, pnt{ 3, 3, -1 } } );
	//pnt_list.push_back( tri{ pnt{ -3, -3, -1 }, pnt{ 3, 3, -1 }, pnt{ -3, 3, -1 } } );
	
	// top
	pnt_list.push_back( tri{ pnt{ -1, -1, 1 }, pnt{ 1, -1, 1 }, pnt{ 1, 1, 1 } } );
	pnt_list.push_back( tri{ pnt{ -1, -1, 1 }, pnt{ 1, 1, 1 }, pnt{ -1, 1, 1 } } );

	// bottom
	pnt_list.push_back( tri{ pnt{ -1, 1, -1 }, pnt{ 1, 1, -1 }, pnt{ 1, -1, -1 } } );
	pnt_list.push_back( tri{ pnt{ -1, 1, -1 }, pnt{ 1, -1, -1 }, pnt{ -1, -1, -1 } } );

	// back
	pnt_list.push_back( tri{ pnt{ 1, -1, 1 }, pnt{ -1, -1, 1 }, pnt{ -1, -1, -1 } } );
	pnt_list.push_back( tri{ pnt{ 1, -1, 1 }, pnt{ -1, -1, -1 }, pnt{ 1, -1, -1 } } );

	// front
	pnt_list.push_back( tri{ pnt{ -1, 1, 1 }, pnt{ 1, 1, 1 }, pnt{ 1, 1, -1 } } );
	pnt_list.push_back( tri{ pnt{ -1, 1, 1 }, pnt{ 1, 1, -1 }, pnt{ -1, 1, -1 } } );
	
	// right
	pnt_list.push_back( tri{ pnt{ 1, 1, 1 }, pnt{ 1, -1, 1 }, pnt{ 1, -1, -1 } } );
	pnt_list.push_back( tri{ pnt{ 1, 1, 1 }, pnt{ 1, -1, -1 }, pnt{ 1, 1, -1 } } );

	// left
	pnt_list.push_back( tri{ pnt{ -1, -1, 1 }, pnt{ -1, 1, 1 }, pnt{ -1, 1, -1 } } );
	pnt_list.push_back( tri{ pnt{ -1, -1, 1 }, pnt{ -1, 1, -1 }, pnt{ -1, -1, -1 } } );

	print(pnt_list.size());
	
	in_data = gpu_data_new( sizeof( tri ) * pnt_list.size(), pnt_list.data() );
	gpu_data_bind( in_data, 2 );
	
}

draw()
{
	tex_set( win_tex, 0, 0, win_tex_w, win_tex_h, pixels.data() );
	
	glUseProgram( vert_gpu );

	//vec3 eye = vec3(sin(TIME),cos(TIME),.5);
	//vec3 focus = vec3( 0 );

	//mat4 eye_view = lookAt( eye, focus, vec3( 0, 0, 1 ) );
	//mat4 eye_proj = perspective( ( 2. * M_PI ) / 5., double( win_tex_w ) / double( win_tex_h ), .1, 100. );

	//gpu_uni_vec3( "eye_pos", eye );
	//gpu_uni_mat4( "eye_view", eye_view );
	//gpu_uni_mat4( "eye_proj", eye_proj );
	
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, dispatch_args_buffer );
	glDispatchCompute( 1, 1, 1 );
	
	glUseProgram( rast_gpu );
	glMemoryBarrier( GL_COMMAND_BARRIER_BIT );
	glBindBuffer( GL_DISPATCH_INDIRECT_BUFFER, dispatch_args_buffer );
	glDispatchComputeIndirect( 0 );
}
*/
//

/*
global glsl vert_glsl, geom_glsl, frag_glsl;
global gpu main_gpu;
global tex main_tex;
global vert_list main_verts = {
	vert( -.5, -.5, -1., 0., 1. ),
	vert( -.5, .5, -1., 0., 0. ),
	vert( .5, .5, -1., 1., 0. ),
	vert( .5, -.5, -1., 1., 1. ),
};
global vert_tri_list main_tris = {
	0, 1, 2,
	0, 2, 3 };
global vert_data main_vert_data;

main( "hept", 640, 320, 2 )
{
	main_vert_data = vert_data_new( main_verts, main_tris );

	main_tex = tex_new( win_tex_w, win_tex_h, null );
	tex_bind( main_tex, 1 );

	vert_glsl = glsl_new( glsl_vert, "vert.glsl" );
	frag_glsl = glsl_new( glsl_frag, "frag.glsl" );
	main_gpu = gpu_new( vert_glsl, frag_glsl );
}

draw()
{
	gpu_set( main_gpu );
	gpu_uni_int( "win_tex", 0 );
	gpu_uni_int( "tex", 1 );

	vec3 eye = vec3( sin( TIME ) * 4., cos( TIME ) * 4., ( -sin( TIME ) ) + 1.5 );
	vec3 focus = vec3( 0 );

	mat4 view_mat = lookAt( eye, focus, vec3( 0, 0, 1 ) );
	mat4 proj_mat = perspective( ( 2. * M_PI ) / 5., double( win_tex_w ) / double( win_tex_h ), .1, 100. );

	gpu_uni_mat4( "in_view", view_mat );
	gpu_uni_mat4( "in_proj", proj_mat );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, main_tex );
	
	vert_data_bind( main_vert_data );
	
	gpu_draw_tris();
	glFlush();
	//glBindTexture( main_gpu, -1 );
}
* /
//

global glsl comp_glsl, rast_glsl;
global gpu comp_gpu, rast_gpu;

mat4 eye_view, eye_proj;

#define vert_rand vec3(                                                  \
	( ( ( float( rand() ) / float( UINT_MAX - 1 ) ) * 2.f ) - 1.f ) * SCL, \
	( ( ( float( rand() ) / float( UINT_MAX - 1 ) ) * 2.f ) - 1.f ) * SCL, \
	( ( ( float( rand() ) / float( UINT_MAX - 1 ) ) * 2.f ) - 1.f ) * SCL )

//

obj pnt
{
	float x, y, z;
};

obj tri
{
	pnt a, b, c;
};

global gpu_data in_data;
main( "", 640, 360, 2 )
{
	comp_glsl = glsl_new( glsl_comp, "comp.glsl", glsl_max );
	//rast_glsl = glsl_new( glsl_comp, "rast.glsl", glsl_max );
	comp_gpu = gpu_new( comp_glsl );
	//rast_gpu = gpu_new( rast_glsl );

	list<tri> pnt_list;
	
	// plane
	pnt_list.push_back( tri{ pnt{ -3, -3, -1 }, pnt{ 3, -3, -1 }, pnt{ 3, 3, -1 } } );
	pnt_list.push_back( tri{ pnt{ -3, -3, -1 }, pnt{ 3, 3, -1 }, pnt{ -3, 3, -1 } } );
	/*
	// top
	pnt_list.push_back( tri{ pnt{ -1, -1, 1 }, pnt{ 1, -1, 1 }, pnt{ 1, 1, 1 } } );
	pnt_list.push_back( tri{ pnt{ -1, -1, 1 }, pnt{ 1, 1, 1 }, pnt{ -1, 1, 1 } } );

	// bottom
	pnt_list.push_back( tri{ pnt{ -1, 1, -1 }, pnt{ 1, 1, -1 }, pnt{ 1, -1, -1 } } );
	pnt_list.push_back( tri{ pnt{ -1, 1, -1 }, pnt{ 1, -1, -1 }, pnt{ -1, -1, -1 } } );

	// back
	pnt_list.push_back( tri{ pnt{ 1, -1, 1 }, pnt{ -1, -1, 1 }, pnt{ -1, -1, -1 } } );
	pnt_list.push_back( tri{ pnt{ 1, -1, 1 }, pnt{ -1, -1, -1 }, pnt{ 1, -1, -1 } } );

	// front
	pnt_list.push_back( tri{ pnt{ -1, 1, 1 }, pnt{ 1, 1, 1 }, pnt{ 1, 1, -1 } } );
	pnt_list.push_back( tri{ pnt{ -1, 1, 1 }, pnt{ 1, 1, -1 }, pnt{ -1, 1, -1 } } );
	
	// right
	pnt_list.push_back( tri{ pnt{ 1, 1, 1 }, pnt{ 1, -1, 1 }, pnt{ 1, -1, -1 } } );
	pnt_list.push_back( tri{ pnt{ 1, 1, 1 }, pnt{ 1, -1, -1 }, pnt{ 1, 1, -1 } } );

	// left
	pnt_list.push_back( tri{ pnt{ -1, -1, 1 }, pnt{ -1, 1, 1 }, pnt{ -1, 1, -1 } } );
	pnt_list.push_back( tri{ pnt{ -1, -1, 1 }, pnt{ -1, 1, -1 }, pnt{ -1, -1, -1 } } );* /

	in_data = gpu_data_new( sizeof( tri ) * pnt_list.size(), pnt_list.data() );
	gpu_data_bind( in_data, 2 );
}

draw()
{
	static fs::file_time_type ft;
	fs::file_time_type lwt = fs::last_write_time( "glsl/comp.glsl" );
	if( ft != lwt )
	{
		glDeleteShader( comp_glsl );
		glDeleteProgram( comp_gpu );

		comp_glsl = glsl_new( glsl_comp, "comp.glsl", glsl_max );
		comp_gpu = gpu_new( comp_glsl );

		ft = lwt;
	}

	//

	static vec3 eye = vec3( 0 ), focus = vec3( 0 );
	static vec3 eye_an = eye, focus_an = focus;

	vec3 look = vec3( sin( TIME ) * 4., cos( TIME ) * 4., (-sin(TIME))+1.5 ); //vec3( sin( TIME * .25 ) * 3., cos( TIME * .25 ) * 3., -sin( TIME * .25 ) * 3. );

	if( KEY[ SDL_SCANCODE_SPACE ] > 0 )
	{
		eye = vec3( 0 );
		focus = mix( focus, look, .01 );
	} else
	{
		eye = look;
		focus = vec3( 0 );
	}

	eye = mix( eye, ( KEY[ SDL_SCANCODE_SPACE ] > 0 ) ? ( vec3( 0 ) ) : ( look ), .01 );
	focus = vec3( 0 );

	eye_an = mix( eye_an, eye, .01 );
	focus_an = mix( focus_an, focus, .01 );

	eye_view = lookAt( eye_an, focus_an, vec3( 0, 0, 1 ) );
	eye_proj = perspective( ( 2. * M_PI ) / 5., double( win_tex_w ) / double( win_tex_h ), .1, 100. );

	//	mat4 eye_mat = eye_view * eye_proj;

	gpu_set( comp_gpu );
	gpu_uni_vec3( "eye_pos", eye );
	gpu_uni_mat4( "eye_view", eye_view );
	gpu_uni_mat4( "eye_proj", eye_proj );
	gpu_compute( 1 );
}

//

/*#define vsync
#include "hept.h"

global glsl test_glsl;
global gpu test_gpu;

global glsl test2_glsl;
global gpu test2_gpu;

struct star
{
		vec3 pos;
};
list<star> stars;

const uint obj_n = 1000000;

main("hept",1920/3,1080/3)
{
	test_glsl = glsl_new( glsl_comp, "comp.glsl", 27 );
	test_gpu = gpu_new( test_glsl );

	test2_glsl = glsl_new( glsl_comp, "comp2.glsl", glsl_max );
	test2_gpu = gpu_new( test2_glsl );

	for( uint n = 0; n < obj_n; n++ )
	{
		stars.push_back( { vec3(
			( ( pow( float( rand() % 77777 ) / 77777.f, 1.f ) * 2.f ) - 1.f ) * 200.f,
			( ( pow( float( rand() % 77777 ) / 77777.f, 1.f ) * 2.f ) - 1.f ) * 200.f,
			( ( pow( float( rand() % 77777 ) / 77777.f, 1.f ) * 2.f ) - 1.f ) * 200.f ) } );
	}
	gpu_data in_gpu_data = gpu_data_new( stars.size() * sizeof( star ), stars.data() );
	gpu_data_bind( in_gpu_data, 1 );
}

draw()
{
	static fs::file_time_type ft;
	fs::file_time_type lwt = fs::last_write_time("glsl/comp2.glsl");
	if( ft != lwt )
	{
		glDeleteProgram(test2_gpu);
		glDeleteShader(test2_glsl);
		
		test2_glsl = glsl_new( glsl_comp, "comp2.glsl", glsl_max );
		test2_gpu = gpu_new( test2_glsl );
		
		ft = lwt;
	}
	
	//
	
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
*/