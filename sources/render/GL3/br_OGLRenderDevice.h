#ifndef _BR_OGLRENDERDEVICE_H_
#define _BR_OGLRENDERDEVICE_H_

#include "br_OGLCommon.h"
#include "render/ibr_RenderDevice.h"
#include <memory>

namespace brUGE
{
namespace render
{
	// 
	//------------------------------------------
	class brOGLRenderDevice : public ibrRenderDevice
	{
	protected:
		enum
		{
			MAX_VERTEX_INPUT_SLOTS = 4,
			MAX_VERTEX_ATTRIBS = 16 // TODO: Брать эту инфу из списка капсов для OpenGL.
		};

	public:
		brOGLRenderDevice();
		virtual ~brOGLRenderDevice();
		
		virtual void init(HWND hWindow, const brVideoMode &videoMode);
		virtual void shutDown();

		virtual brPtr<ibrFont> createFontObject();

		virtual void setBuffersClearValues(const brColourF& colour,	float depth, uint8 stencil);
		virtual void clearBuffers(bool colour, bool depth, bool stencil);
		virtual void swapBuffers();

		virtual void setPrimitiveTopology(ebrPrimitiveTopology topology);
		virtual void setVertexLayout(const brPtr<const ibrVertexLayout>& layout);
		virtual void setVertexBuffer(uint inputSlot, const brPtr<const ibrVertexBuffer>& buffer, uint offset);
		virtual void setIndexBuffer(const brPtr<const ibrIndexBuffer>& buffer);

		virtual void setShadingProgram(const brPtr<const ibrShadingProgram>& program);

		//virtual void draw(uint first, uint count);
		virtual void drawIndexed(uint first, uint count);

		// Установка стейт-объектов.
		virtual void setRasterizerState(const brPtr<const ibrRasterizerState>& state);
		virtual void setDepthStencilState(const brPtr<const ibrDepthStencilState>& state, uint stencilRef);
		virtual void setBlendState(const brPtr<const ibrBlendState>& state);

	private:
		bool _checkExtensions() const;
		bool _isVertexBufferBound() const;

	private:
		std::auto_ptr<brOGLSwapChain>		swapChain_;
		brPtr<const brOGLVertexLayout>		vertexLayout_;
		brPtr<const brGLIndexBuffer>		indexBuffer_;
		brPtr<const brGLCgShadingProgram>	shadingProgram_;
		GLenum primitiveTopology_;
		brPtr<const brGLVertexBuffer> vertexBuffers_[MAX_VERTEX_INPUT_SLOTS]; 
	};

}
}

#endif /*_BR_OGLRENDERDEVICE_H_*/