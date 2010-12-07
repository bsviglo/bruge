#pragma once

#include "Dx_common.hpp"
#include "render/ITexture.h"

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	class DXTexture : public ITexture
	{
	public:
		DXTexture(const Desc& desc);
		virtual ~DXTexture();

		bool init(const ITexture::Data* data = NULL, uint size = 0);
		bool init(const ComPtr<ID3D10Texture2D>& tex2D);
		
		ID3D10Texture1D*		  getTex1D()  const { return m_tex1D.get(); }
		ID3D10Texture2D*		  getTex2D()  const { return m_tex2D.get(); }
		ID3D10Texture3D*		  getTex3D()  const { return m_tex3D.get(); }
		ID3D10Resource*			  getTex()	  const { return m_tex; }

		ID3D10RenderTargetView*	  getRTView() const { return m_RTView.get(); }
		ID3D10ShaderResourceView* getSRView() const { return m_SRView.get(); }
		ID3D10DepthStencilView*	  getDSView() const { return m_DSView.get(); }

	protected:
		virtual void doGenerateMipmaps();
	
	private:
		bool _createTex1D(UINT bindFlags, const Data* data, uint size);
		bool _createTex2D(UINT bindFlags, const Data* data, uint size, bool cubeMap = false);
		bool _createTex3D(UINT bindFlags, const Data* data, uint size);

		bool _createRTView();
		bool _createSRView();
		bool _createDSView();

	private:
		ComPtr<ID3D10Texture1D>	 m_tex1D;
		ComPtr<ID3D10Texture2D>	 m_tex2D;
		ComPtr<ID3D10Texture3D>	 m_tex3D;
		ID3D10Resource*			 m_tex;		

		ComPtr<ID3D10RenderTargetView> 	 m_RTView;
		ComPtr<ID3D10ShaderResourceView> m_SRView;
		ComPtr<ID3D10DepthStencilView>	 m_DSView;
	};

} // render
} // brUGE