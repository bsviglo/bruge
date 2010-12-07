#include "Functors.h"
#include "utils/string_utils.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"

namespace brUGE
{
	using namespace utils;

	// Parser of input parameters. 
	// Note: put other types here.
	//------------------------------------------
	template <typename T>
	T Functor::parse(const std::string& str)
	{
		T* error_specialization_not_provided[-1]; // ToDo: use static_assert() if available
	}

	//------------------------------------------
	template <>
	bool Functor::parse<bool>(const std::string& str)
	{
		return strToBool(str);
	}

	//------------------------------------------
	template <>
	int Functor::parse<int>(const std::string& str)
	{
		return strToInt(str);
	}

	//------------------------------------------
	template <>
	double Functor::parse<double>(const std::string& str)
	{
		return static_cast<double>(strToFloat(str));
	}

	//------------------------------------------
	template <>
	float Functor::parse<float>(const std::string& str)
	{
		return strToFloat(str);
	}

	//------------------------------------------
	template <>
	vec2f Functor::parse<vec2f>(const std::string& str)
	{
		vec2f vec;
		if (sscanf(str.c_str(), "vec2f(%f,%f)", &vec.x, &vec.y) != 2) throw std::runtime_error("can't convert to vec2f.");
		return vec;
	}
	
	//------------------------------------------
	template <>
	vec3f Functor::parse<vec3f>(const std::string& str)
	{
		vec3f vec;
		if (sscanf(str.c_str(), "vec3f(%f,%f,%f)", &vec.x, &vec.y, &vec.z) != 3) throw std::runtime_error("can't convert to vec3f.");
		return vec;
	}

	//------------------------------------------
	template <>
	vec4f Functor::parse<vec4f>(const std::string& str)
	{
		vec4f vec;
		if (sscanf(str.c_str(), "vec4f(%f,%f,%f,%f)", &vec.x, &vec.y, &vec.z, &vec.w) != 4) throw std::runtime_error("can't convert to vec4f.");
		return vec;
	}
	
	//------------------------------------------
	template <>
	std::string Functor::parse<std::string>(const std::string& str)
	{
		return str;
	}

} // brUGE