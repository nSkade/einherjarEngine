// adapted from tinybvh traverse.comp
// Copyright (c) 2024 Jacco Bikker

//layout (std430) buffer;

struct BVHNode
{
	vec4 lmin; // unsigned left in w
	vec4 lmax; // unsigned right in w
	vec4 rmin; // unsigned triCount in w
	vec4 rmax; // unsigned firstTri in w
};

/**
usage:

// define this struct layout

layout (std430, binding = 0) readonly buffer I0 { BVHNode node[]; } nodeData;
layout (std430, binding = 1) readonly buffer I1 { uint idx[]; } idxData;
layout (std430, binding = 2) readonly buffer I2 { vec4 tri[]; } triData;
DEFINE_TRAVERSE_FUNCTION(traverse_bvh,nodeData.node,idxData.idx,triData.tri)

 * @brief traverse bvh
 * @param O ray origin
 * @param D ray direction
 * @param rD inverse ray dir (1./D)
 * @param tmax maximum trace distance
 * @return vec4( dist, u, v, triIdx )
 */
//vec4 traverse_ailalaine
#define DEFINE_TRAVERSE_FUNCTION(FUNC_NAME, NODE_ARRAY, INDEX_ARRAY, TRI_ARRAY, TRI_POST) \
vec4 FUNC_NAME( const vec3 O, const vec3 D, const vec3 rD, const float tmax ) \
{ \
	vec4 hit; \
	hit.x = tmax; \
	uint node = 0, stack[64], stackPtr = 0; \
	while (true) \
	{ \
		const BVHNode n = NODE_ARRAY[node]; \
		const vec4 lmin = n.lmin, lmax = n.lmax; \
		const vec4 rmin = n.rmin, rmax = n.rmax; \
		const uint triCount = floatBitsToUint( rmin.w ); \
		if (triCount > 0) \
		{ \
			const uint firstTri = floatBitsToUint( rmax.w ); \
			for (uint i = 0; i < triCount; i++) \
			{ \
				const uint triIdx = INDEX_ARRAY[firstTri + i]; \
				const uint v0 = triIdx * 3; \
				const vec3 edge1 = TRI_ARRAY[v0 + 1]TRI_POST - TRI_ARRAY[v0]TRI_POST; \
				const vec3 edge2 = TRI_ARRAY[v0 + 2]TRI_POST - TRI_ARRAY[v0]TRI_POST; \
				const vec3 h = cross( D, edge2.xyz ); \
				const float a = dot( edge1.xyz, h ); \
				if (abs( a ) < 0.0000001f) continue; \
				const float f = 1.0 / a; \
				const vec3 s = O - TRI_ARRAY[v0]TRI_POST.xyz; \
				const float u = f * dot( s, h ); \
				const vec3 q = cross( s, edge1.xyz ); \
				const float v = f * dot( D, q ); \
				if (u < 0.0 || v < 0.0 || u + v > 1.0) continue; \
				const float d = f * dot( edge2.xyz, q ); \
				if (d > 0.0 && d < hit.x) hit = vec4( d, u, v, uintBitsToFloat( triIdx ) ); \
			} \
			if (stackPtr == 0) break; \
			node = stack[--stackPtr]; \
			continue; \
		} \
		uint left = floatBitsToUint( lmin.w ), right = floatBitsToUint( lmax.w ); \
		const vec3 t1a = (lmin.xyz - O) * rD, t2a = (lmax.xyz - O) * rD; \
		const vec3 t1b = (rmin.xyz - O) * rD, t2b = (rmax.xyz - O) * rD; \
		const vec3 minta = min( t1a, t2a ), maxta = max( t1a, t2a ); \
		const vec3 mintb = min( t1b, t2b ), maxtb = max( t1b, t2b ); \
		const float tmina = max( max( max( minta.x, minta.y ), minta.z ), 0.0 ); \
		const float tminb = max( max( max( mintb.x, mintb.y ), mintb.z ), 0.0 ); \
		const float tmaxa = min( min( min( maxta.x, maxta.y ), maxta.z ), hit.x ); \
		const float tmaxb = min( min( min( maxtb.x, maxtb.y ), maxtb.z ), hit.x ); \
		float dist1 = tmina > tmaxa ? 1e30 : tmina; \
		float dist2 = tminb > tmaxb ? 1e30 : tminb; \
		if (dist1 > dist2) \
		{ \
			float h = dist1; dist1 = dist2; dist2 = h; \
			uint t = left; left = right; right = t; \
		} \
		if (dist1 == 1e30f) \
		{ \
			if (stackPtr == 0) break; else node = stack[--stackPtr]; \
		} \
		else \
		{ \
			node = left; \
			if (dist2 != 1e30f) stack[stackPtr++] = right; \
		} \
	} \
	return hit; \
}
