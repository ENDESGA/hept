in_tex( 0 ) tex;
in_tex( 1 ) cam_tex;

draw()
{
	rgba c = get(cam_tex, ID.xy);
	set(tex, ID.xy, c);//rgba(255,0,64,255));
}