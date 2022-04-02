tex( 0 ) tex;

#define iTime 3
vec4 iMouse = vec4(0.);

vec2 asphere(in vec3 ro, in vec3 rd, in vec3 sp, in float sr, in float ow){
	// geometric solution for analytic ray-sphere intersection
	vec3 e0 = sp - ro;//vector from ray origin to center of sphere
	float e1 = dot(e0, rd);//distance along ray to closest point to center of sphere
	float r2 = dot(e0, e0) - e1*e1;//square of radius of closest point on ray to center of sphere
	float or2 = (sr+ow)*(sr+ow);//square of outline radius
	if (r2 > or2) return vec2(1001.0, 0.0);//miss value over 1k
	float sr2 = sr*sr;//square of sphere radius
	float e2 = sqrt(sr2 - r2);//distance along ray from closest point to center of sphere to edge of sphere.
	float st = e1-e2;//shortest distance to intersection of ray and sphere along ray
	float ot = e1;//distance along ray to closest point to center of sphere
	if (r2 < sr2) return vec2(st, 1.0);//distance to surface of sphere
	if (r2 < or2) return vec2(ot, 0.5);//distance to flat ring around sphere
	return vec2(-1.0);//error value -1
}

vec2 mins(in vec2 s1, in vec2 s2){ //returns vec2 with smallest first element.
	return (s1.x<s2.x)?s1:s2;
}

vec3 ascene(in vec3 ro, in vec3 rd, in float ow){
	vec2 t = mins(asphere(ro, rd, vec3(0.0, 0.0, 0.0), 1.5, ow),
	mins(asphere(ro, rd, vec3(-2, 0.0, 0.0), 1.0, ow),
	mins(asphere(ro, rd, vec3(0.0, -2, 0.0), 1.0, ow),
	mins(asphere(ro, rd, vec3(1.15, 1.15, 1.15), 1.0, ow),
	mins(asphere(ro, rd, vec3(0.0, 0.0, -2), 1.0, ow),
	asphere(ro, rd, vec3(3., 3., 3.), 0.2, ow)
	)
	)
	)
	)
	);
	vec3 col = vec3(0);
	if (t.y==1.0) {
		vec3 loc = t.x*rd+ro;
		loc = loc*0.5;
		col =  vec3(clamp(loc.x, 0.0, 1.0), clamp(loc.y, 0.0, 1.0), clamp(loc.z, 0.0, 1.0));
	}
	if (t.y==0.0){ //if a "miss" is returned, set pixel to 50% grey
		col = vec3(0.5);
	}
	return col;
}

