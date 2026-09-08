#version 450 core

#ifdef COMPUTE_SHADER

// In
layout(binding = 0, std430) readonly  buffer InMap     { float in_map[]; };
layout(binding = 1, std430) readonly  buffer AddMap    { float add_map[]; };
layout(binding = 3, std430) readonly  buffer CoeffMap  { float coeff_map[]; };

// Out
layout(binding = 2, std430) writeonly buffer OutMap { float out_map[]; };

uniform int buffer_size_x;
uniform int buffer_size_y;


float GetHardness(float height, vec2 uv)
{
    float offset = -.2 * sin(6.283185 * uv.x) + .2 * sin(6.283185 * uv.y);

    float a = (height - offset) * 6.283185 * 12.5;
    return mix(mix(sin(a) * .5 + .5, sin(a * 1.618 + dot(uv, vec2(4, 2)) * 6.283185) * .5 + .5, .5),
        sin(a * 6.18) * .5 + .5, .2);
}


int ToIndex1D(ivec2 p) { return p.x + buffer_size_x * p.y; }


layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main() {
    int x = int(gl_GlobalInvocationID.x);
    int y = int(gl_GlobalInvocationID.y);
    if (x < 0)   return;
    if (y < 0)   return;
    if (x >= buffer_size_x) return;
    if (y >= buffer_size_y) return;

    ivec2 pos = ivec2(x, y);
    int id = ToIndex1D(pos);
    
    out_map[id] = max(0., in_map[id] + add_map[id] * coeff_map[id]);
    //out_map[id] = max(0., in_map[id] + add_map[id] * GetHardness(in_map[id] * 0.0005, vec2(x, y) / vec2(buffer_size_x, buffer_size_y)));
}

#endif
