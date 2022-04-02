in_tex( 0 ) tex;
//in_tex( 1 ) tiles_tex;
in_tex( 2 ) lighting_tex;

//#define T 8.95

draw()
{
	rgba c = rgba(256);
	rgba cp = c;
	rgba in_c = get(tex, ivec2(ID));
	//if(in_c.a>=0x11u ) return;

	//
	int x1 = ID.x, y1 = ID.y;
	int x2 = (R.x/2)+int(sin(T)*2000.), y2 = (R.y/2)+int(cos(T)*2000.);
	//x2 = x1 + 64;
	//y2 = y1 - 256;

	int xd = x2 - x1, yd = y2 - y1, hs = (x1 << 8) + 0x80, vs = (y1 << 8) + 0x80;
	int a = max(abs(xd), abs(yd)) * 2;// a must not = 0
	int xs = ((xd << 8) / a), ys = ((yd << 8) / a);// only 2 int-divides per-line
	//bool b = false;
	rgba tcol;
	ivec2 p;
	while (a-- >= 0)
	//to(n,a)
	{
		p = ivec2(((hs += xs) >> 8), ((vs += ys) >> 8));

		if ((p.x < 0 || p.x >= R.x || p.y < 0 || p.y >= R.y)) break;
		if (get(tex, p).a>=0x11u)
		{
			c = max(rgba(0), c - rgba(4));
			if (c.a <=0) break;
		}

		//hs += xs;
		//vs += ys;
	};
	rgba in_light = get(lighting_tex, ivec2(ID));//(get(lighting_tex, ivec2(ID)) + c + c + c + c + c + c) / 7;
	c = ((in_light << 6) + c) / 65;
	set(lighting_tex, ivec2(ID), c);
}