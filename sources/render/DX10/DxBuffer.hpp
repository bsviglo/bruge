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

		virtual void* doMap(EAccess flag);
		virtual void  doUnmap();
		
		bool init(const void* data, uint elemCount, uint elemSize);
		ID3D10Buffer* getBuffer() const { return m_buffer.get(); }
		ID3D10ShaderResourceView* getSRView() const { return m_SRView.get(); }
		
	private:
		ComPtr<ID3D10Buffer> m_buffer;
		ComPtr<ID3D10ShaderResourceView> m_SRView;
	};

} // render
} // brUGE