void main()
{
	vec4 fragColor = vec4(0.);
	vec2 fragCoord = vec2(ID);

	//THIS v
	const int lensRes = 2;//THIS <
	const int ssaa = 1;//THIS <
	float lensDis = 0.5;//THIS <
	float lensSiz = 1.0;//THIS <
	float focalDis = 10.0;//THIS <
	float aberration = 0.1;//THIS <
	float outlineWidth = 0.1;//THIS <
	//THIS ^
	//fragcoord is the center of the pixel
	vec2 sensorLoc =
	vec2(0.5, 0.5*(iResolution.y/iResolution.x))//sets x limits from 0-1, y at same scale, center at (0.5,0.?)
	- fragCoord.xy / iResolution.x;//reverse sensor and center on (0,0)

	vec3 trueY = vec3(0.0, 1.0, 0.0);//useful later could be hardcoded later instead
	float tau = 0.5*iTime - 5.0*iMouse.x/iResolution.x;//tau used to determine camera position

	vec3 cameraPos = 10.0*vec3(1.0*sin(2.0*tau), 1.0*cos(1.5*tau), 1.0*cos(2.0*tau));//this is not normalized
	vec3 cameraDir = normalize(-cameraPos);//normalized ray from sensor center to 0,0,0
	vec3 cameraX = normalize(cross(cameraDir, trueY));//right dir for camera
	vec3 cameraY = normalize(cross(cameraX, cameraDir));//up dir for camera

	vec3 colorTotal = vec3(0.0);//for each pixel reset the accumulated color
	float colorCount = 0.0;//keep track of how many color samples are in final sum
	float lensResF = float(lensRes);//for comparing to float later
	float focal = 1.0+lensDis/focalDis;//brings the image to focus at focalDis from the cameraPos
	float ssaaF = float(ssaa);//for using later to save a cast.
	float sscale = 1.0/(iResolution.x);//size of a pixel
	float sstep = 1.0/ssaaF;//step for SSAA
	float sstart = sstep/2.0-0.5;//location of first SSAA step
	float lstep = 1.0/lensResF;//step for lens
	float lstart = lstep/2.0-0.5;//location of first lens step

	//Red Channel
	float rFocalDis = focalDis*(1.0+aberration);
	float rFocal = 1.0+lensDis/rFocalDis;


	//Green Channel
	float gFocalDis = focalDis;
	float gFocal = 1.0+lensDis/gFocalDis;


	//Blue Channel
	float bFocalDis = focalDis*(1.0-aberration);
	float bFocal = 1.0+lensDis/bFocalDis;

	float sx = sstart, sy = sstart;

	//for (float sx = sstart; sx < 0.5; sx += sstep){ //SSAA x direction
	//	for (float sy = sstart; sy < 0.5; sy += sstep){ //SSAA y direction

	vec2 ss = vec2(sx, sy)*sscale;//sub pixel offset for SSAA
	vec3 sensorRel = cameraX*(sensorLoc.x+ss.x) + cameraY*(sensorLoc.y+ss.y);//position on sensor relative to center of sensor. Used once
	vec3 sensorPos = cameraPos - lensDis*cameraDir + sensorRel;//3d position of ray1 origin on sensor

	for (float lx = lstart; lx < 0.5; lx+=lstep){
		for (float ly = lstart; ly < 0.5; ly+=lstep){

			vec2 lensCoord = vec2(lx, ly);//fragCoord analog for lens array. lens is square
			vec2 lensLoc = (lensCoord)*lensSiz;//location on 2d lens plane

			if (length(lensLoc)<(lensSiz/2.0)){ //trim lens to circle

				vec3 lensRel = cameraX*(lensLoc.x) + cameraY*(lensLoc.y);//position on lens relative to lens center. Used twice
				vec3 lensPos = cameraPos + lensRel;// 3d position of ray1 end and ray2 origin on lens
				vec3 senlenRay = lensPos - sensorPos;//direction of ray from sensor to lens

				//Red channel
				vec3 rRay = senlenRay - rFocal*(lensRel);//direction of ray afer being focused by lens
				rRay = normalize(rRay);//normalize after focus
				float red = ascene(lensPos, rRay, outlineWidth).x;//scene returns red

				//Green channel
				vec3 gRay = senlenRay - gFocal*(lensRel);//direction of ray afer being focused by lens
				gRay = normalize(gRay);//normalize after focus
				vec3 ucolor = ascene(lensPos, gRay, outlineWidth);//unaberrated scene
				float green = ucolor.y;//scene returns green

				//Blue channel
				vec3 bRay = senlenRay - bFocal*(lensRel);//direction of ray afer being focused by lens
				bRay = normalize(bRay);//normalize after focus
				float blue = ascene(lensPos, bRay, outlineWidth).z;//scene returns blue

				colorTotal = colorTotal+vec3(red, green, blue);//sum color over all points from lens
				colorCount += 1.0;//total number of colors added.
			}
		}
	}
	//	}
	//}
	fragColor = vec4(colorTotal/colorCount, 0.0);

	set(tex, ivec2(ID), rgba(fragColor * 255.));
}

