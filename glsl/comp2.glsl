tex( 0 ) in_tex;

/*struct data_type
{
	ivec2 pos, size;
	uint col;
	uint id;
};*/

struct data_type
{
	vec3 pos;
};

data( 1 ) in_data
{
	data_type list[];
};
const int list_n = list.length();

uniform mat4 in_view;
uniform mat4 in_proj;

void main()
{
	if (N >= list_n) return;
	//
	//to(i, list_n)
	{
		//list[i].pos.z = 0.;
		//list[i].pos
		data_type d = list[N];
		vec3 d_pos = d.pos;//+(vec3(sin(T*3.+N),cos(T*3.+N),0.)*.1);//vec3(d.x, d.y, d.z);

		d_pos = mix(d_pos * 20., vec3(0.), pow((sin(T*.5)+1.)*.5, .1));

		//d_pos = vec3(0);//(mix(d_pos, vec3(0), 1./(T+1.)));

		vec4 in_pos = (in_proj * in_view * vec4(d_pos, 1.)).xyzw;

		//in_pos.z = mix(in_pos.z, 0., (sin(T)+1.)*2.);

		if (in_pos.z <= 0.) return;

		vec2 pos = in_pos.xy / -(in_pos.z*.5);
		if (abs(pos.x) > 1. || abs(pos.y) > 1.) return;

		pos = vec2(0.) + vec2(R) * (0.5 + pos / 2.);
		//

		//ivec2 tex_pos = ivec2(floor(d.pos.xy+.5));
		uint c = uint(4);//uint(pow(1. / length(d_pos),1.)*8.);
		c = min(255, uint(((pow(1./length((sqrt(abs(d_pos))*sign(d_pos))*.5), 3.)*511.)+1.)*.125));
		rgba col = rgba(c, c, c, 255);

		set(in_tex, ivec2(pos), col + get(in_tex, ivec2(pos)));

		//col = rgba(15,7,63,255);
		//set(in_tex, ivec2(pos)+ivec2(1,0), col + get(in_tex,ivec2(pos)+ivec2(1, 0)));
		//set(in_tex, ivec2(pos)+ivec2(-1,0), col + get(in_tex,ivec2(pos)+ivec2(-1, 0)));
		//set(in_tex, ivec2(pos)+ivec2(0,1), col + get(in_tex,ivec2(pos)+ivec2(0, 1)));
		//set(in_tex, ivec2(pos)+ivec2(0,-1), col + get(in_tex,ivec2(pos)+ivec2(0, -1)));

		//set(in_tex, ivec2(pos)+ivec2(1,0), col); //  + get(in_tex,ivec2(pos))

	}

	//set(in_tex, ivec2(pos), col2);

	//to(i, list_n)
	/*
	uint i = N;
	{
		data_type d = list[i];
		ivec2 pos = d.pos+ivec2(vec2(
		int(sin(float(T+d.id+N)*2.)*32.),
		int(cos(float(T+d.id+N)*2.)*32.))*-sin(float((T*.5)+(N*2)))),
		size = d.size;
		uint in_col = d.col;
		rgba col2 = rgba(
		((in_col) & 0xffu),
		((in_col>>8) & 0xffu),
		((in_col>>16) & 0xffu),
		((in_col>>24) & 0xffu));

		to(y, size.y)
		to(x, size.x)
		{ set(in_tex, ivec2(pos)+ivec2(x, y), col2); }
	}*/

	/*
		vec2 ppos = pos;
		//set(in_tex,ivec2(pos),rgba(0,255,127,255));
		//pos += vec2(sin((T*4.)+(float(N)*3.14159)),-sin((T*4.)+(float(N*7)*3.14159))) * .5;
		vec2 tpos, avgpos = vec2(0);
		float d;
		vec2 g;

		to(n, list_n)
		{
			if(n == N) continue;
			tpos = list[n].pos;
			avgpos += tpos;
		}

		avgpos /= float(list_n-1);

		g = pos - avgpos;
		d = max(abs(g.x),abs(g.y));
		pos += (vec2(g.x/d,g.y/d)/sqrt(1.+distance(avgpos,pos)))*2.;*/

	//pos = mix(pos,vec2(R/2),.2);//,pos,(((1. / (1.+distance(pos,vec2(R/2))))+2.)*.5)-.5);
	//pos = avg(pos,ppos);
	//pos = avg(pos,ppos);
	//list[N] = pos;


}