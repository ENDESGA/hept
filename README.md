# hept
*the* ***hept*** *abstraction is a minimal lightweight layer above C++ and SDL*

### example use:

```C++
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
		vel = vec2(5,-1);
	}
} // global objects:
oPlayer;

main()
{
	oPlayer.pos = vec2( 2, 8 );
	oPlayer.step();
	//
	print( oPlayer.pos.x, " ",oPlayer.pos.y );
}
```