/*
struct ray {
	vec3 o;//Origin
	vec3 d;//Direction
};
struct rec {
	bool hit;//hit?
	vec3 normal;//normal
	float d;//distance
	vec3 color;
	int mat;//materal
};
const rec noHit = rec(false, vec3(0), 10000., vec3(0), 0);

//Ray intersections by iq
rec box(in ray r, vec3 offset, vec3 boxSize, int mat, vec3 color, in vec3 m)
{
	r.o -= offset;
	vec3 n = m*r.o;
	vec3 k = abs(m)*boxSize;
	vec3 t1 = -n - k;
	vec3 t2 = -n + k;
	float tN = max(max(t1.x, t1.y), t1.z);
	float tF = min(min(t2.x, t2.y), t2.z);
	if (tN>tF || tF<0.0) return noHit;
	return rec(true, -sign(r.d)*step(t1.yzx, t1.xyz)*step(t1.zxy, t1.xyz), tN, color, mat);
}
rec sphere(ray r, vec3 offset, float ra, int mat, vec3 color)
{
	r.o -= offset;
	float b = dot(r.o, r.d);
	float c = dot(r.o, r.o) - ra*ra;
	float h = b*b - c;
	if (h<0.0) return noHit;
	h = sqrt(h);
	float d = -b-h;
	if (d < 0.0) return noHit;
	vec3 p = (r.o+offset)+r.d*d;
	return rec(true, (p-offset)/ra, d, color, mat);
}

	#define samples 1

	//0 is defalut scene, 1 is cooler scene (:
	#define scene 1


uint base_hash(uvec2 p) {
	p = 1103515245U*((p >> 1U)^(p.yx));
	uint h32 = 1103515245U*((p.x)^(p.y>>3U));
	return h32^(h32 >> 16);
}

float g_seed = 0.;

float hash() {
	uint n = base_hash(floatBitsToUint(vec2(g_seed+=.1, g_seed+=.1)));
	return float(n)*(1.0/float(0xffffffffU));
}

vec2 hash2() {
	uint n = base_hash(floatBitsToUint(vec2(g_seed+=.1, g_seed+=.1)));
	uvec2 rz = uvec2(n, n*48271U);
	return vec2(rz.xy & uvec2(0x7fffffffU))/float(0x7fffffff);
}

vec3 hash3() {
	uint n = base_hash(floatBitsToUint(vec2(g_seed+=.1, g_seed+=.1)));
	uvec3 rz = uvec3(n, n*16807U, n*48271U);
	return vec3(rz & uvec3(0x7fffffffU))/float(0x7fffffff);
}

ray cam(in vec2 coord) { //camera
	vec2 ri = 1./iResolution.xy;
	vec2 uv = ((coord.xy*ri)-0.5);
	uv.x *= iResolution.x/iResolution.y;
	uv.y *= -1.;
	vec2 m = vec2(sin(T*.1)+1.,cos(T*.1)+1.)*.5;
	float Y = cos(m.y);
	vec3 ro = vec3(sin(m.x)*Y, sin(m.y), cos(m.x)*Y)*17.;
	#if scene
	ro *= 0.08;
	#endif
	vec3 cam_z = normalize(-ro);
	vec3 cam_x = normalize(cross(vec3(0, 1, 0), cam_z));
	vec3 cam_y = cross(cam_z, cam_x);
	ro.y += 1.0;
	#if scene
	ro.y -= 1.5;
	#endif
	return ray(ro, normalize(uv.x * cam_x + uv.y * cam_y + 1. * cam_z));
}
	#define BOX(p, s, m, c) tmp = box(r, p, s, m, c, rdINV);if (tmp.d < o.d) { o = rec(true, tmp.normal, tmp.d, tmp.color, tmp.mat); }
	#define SPHERE(p, s, m, c) tmp = sphere(r, p, s, m, c);if (tmp.d < o.d) { o = rec(true, tmp.normal, tmp.d, tmp.color, tmp.mat); }
rec hitScene(in ray r) {
	vec3 rdINV = 1./r.d;
	int mat = 0;
	rec o = noHit;
	rec tmp;
	#if scene
	//       position            size            material     color
	BOX(vec3(0, -0.8, 0.0), vec3(5.0, 0.01, 5.0), 1, vec3(0.8));
//legs
BOX(vec3(0.25, -0.7, 0.25), vec3(0.025, 0.1, 0.025), 0, vec3(0.8));
BOX(vec3(0.25, -0.7, -0.25), vec3(0.025, 0.1, 0.025), 0, vec3(0.8));
BOX(vec3(-0.25, -0.7, 0.25), vec3(0.025, 0.1, 0.025), 0, vec3(0.8));
BOX(vec3(-0.25, -0.7, -0.25), vec3(0.025, 0.1, 0.025), 0, vec3(0.8));
//middle plate
BOX(vec3(0.0, -0.575, -0.0), vec3(0.275, 0.025, 0.275), 0, vec3(0.8));
BOX(vec3(0.0, -0.575, -0.0), vec3(0.025, 0.025, 0.5), 0, vec3(0.8));
BOX(vec3(0.0, -0.575, -0.0), vec3(0.5, 0.025, 0.025), 0, vec3(0.8));
//upper legs
BOX(vec3(0.475, -0.45, -0.0), vec3(0.025, 0.1, 0.025), 0, vec3(0.8, 0.1, 0.1));
BOX(vec3(-0.475, -0.45, 0.0), vec3(0.025, 0.1, 0.025), 0, vec3(0.1, 0.8, 0.1));
BOX(vec3(0.0, -0.45, 0.475), vec3(0.025, 0.1, 0.025), 0, vec3(0.1, 0.1, 0.8));
BOX(vec3(0.0, -0.45, -0.475), vec3(0.025, 0.1, 0.025), 0, vec3(0.8, 0.8, 0.1));
BOX(vec3(0.0, -0.4, 0.0), vec3(0.025, 0.15, 0.025), 0, vec3(0.5));
//upper plates
BOX(vec3(0.45, -0.325, 0.0), vec3(0.15, 0.025, 0.15), 0, vec3(0.8, 0.1, 0.1));
BOX(vec3(-0.45, -0.325, -0.0), vec3(0.15, 0.025, 0.15), 0, vec3(0.1, 0.8, 0.1));
BOX(vec3(0.0, -0.325, 0.45), vec3(0.15, 0.025, 0.15), 0, vec3(0.1, 0.1, 0.8));
BOX(vec3(0.0, -0.325, -0.45), vec3(0.15, 0.025, 0.15), 0, vec3(0.8, 0.8, 0.1));
BOX(vec3(0.0, -0.225, 0.0), vec3(0.15, 0.025, 0.15), 0, vec3(0.5));
//balls
SPHERE(vec3(0.0, -0.1, 0.0), 0.1, 3, vec3(0.8));
SPHERE(vec3(-0.45, -0.2, 0.0), 0.1, 2, vec3(0.8));
SPHERE(vec3(0.45, -0.2, 0.0), 0.1, 2, vec3(0.8));
SPHERE(vec3(0.0, -0.2, -0.45), 0.1, 2, vec3(0.8));
SPHERE(vec3(0.0, -0.2, 0.45), 0.1, 2, vec3(0.8));
#else
BOX(vec3(-4, 1.5, -4), vec3(0.4, 2., 0.4), 0, vec3(0.8, 0.3, 0.3));
BOX(vec3(-4, 1.5, 4), vec3(0.4, 2., 0.4), 0, vec3(0.8, 0.3, 0.3));
BOX(vec3(4, 1.5, -4), vec3(0.4, 2., 0.4), 0, vec3(0.8, 0.3, 0.3));
BOX(vec3(4, 1.5, 4), vec3(0.4, 2., 0.4), 0, vec3(0.8, 0.3, 0.3));
BOX(vec3(0, 3.5, 0), vec3(5., .1, 5.), 0, vec3(0.2, 0.6, 0.8));
BOX(vec3(0, 3.65, 0), vec3(4., .25, 4.), 1, vec3(0.8, 0.8, 0.3));
BOX(vec3(0, -.6, 0), vec3(20., .1, 20.), 0, vec3(0.8));
SPHERE(vec3(0.0, 5.9, 0.0), 2.0, 2, vec3(1));
SPHERE(vec3(0.0, 1.5, 0.0), 2.0, 3, vec3(0.8));
#endif

return o;//rec(d0 != 10000., (r.o+vec3(0, 0.45, 0))+r.d*d0, (normal), d0, color, mat);
}

vec3 rus() {
	vec3 h = hash3() * vec3(2., 6.28318530718, 1.)-vec3(1, 0, 0);
	float phi = h.y;
	float r = pow(h.z, 1./3.);
	return r * vec3(sqrt(1.-h.x*h.x)*vec2(sin(phi), cos(phi)), h.x);
}
vec3 sky(vec3 rd) {
	return vec3(1.);//texture(iChannel1, rd).xyz;//vec3(mix(vec3(0.75, 0.8, 1.0), vec3(0.9, 0.9, 1.0), -rd.y));
}

vec3 hsv(float h, float s, float v) {
	return mix(vec3(1.), clamp((abs(fract(h+vec3(3., 2., 1.)/3.)*6.-3.)-1.), 0., 1.), s)*v;
}

vec3 getColor(ray r) {
	vec3 v = vec3(1);
	to(i,100) {
		rec hit = hitScene(r);
		if (hit.hit) v *= hit.color;
		else return v*sky(r.d);
		vec3 nd;
		if (hit.mat == 0) nd = (hit.normal+rus());
		else if (hit.mat == 1) nd = reflect(r.d, (hit.normal+0.05*rus()));
		else if (hit.mat == 2) {
			if (hash() > (1./-dot(r.d, hit.normal))*0.1) nd = hit.normal+rus();
			else nd = reflect(r.d, hit.normal);
		}
		else if (hit.mat == 3) return (v*(hsv(T*.1,1.,1.) * 100.))*1.;
		r = ray(r.o+r.d*(hit.d-0.00001), normalize(nd));
	}
	return v;
}

void main()
{
	vec4 O = vec4(0);
	vec2 I = vec2(ID.xy);

	vec3 col = vec3(0);

	g_seed = (I.x*(I.y*0.001)+T)*0.1;
	//fragColor = vec4(hash(), hash(), hash(), 1);return;
	to(i,samples)
	{
		vec2 o = vec2(hash(), hash()-0.5);
		ray c = cam(I+o);
		col += max(getColor(c), 0.0);
	}
	col /= vec3(samples);

	O.rgb = 1. - exp(-col);

	vec4 C = vec4(get(tex,ivec2(ID)));

	set(tex,ivec2(ID),rgba(mix(C, (O * 255.), .05)));
}*/

