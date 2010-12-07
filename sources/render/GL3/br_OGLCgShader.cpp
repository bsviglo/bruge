#include "br_OGLCgShader.h"
#include "br_OGLCgContext.h"

namespace brUGE
{
namespace render
{
	brStr brOGLCgShaderImpl::lastListing_ = "";

	//------------------------------------------
	brOGLCgShaderImpl::brOGLCgShaderImpl(
		CGenum sourceType,
		const brStr& sourceStr,
		CGGLenum shaderClass,
		const brStr& entryPoint,
		const brStr& arguments)
	{
		try
		{
			brOGLCgContext::addRef();

			// TODO: Поддерживаем только Shader Model 4.0 если нет кидать исключение.

			profile_ = cgGLGetLatestProfile(shaderClass);
			CGerror error = cgGetError();
			if (error == CG_INVALID_ENUMERANT_ERROR)
				throw brException("brOGLCgShaderImpl::brOGLCgShaderImpl",
					"profileClass is not CG_GL_VERTEX, CG_GL_GEOMETRY or CG_GL_FRAGMENT.");

			const char *args[ 2 ] =
			{
				arguments.c_str(),
				NULL
			};

			program_ = cgCreateProgram(
				brOGLCgContext::getCgContext(),
				sourceType,
				sourceStr.c_str(),
				profile_,
				(entryPoint.length() == 0) ? NULL : entryPoint.c_str(),
				(arguments.length() == 0) ? NULL : args
			);

			error = cgGetError();
			if (error != CG_NO_ERROR)
			{
				lastListing_ = brOGLCgContext::getLastListing();
				throw brException("brOGLCgShaderImpl::brOGLCgShaderImpl",
					"Failed to create Cg shader program. Cg Compiler error: " + lastListing_);
			}

			cgCompileProgram(program_);
			error = cgGetError();
			if (error != CG_NO_ERROR)
			{
				lastListing_ = brOGLCgContext::getLastListing();
				throw brException("brOGLCgShaderImpl::brOGLCgShaderImpl",
					"Failed to create Cg shader program. Cg Compiler error: " + lastListing_);
			}
			// Log compiled shader.
#if 0
			WARNING("Compiled shader:");
			LOG_F("%s", cgGetProgramString(program_, CG_COMPILED_PROGRAM));
#endif
		}
		catch(brException& exp)
		{
			if (cgIsProgram(program_))
				cgDestroyProgram(program_);
			brOGLCgContext::release();
			throw exp;
		}
	}
	
	//------------------------------------------
	brOGLCgShaderImpl::~brOGLCgShaderImpl()
	{
		cgDestroyProgram(program_);
		brOGLCgContext::release();
	}

	brOGLCgVertexShader::brOGLCgVertexShader(
		ibrShader::ebrSourceType sourceType,
		const brStr &sourceStr,
		const brStr &entryPoint,
		const brStr &args
		)
			:	brOGLCgShaderImpl(
					(sourceType == ibrShader::ST_OBJECT) ? CG_OBJECT : CG_SOURCE,
					sourceStr,
					CG_GL_VERTEX,
					entryPoint,
					args
					)
	{

	}

	brOGLCgFragmentShader::brOGLCgFragmentShader(
		ibrShader::ebrSourceType sourceType,
		const brStr &sourceStr,
		const brStr &entryPoint,
		const brStr &args
		)
		:	brOGLCgShaderImpl(
				(sourceType == ibrShader::ST_OBJECT) ? CG_OBJECT : CG_SOURCE,
				sourceStr,
				CG_GL_FRAGMENT,
				entryPoint,
				args
				)
	{

	}

	brOGLCgGeometryShader::brOGLCgGeometryShader(
		ibrShader::ebrSourceType sourceType,
		const brStr &sourceStr,
		const brStr &entryPoint,
		const brStr &args
		)
		:	brOGLCgShaderImpl(
				(sourceType == ibrShader::ST_OBJECT) ? CG_OBJECT : CG_SOURCE,
				sourceStr,
				CG_GL_GEOMETRY,
				entryPoint,
				args
				)
	{

	}

}
}
