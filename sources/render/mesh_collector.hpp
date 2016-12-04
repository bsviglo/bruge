#pragma once

#include "prerequisites.hpp"
#include "render_common.h"
#include "render_system.hpp"

namespace brUGE
{
namespace render
{

	struct MeshInstance;
	class  Mesh;
	struct RenderOp;
	typedef std::vector<RenderOp> RenderOps;

	//-- It's responsible for mesh instancing.
	//----------------------------------------------------------------------------------------------
	class MeshCollector : public NonCopyable
	{
	public:
		MeshCollector();
		~MeshCollector();

		bool init();
		void begin(RenderSystem::EPassType pass);
		bool addMeshInstance(const MeshInstance& instance);
		void end();
		uint gatherROPs(RenderOps& rops);

	private:

		//------------------------------------------------------------------------------------------
		struct Instance
		{
			std::shared_ptr<Mesh>	m_mesh;
			std::vector<mat4f>		m_buffer;
		};

		std::vector<Instance>		m_instances;
		RenderSystem::EPassType		m_pass;
		std::shared_ptr<IBuffer>	m_instanceTB;
	};

} //-- render
} //-- brUGE