/*
void main()
{
	vec2 uv = ID.xy / vec2(R.xy);
	float timeRatio = .0777;
	float s = T * timeRatio;

	vec3 p = vec3(s, s, s);
	vec3 C = vec3(7.777);

	for (int i = 0; i < 20; i++)
	{
		p += vec3(-sin(uv), sin(uv) -cos(uv));

		C += vec3(
		-sin(C.y + sin(p.y)),
		-cos(C.z + sin(p.z)),
		sin(C.x + (cos(p.x) * (cos(T * timeRatio * 2.))))
		);
	}
	C /= 20.;

	vec4 O = vec4(1. - exp(-C), 1.);

	//

	set(tex, ivec2(ID), rgba(O * 255.));
}*/

/*
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
	tri list[];
};
const int list_n = list.length();

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
}* /

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

/*void draw_triangle(vec3 v1, vec3 v2, vec3 v3)
{
	/*vec4 min_max = ceil(get_min_max(v1, v2, v3));
	uint startX = clamp(int(min_max.x), 0, (R.x));
	uint startY = clamp(int(min_max.y), 0, (R.y));
	uint endX = clamp(int(min_max.z), 0, (R.x));
	uint endY = clamp(int(min_max.w), 0, (R.y));* /

	vec2 vBBMin = min(v1.xy, min(v2.xy, v3.xy));
	vec2 vBBMax = max(v1.xy, max(v2.xy, v3.xy));

	// clip AABB to screen
	vBBMin = max(vBBMin.xy, 0);
	vBBMax = min(vBBMax.xy, R);

	//if

	for( uint y = startY; y < endY; y++)
	for( uint x = startX; x < endX; x++)
	/*to(y, endY - startY)
	to(x, endX - startX)* /
	{
		vec2 b = vec2(.4,.4);//bary(v1, v2, v3, vec2(float(x), float(y)));

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
}* /

void main()
{
	if (N >= list_n) return;

	tri in_tri = list[N];

	vec4 p1 = project(vec3(in_tri.a.x, in_tri.a.y, in_tri.a.z)*.333)+vec4(R/2, 0., 0.);
	vec4 p2 = project(vec3(in_tri.b.x, in_tri.b.y, in_tri.b.z)*.333)+vec4(R/2, 0., 0.);
	vec4 p3 = project(vec3(in_tri.c.x, in_tri.c.y, in_tri.c.z)*.333)+vec4(R/2, 0., 0.);

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

				ivec2 delta = ivec2(TriMax.x-TriMin.x, TriMax.y-TriMin.y);

				/*set(tex, ivec2(p1.xy), uvec4(255));
				set(tex, ivec2(p2.xy), uvec4(255));
				set(tex, ivec2(p3.xy), uvec4(255));* /

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
				}
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

	/*vec3 in_pos = project(vec3(in_pnt.x, in_pnt.y, in_pnt.z)*.25);
	ivec2 pos = ivec2(in_pos.xy) + (R/2);* /
	set(tex, ivec2(p1.xy), uvec4(255));
	set(tex, ivec2(p2.xy), uvec4(255));
	set(tex, ivec2(p3.xy), uvec4(255));
}

*/

