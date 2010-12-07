#include "br_GLCgShader.h"
#include "br_GLCgContext.h"

#define BR_USE_LATEST_SHADER_PROFILE	1
#define BR_USE_OPTIMAL_COMPILER_OPTIONS 0 

namespace brUGE
{
namespace render
{

	//------------------------------------------
	brGLCgShaderImpl::brGLCgShaderImpl(
		CGenum sourceType,
		const brStr& sourceStr,
		CGGLenum shaderClass,
		const brStr& entryPoint,
		const brStr& arguments)
	{
		try
		{
			brGLCgContext::addRef();

			// TODO: Поддерживаем только Shader Model 4.0 если нет кидать исключение.

#if BR_USE_LATEST_SHADER_PROFILE
			m_profile = cgGLGetLatestProfile(shaderClass);
			if (cgGetError() == CG_INVALID_ENUMERANT_ERROR)
				BR_EXCEPT("brGLCgShaderImpl::brGLCgShaderImpl",
					"profileClass is not CG_GL_VERTEX, CG_GL_GEOMETRY or CG_GL_FRAGMENT.");
#else
			switch (shaderClass)
			{
			case CG_GL_FRAGMENT:	m_profile = CG_PROFILE_GLSLF; break;
			case CG_GL_VERTEX:		m_profile = CG_PROFILE_GLSLV; break;
			case CG_GL_GEOMETRY:	m_profile = CG_PROFILE_GLSLG; break;
			};
#endif

#if BR_USE_OPTIMAL_COMPILER_OPTIONS
			char const ** ppOptions = cgGLGetOptimalOptions(m_profile);

	#ifdef _DEBUG			
			WARNING("Optimal compiler options:");
			if (ppOptions && *ppOptions)
				while (*ppOptions)
				{
					LOG_F("%s", *ppOptions);
					++ppOptions;
				}
	#endif // _DEBUG
#endif

// TODO: Продумать...

			const char *args[] =
			{
				arguments.c_str(),
//#if BR_USE_OPTIMAL_COMPILER_OPTIONS
//				ppOptions,
//#endif
				NULL
			};

			m_program = cgCreateProgram(
				brGLCgContext::getCgContext(),
				sourceType,
				sourceStr.c_str(),
				m_profile,
				(entryPoint.length() == 0) ? NULL : entryPoint.c_str(),
				(arguments.length() == 0) ? NULL : args
			);

			if (cgGetError() != CG_NO_ERROR)
				BR_EXCEPT_F("brGLCgShaderImpl::brGLCgShaderImpl",
					"Failed to create Cg shader program. Cg Compiler error: \n %s.", brGLCgContext::getLastListing());

			cgCompileProgram(m_program);
			if (cgGetError() != CG_NO_ERROR)
				BR_EXCEPT_F("brGLCgShaderImpl::brGLCgShaderImpl",
					"Failed to create Cg shader program. Cg Compiler error: \n %s.", brGLCgContext::getLastListing());

			// Log compiled shader.
#if 0
			WARNING("Compiled shader:");
			LOG(cgGetProgramString(m_program, CG_COMPILED_PROGRAM));
#endif
		}
		catch(brException& exp)
		{
			if (cgIsProgram(m_program))
				cgDestroyProgram(m_program);
			brGLCgContext::release();
			throw exp;
		}
	}
	
	//------------------------------------------
	brGLCgShaderImpl::~brGLCgShaderImpl()
	{
		cgDestroyProgram(m_program);
		brGLCgContext::release();
	}

	brGLCgVertexShader::brGLCgVertexShader(
		ibrShader::ebrSourceType sourceType,
		const brStr &sourceStr,
		const brStr &entryPoint,
		const brStr &args
		)
			:	brGLCgShaderImpl(
					(sourceType == ibrShader::ST_OBJECT) ? CG_OBJECT : CG_SOURCE,
					sourceStr,
					CG_GL_VERTEX,
					entryPoint,
					args
					)
	{

	}

	brGLCgFragmentShader::brGLCgFragmentShader(
		ibrShader::ebrSourceType sourceType,
		const brStr &sourceStr,
		const brStr &entryPoint,
		const brStr &args
		)
		:	brGLCgShaderImpl(
				(sourceType == ibrShader::ST_OBJECT) ? CG_OBJECT : CG_SOURCE,
				sourceStr,
				CG_GL_FRAGMENT,
				entryPoint,
				args
				)
	{

	}

	brGLCgGeometryShader::brGLCgGeometryShader(
		ibrShader::ebrSourceType sourceType,
		const brStr &sourceStr,
		const brStr &entryPoint,
		const brStr &args
		)
		:	brGLCgShaderImpl(
				(sourceType == ibrShader::ST_OBJECT) ? CG_OBJECT : CG_SOURCE,
				sourceStr,
				CG_GL_GEOMETRY,
				entryPoint,
				args
				)
	{

	}

} // render
} // brUGE
