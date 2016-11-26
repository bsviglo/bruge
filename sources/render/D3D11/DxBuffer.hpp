#pragma once

#include "Dx_common.hpp"
#include "render/IBuffer.h"

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	class DXBuffer : public IBuffer
	{
	public:
		DXBuffer(EType type, EUsage usage, ECPUAccess cpuAccess);
		virtual ~DXBuffer();

		//-- ToDo: take into account pitch and padding
		virtual void* doMap(EAccess flag);
		virtual void  doUnmap();
		
		bool init(const void* data, uint elemCount, uint elemSize);
		ID3D11Buffer* getBuffer() const { return m_buffer.get(); }
		ID3D11ShaderResourceView* getSRView() const { return m_SRView.get(); }
		
	private:
		ComPtr<ID3D11Buffer> m_buffer;
		ComPtr<ID3D11ShaderResourceView> m_SRView;
		ID3D11Resource* m_bufferResource;
	};

} // render
} // brUGE
