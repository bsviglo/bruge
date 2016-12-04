#include "ResourcesManager.h"
#include "render/ITexture.h"
#include "render/IShader.h"
#include "render/render_system.hpp"
#include "render/IRenderDevice.h"
#include "os/FileSystem.h"
#include "utils/string_utils.h"

using namespace brUGE::render;
using namespace brUGE::os;
using namespace brUGE::utils;

namespace brUGE
{	
	DEFINE_SINGLETON(ResourcesManager);
	
	//----------------------------------------------------------------------------------------------
	ResourcesManager::ResourcesManager()
	{
		// ToDo:
		m_resPath = "resources/";
	}

	//----------------------------------------------------------------------------------------------
	ResourcesManager::~ResourcesManager()
	{ 
		m_texLoader.shutdown();

		//-- caches automatically are cleared here.
	}
	
	//----------------------------------------------------------------------------------------------
	bool ResourcesManager::init()
	{
		bool success = true;
		success &= m_texLoader.init();

		return success;
	}

	//----------------------------------------------------------------------------------------------
	std::shared_ptr<ITexture> ResourcesManager::loadTexture(const char* name)
	{
		auto result = m_texturesCache.find(name);
		if (!result)
		{
			FileSystem& fs = os::FileSystem::instance();

			RODataPtr data = fs.readFile(m_resPath + std::string(name));
			if (!data.get())
			{
				return NULL;
			}

			result = m_texLoader.loadTex2D(*data);
			if (result)
			{
				m_texturesCache.add(name, result);
			}
		}
		return result;
	}

	//----------------------------------------------------------------------------------------------
	std::shared_ptr<IShader> ResourcesManager::loadShader(const char* name, const ShaderMacro* macros, uint macrosCount)
	{
		auto result = m_shadersCache.find(name);
		if (!result)
		{
			FileSystem&   fs = FileSystem::instance();
			RenderSystem& rs = RenderSystem::instance();

			RODataPtr data = fs.readFile(makeStr("%sshaders/%s.%s",
				m_resPath.c_str(), name, (rs.gapi() == RENDER_API_GL3) ? "glsl" : "hlsl"
				).c_str()
				);
			if (!data.get())
			{
				return NULL;
			}
			
			// ToDo: reconsider.
			std::string src;
			data->getAsString(src);
			
			result = rd()->createShader(src.c_str(), macros, macrosCount);
			if (result)
			{
				m_shadersCache.add(name, result);
			}
		}
		return result;
	}

	//----------------------------------------------------------------------------------------------
	std::shared_ptr<Mesh> ResourcesManager::loadMesh(const char* name, bool /*simpleMaterial*/)
	{
		std::string meshName = FileSystem::getFileWithoutExt(name);

		auto result = m_meshesCache.find(meshName.c_str());
		if (!result)
		{
			RODataPtr data = FileSystem::instance().readFile(m_resPath + name);
			if (!data.get())
			{
				return NULL;
			}
		
			result = std::make_shared<Mesh>();
			if (result->load(*data, meshName))
			{
				m_meshesCache.add(meshName.c_str(), result);
			}
			else
			{
				result.reset();
			}
		}
		return result;
	}

	//----------------------------------------------------------------------------------------------
	std::shared_ptr<SkinnedMesh> ResourcesManager::loadSkinnedMesh(const char* name)
	{
		std::string meshName = FileSystem::getFileWithoutExt(name);

		auto result = m_skinnedMeshesCache.find(meshName.c_str());
		if (!result)
		{
			RODataPtr data = FileSystem::instance().readFile(m_resPath + name);
			if (!data.get())
			{
				return NULL;
			}

			result = std::make_shared<SkinnedMesh>();
			if (!result->load(*data, meshName))
			{
				m_skinnedMeshesCache.add(meshName.c_str(), result);
			}
		}
		return result;
	}

/*
	//----------------------------------------------------------------------------------------------
	Ptr<Model> ResourcesManager::loadModel(const char* name, bool loadCollision)
	{
		//-- 1. load skinned model.
		if (getFileExt(name) == "md5mesh")
		{
			Ptr<SkinnedMesh> mesh = loadSkinnedMesh(name);
			if (!mesh)
			{
				return NULL;
			}
			
			//-- ToDo:
			return new SkinnedModel(mesh, NULL);
		}
		//-- 2. load static model.
		else
		{
			Ptr<Mesh> mesh = loadMesh(name);
			if (!mesh)
			{
				return NULL;
			}

			Ptr<BSPTree> bsp = NULL;

			if (loadCollision)
			{
				//-- try to load bsp from file.
				bsp = loadBSP(name);
				if (!bsp)
				{
					//-- if is not success, compute it direct from the mesh and then save on the disk.
					bsp = loadBSP(mesh);
					saveBSP(name, bsp);
				}
			}

			//-- Now, create new one model object.
			return new StaticModel(mesh, bsp);
		}
	}
*/

/*
	//----------------------------------------------------------------------------------------------
	Ptr<Animation> ResourcesManager::loadAnimation(const char* name)
	{
		Ptr<Animation> result = m_animationsCashe.find(name);
		if (!result.isValid())
		{
			FileSystem& fs = FileSystem::instance();

			RODataPtr data = fs.readFile(m_resPath + std::string(name));
			if (!data.get())
			{
				return NULL;
			}

			result = new Animation();
			if (result->load(*data))
			{
				m_animationsCashe.add(name, result);
			}
			else
			{
				result = NULL;
			}
		}
		return result;

	}
*/

	//----------------------------------------------------------------------------------------------
	bool ResourcesManager::makeSharedShaderConstants(const char* name, const std::shared_ptr<IBuffer>& newBuffer)
	{
		bool result = false;

		for (const auto& item : m_shadersCache)
		{
			result |= item.second->changeUniformBuffer(name, newBuffer);
		}

		return result;
	}

} // brUGE