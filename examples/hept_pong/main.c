#include <hept.h>

//

global f32 DELTA = 0;

global os_file file_test_bmp = null;
global os_file file_ball_bmp = null;

global image image_paddle = null;
global image image_ball = null;

global shader_input shader_input_paddle = null;
global shader_input shader_input_ball = null;

//

mesh make_mesh_quad_2d( in f32 x, in f32 y, in f32 w, in f32 h, in f32 r, in f32 g, in f32 b )
{
	mesh result = new_mesh( form_mesh_2d );
	f32 half_w = w / 2.0, half_h = h / 2.0;
	mesh_add_quad(
		result,
		struct( vertex_2d ),
		create_struct_vertex_2d( x - half_w, y - half_h, 0, 1, r, g, b ),
		create_struct_vertex_2d( x + half_w, y - half_h, 1, 1, r, g, b ),
		create_struct_vertex_2d( x + half_w, y + half_h, 1, 0, r, g, b ),
		create_struct_vertex_2d( x - half_w, y + half_h, 0, 0, r, g, b )
	);
	out result;
}

//

make_thing(
	entity,
	{
		mesh body;
		struct( fvec2 ) pos;
		struct( fvec2 ) pos_prev;
		struct( fvec2 ) size;
		struct( fvec2 ) vel;
		struct( fvec2 ) acc;
		struct( fvec2 ) force;
		double mass;
		double drag;
		double fric;
	},
	( in f32 x, in f32 y, in f32 w, in f32 h, in f64 mass, in f64 drag, in f64 fric )
)
{
	entity this = assign_entity();
	this->body = make_mesh_quad_2d( 0, 0, 1, 1, 0, 1, 0.75 );
	this->pos.x = x;
	this->pos.y = y;
	this->pos_prev.x = this->pos.x;
	this->pos_prev.y = this->pos.y;
	this->size.x = w;
	this->size.y = h;

	this->mass = mass;
	this->drag = drag;
	this->fric = fric;
	//
	out this;
}

global double time_scale = 1.;
global double pixels_per_metre = 32.;

fn move_entity( in entity in_entity )
{
	in_entity->force.x -= (( in_entity->vel.x / ( 1. + abs( in_entity->vel.x ) ) ) * in_entity->mass * in_entity->fric) / DELTA;
	in_entity->force.y -= (( in_entity->vel.y / ( 1. + abs( in_entity->vel.y ) ) ) * in_entity->mass * in_entity->fric) / DELTA;

	in_entity->acc.x = ( in_entity->force.x - in_entity->drag * in_entity->vel.x * abs( in_entity->vel.x ) ) / in_entity->mass;
	in_entity->acc.y = ( in_entity->force.y - in_entity->drag * in_entity->vel.y * abs( in_entity->vel.x ) ) / in_entity->mass;

	in_entity->vel.x += in_entity->acc.x * DELTA;
	in_entity->vel.y += in_entity->acc.y * DELTA;

	in_entity->pos_prev = in_entity->pos;
	in_entity->pos.x += in_entity->vel.x * DELTA * pixels_per_metre;
	in_entity->pos.y += in_entity->vel.y * DELTA * pixels_per_metre;

	in_entity->force.x = 0;
	in_entity->force.y = 0;
}

fn collide_entity( in entity a, in entity b )
{
	f32 al = a->pos.x - a->size.x * 0.5, ar = a->pos.x + a->size.x * 0.5;
	f32 at = a->pos.y - a->size.y * 0.5, ab = a->pos.y + a->size.y * 0.5;
	f32 bl = b->pos.x - b->size.x * 0.5, br = b->pos.x + b->size.x * 0.5;
	f32 bt = b->pos.y - b->size.y * 0.5, bb = b->pos.y + b->size.y * 0.5;

	if( ( al < br ) and ( ar > bl ) and ( at < bb ) and ( ab > bt ) )
	{
		f32 over_x = ( ( ar < br ? ar : br ) - ( al > bl ? al : bl ) ) * .5;
		f32 over_y = ( ( ab < bb ? ab : bb ) - ( at > bt ? at : bt ) ) * .5;

		if( over_x < over_y )
		{
			a->pos.x += ( a->pos.x < b->pos.x ) ? -over_x : over_x;
			b->pos.x += ( b->pos.x < a->pos.x ) ? -over_x : over_x;

			f32 total_mass = a->mass + b->mass;
			f32 v1_final = ( ( a->mass - b->mass ) * a->vel.x + 2 * b->mass * b->vel.x ) / total_mass;
			f32 v2_final = ( ( b->mass - a->mass ) * b->vel.x + 2 * a->mass * a->vel.x ) / total_mass;

			a->vel.x = v1_final;
			a->vel.y += b->vel.y * a->fric * b->fric;
			b->vel.x = v2_final;
			b->vel.y += a->vel.y * a->fric * b->fric;
		}
		else
		{
			a->pos.y += ( a->pos.y < b->pos.y ) ? -over_y : over_y;
			b->pos.y += ( b->pos.y < a->pos.y ) ? -over_y : over_y;

			f32 total_mass = a->mass + b->mass;
			f32 v1_final = ( ( a->mass - b->mass ) * a->vel.y + 2 * b->mass * b->vel.y ) / total_mass;
			f32 v2_final = ( ( b->mass - a->mass ) * b->vel.y + 2 * a->mass * a->vel.y ) / total_mass;

			a->vel.y = v1_final;
			a->vel.x += b->vel.x * a->fric * b->fric;
			b->vel.y = v2_final;
			b->vel.x += a->vel.x * a->fric * b->fric;
		}
	}
}

