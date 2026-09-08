#version 430 core

#define M_PI 3.14159

#extension GL_ARB_gpu_shader_int64 : require

#define PCG32_DEFAULT_STATE  0x853c49e6748fea9bUL
#define PCG32_DEFAULT_STREAM 0xda3e39cb94b95bdbUL
#define PCG32_MULT           0x5851f42d4c957f2dUL

#define PCG u64vec2
#define RcpPow32 (2.32830616e-10f)

uint randint(inout PCG pcg)
{
	const uint64_t old_state = pcg.x;
	pcg.x = old_state * PCG32_MULT + pcg.y;

	const uint xorshifted = uint(((old_state >> 18) ^ old_state) >> 27);
	const uint rot = uint(old_state >> 59);
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void init_pcg(inout PCG pcg, const uint64_t seed)
{
	pcg.x = 0;
	pcg.y = (0xDA3E39CB94B95BDBUL << 1) | 1;
	randint(pcg);
	pcg.x += seed;
	randint(pcg);
}

float randfloat(inout PCG pcg) 
{ 
	return float(randint(pcg)) * RcpPow32;
}

PCG pcg;


#ifdef COMPUTE_SHADER


// Buffers: elevation + shading (optional)
layout(binding = 0, std430) buffer ElevationBuffer  { float hf[]; };
layout(binding = 1, std430) buffer IsoBuffer        { float iso_buffer[]; };
layout(binding = 2, std430) buffer IsoNbPtsBuffer   { int   iso_nb_pts_buffer[]; };
layout(binding = 3, std430) buffer IsoHeightsBuffer { float iso_heights_buffer[]; };

// Camera data
layout(location = 0) uniform vec3 CamPos;
layout(location = 1) uniform vec3 CamLookAt;
layout(location = 2) uniform vec3 CamUp;
layout(location = 3) uniform float CamAngleOfViewV;
layout(location = 4) uniform vec2 iResolution;

// Heightfield data
layout(location = 5) uniform vec2 a;
layout(location = 6) uniform vec2 b;
layout(location = 7) uniform vec2 zRange;
layout(location = 8) uniform float K;
layout(location = 9) uniform ivec2 texSize;

// isolines
layout(location = 10) uniform int nb_iso;

// ortho
layout(location = 11) uniform int camera_ortho = 0;

// albedo
layout(location = 12) uniform int use_albedo = 0;

// isolines
layout(location = 13) uniform vec3 main_isolines_color = (vec3(230, 69, 0) / 255. + 0.9) / 1.9;
layout(location = 14) uniform vec3 lesser_isolines_color = (vec3(230, 69, 0) / 255. + 0.4) / 1.4;
layout(location = 15) uniform float main_isolines_width = 15.;
layout(location = 16) uniform float lesser_isolines_width = 7.5;
layout(location = 17) uniform int main_isolines_period = 4;

// sea
layout(location = 18) uniform vec3 sea_dark_blue = vec3(0, 123, 224) / 255.;
layout(location = 19) uniform vec3 sea_light_blue = vec3(194, 227, 255) / 255.;
layout(location = 20) uniform float sea_level = -0.1; // percentage of height under the sea

// earth and sky
layout(location = 21) uniform vec3 terrain_color = vec3(1, 1, 0.95);
layout(location = 22) uniform vec3 sky_color = vec3(1);

// Raymarching
layout(location = 23) uniform int dispatch_id;
layout(location = 24) uniform float k = 1.;
layout(location = 25) uniform int nb_samples_per_shader = 128;
layout(location = 26) uniform vec3 lightDir = vec3(-1.0, -2.0, -0.9);
layout(location = 27) uniform int AA = 1;
layout(location = 28) uniform int STEPS = 4096;
layout(location = 29) uniform float epsilon = 0.5;

layout(location = 30) uniform vec2 cameraPlanes;

layout(location = 31) uniform float under_shadow_distance;

// lights
layout(location = 32) uniform vec3 light_blue = vec3(184, 209, 255) / 255.;
layout(location = 33) uniform vec3 light_yellow = vec3(255, 235, 186) / 255.;

layout(location = 34) uniform float h_threshold = 0.;
layout(location = 35) uniform int show_outside_shadows = 0;

layout(location = 36) uniform float alpha_shadows = 1.;

// texture
layout(rgba32f, binding = 0) uniform image2D outImage;
layout(binding = 1) uniform sampler2D albedo_map;


// colors
vec3[3] height_palette = vec3[3](vec3(106, 168, 91) / 255., vec3(217, 204, 154) / 255., vec3(1));
vec3[3] coolwarm_palette = vec3[3](vec3(97, 130, 234) / 255., vec3(221, 221, 221) / 255., vec3(220, 94, 75) / 255.);

vec3 GetHeightColor(float h, vec3[3] palette) {
	float hf = (h - zRange.x) / (zRange.y - zRange.x);

	if (hf < 0.5) return mix(palette[0], palette[1], hf / 0.5);
	else return mix(palette[1], palette[2], (hf - 0.5) / 0.5);
}

void getLocalFrame(in vec3 n, out vec3 X, out vec3 Y) {
	if (dot(n, vec3(1, 0, 0)) < 0.9) {
		X = normalize(cross(n, vec3(1, 0, 0)));
		Y = normalize(cross(n, X));
	}
	else {
		X = normalize(cross(n, vec3(0, 1, 0)));
		Y = normalize(cross(n, X));
	}
}

float random() {
	return randfloat(pcg);
}
vec3 random_direction(vec3 n) {
	float r1 = random();
	float r2 = random();

	float sqrt_r1 = 2. * sqrt((1 - r1) * r1);
	float x = cos(2. * M_PI * r2) * sqrt_r1;
	float y = sin(2. * M_PI * r2) * sqrt_r1;
	float z = 1. - 2. * r1;
	vec3 d = normalize(vec3(x, y, z));
	return dot(d, n) > 0. ? d : -d;
}
vec3 random_direction_CW() {
	float r1 = random();
	float r2 = random();

	float theta = acos(sqrt(r1));
	float phi = 2. * M_PI * r2;

	return vec3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
}


float Bilinear(float a00, float a10, float a11, float a01, float u, float v) {
	return (1 - u) * (1 - v) * a00 + (1 - u) * (v)*a01 + (u) * (1 - v) * a10 + (u) * (v)*a11;
}

float at(int i, int j) {
	return hf[j * texSize.x + i];
}

void GetUV(vec2 p, out vec2 uv, out int i, out int j) {
	p = clamp(p, a, b);

	vec2 q = p - a;
	vec2 d = b - a;

	uv = q / d;
	uv = uv * vec2(texSize.x - 1, texSize.y - 1);

	i = int(uv.x);
	j = int(uv.y);

	uv = vec2(uv.x - i, uv.y - j);
}

float Height(vec2 p) {
	vec2 uv;
	int i, j;
	GetUV(p, uv, i, j);

	return Bilinear(at(i, j), at(i + 1, j), at(i + 1, j + 1), at(i, j + 1), uv.x, uv.y);
}

vec3 BuildRd(vec2 p, ivec2 pa) {
	vec3 view = normalize(CamLookAt - CamPos);
	vec3 horizontal = normalize(cross(view, CamUp));
	vec3 vertical = normalize(cross(horizontal, view));

	float length = 1.0f;
	float rad = CamAngleOfViewV;

	float vLength = tan(rad / 2.0f) * length;
	float hLength = vLength * iResolution.x / iResolution.y;

	vertical *= vLength;
	horizontal *= hLength;

	p.x = p.x - iResolution.x / 2.0f;
	p.y = p.y - iResolution.y / 2.0f;

	p.x += float(pa.x) / float(AA) - 0.5f;
	p.y += float(pa.y) / float(AA) - 0.5f;

	p.x /= iResolution.x / 2.0f;
	p.y /= iResolution.y / 2.0f;

	return normalize(view * length + horizontal * p.x + vertical * p.y);
}

float Intersection(float va, float vb) {
	return max(va, vb);
}

float Box(vec3 p, vec3 va, vec3 vb) {
	vec3 c = 0.5 * (va + vb);
	vec3 r = 0.5 * (vb - va);
	vec3 q = abs(p - c) - r;
	float d = length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
	return d;
}

float Box(vec2 p, vec2 va, vec2 vb) {
	vec2 c = 0.5 * (va + vb);
	vec2 r = 0.5 * (vb - va);
	vec2 q = abs(p - c) - r;
	float d = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0);
	return d;
}

