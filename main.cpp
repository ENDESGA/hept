#include "hept.h"

global glsl test_glsl;
global gpu test_gpu;

main()
{
	test_glsl = glsl_new( glsl_comp, "glsl/comp.glsl" );
	test_gpu = gpu_new( test_glsl );
}

draw()
{
	gpu_set( test_gpu );
	gpu_compute( 7, 0, 0 );
}
