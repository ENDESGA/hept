in_tex( 0 ) eye_tex;
in_tex( 1 ) cam_tex;
//in_tex( 2 ) world_tex;

#define CAM_BUFFER 64

draw()
{
	rgba c = get( cam_tex, ID.xy + CAM_BUFFER );
	set( tex, ID.xy, c );
}