float sdfSeg(vec3 p, vec3 aa, vec3 bb) {
	vec3 pa = p - aa, ba = bb - aa;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
	return length(pa - ba * h);
}

float Map(vec3 p) {
	float t = p.z - Height(p.xy);
	return t;
	/*
	float delta = 0.1f * (zRange.y - zRange.x);
	return Intersection(Box(p, vec3(a.x, a.y, zRange.x - delta), vec3(b.x, b.y, zRange.y + delta)), t);
	*/
}

float sdf3d(vec3 p) {
	float d = 999.;
	/*
	int nb_pts_offset = 0;
	for (int iso_id = 0; iso_id < nb_iso; iso_id++) {

		int iso_nb_pts = iso_nb_pts_buffer[iso_id];
		bool lesser_iso = (iso_id % main_isolines_period) != 0;

		for (int seg_id = 0; seg_id < iso_nb_pts / 2; seg_id++) {
			vec3 aa = vec3(iso_buffer[2 * nb_pts_offset + 4 * seg_id], iso_buffer[2 * nb_pts_offset + 4 * seg_id + 1], iso_heights_buffer[iso_id]);
			vec3 bb = vec3(iso_buffer[2 * nb_pts_offset + 4 * seg_id + 2], iso_buffer[2 * nb_pts_offset + 4 * seg_id + 3], iso_heights_buffer[iso_id]);
			d = min(d, sdfSeg(p, aa, bb) - 100.);

		}

		nb_pts_offset += iso_nb_pts;
	}
	*/

	return d;
	/*
	return d;
	float s = length(p - vec3(0., 0., 3500.)) - 500.;
	return s;
	*/

}


