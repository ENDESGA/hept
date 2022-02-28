#version 460 core

precision lowp float;

//layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
layout(rgba8ui, binding = 0)
uniform uimage2D screen;
uniform int in_frame;

layout(rgba8ui, binding = 1)
uniform uimage2D tex_in;

uniform float in_uni;

vec3 iResolution = vec3(640., 320., 0.);
float iTime = in_uni;
vec4 iMouse = vec4(0.);

int iFrame = in_frame;

//

#define draw imageStore

//

#define AO_SAMPLES 1

// comment these out to see the raw effect
//#define DITHER
//#define BLEND_FRAMES

// comment/uncomment these to turn different light effects on/off
#define LIGHTS
#define AMBIENT_LIGHT
//#define COLOURISE_AMBIENT

const float phi = 1.6180339887498948;
const float tau = 6.2831853071795865;

uvec2 quasi2 = uvec2(0xC13FA9A9u, 0x91E10DA5u);
uvec3 quasi3 = uvec3(3518319155, 2882110345, 2360945575);

float sdf(vec3 p)
{
    return
    min(min(min(min(min(
    max(max(max(
    length(p)-1.,
    .3 - length((p-vec3(.5, 0, 0)).xz)),
    .3 - length((p-vec3(-.25, 0, -.5)).xz)),
    dot(p, normalize(vec3(-1, 0, -2)))-.4 + .003*sin(p.y*100.)
    ),
    max(max(
    length(p-vec3(0, -.5, 1.2))-.5,
    .45-length(p-vec3(0, -.5, 1.2))),
    dot(p-vec3(0, -.5, 1.2), normalize(vec3(1, 2, -3)))
    )),
    length(p-vec3(0, -.5, 1.2))-.3),
    max(
    length(p.xz)-.5,
    p.y+.9
    )),
    max(
    length((p-vec3(-.25, 0, -.5)).xz) - .22,
    dot(p, normalize(vec3(-1, 4, -2)))-.95
    )),
    p.y + 1.
    );
}

    #define maxSteps 1000
    #define stepSize .01
float OcclusionTrace(vec3 pos, vec3 target, float rpert)
{
    vec3 toTarget = target-pos;
    float end = length(toTarget);
    vec3 ray = toTarget/end;
    float t = 1.;
    float visibility = 1.;
    for (uint i=min(0, in_frame); i < maxSteps; i++)
    {
        float h = sdf(ray*t+pos);
        if (t >= end) break;
        float r = rpert*t;
        visibility = min(visibility, smoothstep(-r, r, h));
        if (visibility <= 0.) break;
        t += (h+r) * stepSize;
    }
    return visibility;
}

uint _HEPT32_X = 0x77777777u, _HEPT32_Y = 0x77777777u, _HEPT32_Z = 0x77777777u;

uint hept32(uint x, uint y, uint z) {
    x = (x * _HEPT32_X) - (~x * 0x77777777u) - ~(x * _HEPT32_Y);
    y = (y * _HEPT32_Y) - (~y * 0x77777777u) - ~(y * _HEPT32_Z);
    z = (z * _HEPT32_Z) - (~z * 0x77777777u) - ~(z * _HEPT32_X);
    return ~(~(~x * y * z) * ~(x * ~y * z) * ~(x * y * ~z));
}

void hept32_seed(uint seed) {
    // multi-layer seed expansion and extraction
    seed = (hept32(seed, seed, seed) * 0x77777777u) - 0x77777777u;
    _HEPT32_X = (hept32(0x77777777u, seed, seed) * seed) - seed;
    _HEPT32_Y = (hept32(seed, 0x77777777u, seed) * seed) - seed;
    _HEPT32_Z = (hept32(seed, seed, 0x77777777u) * seed) - seed;
}

float noise(uint x, uint y, uint z) {
    return (float(hept32((x), (y), (z))) * 2.32830644e-10);
}

