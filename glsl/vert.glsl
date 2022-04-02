tex( 0 ) tex;

//

obj pnt
{
	float x, y, z;
};
obj tri
{
	pnt a, b, c;
};

data( 2 ) in_data
{
	tri in_list[];
};
const int list_n = in_list.length();

//

obj tri_bnd
{
	pnt a, b, c;
	uint w, h;
};

data( 3 ) out_data
{
	tri_bnd out_list[];
};
//const int list_n = out_list.length();

//

uniform vec3 eye_pos;
uniform mat4 eye_view;
uniform mat4 eye_proj;

const float SCL = 1.;

vec3 hsv(float h, float s, float v) {
	return mix(vec3(1.), clamp((abs(fract(h+vec3(3., 2., 1.)/3.)*6.-3.)-1.), 0., 1.), s)*v;
}

//

vec4 project(vec3 v) {
	vec4 p = eye_proj * eye_view * vec4(v.x, v.y, v.z, 1.0);
	p.x = (p.x / -p.w) * float(R.x);
	p.y = (p.y / -p.w) * float(R.y);
	p.w = 1./p.w;
	return p;
}

/*vec3 bary(vec3 v1, vec3 v2, vec3 v3, vec2 p) {
	vec3 u = cross(
	vec3(v3.x - v1.x, v2.x - v1.x, v1.x - p.x),
	vec3(v3.y - v1.y, v2.y - v1.y, v1.y - p.y)
	);

	if (abs(u.z) < 1.0) {
		return vec3(-1.0, 1.0, 1.0);
	}

	return vec3(1.0 - (u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}*/

vec2 bary(vec3 v1, vec3 v2, vec3 v3, vec2 pos)
{
	// Note some 3D pipelines will already have 1/w for each vertex, so this line may not be needed.
	vec3 recipw = vec3 (1.0/v1.z, 1.0/v2.z, 1.0/v3.z);

	vec2 posv1 = pos - vec2(v1);

	vec2 v21 = vec2(v2) - vec2(v1);
	vec2 v31 = vec2(v3) - vec2(v1);

	float baryis = (posv1.x * v31.y - posv1.y * v31.x);
	float baryjs = (posv1.y * v21.x - posv1.x * v21.y);

	float newis = recipw.y * baryis;
	float newjs = recipw.z * baryjs;

	float rnewws = 1.0/(recipw.x * ((v21.x * v31.y - v21.y * v31.x) - baryis - baryjs) + newis + newjs);
	return vec2(newis * rnewws, newjs * rnewws);
}

vec4 get_min_max(vec3 v1, vec3 v2, vec3 v3) {
	return vec4(
	min(min(v1.x, v2.x), v3.x),
	min(min(v1.y, v2.y), v3.y),
	max(max(v1.x, v2.x), v3.x),
	max(max(v1.y, v2.y), v3.y));
}

void draw_triangle(vec3 v1, vec3 v2, vec3 v3)
{
	vec4 min_max = ceil(get_min_max(v1, v2, v3));
	uint startX = clamp(int(min_max.x), 0, (R.x));
	uint startY = clamp(int(min_max.y), 0, (R.y));
	uint endX = clamp(int(min_max.z), 0, (R.x));
	uint endY = clamp(int(min_max.w), 0, (R.y));

	vec2 vBBMin = min(v1.xy, min(v2.xy, v3.xy));
	vec2 vBBMax = max(v1.xy, max(v2.xy, v3.xy));

	// clip AABB to screen
	vBBMin = max(vBBMin.xy, 0);
	vBBMax = min(vBBMax.xy, R);

	//if

	for (uint y = startY; y < endY; y++)
	for (uint x = startX; x < endX; x++)
	/*to(y, endY - startY)
	to(x, endX - startX)*/
	{
		vec2 b = vec2(.4, .4);//bary(v1, v2, v3, vec2(float(x), float(y)));

		if ((b.x >= 0.0) &&
		(b.y >= 0.0) &&
		(b.x + b.y < 1.0))
		{
			vec3 col;
			col.xy = b;
			col.z = 1.0;
			if (fract (b.x * 4.0) < 0.05) { col.z = 0.0; }
			if (fract (b.y * 4.0) < 0.05) { col.z = 0.0; }

			set(tex, ivec2(x, y), uvec4(uvec3(col*255.), 255));
		}
	}
}

