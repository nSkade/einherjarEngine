#include <json/json.h>
#include <string>


namespace ehj {

// Base Template
template<typename T> struct ConfigFileSerializer;

// if EHJ_SERIALIZER_DECLARATION_ONLY is defined, only generate function signatures
// otherwise, generate the actual function bodies
#ifdef EHJ_SERIALIZER_DECLARATION_ONLY
	#define SERIALIZER_ENTRY(Type, FromBody, ToBody)		\
		template<> struct ConfigFileSerializer<Type> {		\
			static Type from(const Json::Value& v);			\
			static Json::Value to(const Type& v);			\
		};
#else
	#define SERIALIZER_ENTRY(Type, FromBody, ToBody)		\
		template<> struct ConfigFileSerializer<Type> {		\
				static Type from(const Json::Value& v);		\
				static Json::Value to(const Type& v);		\
			};												\
		Type ConfigFileSerializer<Type>::from(const Json::Value& v) FromBody; \
		Json::Value ConfigFileSerializer<Type>::to(const Type& v) ToBody;
#endif

SERIALIZER_ENTRY(int,
	{ return v.asInt(); },
	{ return Json::Value(v); }
)

SERIALIZER_ENTRY(float,
	{ return v.asFloat(); },
	{ return Json::Value(v); }
)

SERIALIZER_ENTRY(bool,
	{ return v.asBool(); },
	{ return Json::Value(v); }
)

SERIALIZER_ENTRY(std::string,
	{ return v.asString(); },
	{ return Json::Value(v); }
)

// -------------------------------------------------------------------
// add new types here.
// format: Type, { From Logic }, { To Logic }
// -------------------------------------------------------------------

SERIALIZER_ENTRY(glm::vec3,
	{
		if (v.isArray() && v.size() == 3) {
			return glm::vec3(v[0].asFloat(), v[1].asFloat(), v[2].asFloat());
		}
		return glm::vec3(0.0f, 0.0f, 0.0f);
	},
	{
		Json::Value arr(Json::arrayValue);
		arr.append(v.x);
		arr.append(v.y);
		arr.append(v.z);
		return arr;
	}
)

SERIALIZER_ENTRY(glm::ivec2,
	{
		if (v.isArray() && v.size() == 2) {
			return glm::ivec2(v[0].asInt(), v[1].asInt());
		}
		return glm::ivec2(0, 0);
	},
	{
		Json::Value arr(Json::arrayValue);
		arr.append(v.x);
		arr.append(v.y);
		return arr;
	}
)

#undef SERIALIZER_ENTRY
}//ehj