vec3 Normal(vec3 p) {
	const vec3 e = vec3((b - a) / vec2(texSize - ivec2(1, 1)), 0.0);
	if (sdf3d(p) < Map(p)) {
		float eps = 0.001;
		return normalize(vec3((sdf3d(p + vec3(eps, 0, 0)) - sdf3d(p - vec3(eps, 0, 0))) / (2 * eps),
							  (sdf3d(p + vec3(0, eps, 0)) - sdf3d(p - vec3(0, eps, 0))) / (2 * eps),
							  (sdf3d(p + vec3(0, 0, eps)) - sdf3d(p - vec3(0, 0, eps))) / (2 * eps)));
	}
	return normalize(vec3(
		-((Height(p.xy + e.xz) - Height(p.xy - e.xz)) / (2.0f * e.x)),
		-((Height(p.xy + e.zy) - Height(p.xy - e.zy)) / (2.0f * e.y)),
		1.0f
	));
}

bool IntersectBox(vec3 ro, vec3 rd, out float tN, out float tF) {
	vec3 rinvDir = 1.0 / rd;
	float delta = 0.1 * (zRange.y - zRange.x);
	vec3 tbot = rinvDir * (vec3(a.x, a.y, zRange.x - delta) - ro);
	vec3 ttop = rinvDir * (vec3(b.x, b.y, zRange.y + delta) - ro);
	vec3 tmin = min(ttop, tbot);
	vec3 tmax = max(ttop, tbot);
	vec2 t = max(tmin.xx, tmin.yz);
	float t0 = max(t.x, t.y);
	t = min(tmax.xx, tmax.yz);
	float t1 = min(t.x, t.y);
	tN = t0;
	tF = t1;
	return t1 > max(t0, 0.0);
}

