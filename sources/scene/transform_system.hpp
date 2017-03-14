#pragma once

#include "prerequisites.hpp"
#include "engine/ISystem.hpp"
#include "utils/Data.hpp"
#include "math/AABB.hpp"
#include "math/Matrix4x4.hpp"
#include "SDL/SDL_events.h"
#include <vector>

namespace brUGE
{
	//-- Exists for every node of every model.
	//------------------------------------------------------------------------------------------------------------------
	class Node : public NonCopyable
	{
	public:
		Node(const char* name, mat4f& matrix) : m_name(name), m_matrix(matrix) { }
		~Node() { }

		void		 matrix	(const mat4f& mat)	{ m_matrix = mat; }
		const mat4f& matrix	() const			{ return m_matrix; }
		const char*  name	() const			{ return m_name; }

	private:
		const char* m_name;
		mat4f&		m_matrix;
	};
	typedef std::vector<std::unique_ptr<Node>> Nodes;


	//------------------------------------------------------------------------------------------------------------------
	struct Transform : public NonCopyable
	{
		Transform();
		~Transform();

		AABB  m_localBounds;
		AABB  m_worldBounds;
		mat4f m_worldMat;
		Nodes m_nodes;
	};

	//------------------------------------------------------------------------------------------------------------------
	class ISystemComponent : public IComponent
	{

	};


	//------------------------------------------------------------------------------------------------------------------
	class TransformComponent : public IComponent
	{
	public:
		TransformComponent() : IComponent(IComponent::TYPE_TRANSFORM) { }
		virtual ~TransformComponent() override { }

	private:
	};

	//------------------------------------------------------------------------------------------------------------------
	class TransformSystem : public ISystem
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		class World : public ISystem::IWorld
		{
		public:
			World() { }
			virtual ~World() override { }

			virtual bool	init() override;

			virtual void	activate() override;
			virtual void	deactivate() override;

			virtual Handle	createComponent() override;
			virtual Handle	cloneComponent(Handle id) override;
			virtual bool	removeComponent(Handle id) override;

			virtual bool	registerGameObject(Handle entity) override;
			virtual bool	unregisterGameObject(Handle entity) override;

		private:
			std::vector<std::unique_ptr<TransformComponent>>	m_components;
			std::vector<Transform>								m_transforms;
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context : public ISystem::IContext
		{
		};

	public:
	
	private:
	};
}