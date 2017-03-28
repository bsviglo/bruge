#pragma once

#include "engine/IComponent.hpp"
#include "engine/ISystem.hpp"

#include <tuple>
#include <vector>
#include <unordered_map>

namespace brUGE
{
namespace render
{

	//------------------------------------------------------------------------------------------------------------------
	class CullingSystem : public ISystem
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		class VisibilitySet
		{
		private:
			std::unordered_map<ISystem::TypeID, std::vector<Handle>> m_buckets;
		};

	public:
		
		//--------------------------------------------------------------------------------------------------------------
		class World : public IWorld
		{
		private:
			std::vector<std::tuple<ISystem::TypeID, AABB, Handle>> m_objects;
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context : public IContext
		{
		public:

			//----------------------------------------------------------------------------------------------------------
			struct Config
			{
				bool			m_gatherAABB;
				RenderCamera	m_camera;
			};

		public:
			virtual bool	init(const Config& cfg) override;

			virtual void	update(IWorld* world, const DeltaTime& dt) const override;
			virtual void	process(IContext* context) const override;

		private:
			Config			m_cfg;
			VisibilitySet	m_visibilitySet;
		};

	public:


	private:
	};

}
}