//

fn update_entity( in entity in_entity )
{
	engage_spinlock( in_entity->body->lock );
	f32 half_w = in_entity->size.x / 2., half_h = in_entity->size.y / 2.;
	list_get( in_entity->body->vertices, struct( vertex_2d ), 0 ).pos =
		make_struct_fvec2( ( in_entity->pos.x - half_w ) / ( main_window_width >> 1 ), ( in_entity->pos.y - half_h ) / ( main_window_height >> 1 ) );
	list_get( in_entity->body->vertices, struct( vertex_2d ), 1 ).pos =
		make_struct_fvec2( ( in_entity->pos.x + half_w ) / ( main_window_width >> 1 ), ( in_entity->pos.y - half_h ) / ( main_window_height >> 1 ) );
	list_get( in_entity->body->vertices, struct( vertex_2d ), 2 ).pos =
		make_struct_fvec2( ( in_entity->pos.x + half_w ) / ( main_window_width >> 1 ), ( in_entity->pos.y + half_h ) / ( main_window_height >> 1 ) );
	list_get( in_entity->body->vertices, struct( vertex_2d ), 3 ).pos =
		make_struct_fvec2( ( in_entity->pos.x - half_w ) / ( main_window_width >> 1 ), ( in_entity->pos.y + half_h ) / ( main_window_height >> 1 ) );
	vacate_spinlock( in_entity->body->lock );
	update_mesh( in_entity->body );
}

//

entity player = null;
entity other = null;
entity ball = null;

//

fn move_player()
{
	player->force.x = ( ( -256 ) - player->pos.x ) * 10.;
	player->force.y = ( held_key( key_down ) - held_key( key_up ) ) * 1000.;
	move_entity( player );
}

fn move_other()
{
	player->force.x = ( ( 256 ) - player->pos.x ) * 10.;
	player->force.y = ( ball->pos.y - other->pos.y ) * 500.;
	move_entity( other );
}

fn move_ball()
{
	if( pressed_key( key_space ) )
	{
		ball->force.x -= 200.;
	}

	move_entity( ball );

	if( ( ball->pos.y <= -to_f32( main_window_height >> 1 ) ) or ( ball->pos.y >= to_f32( main_window_width >> 1 ) ) )
		ball->vel.y = abs( ball->vel.y ) * ( ( ball->pos.y > 0. ) ? -1. : 1. );
}

//

fn update_player()
{
	update_entity( player );
}

fn update_other()
{
	update_entity( other );
}

fn update_ball()
{
	collide_entity( ball, player );
	collide_entity( ball, other );
	update_entity( ball );
}

//

fn move_game()
{
	DELTA = ( 1.0 / timer_main_thread->fps ) * sqrt( time_scale );

	if( ball->pos.x - ball->size.x * .5 <= player->pos.x + player->size.x * .5 )
		time_scale = ( time_scale * .1 ) + ( .25 * .9 );
	else
		time_scale = ( time_scale * .01 ) + ( 1. * .99 );

	if( ball->pos.x <= -to_f32( main_window_width >> 1 ) or ball->pos.x >= to_f32( main_window_height >> 1 ) )
	{
		ball->pos.x = 0;
		ball->pos.y = 0;
		ball->vel.x = 0;
		ball->vel.y = 0;
	}
}

//

fn command()
{
	do_once
	{
		update_image( current_renderer, image_paddle );
		update_image( current_renderer, image_ball );
	}
	start_shader( shader_2d, main_window_width, main_window_height );
	use_shader_input( shader_input_paddle );
	draw_mesh( player->body );
	draw_mesh( other->body );
	use_shader_input( shader_input_ball );
	draw_mesh( ball->body );
	end_shader();
}

//

main( "ENDESGA", "hept_pong", 1024, 512, command )
{
	player = new_entity( -256, 0, 32, 128, 10., 10., .5 );
	other = new_entity( 256, 0, 32, 128, 10., 10., .5 );
	ball = new_entity( 0, 0, 64, 64, .1, 0., 0. );
	//
	file_test_bmp = new_os_file( "paddle.bmp" );
	image_paddle = load_bmp_image( file_test_bmp );
	shader_input_paddle = new_shader_input( default_form_shader_2d_tri );
	update_shader_input_image( current_os_machine, shader_input_paddle, image_paddle );
	//
	file_ball_bmp = new_os_file( "ball.bmp" );
	image_ball = load_bmp_image( file_ball_bmp );
	shader_input_ball = new_shader_input( default_form_shader_2d_tri );
	update_shader_input_image( current_os_machine, shader_input_ball, image_ball );
	//
	list move_functions = new_list( fn_event );
	list_add( move_functions, fn_event, move_game );
	list_add( move_functions, fn_event, move_player );
	list_add( move_functions, fn_event, move_other );
	list_add( move_functions, fn_event, move_ball );
	new_event( always, move_functions );
	//
	list update_functions = new_list( fn_event );
	list_add( update_functions, fn_event, update_player );
	list_add( update_functions, fn_event, update_other );
	list_add( update_functions, fn_event, update_ball );
	new_event( always, update_functions );
}