float LinearStep(float x, float a, float b) {
	if (x < a)
		return 0.0f;
	else if (x > b)
		return 1.0f;
	else
		return (x - a) / (b - a);
}

struct Hit {
	bool hit;
	vec3 p;
	float t;
	int s;
};

Hit Raymarch(vec3 ro, vec3 rd) {
	Hit h;
	
	// Check if ray intersects the bounding box of the heightfield
	float ta, tb;
	if (!IntersectBox(ro, rd, ta, tb))
	{
		h.t = max(ta, tb);
		h.p = ro + h.t * rd;
		h.hit = false;
		return h;
	}

	// Raymarch
	h.t = max(ta, 0.0f);
	float d = 0.0f;
	float uz = abs(rd.z);
	float kr = uz + K * sqrt(1.0f - (uz * uz));

	h.s = 0;
	for (int s = 0; s < STEPS; s++) {
		if (h.t > tb) {
			h.t = cameraPlanes.y;
			h.hit = false;
			h.s = s;
			return h;
		}

		h.p = ro + rd * h.t;
		//d = min(Map(h.p) / kr, sdf3d(h.p));
		d = Map(h.p) / kr;
		if (d <= 0.0) {
			h.hit = true;
			h.s = s;
			return h;
		}
		h.t += max(d, epsilon);
		h.s = s;
	}
	h.t = cameraPlanes.y;
	h.hit = false;
	return h;
}

Hit RaymarchUnder(vec3 ro, vec3 rd) {
	Hit h;

	// Check if ray intersects the bounding box of the heightfield
	float ta, tb;
	if (!IntersectBox(ro, rd, ta, tb)) {
		h.t = cameraPlanes.y;
		h.hit = false;
		return h;
	}

	// Raymarch
	h.t = max(ta, 0.0f);
	float d = 0.0f;
	float uz = abs(rd.z);
	float kr = uz + K * sqrt(1.0f - (uz * uz));

	h.s = 0;
	for (int s = 0; s < STEPS; s++) {
		if (h.t > tb) {
			h.t = cameraPlanes.y;
			h.hit = false;
			h.s = s;
			return h;
		}

		h.p = ro + rd * h.t;
		d = -Map(h.p);
		if (d <= 0.0) {
			h.hit = true;
			h.s = s;
			return h;
		}
		h.t += max(d / kr, epsilon);
		h.s = s;
	}
	h.t = cameraPlanes.y;
	h.hit = false;
	return h;
}

Hit RaymarchUnderFlat(vec3 ro, vec3 rd) {
	Hit h;

	h.t = (zRange.x + h_threshold - ro.z) / rd.z;
	h.p = ro + h.t * rd;

	h.hit = h.p.x > a.x && h.p.x < b.x && h.p.y > a.y && h.p.y < b.y;
	h.hit = h.hit && h.p.z < Height(h.p.xy);

	return h;
}

vec3 perp(vec3 u) {
	vec3 a = abs(u);
	vec3 v;
	if (a.x <= a.y && a.x <= a.z)
		v = vec3(0, -u.z, u.y);
	else if (a.y <= a.x && a.y <= a.z)
		v = vec3(-u.z, 0.0, u.x);
	else
		v = vec3(-u.y, u.x, 0.0);
	return v;
}

float BRDF(vec3 p, vec3 n, vec3 din, vec3 dout) {
	return 1.;
}

float SmoothLight(vec3 ro_light, vec3 n, int nb_sample) {

	vec3 rd_light = normalize(-lightDir);
	
	int nb_clear_sight = 0;
	int nb_bad_start = 0;
	for (int i = 0; i < nb_sample; i++) {

		vec3 jittered_ro_light = ro_light + 40. * random_direction(n);

		Hit h_light = Raymarch(jittered_ro_light, rd_light);
		if (!h_light.hit) nb_clear_sight++;
		else if (h_light.s < 2) nb_bad_start++;
	}

	return float(nb_clear_sight) / (nb_sample - nb_bad_start) * max(dot(n, rd_light), 0.);
}

