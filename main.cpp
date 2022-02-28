#include "hept.h"

obj entity : obj_default
{
	vec2 pos; // position
	vec2 vel; // velocity
	quad bnd; // boundary
	//
	void step()
	{
		pos += vel;
	}
}; // no globals

obj player : entity
{
	float variable = 1;

	player() // upon creation
	{
		vel = vec2( 5, -1 );
	}
} // global objects:
oPlayer;

draw()
{
	//ren_fill( rgba{ 255, 0, 64, 255 } );
}

main()
{
	oPlayer.pos = vec2( 2, 8 );
	oPlayer.step();
	//
	print( mix( 2., 8., .0 ), " ", mix( 2., 8., .25 ), " ", mix( 2., 8., .5 ), " ", mix( 2., 8., .75 ), " ", mix( 2., 8., 1. ) );
	print( mixinv( 2., 8., 2. ), " ", mixinv( 2., 8., 3.5 ), " ", mixinv( 2., 8., 5. ), " ", mixinv( 2., 8., 6.5 ), " ", mixinv( 2., 8., 8. ) );
	print( mixex( 2., 8., .0 ), " ", mixex( 2., 8., .25 ), " ", mixex( 2., 8., .5 ), " ", mixex( 2., 8., .75 ), " ", mixex( 2., 8., 1. ) );
	print( mixinvex( 2., 8., 2. ), " ", mixinvex( 2., 8., 3.5 ), " ", mixinvex( 2., 8., 5. ), " ", mixinvex( 2., 8., 6.5 ), " ", mixinvex( 2., 8., 8. ) );
}
