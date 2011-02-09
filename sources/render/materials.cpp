#include "materials.hpp"
#include "pugixml/pugixml.hpp"
#include "utils/Data.hpp"
#include "os/FileSystem.h"
#include "loader/ResourcesManager.h"

using namespace brUGE;
using namespace brUGE::os;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//-- xml file.
	const char* g_materialsDescXML = "resources/system/materials.xml";
}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.


namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	Materials::Materials()
	{

	}

	//----------------------------------------------------------------------------------------------
	Materials::~Materials()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool Materials::init()
	{
		//-- parse and load all available materials.
		RODataPtr data = FileSystem::instance().readFile(g_materialsDescXML);
		if (!data.get())
		{
			ERROR_MSG("Can't open %s file", g_materialsDescXML);
			return false;
		}

		pugi::xml_document doc;

		if (!doc.load_buffer(data->ptr(), data->length()))
		{
			ERROR_MSG("Can't parse XML file %s. Maybe it was corrupted.", g_materialsDescXML);
			return false;
		}

		//-- parse individual materials.
		for (auto mat =  doc.document_element().child("material"); mat; mat = mat.next_sibling("material"))
		{
			//-- material data.
			RenderMat renderMat;

			renderMat.m_isMultipass = mat.attribute("multipass").as_bool();
			renderMat.m_isBumpMaped = mat.attribute("bumpMaped").as_bool();
			renderMat.m_isSkinned   = mat.attribute("skinned").as_bool();

			const char* name   = mat.attribute("name").value();
			auto		params = mat.child("params");

			if (renderMat.m_isMultipass)
			{
				//-- z pre pass.
				if (auto param = params.attribute("pass0"))
				{
					renderMat.m_shaders[0] = rs().shaderContext()->getShader(param.value());
					assert(renderMat.m_shaders[0] != CONST_INVALID_HANDLE);
				}
				else
				{
					assert(0);
				}

				//-- shadow casting pass.
				/*
				if (auto param = params.attribute("pass1"))
				{
					renderMat.m_shaders[1] = rs().shaderContext()->getShader(param.value());
					assert(renderMat.m_shaders[1] != CONST_INVALID_HANDLE);
				}
				*/

				//-- main color pass.
				if (auto param = params.attribute("pass2"))
				{
					renderMat.m_shaders[2] = rs().shaderContext()->getShader(param.value());
					assert(renderMat.m_shaders[2] != CONST_INVALID_HANDLE);
				}
				else
				{
					assert(0);
				}

				//-- vertex declaration.
				if (auto param = params.attribute("vertex"))
				{
					renderMat.m_vertDecl = rs().shaderContext()->getVertexLayout(
						renderMat.m_shaders[2], param.value()
						);
					assert(renderMat.m_vertDecl != CONST_INVALID_HANDLE);
				}
				else
				{
					assert(0);
				}
			}
			else
			{
				//-- shader.
				if (auto param = params.attribute("pass0"))
				{
					renderMat.m_shaders[0] = rs().shaderContext()->getShader(param.value());
					assert(renderMat.m_shaders[0] != CONST_INVALID_HANDLE);
				}
				else
				{
					assert(0);
				}

				//-- vertex declaration.
				if (auto param = params.attribute("vertex"))
				{
					renderMat.m_vertDecl = rs().shaderContext()->getVertexLayout(
						renderMat.m_shaders[0], param.value()
						);
					assert(renderMat.m_vertDecl != CONST_INVALID_HANDLE);
				}
				else
				{
					assert(0);
				}
			}
			
			auto iter = m_materials.find(name);
			if (iter != m_materials.end())
			{
				assert(0);
			}
			else
			{
				m_materials[name] = renderMat;
			}
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool Materials::fini()
	{
		m_materials.clear();

		return true;
	}

	//----------------------------------------------------------------------------------------------
	Ptr<Material> Materials::createMaterial(const utils::ROData& data)
	{
		Ptr<Material> out(new Material());

		pugi::xml_document doc;
		if (!doc.load_buffer(data.ptr(), data.length()))
		{
			return false;
		}

		pugi::xml_node matNode = doc.document_element();

		//-- parse shader.
		if (auto name = matNode.attribute("name"))
		{
			auto iter = m_materials.find(name.value());
			if (iter != m_materials.end())
			{
				out->m_fx.m_shader = &iter->second;
			}
			else
			{
				assert(0);
			}
		}

		//-- parse shader properties.
		if (auto props = matNode.child("properties"))
		{
			for (pugi::xml_node prop = props.child("property"); prop; prop = prop.next_sibling("property"))
			{
				const char* type = prop.attribute("type").value();
				std::string name = prop.attribute("name").value();

				if (strcmp(type, "texture") == 0)
				{
					const char* texName = prop.attribute("value").value();
					Ptr<ITexture> tex = ResourcesManager::instance().loadTexture(texName);

					//-- firstly try to load system resources.
					if (name == "diffuseMap")
					{
						out->m_diffuseTex = tex;
						out->m_fx.m_sysProps.m_diffuseMap = tex.get();
					}
					else if (name == "bumpMap")
					{
						out->m_bumpTex = tex;
						out->m_fx.m_sysProps.m_bumpMap = tex.get();
					}
					else
					{
						out->m_props.push_back(new TextureProperty(name, tex));
					}
				}
				else
				{
					assert(0 && "another types currently are not implemented.");
				}
			}

			out->m_fx.m_props	   = (out->m_props.size() > 0) ? &out->m_props[0] : nullptr;
			out->m_fx.m_propsCount = out->m_props.size();
		}

		return out;
	}

	//----------------------------------------------------------------------------------------------
	Materials::RenderMat::RenderMat()
		: m_isSkinned(false), m_isBumpMaped(false), m_isMultipass(false), m_vertDecl(CONST_INVALID_HANDLE)
	{
		for (uint i = 0; i < ShaderContext::SHADER_RENDER_PASS_COUNT; ++i)
			m_shaders[i] = CONST_INVALID_HANDLE;
	}

	//----------------------------------------------------------------------------------------------
	Material::Material()
	{

	}

	//----------------------------------------------------------------------------------------------
	Material::~Material()
	{

	}

} //-- render
} //-- brUGE