void main()
{
    int F = iFrame;
    iFrame = 7;

    //vec4 pixel = vec4(0.);
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    vec2 u = vec2(float(pixel_coords.x), iResolution.y - float(pixel_coords.y));

    vec2 _fragCoord = u;
    vec4 fragColour = vec4(1.);
    vec2 I = u;
    //vec4 O = vec4(0.);

    //

    //hept32_seed(uint(_fragCoord.x + _fragCoord.y) + iFrame + 7);
    hept32_seed(7);

    uint seed = uint(_fragCoord.x)*quasi3.x
    + uint(_fragCoord.y)*quasi3.y
    + uint(iFrame)*quasi3.z;
    seed = hept32(pixel_coords.x, pixel_coords.y, iFrame);
    float N = noise(pixel_coords.x, pixel_coords.y, iFrame);



    vec2 jitter = vec2(noise(pixel_coords.x, pixel_coords.y, (F*2)), noise(pixel_coords.x, pixel_coords.y, (F*2)+1));//vec2(quasi2 * seed) / exp2(32.);

    if (((F / 1) % 2) == 0) { N = 1. - N; seed = ~seed; jitter = 1. - jitter; }

    vec2 fragCoord = _fragCoord + jitter;// - vec2(iFrame % 3);

    vec3 ray = vec3((fragCoord-iResolution.xy*.5)/sqrt(iResolution.x*iResolution.y), iTime * .1);

    vec3 camPos = vec3(0, 0, -8.);
    vec2 cama = vec2(.3, iTime*.3);//-1.5);//
    vec2 d = vec2(-1, 1);
    camPos.yz = camPos.yz*cos(cama.x) + camPos.zy*d.xy*sin(cama.x);
    camPos.zx = camPos.zx*cos(cama.y) + camPos.xz*d.xy*sin(cama.y);

    vec3 camTarget = vec3(0);

    vec3 camK = normalize(camTarget - camPos);
    vec3 camI = normalize(cross(vec3(0, 1, 0), camK));
    vec3 camJ = cross(camK, camI);

    ray = ray.x*camI + ray.y*camJ + ray.z*camK;
    ray = normalize(ray);

    float t = 0.;
    for (uint i=min(0, in_frame); i < 200; i++)
    {
        float h = sdf(ray*t + camPos);
        t += h;
        if (h < .001)
        {
            break;
        }
    }

    vec3 pos = ray*t+camPos;

    d *= .001;
    vec3 normal =
    normalize(
    sdf(pos+d.xxx)*d.xxx +
    sdf(pos+d.yyx)*d.yyx +
    sdf(pos+d.yxy)*d.yxy +
    sdf(pos+d.xyy)*d.xyy
    );

    struct Light
    {
        vec3 pos;
        vec3 col;
    };

    Light lights[] = Light[]
    (
    //Light( vec3(4,8,-6), vec3(50) ),
    //Light( vec3(.7,-.7,0), vec3(5,0,0) ) // shows multiple bugs with the shadow tech - all fixed by increasing loop limit on shadow test!
    /*Light( vec3(4,8,-6), vec3(50,0,0) ),
    Light( vec3(4,8,-5), vec3(0,50,0) ),
    Light( vec3(4,8,-4), vec3(0,0,50) )*/
    Light(vec3(sin(iTime) * 3., 1., cos(iTime) * 3.), vec3(1., 0., .25)*4.)
    //Light(vec3(2, -.5, .5), vec3(1, .3, 0)),
    //Light(vec3(1, .5, -1.2), vec3(1, 0, 0)),
    //Light(vec3(-.25, -.5, -1), vec3(1, 0, .7)*.2),
    //Light(vec3(-1, 1, .5), vec3(0, .2, 1)*2.)
    );

    fragColour.rgb = vec3(0);

    for (uint i=min(0, in_frame); i < lights.length(); i++)
    {
        vec3 toLight = lights[i].pos - pos;
        float lightStrength = ((max(0., dot(normal, normalize(toLight))) / dot(toLight, toLight)) * OcclusionTrace(pos, lights[i].pos, .05));
        #ifdef LIGHTS
        fragColour.rgb += lights[i].col*lightStrength;
        #endif
    }

    //const int aoSamples = AO_SAMPLES;
    float weightSum = 0.;
    //float ao = 0.;
    vec3 ambient = vec3(0);
    //for (uint i=min(0, in_frame); i < aoSamples; i++)
    //{
    // distribute samples uniformly on a disc and project onto hemisphere
    // this gives n.l implicitly within the sample distribution!

    float r = pow((jitter.y - jitter.x) * .5, 3.) * 4.;//cos(iTime*2.);//sqrt(.5);
    float a = pow((jitter.x - jitter.y) * .5, 3.) * 4.;//sin(iTime);//length(jitter);//tau*N;//(tau*(float(seed)/exp2(32.)));

    vec3 dir = vec3(sin(a)*r, sqrt(1.-r*r), cos(a)*r);

    float area = 1. / dir.y;///float(aoSamples);
    float radius = sqrt(area*2./tau);

    //AAARGH have to rotate it to be perp to normal!
    vec3 tangent = normalize(cross(normal, vec3(1, 0, 0)));
    if (abs(normal.x) > .9999) tangent = vec3(0, 1, 0);
    vec3 bitangent = cross(tangent, normal);
    dir = dir.x*tangent + dir.y*normal + dir.z*bitangent;

    float ao = OcclusionTrace(pos, dir*10., radius);

    #ifdef COLOURISE_AMBIENT
    vec3 ambientColour = mix(vec3(1, .3, 0), vec3(0, .3, 1), dir.x*.5+.5);
    #else
    vec3 ambientColour = vec3(1./3.);
    #endif

    ambient += ambientColour;
    weightSum += area;
    //}
    ambient /= weightSum;

    #ifdef AMBIENT_LIGHT
    fragColour.rgb += ambient;
    #endif
    /*
     #ifdef BLEND_FRAMES
     fragColour = mix(texelFetch(iChannel0, ivec2(_fragCoord), 0), fragColour, .33);
     #endif
     fragColour.a = 1.;*/
    // if(pixel_coords.x < 320) {
    fragColour = 1. - exp(-fragColour * 2.);
    fragColour *= fragColour;
    //fragColour = ((fragColour* fragColour)+fragColour)*.5;
    //}

    //

    //vec3 pixel = vec3(1.);

    //if ((gl_GlobalInvocationID.x == 0) && (gl_GlobalInvocationID.y == 0) && (gl_GlobalInvocationID.z == 0))
    //{
    //draw(screen, pixel_coords, uvec4(uvec3(floor((pixel*255.)+.0)), 255));
    //draw(screen, pixel_coords + ivec2(2, 2), uvec4(uvec3(floor((pixel*255.)+.0)), 255));

    /*
      for(int x = min(0, in_frame); x < (in_frame / 2); x++)
      {
       draw(screen, ivec2(x % 640, int(float(in_frame)/320.)), uvec4(255));
      }*/


    //draw(pixel_coords,rgba{});
    //imageStore(screen, pixel_coords, uvec4(uvec3(floor((pixel*255.)+.0)), 255));
    //}
    vec3 LOAD = vec3(imageLoad(screen, pixel_coords).rgb / vec3(255.));
    fragColour.rgb = ((LOAD) + (fragColour.rgb))*.5;
    draw(screen, pixel_coords, uvec4(uvec3(fragColour.rgb * 255.), 255));
}