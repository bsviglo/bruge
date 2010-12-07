#ifndef _BR_OGLCGSHADER_H_
#define _BR_OGLCGSHADER_H_

#include "br_OGLCommon.h"
#include "render/ibr_Shader.h"

namespace brUGE
{
namespace render
{
	// Идеалогия анолагична той что и ibrBuffer иерархии.
	// смотри реализацию brOGLCgBuffer.
	class brOGLCgShaderImpl
	{
	public:
		brOGLCgShaderImpl(
			CGenum sourceType,
			const brStr& sourceStr,
			CGGLenum profileClass,
			const brStr& entryPoint,
			const brStr& arguments
			);

		virtual ~brOGLCgShaderImpl();

		const CGprofile& getCgProfile() const { return profile_; }
		const CGprogram& getCgProgram() const { return program_; }

		static const brStr& getLastListing() { return lastListing_; }

	protected:
		CGprofile profile_;
		CGprogram program_;
		static brStr lastListing_;
	};
	
	//------------------------------------------
	class brOGLCgVertexShader :	public ibrVertexShader,
								public brOGLCgShaderImpl
	{
	public:
		brOGLCgVertexShader(
			ibrShader::ebrSourceType sourceType,
			const brStr& sourceStr,
			const brStr& entryPoint,
			const brStr& args
			);
		virtual ~brOGLCgVertexShader() {}
	};
	
	//------------------------------------------
	class brOGLCgFragmentShader	:	public ibrFragmentShader,
									public brOGLCgShaderImpl
	{
	public:
		brOGLCgFragmentShader(
			ibrShader::ebrSourceType sourceType,
			const brStr& sourceStr,
			const brStr& entryPoint,
			const brStr& args
			);
		virtual ~brOGLCgFragmentShader() {}
	};
	
	//------------------------------------------
	class brOGLCgGeometryShader	:	public ibrGeometryShader,
									public brOGLCgShaderImpl
	{
	public:
		brOGLCgGeometryShader(
			ibrShader::ebrSourceType sourceType,
			const brStr& sourceStr,
			const brStr& entryPoint,
			const brStr& args
			);
		virtual ~brOGLCgGeometryShader() {}
	};

} // render
} // brUGE

#endif /*_BR_OGLCGSHADER_H_*/