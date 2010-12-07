#ifndef _BR_GLCGSHADER_H_
#define _BR_GLCGSHADER_H_

#include "br_GLCommon.h"
#include "render/ibr_Shader.h"

namespace brUGE
{
namespace render
{
	// Идеалогия анолагична той что и ibrBuffer иерархии.
	// смотри реализацию brOGLCgBuffer.
	class brGLCgShaderImpl
	{
	public:
		brGLCgShaderImpl(
			CGenum sourceType,
			const brStr& sourceStr,
			CGGLenum profileClass,
			const brStr& entryPoint,
			const brStr& arguments
			);

		virtual ~brGLCgShaderImpl();

		const CGprofile& getCgProfile() const { return m_profile; }
		const CGprogram& getCgProgram() const { return m_program; }

	protected:
		CGprofile m_profile;
		CGprogram m_program;
	};
	
	//------------------------------------------
	class brGLCgVertexShader :	public ibrVertexShader,
								public brGLCgShaderImpl
	{
	public:
		brGLCgVertexShader(
			ibrShader::ebrSourceType sourceType,
			const brStr& sourceStr,
			const brStr& entryPoint,
			const brStr& args
			);
		virtual ~brGLCgVertexShader() {}
	};
	
	//------------------------------------------
	class brGLCgFragmentShader	:	public ibrFragmentShader,
									public brGLCgShaderImpl
	{
	public:
		brGLCgFragmentShader(
			ibrShader::ebrSourceType sourceType,
			const brStr& sourceStr,
			const brStr& entryPoint,
			const brStr& args
			);
		virtual ~brGLCgFragmentShader() {}
	};
	
	//------------------------------------------
	class brGLCgGeometryShader	:	public ibrGeometryShader,
									public brGLCgShaderImpl
	{
	public:
		brGLCgGeometryShader(
			ibrShader::ebrSourceType sourceType,
			const brStr& sourceStr,
			const brStr& entryPoint,
			const brStr& args
			);
		virtual ~brGLCgGeometryShader() {}
	};

} // render
} // brUGE

#endif /*_BR_OGLCGSHADER_H_*/