/*
void line(vec3 a, vec3 b)
{
	//if(a.z<=0. || b.z<=0.) return;
	int x1 = int(a.x), y1 = int(a.y);
	int x2 = int(b.x), y2 = int(b.y);

	//int xd = x2 - x1, yd = y2 - y1;
	//int hs = (x1 << 8) + 0x7f, vs = (y1 << 8) + 0x7f;

	int xd, yd, hs, vs;
	float d1, d2;
	if (a.z >= b.z)
	{
		xd = x2 - x1;
		yd = y2 - y1;
		hs = (x1 << 8) + 0x7f;
		vs = (y1 << 8) + 0x7f;
		d1 = b.z;
		d2 = a.z;
	} else
	{
		xd = x1 - x2;
		yd = y1 - y2;
		hs = (x2 << 8) + 0x7f;
		vs = (y2 << 8) + 0x7f;
		d1 = a.z;
		d2 = b.z;
	}


	int g = max(abs(xd), abs(yd));
	if (g<=0) return;
	float gm = g;

	int xs = ((xd << 8) / (g)), ys = ((yd << 8) / (g));

	ivec2 p;
	float d;
	while (g-- >= 0)
	{
		p = ivec2((hs >> 8), (vs >> 8));
		d = mix(d1, d2, float(g)/gm);

		if ((d >= 0.) && (p.x >= 0 && p.x < R.x && p.y >= 0 && p.y < R.y))
		{
			set(tex,p,
				max(uvec4((1./d)*255.), get(tex,p))
			);
		} else break;

		hs += xs;
		vs += ys;
	};
}

void arc(int xx, int yy, int r)
{

	// Draw the arc
	int x = 0;
	int y = r;
	int d = 1 - r;
	int deltaE = 3;
	int deltaSE = -2 * r + 5;
	set(tex,ivec2(xx+x,yy+y),uvec4(255));
	set(tex,ivec2(xx+y,yy+x),uvec4(255));
	set(tex,ivec2(xx-x,yy-y),uvec4(255));
	set(tex,ivec2(xx-y,yy-x),uvec4(255));
	while (y > x)
	{
		if (d < 0)
		{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
		}
		else
		{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;
			y--;
		}
		x++;
		set(tex, ivec2(xx+x, yy+y), uvec4(255));
		set(tex, ivec2(xx+y, yy+x), uvec4(255));

		set(tex, ivec2(xx-x, yy-y), uvec4(255));
		set(tex, ivec2(xx-y, yy-x), uvec4(255));

		set(tex, ivec2(xx+x, yy-y), uvec4(255));
		set(tex, ivec2(xx+y, yy-x), uvec4(255));

		set(tex, ivec2(xx-x, yy+y), uvec4(255));
		set(tex, ivec2(xx-y, yy+x), uvec4(255));
	}
}

void main()
{
	if (N > list_n) return;

	//if(N>((sin(T)+1.)*3.)) return;
	//pnt in_pnt = list[N];

	//to(n,3)
	{
		vec3 pos_eye = (eye_proj * eye_view * vec4(list[n] * SCL, 1.)).xyz;
		vec3 pos_tex1 = vec3(
			vec2(R) * (.5 + (pos_eye.xy / -max((pos_eye.z < 0.) ? ((pos_eye.z)) : (pos_eye.z),.001)) / 2.),
			pos_eye.z);

		pos_eye = (eye_proj * eye_view * vec4(list[n+1] * SCL, 1.)).xyz;
		vec3 pos_tex2 = vec3(
			vec2(R) * (.5 + (pos_eye.xy / -max((pos_eye.z < 0.) ? ((pos_eye.z)) : (pos_eye.z), .001)) / 2.),
			pos_eye.z);

		pos_eye = (eye_proj * eye_view * vec4(list[n+2] * SCL, 1.)).xyz;
		vec3 pos_tex3 = vec3(
		vec2(R) * (.5 + (pos_eye.xy / -max((pos_eye.z < 0.) ? ((pos_eye.z)) : (pos_eye.z), .001)) / 2.),
		pos_eye.z);

		if (
		(pos_tex1.x >= 0 && pos_tex1.x < R.x && pos_tex1.y >= 0 && pos_tex1.y < R.y) ||
		(pos_tex2.x >= 0 && pos_tex2.x < R.x && pos_tex2.y >= 0 && pos_tex2.y < R.y) ||
		(pos_tex3.x >= 0 && pos_tex3.x < R.x && pos_tex3.y >= 0 && pos_tex3.y < R.y))
		{
			line(pos_tex1, pos_tex2);
			line(pos_tex2, pos_tex3);
			line(pos_tex3, pos_tex1);
		}

		set(tex, ivec2(pos_tex1), uvec4(uvec3(hsv((pos_eye.z*.3), 1., 1.)*255.), 255));
		set(tex, ivec2(pos_tex2), uvec4(uvec3(hsv((pos_eye.z*.3), 1., 1.)*255.), 255));
		set(tex, ivec2(pos_tex3), uvec4(uvec3(hsv((pos_eye.z*.3), 1., 1.)*255.), 255));

		arc(int(pos_tex1.x),int(pos_tex1.y),(3 + int(((sin(T*2.)+1.)*7.)))/2);
		arc(int(pos_tex2.x),int(pos_tex2.y),(3 + int(((sin(T*2.)+1.)*7.)))/2);
		arc(int(pos_tex3.x),int(pos_tex3.y),(3 + int(((sin(T*2.)+1.)*7.)))/2);
	}
}
*/
/*

	pnt in_pnt = list[N];
	vec3 pnt_pos = in_pnt.p1 * SCL;
	pnt_pos /= length(pnt_pos);

	vec3 pos_eye = (eye_proj * eye_view * vec4(pnt_pos, 1.)).xyz;
	//if (pos_eye.z <= 0.) return;

	vec2 pos_tex1 = pos_eye.xy / -(pos_eye.z*.5);
	//if (abs(pos_tex1.x) > 1. || abs(pos_tex1.y) > 1.) return;
	pos_tex1 = vec2(R) * (0.5 + pos_tex1 / 2.);
	float d1 = pos_eye.z;//distance(pnt_pos, eye_pos);

	//

	pnt_pos = in_pnt.p2 * SCL;
	pnt_pos /= length(pnt_pos/3.);

	pos_eye = (eye_proj * eye_view * vec4(pnt_pos, 1.)).xyz;
	//if (pos_eye.z <= 0.) return;

	vec2 pos_tex2 = pos_eye.xy / -(pos_eye.z*.5);
	//if (abs(pos_tex2.x) > 1. || abs(pos_tex2.y) > 1.) return;
	pos_tex2 = vec2(R) * (0.5 + pos_tex2 / 2.);
	float d2 = pos_eye.z;//distance(pnt_pos, eye_pos);

	//
	/*
		pnt_pos = in_pnt.p3 * SCL;

		pos_eye = (eye_proj * eye_view * vec4(pnt_pos, 1.)).xyz;
		if (pos_eye.z <= 0.) return;

		vec2 pos_tex3 = pos_eye.xy / -(pos_eye.z*.5);
		if (abs(pos_tex3.x) > 1. || abs(pos_tex3.y) > 1.) return;
		pos_tex3 = vec2(R) * (0.5 + pos_tex3 / 2.);
		float d3 = distance(pnt_pos, eye_pos);* /

//

//if ((pos_tex1.x < 0 || pos_tex1.x >= R.x || pos_tex1.y < 0 || pos_tex1.y >= R.y)) return;
//if ((pos_tex2.x < 0 || pos_tex2.x >= R.x || pos_tex2.y < 0 || pos_tex2.y >= R.y)) return;

int x1 = int(pos_tex1.x), y1 = int(pos_tex1.y);
int x2 = int(pos_tex2.x), y2 = int(pos_tex2.y);

int xd, yd;
if (d1>=d2)
{ xd = x2 - x1;
	yd = y2 - y1; } else
{ xd = x2 - x1;
	yd = y2 - y1; }

int hs = (x1 << 16) + 0x8000, vs = (y1 << 16) + 0x8000;
int a = max(1, max(abs(xd), abs(yd)));// a must not = 0
if (a<=0) return;
//if(a>sqrt((R.x*R.x)+(R.y*R.y))) return;
float am = a;
int xs = ((xd << 16) / (a)), ys = ((yd << 16) / (a));// only 2 int-divides per-line
ivec2 p;
float d;
while (a-- >= 0)
{

	p = ivec2((hs >> 16), (vs >> 16));
	if (!(p.x < 0 || p.x >= R.x || p.y < 0 || p.y >= R.y))
	{
		d = mix(d2, d1, float(a)/am);
		if (d>0.)
		{ set(tex, p,
		max(get(tex, p),
		uvec4(uvec3(hsv(((T*.5)+(float(N)/360.))*.5, 1., 1.)*255.), 255))
		); } }

	hs += xs;
	vs += ys;
};

//

/*
	float d = distance(pnt_pos,eye_pos);
	vec3 pcol = vec3(get(tex,ivec2(pos_tex)).rgb);
	vec3 col = vec3(pow(1. / (max(1.,d*.5)),2.) * 10000.);* /

//set(tex, ivec2(pos_tex1), ivec4(255));
*/