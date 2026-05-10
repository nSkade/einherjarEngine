#ifdef __cplusplus
	#pragma once
	#include <cstdint>
	#include <type_traits>

	#pragma pack(push,1)
#else // glsl
	#define uint32_t uint
#endif

struct EHJSD_Instance {
	vec4 pos;
	vec4 col;
};

struct EHJSD_DrawElementsIndirectCommand {
	uint32_t count;
	uint32_t instanceCount;
	uint32_t firstIndex;
	uint32_t baseVertex;
	uint32_t baseInstance;
};

#ifdef __cplusplus
	#pragma pack(pop)

	static_assert(sizeof(EHJSD_DrawElementsIndirectCommand) == 20, "DrawElementsIndirectCommand size mismatch");
	static_assert(alignof(EHJSD_DrawElementsIndirectCommand) <= 4, "Unexpected alignment");
#else // glsl
	#undef uint32_t
#endif