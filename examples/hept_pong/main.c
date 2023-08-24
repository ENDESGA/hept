#include <hept.h>

global f64 DELTA = 0.;
#define pixels_per_metre 16

//

global os_file file_test_bmp = null;
global os_file file_ball_bmp = null;

global image image_paddle = null;
global image image_ball = null;

global buffer buffer_storage = null;

global list sprites = null;

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
		struct( fvec2 ) pos;
		struct( fvec2 ) pos_prev;
		struct( fvec2 ) size;
		struct( fvec2 ) squish;
		struct( fvec2 ) vel;
		struct( fvec2 ) acc;
		struct( fvec2 ) force;
		double mass;
		double drag;
		double fric;
		double jump_charge;
		double stride_timer;
		double stride_force;
		mesh body;
		flag hit_left;
		flag hit_right;
		flag hit_above;
		flag hit_below;
	},
	( in f32 x, in f32 y, in f32 w, in f32 h, in f64 mass, in f64 drag, in f64 fric )
)
{
	entity this = assign_entity();
	this->pos.x = x;
	this->pos.y = y;
	this->pos_prev.x = this->pos.x;
	this->pos_prev.y = this->pos.y;
	this->size.x = w;
	this->size.y = h;
	this->squish.x = 1;
	this->squish.y = 1;
	this->mass = mass;
	this->drag = drag;
	this->fric = fric;
	this->body = make_mesh_quad_2d( this->pos.x, this->pos.y, this->size.x, this->size.y, 0, .5, 1. );
	//
	out this;
}

global f32 time_scale = 1.;

void move_entity( entity in_entity )
{
	double dt = ( 1.0 / timer_main_thread->fps ) * sqrt( time_scale );

	in_entity->acc.x = ( in_entity->force.x - in_entity->drag * in_entity->vel.x * abs( in_entity->vel.x ) ) / in_entity->mass;
	in_entity->acc.y = ( in_entity->force.y - in_entity->drag * in_entity->vel.y * abs( in_entity->vel.x ) ) / in_entity->mass;

	in_entity->vel.x += in_entity->acc.x * dt;
	in_entity->vel.y += in_entity->acc.y * dt;

	in_entity->pos_prev = in_entity->pos;
	in_entity->pos.x += in_entity->vel.x * dt * pixels_per_metre;
	in_entity->pos.y += in_entity->vel.y * dt * pixels_per_metre;

	in_entity->force.x = 0;
	in_entity->force.y = 0;
}

fn collide_entity( in entity a, in entity b )
{
	// Compute the bounding boxes for a and b
	double al = a->pos.x - a->size.x * 0.5, ar = a->pos.x + a->size.x * 0.5;
	double at = a->pos.y - a->size.y * 0.5, ab = a->pos.y + a->size.y * 0.5;

	double bl = b->pos.x - b->size.x * 0.5, br = b->pos.x + b->size.x * 0.5;
	double bt = b->pos.y - b->size.y * 0.5, bb = b->pos.y + b->size.y * 0.5;

	// Check for collision
	if( al < br && ar > bl && at < bb && ab > bt )
	{
		// Compute the penetration depths in x and y directions
		double overlapX = ( ar < br ? ar : br ) - ( al > bl ? al : bl );
		double overlapY = ( ab < bb ? ab : bb ) - ( at > bt ? at : bt );

		// Resolve the collision based on least penetration axis
		if( overlapX < overlapY )
		{
			double moveAmount = overlapX / 2.0;
			a->pos.x += ( a->pos.x < b->pos.x ) ? -moveAmount : moveAmount;
			b->pos.x += ( b->pos.x < a->pos.x ) ? -moveAmount : moveAmount;

			// Elastic collision in x direction
			double total_mass = a->mass + b->mass;
			double v1_final = ( ( a->mass - b->mass ) * a->vel.x + 2 * b->mass * b->vel.x ) / total_mass;
			double v2_final = ( ( b->mass - a->mass ) * b->vel.x + 2 * a->mass * a->vel.x ) / total_mass;

			a->vel.x = v1_final;
			a->vel.y += b->vel.y * a->fric * b->fric;
			b->vel.x = v2_final;
			b->vel.y += a->vel.y * a->fric * b->fric;
		}
		else
		{
			double moveAmount = overlapY / 2.0;
			a->pos.y += ( a->pos.y < b->pos.y ) ? -moveAmount : moveAmount;
			b->pos.y += ( b->pos.y < a->pos.y ) ? -moveAmount : moveAmount;

			// Elastic collision in y direction
			double total_mass = a->mass + b->mass;
			double v1_final = ( ( a->mass - b->mass ) * a->vel.y + 2 * b->mass * b->vel.y ) / total_mass;
			double v2_final = ( ( b->mass - a->mass ) * b->vel.y + 2 * a->mass * a->vel.y ) / total_mass;

			a->vel.y = v1_final;
			a->vel.x += b->vel.x * a->fric * b->fric;
			b->vel.y = v2_final;
			b->vel.x += a->vel.x * a->fric * b->fric;
		}
	}
}

global f32 zoom = 1.;

