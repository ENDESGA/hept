in_tex( 0 ) tex;
in_tex( 1 ) tiles_tex;
in_tex( 2 ) lighting_tex;

draw()
{
	rgba c = get(tiles_tex, ivec2(ID));
	vec3 sky = mix(vec3(.125, .25, 1.)*.25, (spectra((sin((T) - (PI/2.))+1.)*.25, .75)), .5);
	rgba col = rgba(sky * 255., 0);
	if (c.a <= 0x11u) c = col;
	set(tex, ivec2(ID), c);
	//if(N == 0)
	set(tex, ivec2(vec2(200, 100)+vec2(sin(T)*32., cos(T)*32.)), rgba(255));
}