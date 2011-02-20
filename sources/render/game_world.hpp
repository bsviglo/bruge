#pragma once

#include "prerequisites.h"
#include "utils/Data.hpp"
#include "math/AABB.hpp"
#include "math/Matrix4x4.hpp"
#include <vector>

namespace brUGE
{
	//-- forward declaration.
	namespace physic
	{
		struct PhysObj;
	}

	//-- Exists for every node of every model.
	//----------------------------------------------------------------------------------------------
	class Node
	{
	public:
		Node(const char* name, mat4f& matrix) : m_name(name), m_matrix(matrix) { }
		~Node() { }

		void		 matrix	(const mat4f& mat) { m_matrix = mat; }
		const mat4f& matrix	() const			  { return m_matrix; }
		const char*  name	() const			  { return m_name; }

	private:
		const char* m_name;
		mat4f&		m_matrix;

	private:
		//-- make non-copyable.
		Node(const Node&);
		Node& operator = (const Node&);
	};
	typedef std::vector<Node*> Nodes;


	//----------------------------------------------------------------------------------------------
	struct Transform
	{
		Transform();
		~Transform();

		AABB  m_localBounds;
		AABB  m_worldBounds;
		mat4f m_worldMat;
		Nodes m_nodes;
	};


	//-- The minimal point of the engines game objects subsystem.
	//----------------------------------------------------------------------------------------------
	class IGameObj
	{
	public:
		IGameObj() { }
		virtual ~IGameObj() { }

		//-- serialization functions.
		virtual bool load(const utils::ROData& inData, Handle objID, const mat4f* orient = NULL);
		virtual bool save(utils::WOData& outData);

		virtual void beginUpdate(float /*dt*/) { }
		virtual void preAnimUpdate() { }
		virtual void postAnimUpdate() { }
		virtual void endUpdate() { }

		//-- ToDo: remove.
		Handle animCtrl() { return m_animCtrl; }

	private:
		Handle				m_self;
		Handle				m_meshInst;
		Handle				m_animCtrl;
		physic::PhysObj*	m_physObj;
		//SoundData*		m_soundData;
		//ScriptData*		m_scriptData;
		//AIData*			m_aiData;
		Transform			m_transform;

	private:
		IGameObj(const IGameObj&);
		IGameObj& operator = (const IGameObj&);
	};


	//-- Represents the all game world. It manages game specific logic of all the game objects.
	//----------------------------------------------------------------------------------------------
	class GameWorld
	{
	public:
		bool		init();
		bool		fini();

		//-- load and save map.
		bool		loadMap(const char* mapName);
		bool		saveMap(const char* mapName);

		//-- add/delete some game objects to/from game world.
		Handle		addGameObj(const char* desc, const mat4f* orient = NULL);
		bool		delGameObj(Handle handle);
		IGameObj*	getGameObj(Handle handle) { return m_objs[handle]; }

		//-- update functions bucket.
		void		beginUpdate(float /*dt*/);
		void		preAnimUpdate();
		void		postAnimUpdate();
		void		endUpdate();

	private:
		typedef std::vector<IGameObj*> GameObjs;
		GameObjs m_objs;
	};

} //-- brUGE