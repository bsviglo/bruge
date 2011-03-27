#include "prerequisites.hpp"
#include "utils/string_utils.h"
#include "math/Vector2.hpp"
#include "math/Vector3.hpp"
#include "math/Vector4.hpp"

namespace brUGE
{
namespace utils
{

	//-- Transform to the desired type from the input string. 
	//-- Note: put other types here.
	//---------------------------------------------------------------------------------------------
	template <typename T>
	T parseTo(const std::string& str)
	{
		static_assert("specialization not provided.");
	}

	//---------------------------------------------------------------------------------------------
	template <>
	bool parseTo<bool>(const std::string& str)
	{
		return strToBool(str);
	}

	//---------------------------------------------------------------------------------------------
	template <>
	int parseTo<int>(const std::string& str)
	{
		return strToInt(str);
	}

	//---------------------------------------------------------------------------------------------
	template <>
	uint parseTo<uint>(const std::string& str)
	{
		return strToInt(str);
	}

	//---------------------------------------------------------------------------------------------
	template <>
	double parseTo<double>(const std::string& str)
	{
		return static_cast<double>(strToFloat(str, 3));
	}

	//---------------------------------------------------------------------------------------------
	template <>
	float parseTo<float>(const std::string& str)
	{
		return strToFloat(str, 3);
	}

	//---------------------------------------------------------------------------------------------
	template <>
	vec2f parseTo<vec2f>(const std::string& str)
	{
		vec2f vec;
		if (sscanf(str.c_str(), "vec2f(%f,%f)", &vec.x, &vec.y) != 2)
			throw std::runtime_error("can't convert to vec2f.");
		return vec;
	}

	//---------------------------------------------------------------------------------------------
	template <>
	vec3f parseTo<vec3f>(const std::string& str)
	{
		vec3f vec;
		if (sscanf(str.c_str(), "vec3f(%f,%f,%f)", &vec.x, &vec.y, &vec.z) != 3)
			throw std::runtime_error("can't convert to vec3f.");
		return vec;
	}

	//---------------------------------------------------------------------------------------------
	template <>
	vec4f parseTo<vec4f>(const std::string& str)
	{
		vec4f vec;
		if (sscanf(str.c_str(), "vec4f(%f,%f,%f,%f)", &vec.x, &vec.y, &vec.z, &vec.w) != 4)
			throw std::runtime_error("can't convert to vec4f.");
		return vec;
	}

	//---------------------------------------------------------------------------------------------
	template <>
	std::string parseTo<std::string>(const std::string& str)
	{
		return str;
	}
	

	//-- Transform the desired type to the string.
	//-- Note: put other types here.
	//---------------------------------------------------------------------------------------------
	template <typename T>
	std::string parseFrom(const T& value)
	{
		T* error_specialization_not_provided[-1]; // ToDo: use static_assert() if available
	}
	
	//---------------------------------------------------------------------------------------------
	template <>
	std::string parseFrom<bool>(const bool& value)
	{
		return boolToStr(value);
	}

	//---------------------------------------------------------------------------------------------
	template <>
	std::string parseFrom<int>(const int& value)
	{
		return intToStr(value);
	}

	//---------------------------------------------------------------------------------------------
	template <>
	std::string parseFrom<uint>(const uint& value)
	{
		return intToStr(value);
	}

	//---------------------------------------------------------------------------------------------
	template <>
	std::string parseFrom<double>(const double& value)
	{
		return floatToStr(static_cast<float>(value));
	}

	//---------------------------------------------------------------------------------------------
	template <>
	std::string parseFrom<float>(const float& value)
	{
		return floatToStr(static_cast<float>(value));
	}

	//---------------------------------------------------------------------------------------------
	template <>
	std::string parseFrom<vec2f>(const vec2f& value)
	{
		return makeStr("vec2f(%.3f,%.3f)", value.x, value.y);
	}

	//---------------------------------------------------------------------------------------------
	template <>
	std::string parseFrom<vec3f>(const vec3f& value)
	{
		return makeStr("vec3f(%.3f,%.3f,%.3f)", value.x, value.y, value.z);
	}

	//---------------------------------------------------------------------------------------------
	template <>
	std::string parseFrom<vec4f>(const vec4f& value)
	{
		return makeStr("vec4f(%.3f,%.3f,%.3f,%.3f)", value.x, value.y, value.z, value.w);
	}

	//---------------------------------------------------------------------------------------------
	template <>
	std::string parseFrom<std::string>(const std::string& str)
	{
		return str;
	}

} // utils
} // brUGE