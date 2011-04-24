#pragma once

#include "prerequisites.hpp"
#include "render_common.h"

namespace brUGE
{
namespace render
{
	class  Mesh;
	struct RenderOp;
	typedef std::vector<RenderOp> RenderOps;

	//-- Sky box.
	//-- ToDo: document.
	//----------------------------------------------------------------------------------------------
	class SkyBox : public NonCopyable
	{
	public:
		SkyBox();
		~SkyBox();

		bool init();
		uint gatherROPs(RenderOps& rops) const;

	private:
		Ptr<Mesh> m_skybox;
	};

} //-- render
} //-- brUGE