fn update_quad_mesh( in mesh in_mesh, in f32 in_x, in f32 in_y, in f32 in_w, in f32 in_h )
{
	engage_spinlock( in_mesh->lock );
	f32 half_w = in_w / 2., half_h = in_h / 2.;
	list_get( in_mesh->vertices, struct( vertex_2d ), 0 ).pos =
		make_struct_fvec2(
			( ( in_x - half_w ) / ( 640 >> 1 ) ) * zoom,
			( ( in_y - half_h ) / ( 320 >> 1 ) ) * zoom
		);
	list_get( in_mesh->vertices, struct( vertex_2d ), 1 ).pos =
		make_struct_fvec2(
			( ( in_x + half_w ) / ( 640 >> 1 ) ) * zoom,
			( ( in_y - half_h ) / ( 320 >> 1 ) ) * zoom
		);
	list_get( in_mesh->vertices, struct( vertex_2d ), 2 ).pos =
		make_struct_fvec2(
			( ( in_x + half_w ) / ( 640 >> 1 ) ) * zoom,
			( ( in_y + half_h ) / ( 320 >> 1 ) ) * zoom
		);
	list_get( in_mesh->vertices, struct( vertex_2d ), 3 ).pos =
		make_struct_fvec2(
			( ( in_x - half_w ) / ( 640 >> 1 ) ) * zoom,
			( ( in_y + half_h ) / ( 320 >> 1 ) ) * zoom
		);
	vacate_spinlock( in_mesh->lock );
	update_mesh( in_mesh );
}

fn update_entity( in entity in_entity )
{
	update_quad_mesh(
		in_entity->body, in_entity->pos.x, in_entity->pos.y - ( ( in_entity->size.x ) * ( in_entity->squish.y - 1. ) ), in_entity->size.x * in_entity->squish.x, in_entity->size.y * in_entity->squish.y
	);
}

//

global entity player = null;
global entity eo = null;
global mesh mesh_ground = null;

//

fn move_game()
{
	DELTA = ( 1.0 / timer_main_thread->fps ) * sqrt( time_scale );
}

fn move_player()
{
	const double gravity = 9.81 * 2.;
	const float gravity_force = gravity * player->mass;

	player->force.y += gravity_force;

	const float JUMP_HEIGHT = 1.5;
	const float jump_velocity = sqrt( 2 * gravity * JUMP_HEIGHT );
	const float MAX_JUMP_MOMENTUM = player->mass * jump_velocity;
	const float JUMP_CHARGE_RATE = MAX_JUMP_MOMENTUM * 10.0;

	flag is_moving = held_key( key_d ) or held_key( key_a );
	flag is_running = held_key( key_s );

	f32 cadence = is_running ? 280 : 140;

	static f32 footfall_timer = 0.0;
	footfall_timer += DELTA;

	f32 desired_velocity = is_running ? 6 : 2;

	if( player->hit_below )
	{
		// if(is_moving)
		{
			if( is_moving and footfall_timer >= ( 60. / cadence ) )
			{
				footfall_timer = 0.0;
				f32 required_momentum = player->mass * desired_velocity;
				f32 impulse = required_momentum - player->mass * abs( player->vel.x );
				f32 force_x = impulse / DELTA;

				if( held_key( key_a ) )
				{
					force_x = -force_x;
				}
				player->force.x += force_x * ( ( 2. - ( player->jump_charge / MAX_JUMP_MOMENTUM ) ) * .5 );
			}
			else
			{
				f32 friction_force = ( 1. / ( 1. + abs( player->vel.x ) ) ) * player->vel.x * player->mass * player->fric;
				player->force.x -= friction_force / DELTA;
			}
		}

		if( held_key( key_space ) )
		{
			player->jump_charge += JUMP_CHARGE_RATE * DELTA;
			player->jump_charge = min( player->jump_charge, MAX_JUMP_MOMENTUM );
		}
		elif( player->jump_charge > 0.0 )
		{
			f32 jump_impulse = player->jump_charge;
			f32 jump_force = jump_impulse / DELTA;
			player->force.y -= jump_force;

			if( is_moving )
			{
				const float RUN_JUMP_MULTIPLIER = 0.5 + is_running;
				float horizontal_force = jump_force * RUN_JUMP_MULTIPLIER * .5;

				if( held_key( key_a ) )
				{
					horizontal_force = -horizontal_force;
				}

				player->force.x += horizontal_force;
			}

			player->jump_charge = 0.0;
		}
	}
	else
	{
		footfall_timer = 0.0;
	}

	move_entity( player );
}

fn move_eo()
{
	move_entity( eo );
}

//
float log_scale( float value, float base )
{
	return log( value ) / log( base );
}

