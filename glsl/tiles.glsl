in_tex( 0 ) tex;
/*in_tex( 1 ) tiles_tex;

obj tile
{
	ivec2 pos;
	ivec2 id;
};
in_data( 3 ) tiles_data
{
	tile in_list[];
};
const int list_n = in_list.length();*/

draw()
{
	rgba c = get(tex, ID.xy);
	if (((ID.x + ID.y) % 2) == 0) set(tex, ID.xy, rgba(vec3((.5+sin(T)*.5)*255., 0, 0), 255));//(c + c + c + (rgba(4, 0, 1, 4) * rgba(((ID.x + ID.y + int(F/20)) % 2)*64))) / 4);
	/*uint tile_index = (N / 256);
	if( tile_index>=list_n ) return;
	tile t = in_list[tile_index];
	ivec2 p = ivec2(N % 16, (N / 16) % 16);
	rgba c = get( tiles_tex, p + (ivec2(16,16)*t.id));
	if (c.a<=0x11u) return;
	set(tex,p + t.pos + ivec2(64,8) + ivec2(vec2(
	sin((T*3)+(float(tile_index+1)*4))*16,
	cos((T*3)+(float(tile_index+1)*4))*16)),c);*/
}