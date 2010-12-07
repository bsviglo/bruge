#ifndef _BR_OGLCGSHADERMANAGER_H_
#define _BR_OGLCGSHADERMANAGER_H_

#include "br_OGLCommon.h"
#include "render/ibr_ShaderManager.h"

namespace brUGE
{
namespace render
{
	// 
	//------------------------------------------
	class brOGLCgShaderManager : public ibrShaderManager
	{
	public:
		brOGLCgShaderManager();
		virtual ~brOGLCgShaderManager();

		virtual Ptr<ibrVertexShader> createVertexShader(
			ibrShader::ebrSourceType sourceType,
			const brStr& sourceStr,
			const brStr& entryPoint,
			const brStr& args
			);

		virtual Ptr<ibrFragmentShader> createFragmentShader(
			ibrShader::ebrSourceType sourceType,
			const brStr& sourceStr,
			const brStr& entryPoint,
			const brStr& args
			);

		virtual Ptr<ibrGeometryShader> createGeometryShader(
			ibrShader::ebrSourceType sourceType,
			const brStr& sourceStr,
			const brStr& entryPoint,
			const brStr& args
			);

		virtual Ptr<ibrShadingProgram> createShadingProgram();
	};

}
}

#endif /*_BR_OGLCGSHADERMANAGER_H_*/