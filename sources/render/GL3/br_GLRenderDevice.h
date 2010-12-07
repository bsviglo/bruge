#ifndef _BR_GLRENDERDEVICE_H_
#define _BR_GLRENDERDEVICE_H_

#include "br_GLCommon.h"
#include "render/ibr_RenderDevice.h"
#include <memory>

namespace brUGE
{
namespace render
{
	// 
	//------------------------------------------
	class brGLRenderDevice : public ibrRenderDevice
	{
	protected:
		enum
		{
			MAX_VERTEX_INPUT_SLOTS	= 4,
			MAX_VERTEX_ATTRIBS		= 16 // TODO: Брать эту инфу из списка капсов для OpenGL.
		};

	public:
		brGLRenderDevice();
		virtual ~brGLRenderDevice();
		
		virtual void init(HWND hWindow, const VideoMode &videoMode);
		virtual void shutDown();

		virtual void setBuffersClearValues(const Color& colour,	float depth, uint8 stencil);
		virtual void clearBuffers(bool colour, bool depth, bool stencil);
		virtual void swapBuffers();

		virtual void setPrimitiveTopology(ebrPrimitiveTopology topology);
		virtual void setVertexLayout(const Ptr<const ibrVertexLayout>& layout);
		virtual void setVertexBuffer(uint inputSlot, const Ptr<const ibrVertexBuffer>& buffer, uint offset);
		virtual void setIndexBuffer(const Ptr<const ibrIndexBuffer>& buffer);

		virtual void setShadingProgram(const Ptr<const ibrShadingProgram>& program);

		virtual void setRenderTargets(
			uint numTargets,
			const Ptr<ibrTexture>* renderTargets,
			const Ptr<ibrTexture>& depthStencilTarget
			);

		//virtual void draw(uint first, uint count);
		virtual void drawIndexed(uint first, uint count);

		// Установка стейт-объектов.
		virtual void setRasterizerState(const Ptr<const ibrRasterizerState>& state);
		virtual void setDepthStencilState(const Ptr<const ibrDepthStencilState>& state, uint stencilRef);
		virtual void setBlendState(const Ptr<const ibrBlendState>& state);

		// Buffers.
		//------------------------------------------
		virtual Ptr<ibrVertexBuffer> createVertexBuffer(
			uint vertexSize,
			uint vertexCount,
			void* data,
			ibrBuffer::ebrUsageFlag usage = ibrBuffer::UF_DEFAULT,
			ibrBuffer::ebrCPUAccessFlag	cpuAccess = ibrBuffer::CPU_AF_NONE
			);

		virtual Ptr<ibrIndexBuffer> createIndexBuffer(
			ibrIndexBuffer::ebrIndexType indexType,
			uint indexCount,
			void* data,
			ibrBuffer::ebrUsageFlag usage = ibrBuffer::UF_DEFAULT,
			ibrBuffer::ebrCPUAccessFlag	cpuAccess = ibrBuffer::CPU_AF_NONE
			);

		virtual Ptr<ibrVertexLayout> createVertexLayout(
			const brVertexDesc *vd,
			uint count,
			const Ptr<ibrShadingProgram>& shader
			);

		// Shaders.
		//------------------------------------------
		virtual Ptr<ibrVertexShader> createVertexShader(
			ibrShader::ebrSourceType sourceType,
			const brStr& sourceStr,
			const brStr& entryPoint = "VS",
			const brStr& arguments = ""
			);

		virtual Ptr<ibrFragmentShader> createFragmentShader(
			ibrShader::ebrSourceType sourceType,
			const brStr& sourceStr,
			const brStr& entryPoint = "FS",
			const brStr& arguments = ""
			);

		virtual Ptr<ibrGeometryShader> createGeometryShader(
			ibrShader::ebrSourceType sourceType,
			const brStr& sourceStr,
			const brStr& entryPoint = "GS",
			const brStr& arguments = ""
			);

		virtual Ptr<ibrShadingProgram> createShadingProgram();

		// State objects.
		//------------------------------------------
		virtual Ptr<ibrDepthStencilState> createDepthStencilState(const ibrDepthStencilState::Desc_s& desc);
		virtual Ptr<ibrRasterizerState>	createRasterizedState(const ibrRasterizerState::Desc_s& desc);
		virtual Ptr<ibrBlendState>		createBlendState(const ibrBlendState::Desc_s& desc);
		virtual Ptr<ibrSamplerState>		createSamplerState(const ibrSamplerState::Desc_s& desc);

		// Textures.
		//------------------------------------------
		virtual Ptr<ibrTexture> createTexture(
			const ibrTexture::Desc_s& desc,
			const void* data = NULL
			);

	private:
		bool _isVertexBufferBound() const;

	private:
		std::auto_ptr<brGLSwapChain>		m_swapChain;
		std::auto_ptr<brGLFrameBuffer>		m_frameBuffer;
		Ptr<const brGLVertexLayout>		m_vertexLayout;
		Ptr<const brGLIndexBuffer>		m_indexBuffer;
		Ptr<const brGLCgShadingProgram>	m_shadingProgram;
		Ptr<const brGLVertexBuffer>		m_vertexBuffers[MAX_VERTEX_INPUT_SLOTS]; 
		GLenum								m_primitiveTopology;
	};

} // render
} // brUGE

#endif /*_BR_GLRENDERDEVICE_H_*/