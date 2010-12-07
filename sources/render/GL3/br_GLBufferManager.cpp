#include "br_GLBufferManager.h"
#include "br_GLBuffer.h"
#include "br_OGLVertexLayout.h"

namespace brUGE
{
namespace render
{

	//------------------------------------------
	brGLBufferManager::brGLBufferManager()
	{

	}
	
	//------------------------------------------
	brGLBufferManager::~brGLBufferManager()
	{

	}
	
	//------------------------------------------
	Ptr<ibrVertexBuffer> brGLBufferManager::createVertexBuffer(
		uint vertexSize,
		uint vertexCount,
		void* data,
		ibrBuffer::ebrUsageFlag usage /* = ibrBuffer::UF_DEFAULT */,
		ibrBuffer::ebrCPUAccessFlag	cpuAccess /* = ibrBuffer::CPU_AF_NONE */)
	{
		return new brGLVertexBuffer(vertexCount, vertexSize, usage, cpuAccess, data);
	}
	
	//------------------------------------------		
	Ptr<ibrIndexBuffer> brGLBufferManager::createIndexBuffer(
		ibrIndexBuffer::ebrIndexType indexType,
		uint indexCount,
		void* data,
		ibrBuffer::ebrUsageFlag usage /* = ibrBuffer::UF_DEFAULT */,
		ibrBuffer::ebrCPUAccessFlag	cpuAccess /* = ibrBuffer::CPU_AF_NONE */)
	{
		return new brGLIndexBuffer(indexCount, indexType, usage, cpuAccess, data);
	}
	
	//------------------------------------------
	Ptr<ibrVertexLayout> brGLBufferManager::createVertexLayout(
		const brVertexDesc *vd,
		uint count,
		const Ptr<ibrShadingProgram>& shader )
	{
		return new brGLVertexLayout(vd, count, shader);
	}
}
}