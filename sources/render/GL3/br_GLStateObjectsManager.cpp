#include "br_GLStateObjectsManager.h"
#include "br_GLStateObjects.h"


namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	brGLStateObjectsManager::brGLStateObjectsManager()
	{

	}
	
	//------------------------------------------
	brGLStateObjectsManager::~brGLStateObjectsManager()
	{

	}
	
	//------------------------------------------
	Ptr<ibrDepthStencilState> brGLStateObjectsManager::createDepthStencilState(
		const ibrDepthStencilState::Desc_s& desc)
	{
		return new brGLDepthStencilState(desc);
	}
	
	//------------------------------------------
	Ptr<ibrRasterizerState> brGLStateObjectsManager::createRasterizedState(
		const ibrRasterizerState::Desc_s& desc)
	{
		return new brGLRasterizerState(desc);
	}
	
	//------------------------------------------
	Ptr<ibrBlendState> brGLStateObjectsManager::createBlendState(
		const ibrBlendState::Desc_s& desc)
	{
		return new brGLBlendState(desc);
	}

} // render
} // brUGE
