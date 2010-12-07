#ifndef _BR_OGLCGUNIFORM_H_
#define _BR_OGLCGUNIFORM_H_

#include "br_OGLCommon.h"
#include "render/ibr_Uniform.h"
#include "utils/br_String.h"

namespace brUGE
{
namespace render
{
	class brOGLCgNumericUniform : public ibrNumericUniform
	{
	public:
		brOGLCgNumericUniform(const std::string& name, CGparameter parameter);
		virtual ~brOGLCgNumericUniform();
		
		virtual void setBool(bool x);
		virtual void setFloat(float x);
		virtual void setInt(int x);
		virtual void setUInt(uint x);

		virtual void setVec2f(const vec2f& vec);
		virtual void setVec2i(const vec2i& vec);
		virtual void setVec2ui(const vec2ui& vec);

		virtual void setVec3f(const vec3f& vec);
		virtual void setVec3i(const vec3i& vec);
		virtual void setVec3ui(const vec3ui& vec);

		virtual void setVec4f(const vec4f& vec);
		virtual void setVec4i(const vec4i& vec);
		virtual void setVec4ui(const vec4ui& vec);

		virtual void setMat2f(const mat2f& mat, ebrMatrixOrder order);
		virtual void setMat2i(const mat2i& mat, ebrMatrixOrder order);
		virtual void setMat2ui(const mat2ui& mat, ebrMatrixOrder order);

		virtual void setMat3f(const mat3f& mat, ebrMatrixOrder order);
		virtual void setMat3i(const mat3i& mat, ebrMatrixOrder order);
		virtual void setMat3ui(const mat3ui& mat, ebrMatrixOrder order);

		virtual void setMat4f(const mat4f& mat, ebrMatrixOrder order);
		virtual void setMat4i(const mat4i& mat, ebrMatrixOrder order);
		virtual void setMat4ui(const mat4ui& mat, ebrMatrixOrder order);

	private:
		std::string name_;
		CGparameter parameter_;
	};

	class brOGLCgSamplerUniform : public ibrSamplerUniform
	{

	};
}
}

#endif /*_BR_OGLCGUNIFORM_H_*/

