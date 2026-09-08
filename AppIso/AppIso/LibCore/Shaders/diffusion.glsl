#version 450 core

#ifdef COMPUTE_SHADER

layout(binding = 0, std430) readonly  buffer InMap       { float in_diffused[];      };
layout(binding = 1, std430) writeonly buffer OutMap      { float out_diffused[];     };
layout(binding = 2, std430) readonly  buffer Mask        { int   mask[];             };
layout(binding = 3, std430) readonly  buffer Constraints { float constraints[];      };
layout(binding = 4, std430) readonly  buffer Lap_target  { float laplacian_target[]; };

uniform int buffer_size_x;
uniform int buffer_size_y;

const ivec2 next8[8] = ivec2[8](ivec2(0, 1), ivec2(1, 1), ivec2(1, 0), ivec2(1, -1),
    ivec2(0, -1), ivec2(-1, -1), ivec2(-1, 0), ivec2(-1, 1));

int ToIndex1D(ivec2 p) { return p.x + buffer_size_x * p.y; }

float getValue(int id) {
    return in_diffused[id];
}

float getValue(int id, inout int cpt) {
    if (id < 0. || id >= (buffer_size_x * buffer_size_y)) return 0.;
    cpt++;
    return in_diffused[id];
}

float getValue(int x, int y, inout int cpt) {
    if (x < 0 || x >= buffer_size_x || y < 0 || y >= buffer_size_y) return 0.;
    cpt++;
    return in_diffused[ToIndex1D(ivec2(x, y))];
}

float Laplacian(int id) {
    int cpt = 0;
    float lap = getValue(id + 1, cpt) + getValue(id - 1, cpt) + getValue(id + buffer_size_x, cpt) + getValue(id - buffer_size_x, cpt);
    return cpt * getValue(id) - lap;
}

float NeighborSum(int id) {
    int cpt = 0;
    float lap = getValue(id + 1, cpt) + getValue(id - 1, cpt) + getValue(id + buffer_size_x, cpt) + getValue(id - buffer_size_x, cpt);
    return lap / float(cpt);
}

float NeighborSum(int x, int y) {
    int cpt = 0;
    float lap = getValue(x + 1, y, cpt) + getValue(x  -1, y, cpt) + getValue(x , y + 1, cpt) + getValue(x, y - 1, cpt);
    return lap / float(cpt);
}

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


    if (mask[id] > 0) out_diffused[id] = constraints[id];
    else out_diffused[id] = NeighborSum(x, y) - laplacian_target[id];
}

#endif
