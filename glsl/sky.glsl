in_tex( 0 ) tex;
in_tex( 1 ) cam_tex;

draw()
{
	//rgba c = get(tex, ID.xy);
	set(cam_tex, ID.xy, (ID.x > R.x) ? rgba(255, 0, 0, 255) : rgba(0, 255, 0, 255));//rgba(0,uint((.5+cos(T)*.5)*255.),0,255));
}