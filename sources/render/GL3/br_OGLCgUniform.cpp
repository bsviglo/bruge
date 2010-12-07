#include "br_OGLCgUniform.h"

namespace brUGE
{
namespace render
{
	//------------------------------------------
	brOGLCgNumericUniform::brOGLCgNumericUniform(
		const std::string& name,
		CGparameter parameter)
			:	name_(name),
				parameter_(parameter)
	{

	}
	
	//------------------------------------------
	brOGLCgNumericUniform::~brOGLCgNumericUniform()
	{

	}
	
	//------------------------------------------	
	void brOGLCgNumericUniform::setBool(bool x)
	{
		cgSetParameter1i(parameter_, x);
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setFloat(float x)
	{
		cgSetParameter1f(parameter_, x);
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setInt(int x)
	{
		cgSetParameter1i(parameter_, x);
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setUInt(uint /*x*/)
	{
		// TODO: ждем поддержки от Cg.
		throw brException("brOGLCgNumericUniform::setUInt", "Method is not implemented.");
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setVec2f(const vec2f& vec)
	{
		cgSetParameter2fv(parameter_, vec.pointer());			
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setVec2i(const vec2i& vec)
	{
		cgSetParameter2iv(parameter_, vec.pointer());			
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setVec2ui(const vec2ui& /*vec*/)
	{
		// TODO: ждем поддержки от Cg.
		throw brException("brOGLCgNumericUniform::setVec2ui", "Method is not implemented.");
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setVec3f(const vec3f& vec)
	{
		cgSetParameter3fv(parameter_, vec.pointer());	
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setVec3i(const vec3i& vec)
	{
		cgSetParameter3iv(parameter_, vec.pointer());	
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setVec3ui(const vec3ui& /*vec*/)
	{
		// TODO: ждем поддержки от Cg.
		throw brException("brOGLCgNumericUniform::setVec3ui", "Method is not implemented.");
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setVec4f(const vec4f& vec)
	{
		cgSetParameter4fv(parameter_, vec.pointer());
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setVec4i(const vec4i& vec)
	{
		cgSetParameter4iv(parameter_, vec.pointer());
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setVec4ui(const vec4ui& /*vec*/)
	{
		// TODO: ждем поддержки от Cg.
		throw brException("brOGLCgNumericUniform::setVec4ui", "Method is not implemented.");
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setMat2f(const mat2f& mat, ebrMatrixOrder order)
	{
		// TODO: Так как в текущей реализации матрица храниться в памяти по СТОЛБЦАМ,
		//		для того, что бы передать ее в этом же состоянии мы будем использовать передачу 
		//		по строкам, и наоброт если нам нужно получить матрицу хранимую по строкам, мы
		//		передадим ее по столбцам, тем самым инвертируя ее изначальное состояние.
		
		// TODO: Вполне возможно, что в ближайшее время мат библиотека будет переписана по-нормальному.
		//		Вообще-то более логичным было бы, что бы матрица хранилась по строкам в памяти,
		//		так как это проще для понимания.

		if (order == MO_COLUMN)
			cgSetMatrixParameterfr(parameter_, mat.pointer());
		else
			cgSetMatrixParameterfc(parameter_, mat.pointer());
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setMat2i(const mat2i& mat, ebrMatrixOrder order)
	{
		if (order == MO_COLUMN)
			cgSetMatrixParameterir(parameter_, mat.pointer());
		else
			cgSetMatrixParameteric(parameter_, mat.pointer());
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setMat2ui(const mat2ui& /*mat*/, ebrMatrixOrder /*order*/)
	{
		// TODO: ждем поддержки от Cg.
		throw brException("brOGLCgNumericUniform::setMat2ui", "Method is not implemented.");
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setMat3f(const mat3f& mat, ebrMatrixOrder order)
	{
		// TODO: Так как в текущей реализации матрица храниться в памяти по СТОЛБЦАМ,
		//		для того, что бы передать ее в этом же состоянии мы будем использовать передачу 
		//		по строкам, и наоброт если нам нужно получить матрицу хранимую по строкам, мы
		//		передадим ее по столбцам, тем самым инвертируя ее изначальное состояние.

		// TODO: Вполне возможно, что в ближайшее время мат библиотека будет переписана по-нормальному.
		//		Вообще-то более логичным было бы, что бы матрица хранилась по строкам в памяти,
		//		так как это проще для понимания.

		if (order == MO_COLUMN)
			cgSetMatrixParameterfr(parameter_, mat.pointer());
		else
			cgSetMatrixParameterfc(parameter_, mat.pointer());
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setMat3i(const mat3i& mat, ebrMatrixOrder order)
	{
		if (order == MO_COLUMN)
			cgSetMatrixParameterir(parameter_, mat.pointer());
		else
			cgSetMatrixParameteric(parameter_, mat.pointer());
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setMat3ui(const mat3ui& /*mat*/, ebrMatrixOrder /*order*/)
	{
		// TODO: ждем поддержки от Cg.
		throw brException("brOGLCgNumericUniform::setMat3ui", "Method is not implemented.");
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setMat4f(const mat4f& mat, ebrMatrixOrder order)
	{
		// TODO: Так как в текущей реализации матрица храниться в памяти по СТОЛБЦАМ,
		//		для того, что бы передать ее в этом же состоянии мы будем использовать передачу 
		//		по строкам, и наоброт если нам нужно получить матрицу хранимую по строкам, мы
		//		передадим ее по столбцам, тем самым инвертируя ее изначальное состояние.

		// TODO: Вполне возможно, что в ближайшее время мат библиотека будет переписана по-нормальному.
		//		Вообще-то более логичным было бы, что бы матрица хранилась по строкам в памяти,
		//		так как это проще для понимания.

		if (order == MO_COLUMN)
			cgSetMatrixParameterfr(parameter_, mat.pointer());
		else
			cgSetMatrixParameterfc(parameter_, mat.pointer());
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setMat4i(const mat4i& mat, ebrMatrixOrder order)
	{
		if (order == MO_COLUMN)
			cgSetMatrixParameterir(parameter_, mat.pointer());
		else
			cgSetMatrixParameteric(parameter_, mat.pointer());
	}
	
	//------------------------------------------
	void brOGLCgNumericUniform::setMat4ui(const mat4ui& /*mat*/, ebrMatrixOrder /*order*/)
	{
		// TODO: ждем поддержки от Cg.
		throw brException("brOGLCgNumericUniform::setMat3ui", "Method is not implemented.");
	}
}
}