void main()
{


	if (N >= list_n) return;

	tri in_tri = in_list[N];

	vec4 p1 = project(vec3(in_tri.a.x, in_tri.a.y, in_tri.a.z)*.333)+vec4(R/2, 0., 0.);
	vec4 p2 = project(vec3(in_tri.b.x, in_tri.b.y, in_tri.b.z)*.333)+vec4(R/2, 0., 0.);
	vec4 p3 = project(vec3(in_tri.c.x, in_tri.c.y, in_tri.c.z)*.333)+vec4(R/2, 0., 0.);

	//set(tex, ivec2(p1.xy), rgba(255,0,0,255));
	//set(tex, ivec2(p2.xy), rgba(255,0,0,255));
	//set(tex, ivec2(p3.xy), rgba(255,0,0,255));
	/*
		if (
		(p1.x>=0 && p1.x < R.x) && (p1.y>=0 && p1.y < R.y) ||
		(p2.x>=0 && p2.x < R.x) && (p2.y>=0 && p2.y < R.y) ||
		(p3.x>=0 && p3.x < R.x) && (p3.y>=0 && p3.y < R.y)
		) {
			vec2 vBBMin = min(p1.xy, min(p2.xy, p3.xy));
			vec2 vBBMax = max(p1.xy, max(p2.xy, p3.xy));

			// clip AABB to screen
			vBBMin = max(vBBMin.xy, 0);
			vBBMax = min(vBBMax.xy, R);

			if (all(lessThan(vBBMin, vBBMax)))
			{
				vec2 v01 = p2.xy-p1.xy;
				vec2 v20 = p1.xy-p3.xy;
				if ((v01.x*v20.y - v20.x*v01.y) < 0)
				{
					vec2 N01 = vec2(-v01.y, v01.x);
					vec2 N20 = vec2(-v20.y, v20.x);
					vec3 E01 = vec3(N01.xy, -dot(N01, p1.xy));
					vec3 E20 = vec3(N20.xy, -dot(N20, p1.xy));
					E01 *= pow(dot(E01, vec3(p3.xy, 1)),-1.);
					E20 *= pow(dot(E20, vec3(p2.xy, 1)),-1.);

					// Setup interpolation functions for Z and 1/w
					vec3 ZPlane = vec3(p1.z, p3.z-p1.z, p2.z-p1.z);
					vec3 WPlane = vec3(p1.w, p3.w-p1.w, p2.w-p1.w);

					// Round AABB to pixels
					vec2 TriMin = floor(vBBMin);
					vec2 TriMax = ceil(vBBMax);

					// offset to pixel center
					TriMin += 0.5f;
					TriMax += 0.5f;

					ivec2 delta = ivec2(TriMax.x-TriMin.x, TriMax.y-TriMin.y);*/

	//set(tex, ivec2(p1.xy), rgba(255));
	//set(tex, ivec2(p2.xy), rgba(255));
	//set(tex, ivec2(p3.xy), rgba(255));

	ivec2 trimin = ivec2(floor(min(p1.xy, min(p2.xy, p3.xy))));
	ivec2 trimax = ivec2(ceil(max(p1.xy, max(p2.xy, p3.xy))));

	uvec2 delta = uvec2(trimax.x-trimin.x, trimax.y-trimin.y);

	//if(N == 0)
	{
		tri_bnd tb;

		tb.a.x = p1.x;
		tb.a.y = p1.y;
		tb.a.z = p1.w;

		tb.b.x = p2.x;
		tb.b.y = p2.y;
		tb.b.z = p2.w;

		tb.c.x = p3.x;
		tb.c.y = p3.y;
		tb.c.z = p3.w;

		tb.w = (delta.x);
		tb.h = (delta.y);

		out_list[N] = tb;
		//workgroups_x = max(workgroups_x, int(ceil(float(delta.x * delta.y) / 1024.)));
	}
	/*
	/*
					vec2 vPos;
					for (vPos.y=TriMin.y; vPos.y < TriMax.y; vPos.y++)
					{
						for (vPos.x=TriMin.x; vPos.x < TriMax.x; vPos.x++)
						{
							float u = dot(vec3(vPos, 1), E01);
							float v = dot(vec3(vPos, 1), E20);

							if (u>=0 && v>=0 && (u+v)<=1)
							{
								float z = dot(vec3(1, u, v), ZPlane);
								float w = dot(vec3(1, u, v), WPlane);
								uint depth = uint((1. - exp(-(1./(z/w))))*400.);

								vec3 col;
								col.xy = vec2(u,v);
								col.z = 1.0;
								if (fract(u * 4.0) < 0.05) { col.z = 0.0; }
								if (fract(v * 4.0) < 0.05) { col.z = 0.0; }

								if(get(tex, ivec2(vPos.xy)).a < depth) set(tex,ivec2(vPos.xy),uvec4(depth));
							}
						}
					}* /
					//memoryBarrierShared(); barrier();
				}
			}

			/*	for (uint y = startY; y < endY; y++)
				for (uint x = startX; x < endX; x++)
				{
					vec2 b = vec2(.4, .4);//bary(v1, v2, v3, vec2(float(x), float(y)));

					if ((b.x >= 0.0) &&
					(b.y >= 0.0) &&
					(b.x + b.y < 1.0))
					{
						vec3 col;
						col.xy = b;
						col.z = 1.0;
						if (fract (b.x * 4.0) < 0.05) { col.z = 0.0; }
						if (fract (b.y * 4.0) < 0.05) { col.z = 0.0; }

						set(tex, ivec2(x, y), uvec4(uvec3(col*255.), 255));
					}
				}* /
		}
	*/
	/*vec3 in_pos = project(vec3(in_pnt.x, in_pnt.y, in_pnt.z)*.25);
	ivec2 pos = ivec2(in_pos.xy) + (R/2);*/
	/*set(tex, ivec2(p1.xy), uvec4(255));
	set(tex, ivec2(p2.xy), uvec4(255));
	set(tex, ivec2(p3.xy), uvec4(255));*/
}





/*tex( 0 ) tex;

data( 1 ) out_args {
	uint workgroups_x;
	uint workgroups_y;
	uint workgroups_z;
};

obj pnt { float x, y, z; };
obj tri { pnt a, b, c; };
data( 2 ) in_data {
	tri list[];
};
const int list_n = list.length();

//

vec4 project(vec3 v) {
	vec4 p = eye_proj * eye_view * vec4(v.x, v.y, v.z, 1.0);
	p.x = (p.x / -p.w) * float(R.x);
	p.y = (p.y / -p.w) * float(R.y);
	p.w = 1./p.w;
	return p;
}

//

void main()
{
	if(N > 3) return;

	workgroups_x = 7;
	workgroups_y = 1;
	workgroups_z = 1;

	set(tex, ivec2(N),uvec4(255));
}*/