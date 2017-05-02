#pragma once

#include "prerequisites.hpp"
#include "engine/ISystem.hpp"
#include "render/render_common.h"
#include "render/Mesh.hpp"
#include "animation/animation.hpp"
#include "TextureLoader.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace brUGE
{

	//-- Manages resources in the system.
	//-- Every resource kind (texture, mesh, animation, sound, etc.) should support "resource not found" place holder
	//-- to be not crash the application in case of missing resource. Only in case if system place holder is also missing
	//-- application may report an error and exit.
	//------------------------------------------------------------------------------------------------------------------
	class ResourceSystem : public ISystem
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		class World : public IWorld
		{
		public:
			std::shared_ptr<render::IShader>		loadShader 		(const char* name, const render::ShaderMacro* macros = nullptr, uint macrosCount = 0);
			std::shared_ptr<render::ITexture>		loadTexture		(const char* name);
			std::shared_ptr<render::Mesh>			loadMesh   		(const char* name, bool simpleMaterial = false);
			std::shared_ptr<render::SkinnedMesh>	loadSkinnedMesh (const char* name);
			std::shared_ptr<Animation>				loadAnimation	(const char* name);

			//-- ToDo: to be removed
			bool makeSharedShaderConstants(const char* name, const std::shared_ptr<render::IBuffer>& newBuffer);

		private:
			std::string												m_resPath;
			TextureLoader 											m_texLoader;

			std::unordered_map<std::string, render::ITexture> 		m_texturesCache;
			std::unordered_map<std::string, render::IShader>  		m_shadersCache;
			std::unordered_map<std::string, render::Mesh>			m_meshesCache;
			std::unordered_map<std::string, render::SkinnedMesh>	m_skinnedMeshesCache;
			std::unordered_map<std::string, Animation>				m_animationsCashe;
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context : public IContext
		{

		};


	public:

		static TypeID	typeID() { return m_typeID; }

	private:

		static const TypeID m_typeID;
	};
		

	//-- ToDo: Legacy...



	//----------------------------------------------------------------------------------------------
	class ResourcesManager : public utils::Singleton<ResourcesManager>
	{
	public:
		ResourcesManager();
		~ResourcesManager();

		bool init();

		//--
		//std::shared_ptr<render::Model>		loadModel  		(const char* name, bool loadCollision = true);
		std::shared_ptr<render::IShader>		loadShader 		(const char* name, const render::ShaderMacro* macros = NULL, uint macrosCount = 0);
		std::shared_ptr<render::ITexture>		loadTexture		(const char* name);
		std::shared_ptr<render::Mesh>			loadMesh   		(const char* name, bool simpleMaterial = false);
		std::shared_ptr<render::SkinnedMesh>	loadSkinnedMesh (const char* name);
		//std::shared_ptr<render::Animation>	loadAnimation	(const char* name);
		//std::shared_ptr<Sound>				loadSound  (const char* name);

		bool makeSharedShaderConstants(const char* name, const std::shared_ptr<render::IBuffer>& newBuffer);

	private:

		template<typename RES>
		class Cache
		{
		public:
			typedef std::map<std::string, std::shared_ptr<RES> > ResMap;

		public:
			void add(const char* name, const std::shared_ptr<RES>& res)
			{
				m_searchMap[name] = res;
			}
			void remove(const char* name)
			{
				auto iter = m_searchMap.find(name);
				if (iter != m_searchMap.end())
				{
					m_searchMap.erase(iter)
				}
			}
			std::shared_ptr<RES> find(const char* name)
			{
				auto iter = m_searchMap.find(name);
				if (iter != m_searchMap.end())
				{
					return iter->second;
				}
				else
				{
					return nullptr;
				}
			}
			void clear()
			{
				m_searchMap.clear();
			}
			bool empty()
			{
				return m_searchMap.empty();
			}
			typename ResMap::iterator begin()
			{
				return m_searchMap.begin();
			}
			typename ResMap::iterator end()
			{
				return m_searchMap.end();
			}

		private:
			ResMap m_searchMap;
		};

		TextureLoader 						m_texLoader;
		//SRManager	  						m_soundLoader;

		Cache<render::ITexture> 			m_texturesCache;
		Cache<render::IShader>  			m_shadersCache;
		Cache<render::Mesh>					m_meshesCache;
		Cache<render::SkinnedMesh>			m_skinnedMeshesCache;
		//Cache<render::Animation>			m_animationsCashe;
		//Cache<Sound>						m_soundesCache;
		
		std::string							m_resPath;
	};

} // brUGE
