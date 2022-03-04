layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba8ui, binding = 0) uniform uimage2D in_tex;

//

void main()
{
    ivec2 pos = ivec2(N);

    set(in_tex, pos, ivec4(255));
}