fn update_game()
{
#define ZOOM_MIN 0.25f
#define ZOOM_MAX 10.0f
#define ZOOM_LOG_BASE 4.0f
#define ZOOM_SPEED 2.f
#define DAMPING 0.0001f

	static float zoom_velocity = 0.0f;
	static float max_zoom_velocity = 1.f;

	if( held_key( key_e ) )
	{
		zoom_velocity += ZOOM_SPEED * DELTA;
	}
	else if( held_key( key_q ) )
	{
		zoom_velocity -= ZOOM_SPEED * DELTA;
	}
	else
	{
		zoom_velocity *= power( DAMPING, DELTA );
	}

	if( zoom_velocity > max_zoom_velocity )
	{
		zoom_velocity = max_zoom_velocity;
	}
	else if( zoom_velocity < -max_zoom_velocity )
	{
		zoom_velocity = -max_zoom_velocity;
	}

	zoom *= exp( zoom_velocity * log( ZOOM_LOG_BASE ) * DELTA );

	if( zoom < ZOOM_MIN )
	{
		zoom = ZOOM_MIN;
		zoom_velocity = 0.0f;
	}
	else if( zoom > ZOOM_MAX )
	{
		zoom = ZOOM_MAX;
		zoom_velocity = 0.0f;
	}
}

fn update_player()
{
	update_quad_mesh( mesh_ground, 0, 32, 2000, 2 );

	//

	player->hit_below = no;
	if( player->pos.y + ( player->size.y / 2 ) >= 32 )
	{
		player->force.y = 0;
		player->acc.y = 0;
		player->vel.y = 0;
		player->pos.y = 32 - ( player->size.y / 2 );
		player->hit_below = yes;
	}
	player->squish.x = ( player->squish.x + ( 1. + ( player->jump_charge * .001 ) ) ) * .5;
	player->squish.y = ( player->squish.y + ( 1. / ( 1. + ( player->jump_charge * .001 ) ) ) ) * .5;
	update_entity( player );
}

fn update_eo()
{
	eo->hit_below = no;
	if( eo->pos.y + ( eo->size.y / 2 ) >= 32 )
	{
		eo->force.y = 0;
		eo->acc.y = 0;
		eo->vel.y = 0;
		eo->pos.y = 32 - ( eo->size.y / 2 );
		eo->hit_below = yes;
	}

	eo->force.x = ( ( player->pos.x - 16 ) - eo->pos.x ) * 20.;
	eo->force.y = ( ( player->pos.y - 16 ) - eo->pos.y ) * 20.;

	update_entity( eo );
}

//

fn command()
{
	do_once
	{
		void* mappedMemory;
		vkMapMemory( current_renderer->form->machine->device, buffer_storage->device_mem, 0, sprites->size * sprites->size_type, 0, &mappedMemory );
		copy_mem( mappedMemory, sprites->data, sprites->size * sprites->size_type );
		vkUnmapMemory( current_renderer->form->machine->device, buffer_storage->device_mem );
	}

	set_frame( current_renderer->frame_window );
	use_shader( shader_2d );
	use_shader_input( default_shader_input_2d );
	start_drawing;
	draw_mesh( player->body );
	draw_mesh( eo->body );
	draw_mesh( mesh_ground );
	stop_drawing;
}

//

make_struct( sprite )
{
	struct( fvec3 ) pos;
	struct( fvec2 ) size;
};

init( "ENDESGA", "NYKRA", 640, 320, command );
main
{
	player = new_entity( 0, -64, 12, 24, 70., .5, .1 );
	eo = new_entity( -32, -32, 8, 8, 10., 100., .0 );

	//

	mesh_ground = make_mesh_quad_2d( 0, 0, 0, 0, 0, .5, 1. );

	//

	sprites = new_list( struct( sprite ) );
	list_add(
		sprites,
		struct( sprite ),
		( ( struct( sprite ) ){
			make_struct_fvec3( 1, 0, 0 ),
			make_struct_fvec2( 0, 0 ) }
		)
	);

	//

	{
		file_ball_bmp = new_os_file( "ball.bmp" );
		image_ball = load_bmp_image( file_ball_bmp );

		form_buffer form_buffer_storage = new_form_buffer( current_os_machine, h_buffer_usage_storage, h_mem_property_device_local | h_mem_property_host_coherent | h_mem_property_host_visible );
		buffer_storage = new_buffer( form_buffer_storage, sprites->size * sprites->size_type );

		VkDescriptorBufferInfo buffer_info = {
			.offset = 0,
			.buffer = buffer_storage->device_buff,
		.range = sprites->size * sprites->size_type};

		VkDescriptorImageInfo image_info = {
			.sampler = image_ball->form->sampler,
			.imageView = image_ball->view,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		VkWriteDescriptorSet descriptor_write[] = {
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = default_shader_input_2d->descriptor_set,
				.dstBinding = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.pBufferInfo = &buffer_info,
			 },
			{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = default_shader_input_2d->descriptor_set,
				.dstBinding = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &image_info }
		};
		vkUpdateDescriptorSets( current_os_machine->device, 1, descriptor_write, 0, NULL );
	}

	//
	list move_functions = new_list( fn_event );
	list_add( move_functions, fn_event, move_game );
	list_add( move_functions, fn_event, move_player );
	list_add( move_functions, fn_event, move_eo );
	new_event( always, move_functions );
	//
	list update_functions = new_list( fn_event );
	list_add( update_functions, fn_event, update_game );
	list_add( update_functions, fn_event, update_player );
	list_add( update_functions, fn_event, update_eo );
	new_event( always, update_functions );
}