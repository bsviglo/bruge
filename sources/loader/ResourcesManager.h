#pragma once

#include "prerequisites.hpp"
#include "utils/Ptr.h"
#include "utils/Singleton.h"
#include "render/render_common.h"
#include "render/Mesh.hpp"
#include "render/Font.h"
#include "TextureLoader.h"
#include "ObjLoader.h"

#include <string>
#include <map>
#include <vector>


namespace SoundSys
{
	class SRManager;
}

namespace brUGE
{

	typedef Ptr<utils::ROData> RODataPtr;

	//----------------------------------------------------------------------------------------------
	class ResourcesManager : public utils::Singleton<ResourcesManager>, public NonCopyable
	{
	public:
		ResourcesManager();
		~ResourcesManager();

		bool init();

		//-- ToDo:
		//Ptr<render::Model>			loadModel  		(const char* name, bool loadCollision = true);
		
		//-- Note: the name of font calculated as fonts file name + underline + the size of the font.
		//-- e.g. verafont_8, this means that the name is 'verafont' and the size is 8.
		Ptr<render::Font>			loadFont   		(const char* name, uint size, const vec2ui& glyphsRange);
		Ptr<render::IShader>		loadShader 		(const char* name, const render::ShaderMacro* macros = NULL, uint macrosCount = 0);
		Ptr<render::ITexture>		loadTexture		(const char* name);
		Ptr<render::Mesh>			loadMesh   		(const char* name);
		Ptr<render::SkinnedMesh>	loadSkinnedMesh (const char* name);
		//Ptr<render::Animation>		loadAnimation	(const char* name);
		//Ptr<Sound>				loadSound  (const char* name);

		Ptr<utils::BSPTree>			loadBSP	   		(const char* name);
		Ptr<utils::BSPTree>			loadBSP	   		(const Ptr<render::Mesh>& mesh);
		bool						saveBSP	   		(const char* name, const Ptr<utils::BSPTree>& bsp);

		bool makeSharedShaderConstants(const char* name, const Ptr<render::IBuffer>& newBuffer);

	private:

		template<typename RES>
		class Cache
		{
		public:
			typedef std::map<std::string, Ptr<RES> > ResMap;

		public:
			void add(const char* name, const Ptr<RES>& res)
			{
				m_searchMap[name] = res;
			}
			void remove(const char* name)
			{
				ResMap::iterator iter = m_searchMap.find(name);
				if (iter != m_searchMap.end())
				{
					m_searchMap.erase(iter)
				}
			}
			Ptr<RES> find(const char* name)
			{
				ResMap::iterator iter = m_searchMap.find(name);
				if (iter != m_searchMap.end())
				{
					return iter->second;
				}
				else
				{
					return NULL;
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
		ObjLoader     						m_objMeshLoader;
		//SRManager	  						m_soundLoader;

		Cache<render::ITexture> 			m_texturesCache;
		Cache<render::IShader>  			m_shadersCache;
		Cache<render::Mesh>					m_meshesCache;
		Cache<render::SkinnedMesh>			m_skinnedMeshesCache;
		Cache<render::Font>					m_fontsCache;
		//Cache<render::Animation>			m_animationsCashe;
		Cache<utils::BSPTree>				m_bspsCache;
		//Cache<Sound>						m_soundesCache;
		
		std::string							m_resPath;
	};

} // brUGE
