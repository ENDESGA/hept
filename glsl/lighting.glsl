in_tex( 0 ) tex;
//in_tex( 1 ) tiles_tex;
in_tex( 2 ) lighting_tex;

//#define T 18.95

draw()
{
	rgba c = rgba(255);
	rgba in_c = get(tex, ivec2(ID));
	if (in_c.a <= 0x11u) return;
	rgba in_lighting = get(lighting_tex, ivec2(ID));

	c = rgba(uvec3((vec3(in_c.rgb)/255.)*mix(pow(vec3(in_lighting.rgb)/255., vec3(2.)), vec3(1.), .2)*255.), 255);

	/*int x1 = ID.x, y1 = ID.y;
	int x2 = (R.x/2)+int(sin(T*2.)*128.), y2 = (R.y/2)+int(cos(T*2.)*64.)-128;
	float d = distance(vec2(x1, y1), vec2(x2, y2));
	if(d < 4) {
		set(tex, ivec2(ID), rgba(255));
		set(lighting_tex, ivec2(ID), rgba(255));
	}*/
	//if(in_c.a<=0x11u) in_c = rgba(0);
	set(tex, ivec2(ID), c);//(c/rgba(0x11u))*rgba(0x11u));//(rgba(c / (1.+(d*.005))) + c)/2);

	//set(tex,ivec2(ID), in_lighting);
}