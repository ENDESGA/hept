in_tex( 0 ) tex;
in_tex( 1 ) cam_tex;

#define CAM_BUFFER 64

draw()
{
	//rgba c = get(tex, ID.xy);
	set( cam_tex, ID.xy, ( ( ID.x < CAM_BUFFER || ID.x > R.x + CAM_BUFFER ) || ( ID.y < CAM_BUFFER || ID.y > R.y + CAM_BUFFER ) ) ? rgba( 255, 0, 0, 255 ) : rgba( 0, 255, 0, 255 ) ); //rgba(0,uint((.5+cos(T)*.5)*255.),0,255));
}