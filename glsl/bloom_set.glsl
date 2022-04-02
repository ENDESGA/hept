in_tex( 0 ) tex;
in_tex( 2 ) lighting_tex;

obj bloom_pixel
{
	ivec2 pos;
};
in_data( 4 ) bloom_pixels_data
{
	bloom_pixel in_list[];
};
const int list_n = in_list.length();

draw()
{
	return;
	rgba c = rgba(255);

	uint tile_index = (N / 256);
	//if (tile_index>=list_n) return;
	bloom_pixel t = in_list[tile_index];
	ivec2 p = ivec2(N % 16, (N / 16) % 16);
	//rgba c = get(tiles_tex, p + (ivec2(16, 16)*t.id));

	//set(tex,p + t.pos, c);
}