
/**
 * @brief  ref https://github.com/jbikker/tinybvh/blob/0e4584287823252cf83f0e9cd072848bec5f79c5/kernels/traverse_tlas.cl#L7
 * @param FUNC_INTERSECT_INSTANCE vec4 intersectInstance(const uint instIdx, const vec3 O, const vec3 D, const vec3 rD, float currentTMax)
 */
#define DEFINE_TLAS_TRAVERSE_FUNCTION(FUNC_NAME, TLAS_NODE_ARRAY, TLAS_IDX_ARRAY, FUNC_INTERSECT_INSTANCE) \
vec4 FUNC_NAME(const vec3 O, const vec3 D, const vec3 rD, const float tmax) \
{ \
	vec4 hit = vec4(tmax, 0.0, 0.0, 0.0); \
	if (isnan(O.x + O.y + O.z + D.x + D.y + D.z)) return hit; \
	uint node = 0, stack[32], stackPtr = 0; \
	while (true) \
	{ \
		const BVHNode n = TLAS_NODE_ARRAY[node]; \
		const vec4 lmin = n.lmin, lmax = n.lmax; \
		const vec4 rmin = n.rmin, rmax = n.rmax; \
		const uint triCount = floatBitsToUint(rmin.w); \
		if (triCount > 0) \
		{ \
			const uint firstTri = floatBitsToUint(rmax.w); \
			for (uint i = 0; i < triCount; i++) \
			{ \
				const uint instIdx = TLAS_IDX_ARRAY[firstTri + i]; \
				vec4 instHit = FUNC_INTERSECT_INSTANCE(instIdx, O, D, rD, hit.x); \
				if (instHit.x < hit.x) \
				{ \
					hit = instHit; \
				} \
			} \
			if (stackPtr == 0) break; \
			node = stack[--stackPtr]; \
			continue; \
		} \
		uint left = floatBitsToUint(lmin.w), right = floatBitsToUint(lmax.w); \
		const vec3 t1a = (lmin.xyz - O) * rD, t2a = (lmax.xyz - O) * rD; \
		const vec3 t1b = (rmin.xyz - O) * rD, t2b = (rmax.xyz - O) * rD; \
		const vec3 minta = min(t1a, t2a), maxta = max(t1a, t2a); \
		const vec3 mintb = min(t1b, t2b), maxtb = max(t1b, t2b); \
		const float tmina = max(max(max(minta.x, minta.y), minta.z), 0.0); \
		const float tminb = max(max(max(mintb.x, mintb.y), mintb.z), 0.0); \
		const float tmaxa = min(min(min(maxta.x, maxta.y), maxta.z), hit.x); \
		const float tmaxb = min(min(min(maxtb.x, maxtb.y), maxtb.z), hit.x); \
		float dist1 = tmina > tmaxa ? 1e30f : tmina; \
		float dist2 = tminb > tmaxb ? 1e30f : tminb; \
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

// example
//vec3 TransformPoint(vec3 p, mat4 invTransform) {
//	return (invTransform * vec4(p, 1.0)).xyz;
//}
//
//vec3 TransformVector(vec3 v, mat4 invTransform) {
//	return (invTransform * vec4(v, 0.0)).xyz;
//}
//vec4 intersectInstance(const uint instIdx, const vec3 O, const vec3 D, const vec3 rD, float currentTMax)
//{
//	const Instance inst = instanceData.inst[instIdx];
//	const vec3 Oblas = TransformPoint(O, inst.invTransform);
//	const vec3 Dblas = TransformVector(D, inst.invTransform);
//	const uint objectType = uint(inst.dummy[0]);
//
//	vec4 hit = vec4(currentTMax, 0.0, 0.0, 0.0);
//
//	if (objectType == 0)
//	{
//		// 0 = Triangle Mesh BLAS
//		const vec3 rDblas = vec3(1.0 / Dblas.x, 1.0 / Dblas.y, 1.0 / Dblas.z);
//		vec4 blasHit = traverse_ailalaine(Oblas, Dblas, rDblas, currentTMax);
//		if (blasHit.x < currentTMax)
//		{
//			hit = blasHit;
//			uint primID = floatBitsToUint(hit.w) & 0xffffff;
//			hit.w = uintBitsToFloat(primID | (instIdx << 24));
//		}
//	}
//	else if (objectType == 1)
//	{
//		vec2 s = sphIntersect(Oblas,Dblas,1.);
//		
//		if (s.y > 0. && s.y < currentTMax) {
//			hit.x = s.y;
//			hit.y = 0.0;
//			hit.z = 0.0;
//			hit.w = uintBitsToFloat(0 | (instIdx << 24)); // primID = 0
//		}
//		if (s.x > 0. && s.x < currentTMax) {
//			hit.x = s.x;
//			hit.y = 0.0;
//			hit.z = 0.0;
//			hit.w = uintBitsToFloat(0 | (instIdx << 24)); // primID = 0
//		}
//	}
//	return hit;
//}


// more hints, not required but:

//layout (std430, binding = 0) readonly buffer TLASNodes { BVHNode nodes[]; } tlasNodeData;
//layout (std430, binding = 1) readonly buffer TLASIndices { uint idx[]; } tlasIdxData;
//
//struct Instance {
//	mat4 transform;
//	mat4 invTransform;
//	vec4 aabbMin;
//	vec4 aabbMax;
//	uint dummy[8]; // Padding to 64 bytes
//	// dummy[0]  // objectType (0 = mesh, 1 = sphere)
//};
//
//layout (std430, binding = 2) readonly buffer InstanceBuffer { Instance inst[]; } instanceData;
// is common format