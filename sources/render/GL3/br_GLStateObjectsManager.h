#ifndef _BR_GLSTATEOBJECTSMANAGER_H_
#define _BR_GLSTATEOBJECTSMANAGER_H_

#include "br_OGLCommon.h"
#include "render/ibr_StateObjectsManager.h"

namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	class brGLStateObjectsManager : public ibrStateObjectsManager
	{
	public:
		brGLStateObjectsManager();
		virtual ~brGLStateObjectsManager();

		virtual Ptr<ibrDepthStencilState> createDepthStencilState(
			const ibrDepthStencilState::Desc_s& desc
			);

		virtual Ptr<ibrRasterizerState> createRasterizedState(
			const ibrRasterizerState::Desc_s& desc
			);

		virtual Ptr<ibrBlendState> createBlendState(
			const ibrBlendState::Desc_s& desc
			);
	};

} // render
} // brUGE

#endif /*_BR_GLSTATEOBJECTSMANAGER_H_*/