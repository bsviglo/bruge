#include "materials.hpp"
#include "utils/Data.hpp"
#include "os/FileSystem.h"
#include "loader/ResourcesManager.h"
#include "utils/ArgParser.h"
#include "utils/string_utils.h"
#include "post_processing.hpp"

using namespace brUGE;
using namespace brUGE::os;
using namespace brUGE::utils;
using namespace brUGE::render;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//-- xml file.
	const char* g_materialsDescXML = "resources/system/materials.xml";

	//-- Returns vector of pins based on the incoming text representation.
	//----------------------------------------------------------------------------------------------
	uint getPins(std::vector<std::string>& out, const char* pins)
	{
		StrTokenizer tokenizer(pins, " |");
		std::string	token;

		while (tokenizer.hasMoreTokens())
		{
			tokenizer.nextToken(token);
			out.push_back(token);
		}
		return out.size();
	}

	//----------------------------------------------------------------------------------------------
	ShaderContext::EShaderRenderPassType getPassType(const std::string& type)
	{
		if      (type == "Z_PRE_PASS")	return ShaderContext::SHADER_RENDER_PASS_Z_ONLY;
		else if (type == "SHADOW_CAST") return ShaderContext::SHADER_RENDER_PASS_SHADOW_CAST;
		else if (type == "MAIN_COLOR")	return ShaderContext::SHADER_RENDER_PASS_MAIN_COLOR;
		else							return ShaderContext::SHADER_RENDER_PASS_COUNT;
	}

	//----------------------------------------------------------------------------------------------
	SamplerStateDesc::ETexFilter getFilter(const std::string& type)
	{
		if		(type == "POINT")		return SamplerStateDesc::FILTER_NEAREST;
		else if	(type == "BILINEAR")	return SamplerStateDesc::FILTER_BILINEAR;
		else if (type == "TRINILEAR")	return SamplerStateDesc::FILTER_TRILINEAR;
		else if (type == "ANISO")		return SamplerStateDesc::FILTER_TRILINEAR_ANISO;
		else							{ assert(0); return SamplerStateDesc::FILTER_NEAREST; }
	}

	//----------------------------------------------------------------------------------------------
	SamplerStateDesc::ETexAddressMode getWrapping(const std::string& type)
	{
		if		(type == "CLAMP")	return SamplerStateDesc::ADRESS_MODE_CLAMP;
		else if (type == "WRAP")	return SamplerStateDesc::ADRESS_MODE_WRAP;
		else if (type == "MIRROR")	return SamplerStateDesc::ADRESS_MODE_MIRROR;
		else						{ assert(0); return SamplerStateDesc::ADRESS_MODE_CLAMP; }
	}
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
		for (auto mat = doc.document_element().child("material"); mat; mat = mat.next_sibling("material"))
		{
			//-- material data.
			Ptr<MatShader> rMat(new MatShader);

			rMat->m_isBumpMaped = mat.attribute("bumped").as_bool();
			rMat->m_isSkinned   = mat.attribute("skinned").as_bool();
			rMat->m_isMultipass = true;
			const char* name   = mat.attribute("name").value();

			for (auto pass = mat.child("pass"); pass; pass = pass.next_sibling("pass"))
			{
				const char* type	  = pass.attribute("type").value();
				const char* vertexStr = pass.attribute("vertex").value();
				const char* shaderStr = pass.attribute("shader").value();
				const char* pinsStr   = pass.attribute("pins").value();

				ShaderContext::EShaderRenderPassType passType = getPassType(type);
				if (passType == ShaderContext::SHADER_RENDER_PASS_COUNT)
				{
					ERROR_MSG("Undefined pass %s.", type);
					return false;
				}

				std::pair<Handle, Handle>& shader = rMat->m_shader[passType];
				//-- ToDo: more checking.

				std::vector<std::string> vPins;
				getPins(vPins, pinsStr);

				shader.first  = rs().shaderContext()->getShader(shaderStr, &vPins);
				shader.second = rs().shaderContext()->getVertexLayout(shader.first, vertexStr);
				
				if (shader.first == CONST_INVALID_HANDLE || shader.second == CONST_INVALID_HANDLE)
				{
					ERROR_MSG("Can't create shader %s, or shader's input signature doesn't match"\
						"to vertex layout %s.", shaderStr, vertexStr
						);
					return false;
				}
			}

			auto iter = m_materials.find(name);
			if (iter != m_materials.end())
			{
				ERROR_MSG("Duplication material with name %s. The last one was ignored", name);
				return false;
			}
			else
			{
				m_materials[name] = rMat;
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
		pugi::xml_document doc;
		if (!doc.load_buffer(data.ptr(), data.length()))
		{
			return nullptr;
		}
		return createMaterial(doc.document_element(), nullptr);
	}

	//----------------------------------------------------------------------------------------------
	bool Materials::createMaterials(std::vector<Ptr<Material>>& out, const utils::ROData& data)
	{
		pugi::xml_document doc;
		if (!doc.load_buffer(data.ptr(), data.length()))
		{
			return false;
		}

		pugi::xml_node root = doc.document_element();
		std::string name	= root.name();

		if (name == "materials")
		{
			for (auto mat = root.child("material"); mat; mat = mat.next_sibling("material"))
			{
				if (Ptr<Material> m = createMaterial(mat, nullptr))
				{
					out.push_back(m);
				}
				else
				{
					return false;
				}
			}
		}
		else if (name == "material")
		{
			if (Ptr<Material> m = createMaterial(root, nullptr))
			{
				out.push_back(m);
			}
			else
			{
				return false;
			}
		}
		else
		{
			assert(0);
			return false;
		}
	
		return true;
	}

	//----------------------------------------------------------------------------------------------
	Ptr<Material> Materials::createMaterial(const pugi::xml_node& section, MaterialUI* uiDesc)
	{
		Ptr<Material> out(new Material());

		//-- parse shader.
		if (auto param = section.attribute("use"))
		{
			auto iter = m_materials.find(param.value());
			if (iter != m_materials.end())
			{
				out->m_matShader = iter->second;
				out->m_fx.m_shader = iter->second.get();
			}
			else
			{
				ERROR_MSG("Material with name %s doesn't exist.", param.value());
				return nullptr;
			}
		}
		else
		{
			ShaderContext& sc = *rs().shaderContext();

			const char* shader = section.attribute("shader").value();
			const char* vertex = section.attribute("vertex").value();

			Ptr<Materials::MatShader> matShader(new Materials::MatShader);

			matShader->m_shader[0].first  = sc.getShader(shader, nullptr);
			matShader->m_shader[0].second = sc.getVertexLayout(matShader->m_shader[0].first, vertex);

			if (matShader->m_shader[0].first  == CONST_INVALID_HANDLE ||
				matShader->m_shader[0].second == CONST_INVALID_HANDLE)
			{
				ERROR_MSG("Invalid material shader or vertex declaration.");
				return nullptr;
			}

			out->m_matShader = matShader;
			out->m_fx.m_shader = matShader.get();
		}


		//-- parse shader properties.
		if (auto props = section.child("properties"))
		{
			for (auto prop = props.child("property"); prop; prop = prop.next_sibling("property"))
			{
				std::string type = prop.attribute("type").value();
				std::string name = prop.attribute("name").value();

				if (type == "texture")
				{
					const char* texName = prop.attribute("value").value();

					//-- ToDo: reconsider. Probably needed creating some texture manager, which
					//--	   can generalize idea behind texture loading. I.e. it gives us
					//--	   opportunities to interpret render targets from post-processing
					//--	   framework as well as another common textures, which are implicitly
					//--	   loaded from the hard disk.
					Ptr<ITexture> tex;
					{
						//-- 1. first search texture among post-processing's render targets.
						tex = rs().postProcessing()->find(texName);

						//-- 2. ... then load it from resource manager.h
						if (!tex)
						{
							tex = ResourcesManager::instance().loadTexture(texName);
						}

						//-- 3. ... raise error if we failed to load texture.
						if (!tex)
						{
							ERROR_MSG("Can't load texture %s or find render target.", texName);
							return nullptr;
						}
					}

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
						SamplerStateID stateS = CONST_INVALID_HANDLE;
						if (auto sampler = prop.child("sampler"))
						{
							SamplerStateDesc::ETexAddressMode wrapMode   = getWrapping(sampler.attribute("wrapping").value());
							SamplerStateDesc::ETexFilter	  filterMode = getFilter(sampler.attribute("filter").value());

							SamplerStateDesc sDesc;
							sDesc.minMagFilter	= filterMode;
							sDesc.wrapR			= wrapMode;
							sDesc.wrapS			= wrapMode;
							sDesc.wrapT			= wrapMode;

							if (wrapMode == SamplerStateDesc::FILTER_TRILINEAR_ANISO)
							{
								sDesc.maxAnisotropy = 16;
							}

							stateS = rd()->createSamplerState(sDesc);
						}
						else
						{
							SamplerStateDesc sDesc;
							stateS = rd()->createSamplerState(sDesc);
						}

						out->m_props.push_back(new TextureProperty(name, tex, stateS));
					}
				}
				else if (type == "float")
				{
					float val = prop.attribute("value").as_float();

					//-- firstly try to load system resources.
					if (name == "alphaRef")
					{
						out->m_alphaRef = val / 256.0f;
						out->m_fx.m_sysProps.m_alphaRef = val / 256.0f;
						continue;
					}
					else if (name == "doubleSided")
					{
						out->m_doubleSided = static_cast<bool>(val);
						out->m_fx.m_sysProps.m_doubleSided = static_cast<bool>(val);
						continue;
					}
					
					NumericProperty<float>* shaderProp = new NumericProperty<float>(name, val);

					auto ui = prop.child("ui");
					if (uiDesc && ui)
					{
						std::string uiType = ui.attribute("type").value();
						
						if (uiType == "slider")
						{
							MaterialUI::Slider slider;

							slider.first.m_name  = name;
							slider.first.m_range = parseTo<vec2f>(ui.attribute("range").value());
							slider.first.m_step  = ui.attribute("step").as_float();
							slider.first.m_value = val;

							slider.second = shaderProp;

							uiDesc->m_sliders.push_back(slider);
						}
						else if (uiType == "checkbox")
						{
							MaterialUI::CheckBox checkBox;

							checkBox.first.m_name  = name;
							checkBox.first.m_value = static_cast<bool>(val);

							checkBox.second = shaderProp;

							uiDesc->m_checkBoxes.push_back(checkBox);
						}
						else
						{
							assert(!"another ui types currently are not implemented.");
							return nullptr;
						}
					}

					out->m_props.push_back(shaderProp);
				}
				else if (type == "vec4")
				{
					vec4f val = parseTo<vec4f>(prop.attribute("value").value());
					NumericProperty<vec4f>* shaderProp = new NumericProperty<vec4f>(name, val);
					out->m_props.push_back(shaderProp);
				}
				else
				{
					assert(!"another types currently are not implemented.");
					return nullptr;
				}
			}

			out->m_fx.m_props	   = (out->m_props.size() > 0) ? &out->m_props[0] : nullptr;
			out->m_fx.m_propsCount = out->m_props.size();
		}

		return out;
	}

	//----------------------------------------------------------------------------------------------
	Materials::MatShader::MatShader()
		: m_isSkinned(false), m_isBumpMaped(false), m_isMultipass(false)
	{
		for (uint i = 0; i < ShaderContext::SHADER_RENDER_PASS_COUNT; ++i)
			m_shader[i] = std::make_pair(CONST_INVALID_HANDLE, CONST_INVALID_HANDLE);
	}

	//----------------------------------------------------------------------------------------------
	Materials::MatShader::~MatShader()
	{

	}

	//----------------------------------------------------------------------------------------------
	Material::Material()
	{

	}

	//----------------------------------------------------------------------------------------------
	Material::~Material()
	{
		for (uint i = 0; i < m_props.size(); ++i)
			delete m_props[i];

		m_props.clear();
	}

	//----------------------------------------------------------------------------------------------
	void Material::addProperty(IProperty* prop)
	{
		m_props.push_back(prop);

		m_fx.m_props	  = (m_props.size() > 0) ? &m_props[0] : nullptr;
		m_fx.m_propsCount = m_props.size();
	}

} //-- render
} //-- brUGE