float SmoothLightUnder(vec3 ro_light, vec3 n, int nb_sample) {

	vec3 rd_light = normalize(-lightDir);

	int nb_clear_sight = 0;
	for (int i = 0; i < nb_sample; i++) {

		vec3 jittered_ro_light = ro_light + 40. * random_direction(n);

		Hit hit_under_plane = RaymarchUnderFlat(jittered_ro_light, rd_light);
		if (!hit_under_plane.hit) {
			hit_under_plane = Raymarch(hit_under_plane.p, rd_light);
		}

		if (!hit_under_plane.hit) nb_clear_sight++;

	/*
		Hit h_light = Raymarch(jittered_ro_light, rd_light);
		if (!h_light.hit) nb_clear_sight++;
	*/
	}
	return float(nb_clear_sight) / (nb_sample);// * max(dot(n, rd_light), 0.);

}

float TraceAO(vec3 p, vec3 n) {
	float total_ao = 0.0;

	// random
	uvec2 seed = uvec2(int(gl_GlobalInvocationID.x), int(gl_GlobalInvocationID.y));
	init_pcg(pcg, packUint2x32(seed));

	for (int i = 0; i < nb_samples_per_shader; i++) {
		vec3 rd_ao = random_direction(n);
		
		// trace
		Hit h_ao = Raymarch(p, rd_ao);
		if (!h_ao.hit) total_ao += max(dot(rd_ao, n), 0.);
	}

	return total_ao / nb_samples_per_shader;
}

vec3 TraceAO_CW(vec3 p, vec3 n) {
	vec3 total_ao = vec3(0);

	// local frame
	vec3 X, Y;
	getLocalFrame(n, X, Y);

	for (int i = 0; i < nb_samples_per_shader; i++) {
		vec3 rd_ao = random_direction_CW();
		rd_ao = normalize(rd_ao.x * X + rd_ao.y * Y + rd_ao.z * n);

		// trace
		Hit h_ao = Raymarch(p, rd_ao);
		if (!h_ao.hit) {
			//vec3 light_color = dot(rd_ao, -lightDir) > 0. ? light_yellow : light_blue;
			vec3 light_color = rd_ao.x > 0. ? light_yellow : light_blue;
			total_ao += 0.5 * light_color;
		}
	}

	return total_ao / nb_samples_per_shader;
}

float TraceAO_CW_Under(vec3 p, vec3 n) {
	float total_ao = 0.0;

	for (int i = 0; i < 4 * nb_samples_per_shader; i++) {
		vec3 rd_ao = random_direction_CW();

		// trace
		Hit h_ao = RaymarchUnderFlat(p, rd_ao);
		//if (Height(h_ao.p.xy) < zRange.x + h_threshold) {
		if (!h_ao.hit) {
			//h_ao.hit = false;
			// launch ray to continue tracing and check if we hit some mountains
			//h_ao = Raymarch(h_ao.p, rd_ao);
		}

		if (!h_ao.hit) {
			total_ao += 1.;
		}
	}

	return total_ao / nb_samples_per_shader / 4.;
}

int DetectIsoline(vec3 p) {
	//return 0;
	int nb_pts_offset = 0;
	for (int iso_id = 0; iso_id < nb_iso; iso_id++) {

		int iso_nb_pts = iso_nb_pts_buffer[iso_id];
		bool lesser_iso = (iso_id % main_isolines_period) != 0;

		for (int seg_id = 0; seg_id < iso_nb_pts / 2; seg_id++) {
			vec3 a = vec3(iso_buffer[2 * nb_pts_offset + 4 * seg_id], iso_buffer[2 * nb_pts_offset + 4 * seg_id + 1], iso_heights_buffer[iso_id]);
			vec3 b = vec3(iso_buffer[2 * nb_pts_offset + 4 * seg_id + 2], iso_buffer[2 * nb_pts_offset + 4 * seg_id + 3], iso_heights_buffer[iso_id]);
			float d = sdfSeg(p, a, b);
			if (d < (lesser_iso ? lesser_isolines_width : main_isolines_width)) return (lesser_iso ? 1 : 2);
			//if (d < (lesser_iso ? lesser_isolines_width : 0.8 * main_isolines_width)) return (lesser_iso ? 1 : 2);
		}

		nb_pts_offset += iso_nb_pts;
	}

	return 0;
}

