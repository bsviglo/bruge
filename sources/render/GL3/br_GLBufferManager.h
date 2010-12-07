#ifndef _BR_OGLBUFFERMANAGER_H_
#define _BR_OGLBUFFERMANAGER_H_

#include "br_GLCommon.h"
#include "render/ibr_BufferManager.h"

namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	class brGLBufferManager : public ibrBufferManager
	{
	public:
		brGLBufferManager();
		virtual ~brGLBufferManager();

		virtual Ptr<ibrVertexBuffer> createVertexBuffer(
			uint vertexSize,
			uint vertexCount,
			void* data,
			ibrBuffer::ebrUsageFlag usage /* = ibrBuffer::UF_DEFAULT */,
			ibrBuffer::ebrCPUAccessFlag	cpuAccess /* = ibrBuffer::CPU_AF_NONE */
			);

		virtual Ptr<ibrIndexBuffer> createIndexBuffer(
			ibrIndexBuffer::ebrIndexType indexType,
			uint indexCount,
			void* data,
			ibrBuffer::ebrUsageFlag usage /* = ibrBuffer::UF_DEFAULT */,
			ibrBuffer::ebrCPUAccessFlag	cpuAccess /* = ibrBuffer::CPU_AF_NONE */
			);

		virtual Ptr<ibrVertexLayout> createVertexLayout(
			const brVertexDesc *vd,
			uint count,
			const Ptr<ibrShadingProgram>& shader
			);
	};

}
}

#endif /*_BR_OGLBUFFERMANAGER_H_*/