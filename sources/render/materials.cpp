#include "materials.hpp"
#include "utils/Data.hpp"
#include "os/FileSystem.h"
#include "loader/ResourcesManager.h"
#include "utils/ArgParser.h"
#include "utils/string_utils.h"
#include "post_processing.hpp"

//-- ToDo: reconsider.
#include "engine/Engine.h"
#include "render_world.hpp"

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
	ShaderContext::EPassType getPassType(const std::string& type)
	{
		if      (type == "Z_PRE_PASS")	return ShaderContext::PASS_Z_ONLY;
		else if (type == "SHADOW_CAST") return ShaderContext::PASS_SHADOW_CAST;
		else if (type == "MAIN_COLOR")	return ShaderContext::PASS_MAIN_COLOR;
		else							return ShaderContext::PASS_COUNT;
	}

	//----------------------------------------------------------------------------------------------
	SamplerStateDesc::ETexFilter getFilter(const std::string& type)
	{
		if		(type == "POINT")		return SamplerStateDesc::FILTER_NEAREST;
		else if	(type == "BILINEAR")	return SamplerStateDesc::FILTER_BILINEAR;
		else if (type == "TRINILEAR")	return SamplerStateDesc::FILTER_TRILINEAR;
		else if (type == "ANISO")		return SamplerStateDesc::FILTER_TRILINEAR_ANISO;
		else							return SamplerStateDesc::FILTER_NEAREST;
	}

	//----------------------------------------------------------------------------------------------
	SamplerStateDesc::ETexAddressMode getWrapping(const std::string& type)
	{
		if		(type == "CLAMP")	return SamplerStateDesc::ADRESS_MODE_CLAMP;
		else if (type == "WRAP")	return SamplerStateDesc::ADRESS_MODE_WRAP;
		else if (type == "MIRROR")	return SamplerStateDesc::ADRESS_MODE_MIRROR;
		else						return SamplerStateDesc::ADRESS_MODE_CLAMP;
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

		//-- parse each individual material.
		for (auto mat = doc.document_element().child("material"); mat; mat = mat.next_sibling("material"))
		{
			//-- material data.
			PipelineShader pipelineShader;

			const char* name  = mat.attribute("name").value();

			for (auto pass = mat.child("pass"); pass; pass = pass.next_sibling("pass"))
			{
				const char* type	  = pass.attribute("type").value();
				const char* vertexStr = pass.attribute("vertex").value();
				bool		bumped    = pass.attribute("bumped").as_bool();
				bool		skinned   = pass.attribute("skinned").as_bool();

				ShaderContext::EPassType passType = getPassType(type);
				if (passType == ShaderContext::PASS_COUNT)
				{
					ERROR_MSG("Undefined pass %s.", type);
					return false;
				}

				PipelineShader::Pass& shaderPass = pipelineShader.m_passes[passType];

				shaderPass.m_skinned = skinned;
				shaderPass.m_bumped  = bumped;

				for (auto shader = pass.child("shader"); shader; shader = shader.next_sibling("shader"))
				{
					std::string typeStr   = shader.attribute("type").value();
					const char* shaderStr = shader.attribute("src").value();
					const char* pinsStr	  = shader.attribute("pins").value();

					std::vector<std::string> vPins;
					getPins(vPins, pinsStr);

					Handle			shaderID = rs().shaderContext().getShader(shaderStr, &vPins);
					VertexLayoutID  vDclr	 = rs().shaderContext().getVertexLayout(vertexStr, shaderID);
					
					if (shaderID == CONST_INVALID_HANDLE || vDclr == CONST_INVALID_HANDLE)
					{
						ERROR_MSG("Can't create shader %s, or shader's input signature doesn't match"\
							"to vertex layout %s.", shaderStr, vertexStr
							);
						return false;
					}

					if		(typeStr == "NORMAL")		shaderPass.m_normal	   = shaderID;
					else if	(typeStr == "INSTANCED")	shaderPass.m_instanced = shaderID;
					else								{ assert(0); return false; }

					shaderPass.m_vertexDclr = vDclr;
				}
			}

			auto iter = m_pipelineShaders.find(name);
			if (iter != m_pipelineShaders.end())
			{
				ERROR_MSG("Duplication m_pipeline shader with name %s. The last one was ignored", name);
				return false;
			}
			else
			{
				m_pipelineShaders[name] = pipelineShader;
			}
		}
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

		auto		root = doc.document_element();
		std::string name = root.name();

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
	Ptr<Material> Materials::createMaterial(const pugi::xml_node& section, MaterialUI* oUI)
	{
		Ptr<Material> out(new Material());

		ShaderContext& sc = rs().shaderContext();

		const char* shaderStr = section.attribute("shader").value();
		const char* vertexStr = section.attribute("vertex").value();

		//-- create and load shader and vertex declaration.
		out->m_shader     = sc.getShader(shaderStr, nullptr);
		out->m_vertexDclr = sc.getVertexLayout(vertexStr, out->m_shader);

		if (out->m_shader == CONST_INVALID_HANDLE || out->m_vertexDclr == CONST_INVALID_HANDLE)
		{
			ERROR_MSG("Invalid material shader <%s> or vertex declaration <%s>.", shaderStr, vertexStr);
			return nullptr;
		}

		//-- retrieve materials properties section.
		auto propsSec = section.child("properties");
		auto rsSec	  = section.child("render_states");
		
		//-- 1. load material properties into named map.o
		if (!loadProps(out->m_propsMap, propsSec))
		{
			return nullptr;
		}

		//-- 2. load render state properties.
		if (!loadRenderStates(out->m_rsProps, rsSec))
		{
			return nullptr;
		}

		//-- 3. try to load UI for properties if needed.
		if (oUI && !loadUI(*oUI, out->m_propsMap, propsSec))
		{
			return nullptr;
		}
		
		//-- 4. gather properties pair (i.e. <Handle, IProperty*>) for shader.
		if (!gatherPropsForShader(out->m_props, out->m_propsMap, *sc.shader(out->m_shader), true))
		{
			return nullptr;
		}

		//-- create render FX.
		out->m_fx.m_shader	   = out->m_shader;
		out->m_fx.m_vertexDlcr = out->m_vertexDclr;
		out->m_fx.m_props	   = (out->m_props.size() > 0) ? &out->m_props[0] : nullptr;
		out->m_fx.m_propsCount = out->m_props.size();

		return out;
	}

	//----------------------------------------------------------------------------------------------
	Ptr<PipelineMaterial> Materials::createPipelineMaterial(const utils::ROData& data)
	{
		pugi::xml_document doc;
		if (!doc.load_buffer(data.ptr(), data.length()))
		{
			return nullptr;
		}
		return createPipelineMaterial(doc.document_element(), nullptr);
	}

	//----------------------------------------------------------------------------------------------
	Ptr<PipelineMaterial> Materials::createPipelineMaterial(const pugi::xml_node& section, MaterialUI* oUI)
	{
		Ptr<PipelineMaterial> out(new PipelineMaterial());

		ShaderContext& sc = rs().shaderContext();

		const char* pipelineShaderStr = section.attribute("use").value();

		auto result = m_pipelineShaders.find(pipelineShaderStr);
		if (result == m_pipelineShaders.end())
		{
			ERROR_MSG("Invalid pipeline shader <%s> used.", pipelineShaderStr);
			return nullptr;
		}
		out->m_shaders = &result->second;

		//-- retrieve materials properties section.
		auto propsSec = section.child("properties");
		auto rsSec	  = section.child("render_states");

		//-- 1. load material properties into named map.o
		if (!loadProps(out->m_propsMap, propsSec))
		{
			return nullptr;
		}

		//-- 2. load render state properties.
		if (!loadRenderStates(out->m_rsProps, rsSec))
		{
			return nullptr;
		}

		//-- 3. try to load UI for properties if needed.
		if (oUI && !loadUI(*oUI, out->m_propsMap, propsSec))
		{
			return nullptr;
		}

		//-- 4. gather properties pair (i.e. <Handle, IProperty*>) for each pass's shader.
		for (uint i = 0; i < ShaderContext::PASS_COUNT; ++i)
		{
			PipelineMaterial::Pass& mPass = out->m_passes[i];
			PipelineShader::Pass	sPass = out->m_shaders->m_passes[i];

			//-- 4.1. gather properties for normal shader.
			if (!gatherPropsForShader(mPass.m_normalProps, out->m_propsMap, *sc.shader(sPass.m_normal)))
			{
				return nullptr;
			}
			else
			{
				mPass.m_normalFx.m_shader	  = sPass.m_normal;
				mPass.m_normalFx.m_vertexDlcr = sPass.m_vertexDclr;
				mPass.m_normalFx.m_skinned    = sPass.m_skinned;
				mPass.m_normalFx.m_bumped	  = sPass.m_bumped;
				mPass.m_normalFx.m_props	  = (mPass.m_normalProps.size() > 0) ? &mPass.m_normalProps[0] : nullptr;
				mPass.m_normalFx.m_propsCount = mPass.m_normalProps.size();
				mPass.m_normalFx.m_rsProps	  = &out->m_rsProps;
			}

			//-- 4.2. gather properties for instanced shader if it's available.
			mPass.m_instanced = (sPass.m_instanced != CONST_INVALID_HANDLE);
			if (mPass.m_instanced)
			{
				if (!gatherPropsForShader(mPass.m_instancedProps, out->m_propsMap, *sc.shader(sPass.m_instanced)))
				{
					return nullptr;
				}
				else
				{
					mPass.m_instancedFx.m_shader	 = sPass.m_instanced;
					mPass.m_instancedFx.m_vertexDlcr = sPass.m_vertexDclr;
					mPass.m_instancedFx.m_skinned    = sPass.m_skinned;
					mPass.m_instancedFx.m_bumped	 = sPass.m_bumped;
					mPass.m_instancedFx.m_props	     = (mPass.m_instancedProps.size() > 0) ? &mPass.m_instancedProps[0] : nullptr;
					mPass.m_instancedFx.m_propsCount = mPass.m_instancedProps.size();
					mPass.m_instancedFx.m_rsProps	 = &out->m_rsProps;
				}
			}
		}
		return out;
	}

	//----------------------------------------------------------------------------------------------
	bool Materials::createPipelineMaterials(std::vector<Ptr<PipelineMaterial>>& out, const utils::ROData& data)
	{
		pugi::xml_document doc;
		if (!doc.load_buffer(data.ptr(), data.length()))
		{
			return false;
		}

		auto		root = doc.document_element();
		std::string name = root.name();

		if (name == "materials")
		{
			for (auto mat = root.child("material"); mat; mat = mat.next_sibling("material"))
			{
				if (Ptr<PipelineMaterial> m = createPipelineMaterial(mat, nullptr))
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
			if (Ptr<PipelineMaterial> m = createPipelineMaterial(root, nullptr))
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
	bool Materials::loadProps(PropertyMap& oPpropsMap, const pugi::xml_node& section)
	{
		for (auto prop = section.child("property"); prop; prop = prop.next_sibling("property"))
		{
			std::string type = prop.attribute("type").value();
			std::string name = prop.attribute("name").value();

			std::unique_ptr<IProperty> shaderProp;

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
					tex = Engine::instance().renderWorld().postProcessing().find(texName);

					//-- 2. ... then load it from resource manager.
					if (!tex)
					{
						tex = ResourcesManager::instance().loadTexture(texName);
					}

					//-- 3. ... raise error if we failed to load texture.
					if (!tex)
					{
						ERROR_MSG("Can't load texture <%s> or find render target.", texName);
						return false;
					}
				}

				SamplerStateDesc sDesc;
				SamplerStateID   stateS = CONST_INVALID_HANDLE;
				if (auto sampler = prop.child("sampler"))
				{
					//-- ToDo: reconsider.
					SamplerStateDesc::ETexAddressMode wrapMode   = getWrapping(sampler.attribute("wrapping").value());
					SamplerStateDesc::ETexFilter	  filterMode = getFilter(sampler.attribute("filter").value());

					sDesc.minMagFilter	= filterMode;
					sDesc.wrapR			= wrapMode;
					sDesc.wrapS			= wrapMode;
					sDesc.wrapT			= wrapMode;

					if (wrapMode == SamplerStateDesc::FILTER_TRILINEAR_ANISO)
					{
						sDesc.maxAnisotropy = 16;
					}
				}
				else
				{
					//-- ToDo: replace with system settings.
					sDesc.minMagFilter  = SamplerStateDesc::FILTER_TRILINEAR_ANISO;
					sDesc.maxAnisotropy = 16;
				}
				stateS = rd()->createSamplerState(sDesc);

				shaderProp.reset(new TextureProperty(tex, stateS));
			}
			else if (type == "float")
			{
				float val = prop.attribute("value").as_float();

				shaderProp.reset(new FloatProperty(val));
			}
			else if (type == "byte")
			{
				float val = prop.attribute("value").as_float() / 255.0f;

				shaderProp.reset(new FloatProperty(val));
			}
			else if (type == "vec4")
			{
				vec4f val = parseTo<vec4f>(prop.attribute("value").value());

				shaderProp.reset(new Vec4fProperty(val));
			}
			else
			{
				WARNING_MSG("Type <%s> currently is not implemented.", type.c_str());
				continue;
			}

			auto iter = oPpropsMap.find(name);
			if (iter != oPpropsMap.end())
			{
				WARNING_MSG("Property with name <%s> and type <%s> already exists. The last one will be ignored.",
					name.c_str(), type.c_str()
					);
			}
			else
			{
				oPpropsMap[name] = shaderProp.release();
			}
		}
		
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool Materials::loadRenderStates(RenderStateProperties& oRSProps, const pugi::xml_node& section)
	{
		for (auto state = section.child("state"); state; state = state.next_sibling("state"))
		{
			std::string name = state.attribute("name").value();

			if (name == "doubleSided")
			{
				oRSProps.m_doubleSided = state.attribute("value").as_bool();
			}
			else
			{
				WARNING_MSG("Render state <%s> is not implemented.", name.c_str());
				continue;
			}
		}
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool Materials::gatherPropsForShader(
		Properties& oProps, PropertyMap& oPropsMap,	const IShader& shader, bool warningIfNotExist)
	{
		for (auto iter = oPropsMap.begin(); iter != oPropsMap.end(); ++iter)
		{	
			const char* name   = iter->first.c_str();
			IProperty*  prop   = iter->second;
			Handle      handle = prop->handle(name, shader);

			if (warningIfNotExist && handle == CONST_INVALID_HANDLE)
			{
				WARNING_MSG("Property <%s> doesn't present in material.", name);
				continue;
			}
			
			oProps.push_back(PropertyPair(handle, prop));
		}
		return true;
	}

	//-- ToDo: reconsider. Try to find better way to do this task.
	//----------------------------------------------------------------------------------------------
	bool Materials::loadUI(MaterialUI& oUI, PropertyMap& propsMap, const pugi::xml_node& section)
	{
		for (auto prop = section.child("property"); prop; prop = prop.next_sibling("property"))
		{
			std::string pType = prop.attribute("type").value();
			std::string pName = prop.attribute("name").value();
			auto ui			  = prop.child("ui");


			if (pType == "float" || pType == "byte")
			{
				if (ui)
				{
					std::string uiType = ui.attribute("type").value();

					if (uiType == "slider")
					{
						MaterialUI::Slider slider;

						slider.first.m_name  = pName;
						slider.first.m_range = parseTo<vec2f>(ui.attribute("range").value());
						slider.first.m_step  = ui.attribute("step").as_float();
						slider.first.m_value = prop.attribute("value").as_float();
						slider.second		 = static_cast<FloatProperty*>(propsMap[pName]);

						oUI.m_sliders.push_back(slider);
					}
					else if (uiType == "checkbox")
					{
						MaterialUI::CheckBox checkBox;

						checkBox.first.m_name  = pName;
						checkBox.first.m_value = prop.attribute("value").as_float();
						checkBox.second		   = static_cast<FloatProperty*>(propsMap[pName]);

						oUI.m_checkBoxes.push_back(checkBox);
					}
					else
					{
						WARNING_MSG("UI type <%s> currently is not implemented for property type <%s>.",
							uiType.c_str(), pType.c_str()
							);
					}
				}
			}
			else if (pType == "texture")
			{
				if (ui)
				{
					std::string uiType = ui.attribute("type").value();

					if (uiType == "combobox")
					{
						MaterialUI::ComboBox comboBox;

						comboBox.first.m_name  = pName;
						comboBox.first.m_value = prop.attribute("value").value();
						comboBox.first.m_rts   = true;
						comboBox.second		   = static_cast<TextureProperty*>(propsMap[pName]);

						oUI.m_comboBoxes.push_back(comboBox);
					}
					else
					{
						WARNING_MSG("UI type <%s> currently is not implemented for property type <%s>.",
							uiType.c_str(), pType.c_str()
							);
					}
				}
			}
			else
			{
				if (ui)
				{
					std::string uiType = ui.attribute("type").value();

					WARNING_MSG("UI type <%s> currently is not implemented for property type <%s>.",
						uiType.c_str(), pType.c_str()
						);
				}
			}
		}
		return true;
	}

	//----------------------------------------------------------------------------------------------
	Material::Material()
	{

	}

	//----------------------------------------------------------------------------------------------
	Material::~Material()
	{
		for (auto iter = m_propsMap.begin(); iter != m_propsMap.end(); ++iter)
			delete iter->second;

		m_propsMap.clear();
	}

	//----------------------------------------------------------------------------------------------
	void Material::addProperty(const char* name, IProperty* prop)
	{
		auto iter = m_propsMap.find(name);
		if (iter == m_propsMap.end())
		{
			//-- add to named map.
			m_propsMap[name] = prop;

			Handle handle = prop->handle(name, *rs().shaderContext().shader(m_shader));
			if (handle != CONST_INVALID_HANDLE)
			{
				//-- add to properties pair list.
				m_props.push_back(PropertyPair(handle, prop));

				//-- update properties of renderFx.
				m_fx.m_props	  = (m_props.size() > 0) ? &m_props[0] : nullptr;
				m_fx.m_propsCount = m_props.size();
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	IProperty* Material::getProperty(const char* name)
	{
		auto iter = m_propsMap.find(name);
		if (iter != m_propsMap.end())
		{
			return iter->second;
		}
		return nullptr;
	}

	//----------------------------------------------------------------------------------------------
	PipelineMaterial::PipelineMaterial()
	{

	}

	//----------------------------------------------------------------------------------------------
	PipelineMaterial::~PipelineMaterial()
	{
		for (auto iter = m_propsMap.begin(); iter != m_propsMap.end(); ++iter)
			delete iter->second;

		m_propsMap.clear();
	}

	//----------------------------------------------------------------------------------------------
	const RenderFx* PipelineMaterial::renderFx(ShaderContext::EPassType pass, bool instanced)
	{
		const Pass& rPass = m_passes[pass];
		return instanced ? &rPass.m_instancedFx : &rPass.m_normalFx;
	}

	//----------------------------------------------------------------------------------------------
	void PipelineMaterial::addProperty(const char* name, IProperty* prop)
	{
		auto iter = m_propsMap.find(name);
		if (iter == m_propsMap.end())
		{
			//-- add to named map.
			m_propsMap[name] = prop;

			for (uint i = 0; i < ShaderContext::PASS_COUNT; ++i)
			{
				PipelineMaterial::Pass&			mPass = m_passes[i];
				Materials::PipelineShader::Pass	sPass = m_shaders->m_passes[i];
				
				//-- normal shader.
				Handle handle = prop->handle(name, *rs().shaderContext().shader(sPass.m_normal));
				if (handle != CONST_INVALID_HANDLE)
				{
					//-- add to properties pair list.
					mPass.m_normalProps.push_back(PropertyPair(handle, prop));

					//-- update properties of renderFx.
					mPass.m_normalFx.m_props	  = (mPass.m_normalProps.size() > 0) ? &mPass.m_normalProps[0] : nullptr;
					mPass.m_normalFx.m_propsCount = mPass.m_normalProps.size();
				}

				//-- instanced shader.
				if (mPass.m_instanced)
				{
					Handle handle = prop->handle(name, *rs().shaderContext().shader(sPass.m_instanced));
					if (handle != CONST_INVALID_HANDLE)
					{
						//-- add to properties pair list.
						mPass.m_instancedProps.push_back(PropertyPair(handle, prop));

						//-- update properties of renderFx.
						mPass.m_instancedFx.m_props	     = (mPass.m_instancedProps.size() > 0) ? &mPass.m_instancedProps[0] : nullptr;
						mPass.m_instancedFx.m_propsCount = mPass.m_instancedProps.size();
					}
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	IProperty* PipelineMaterial::getProperty(const char* name)
	{
		auto iter = m_propsMap.find(name);
		if (iter != m_propsMap.end())
		{
			return iter->second;
		}
		return nullptr;
	}

} //-- render
} //-- brUGE