float Network(vec3 p) {
	float d = 9999.;
	int iso_nb_pts = iso_nb_pts_buffer[0];
    for (int seg_id = 0; seg_id < iso_nb_pts / 2; seg_id++) {
        vec2 aa2 = vec2(iso_buffer[4 * seg_id], iso_buffer[4 * seg_id + 1]);
        vec2 bb2 = vec2(iso_buffer[4 * seg_id + 2], iso_buffer[4 * seg_id + 3]);
		vec3 aa = vec3(aa2, 0);
		vec3 bb = vec3(bb2, 0);
        d = min(d, sdfSeg(vec3(p.xy, 0), aa, bb));
    }

	return d;
}


vec4 Pixel(vec2 pix_p, ivec2 pa) {

	// random
	uvec2 seed = uvec2(int(gl_GlobalInvocationID.x) + int(iResolution.x) * int(gl_GlobalInvocationID.y), dispatch_id);
	init_pcg(pcg, packUint2x32(seed));

	// Compute ray
	vec3 ro = CamPos;
	vec3 rd = BuildRd(pix_p, pa);

	// orthogonal
	if (camera_ortho != 0) {
		vec3 view = normalize(CamLookAt - CamPos);
		vec3 horizontal = normalize(cross(view, CamUp));
		vec3 vertical = normalize(cross(horizontal, view));
		vec2 center_frag_p = (pix_p - 0.5 * iResolution) / min(iResolution.x, iResolution.y);
		ro += 1.1 * max(b.x - a.x, b.y - a.y) * (center_frag_p.x * horizontal + center_frag_p.y * vertical);
		rd = view;
	}

	// Compute Intersection with terrain
	vec4 c = vec4(1.);
	Hit h = Raymarch(ro, rd);
	if (h.p.z < zRange.x + h_threshold) h.hit = false;

	if (h.hit) {
		vec3 n = Normal(h.p);


		vec3 ro_light = h.p + 1.5 * epsilon * n;
		vec3 rd_light = normalize(-lightDir);

		// send ray toward light
		float direct_light = SmoothLight(ro_light, n, 16);

		// ambiant occlusion
		vec3 ao_light = TraceAO_CW(ro_light, n);

		// compute diffuse color
		vec3 albedo = vec3(1);
		float nh = (h.p.z - zRange.x) / max(0.1, (zRange.y - zRange.x));
		if (nh < sea_level) {
			float k = clamp((nh - 0.6 * sea_level) / (0.4 * sea_level), 0., 1.);
			albedo = mix(sea_light_blue, sea_dark_blue, k);
		}
		else albedo = terrain_color;

		// stairs rendering
		/*
		albedo = GetHeightColor(h.p.z, coolwarm_palette);
		if (n.z < 0.5) albedo = vec3(1);
		*/

		if (use_albedo != 0) {
			vec2 uv = (h.p.xy - a) / (b - a);
			albedo = texture(albedo_map, uv).xyz;
		}

		//albedo = h.p.z < 553. ? vec3(227, 241, 255) / 255. : vec3(255, 227, 248) / 255.;

		/*
		float sl = 0.03;
		if (nh < sl) {
			float k = clamp((nh - 0.6 * sl) / (0.4 * sl), 0., 1.);
			albedo = mix(sea_light_blue, (sea_dark_blue + 0.3) / 1.3, k);
			//albedo = (sea_dark_blue + 0.6) / 1.6;
		}
		*/

		if (sdf3d(h.p) < Map(h.p)) {
			albedo = vec3(1, 0, 0);
		}

		// isolines
		/*
		*/
		int iso_type = DetectIsoline(h.p);
		if (nh > sea_level && iso_type != 0) {
			// lesser iso
			if (iso_type == 1) albedo = lesser_isolines_color;
			else albedo = main_isolines_color;
		}

		// network
		/*
		float sdf_net = Network(h.p);
		if (sdf_net < main_isolines_width) albedo = vec3(0, 0, 1);
		*/

		c.xyz = mix(albedo, albedo * (0.6 * direct_light + 1.3 * ao_light), alpha_shadows);

		// fog
		/*
		vec3 fog_color = vec3(0.5, 0.6, 0.7);
		float fog_depth = clamp(h.t, 0., length(b - a)) / length(b - a);
		fog_depth = pow(fog_depth, 2.);
		float fog_height = 1. - clamp(h.p.z, 0., zRange.y) / zRange.y;
		fog_height = pow(fog_height, 2.);
		float fog_alpha = 0.5 * fog_height + 0.5 * fog_depth;
		c.xyz = mix(c.xyz, fog_color, fog_alpha);
		*/


		// steps shading
		//c = vec4(vec3(float(h.s) / 8.), 1.);
	}
	
	// Heightfield sides
	bool hit_terrain_sides = h.p.x < a.x + 2. || h.p.x > b.x - 2. || h.p.y < a.y + 2. || h.p.y > b.y - 2.;

	if (!h.hit || hit_terrain_sides) {
		if (under_shadow_distance < 0.01f) return vec4(sky_color, 1.);
		// intersection with shadow plane
		float t_shadow = (zRange.x - under_shadow_distance - ro.z) / rd.z;
		vec3 p_shadow = ro + t_shadow * rd;

		// compute AO under terrain
		//c = vec4(vec3(TraceAO_CW_Under(p_shadow, vec3(0, 0, 1))), 1.);
		float under_ao = TraceAO_CW_Under(p_shadow, vec3(0, 0, 1));
		under_ao = smoothstep(0., 0.75, under_ao);
		if (show_outside_shadows != 0) {
			float under_direct = SmoothLightUnder(p_shadow, vec3(0, 0, 1), 64);
			c = vec4(vec3(0.35 + 0.65 * under_direct), 1.);
			//c = vec4(vec3(0.6 * under_direct + 0.4 * under_ao), 1.);
		}
		else {
			c = vec4(0.5 + 0.5 * vec3(under_ao), 1.);
			//c = vec4(vec3(under_ao), 1.);
		}

		/*
		vec3 fog_color = vec3(0.5, 0.6, 0.7);
		float fog_depth = clamp(h.t, 0., length(b - a)) / length(b - a);
		fog_depth = pow(fog_depth, 2.);
		float fog_alpha = 0.5 * fog_depth;
		c.xyz = mix(c.xyz, fog_color, fog_alpha);
		*/
		c.w = 1. - c.x;
	}

	return c;
}


//layout(rgba32f, binding = 3) uniform image2D outImage;
void WriteColor(ivec2 pix, vec4 color) {
	
	if (dispatch_id != 0) {
		vec4 prev_color = imageLoad(outImage, pix);
		color = (prev_color * dispatch_id + color) / (dispatch_id + 1);
	}
	
	imageStore(outImage, pix, color);
}

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main() {
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	if (pixel_coords.x > iResolution.x || pixel_coords.y > iResolution.y) return;


	if (AA == 1) {
		vec4 color = Pixel(pixel_coords, ivec2(0, 0));
		WriteColor(pixel_coords, color);
	}
	else {
		vec4 tot = vec4(0.0f);
		for (int m = 0; m < AA; m++)
		{
			for (int n = 0; n < AA; n++)
				tot += Pixel(pixel_coords, ivec2(m, n));
		}
		tot = tot / float(AA * AA);
		//tot.a = 1.0;
		WriteColor(pixel_coords, tot);
	}
}

#endif
