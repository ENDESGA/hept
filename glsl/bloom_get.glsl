in_tex( 0 ) tex;
in_tex( 2 ) lighting_tex;
/*
obj bloom_pixel
{
	ivec2 pos;
};
in_data( 4 ) bloom_pixels_data
{
	bloom_pixel in_list[];
};
const int list_n = in_list.length();*/

draw()
{
	return;
	rgba c = rgba(255);
	rgba in_col = get(tex, ivec2(ID));
	//if(N >= list_n) return;
	if (!(in_col.r >= 255 || in_col.g >= 255 || in_col.b >= 255)) return;

	//in_list[N].pos = ivec2() + ivec2(2);
	//in_list[0].pos = ivec2(77);

	//set(lighting_tex,ivec2(ID), c);

	ivec2 pos;
	to(y, 31)
	to(x, 31)
	{
		pos = ivec2(ID) + ivec2(x, y) - 15;
		c = rgba(uvec3(vec3(pow(1.-(distance(vec2(x, y), vec2(15, 15)) / 15.), 1.)) * 1. * in_col.rgb), 255);
		set(lighting_tex, pos, max(c, get(lighting_tex, pos)));
	}

	//set(lighting_tex,ivec2(ID), c);
}