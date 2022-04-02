tex( 0 ) tex;
tex( 1 ) tex_depth;

obj pnt
{
	float x, y, z;
};
obj tri_bnd
{
	pnt a, b, c;
	uint w, h;
};
data( 3 ) in_data
{
	tri_bnd list[];
};
const int list_n = list.length();

//

/*vec3 bary(vec3 p1, vec3 p2, vec3 p3, vec2 pos)
{
	vec3 recipw = vec3 (1.0/p1.z, 1.0/p2.z, 1.0/p3.z);

	vec2 posv1 = pos - vec2(p1);

	vec2 v21 = vec2(p2) - vec2(p1);
	vec2 v31 = vec2(p3) - vec2(p1);

	float baryis = (posv1.x * v31.y - posv1.y * v31.x);
	float baryjs = (posv1.y * v21.x - posv1.x * v21.y);

	float newis = recipw.y * baryis;
	float newjs = recipw.z * baryjs;

	float rnewws = 1.0/(recipw.x * ((v21.x * v31.y - v21.y * v31.x) - baryis - baryjs) + newis + newjs);

	float u = newis * rnewws, v = newjs * rnewws;
	//
	vec3 z = cross(vec3(u, v, 1), recipw);
	//

	return vec3(u,v, z);
}*/

vec3 bary(vec3 v1, vec3 v2, vec3 v3, vec2 p) {
	vec3 u = cross(
	vec3(v3.x - v1.x, v2.x - v1.x, v1.x - p.x),
	vec3(v3.y - v1.y, v2.y - v1.y, v1.y - p.y)
	);

	if (abs(u.z) < 1.0) {
		return vec3(-1.0, 1.0, 1.0);
	}

	return vec3(1.0 - (u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}

float cross2d(in vec2 v1, in vec2 v2)
{
	return (v1.x*v2.y) - (v1.y*v2.x);
}

vec3 bdepth(in vec3 p1, in vec3 p2, in vec3 p3, in vec2 POS)
{

	vec2 tp1 = p1.xy;
	vec2 tp2 = p2.xy;
	vec2 tp3 = p3.xy;

	float triangleArea = abs(cross2d(tp2 - tp1, tp3 - tp1)) * 0.5;

	float w1 = (cross2d(tp2 - POS, tp1 - POS) * 0.5) / triangleArea;
	float w2 = (cross2d(tp2 - POS, POS - tp3) * 0.5) / triangleArea;
	float w3 = (cross2d(tp3 - POS, POS - tp1) * 0.5) / triangleArea;

	if (w1 >= 0.0 || w2 >= 0.0 || w3 >= 0.0)
	{
		return vec3(0);
	}


	float z1 = 1.0 / p1.z;
	float z2 = 1.0 / p2.z;
	float z3 = 1.0 / p3.z;

	float depth = 1.0 / (w2 * z1 + w3 * z2 + w1 * z3);

	// Depth test, reject sample if it's z value is larger than smallest sample.
	if (depth*-255 <= get(tex_depth, ivec2(POS)).r)
	{
		return vec3(-1);
	}

	//z = depth;

	// Perspective correction for UV coordinates
	vec2 uv = w2 * (vec2(0, 0) * z1) + w3 * (vec2(1, 0) * z2) + w1 * (vec2(1, 1) * z3);
	uv *= depth;

	//vec3 color = vec3(depth*-1.);//texture(iChannel0, uv);

	// diffuse lighting
	//float d = dot(t.faceNormal, vec3(0.0, 0.0, -1.0));
	//d = clamp(d, 0.0, 1.0);
	//color *= pow(1./(depth * depth), 2.);//pow(1./depth,4.)*4.;//vec4(d);
	return vec3(uv, depth*-1.);
}

//

void main()
{
	tri_bnd tb;
	to(t, list_n)
	{
		tb = list[t];


		if (N > (tb.w * tb.h)) continue;
		uint W = min(tb.w, R.x), H = min(tb.h, R.y);
		int Nx = int(N % W), Ny = int(N / W);
		vec3 p1 = vec3(tb.a.x, tb.a.y, tb.a.z);
		vec3 p2 = vec3(tb.b.x, tb.b.y, tb.b.z);
		vec3 p3 = vec3(tb.c.x, tb.c.y, tb.c.z);
		if (!(
		((p1.x>=0 && p1.x < R.x) && (p1.y>=0 && p1.y < R.y)) ||
		((p2.x>=0 && p2.x < R.x) && (p2.y>=0 && p2.y < R.y)) ||
		((p3.x>=0 && p3.x < R.x) && (p3.y>=0 && p3.y < R.y))
		)) continue;

		vec2 v01 = p2.xy-p1.xy;
		vec2 v20 = p1.xy-p3.xy;
		if ((v01.x*v20.y - v20.x*v01.y) < 0)
		{

			ivec2 minxy = ivec2(
			clamp(min(min(tb.a.x, tb.b.x), tb.c.x), 0, R.x),
			clamp(min(min(tb.a.y, tb.b.y), tb.c.y), 0, R.y)
			);

			//if (
			//(p1.x>=0 && p1.x < R.x) && (p1.y>=0 && p1.y < R.y) ||
			//(p2.x>=0 && p2.x < R.x) && (p2.y>=0 && p2.y < R.y) ||
			//(p3.x>=0 && p3.x < R.x) && (p3.y>=0 && p3.y < R.y)
			//)
			{

				//vec3 b = bary(p1, p2, p3, vec2(float(Nx + minxy.x), float(Ny + minxy.y)));
				vec3 bd = bdepth(p1, p2, p3, vec2(Nx, Ny) + minxy);

				//if ((b.x >= 0.0) &&
				//(b.y >= 0.0) &&
				//(b.x + b.y < 1.0))
				if (bd.z >0)
				{


					set(tex_depth, ivec2(Nx, Ny) + ivec2(minxy), uvec4(uvec3(bd.z * 255.), 255));
					set(tex, ivec2(Nx, Ny) + ivec2(minxy), uvec4(uvec3(bd.z * 255.), 255));

					//set(tex, ivec2(tb.a.x, tb.a.y), rgba(255));
					//set(tex, ivec2(tb.b.x, tb.b.y), rgba(255));
					//set(tex, ivec2(tb.c.x, tb.c.y), rgba(255));
				}// else set(tex, ivec2(Nx, Ny) + minxy, rgba(127, 0, 0, 255));
			}
		}
	}

}