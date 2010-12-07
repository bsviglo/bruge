#include "br_OGLCgShaderManager.h"
#include "br_OGLCgShader.h"
#include "br_GLCgShadingProgram.h"

namespace brUGE
{
namespace render
{
	//------------------------------------------
	brOGLCgShaderManager::brOGLCgShaderManager()
	{
		
	}
	
	//------------------------------------------
	brOGLCgShaderManager::~brOGLCgShaderManager()
	{

	}

	//------------------------------------------
	Ptr<ibrVertexShader> brOGLCgShaderManager::createVertexShader(
		ibrShader::ebrSourceType sourceType,
		const brStr& sourceStr,
		const brStr& entryPoint,
		const brStr& args
		)
	{
		return new brGLCgVertexShader(sourceType, sourceStr, entryPoint, args);
	}

	//------------------------------------------
	Ptr<ibrFragmentShader> brOGLCgShaderManager::createFragmentShader(
		ibrShader::ebrSourceType sourceType,
		const brStr& sourceStr,
		const brStr& entryPoint,
		const brStr& args
		)
	{
		return new brGLCgFragmentShader(sourceType, sourceStr, entryPoint, args);
	}

	//------------------------------------------
	Ptr<ibrGeometryShader> brOGLCgShaderManager::createGeometryShader(
		ibrShader::ebrSourceType sourceType,
		const brStr& sourceStr,
		const brStr& entryPoint,
		const brStr& args
		)
	{
		return new brGLCgGeometryShader(sourceType, sourceStr, entryPoint, args);
	}
	
	//------------------------------------------
	Ptr<ibrShadingProgram> brOGLCgShaderManager::createShadingProgram()
	{
		return new brGLCgShadingProgram;
	}

}
}