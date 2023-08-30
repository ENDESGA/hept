#define hept_debug
#include <hept.h>

global mesh triangle = null;

fn command()
{
	start_shader( default_shader_2d_tri, main_width, main_height );
	draw_mesh( triangle );
	end_shader();
}

main(
	"ENDESGA",
	"hept_triangle",
	512,
	256,
	command
)
{
	triangle = new_mesh( default_form_mesh_2d );
	mesh_add_tri(
		triangle,
		struct( vertex_2d ),
		create_struct_vertex_2d( 0, -.5, 0, 0, 1, 0, 0 ),
		create_struct_vertex_2d( .5, .5, 1, 1, 0, 1, 0 ),
		create_struct_vertex_2d( -.5, .5, 1, 0, 0, 0, 1 )
	);
	update_mesh( triangle );
}
