#pragma once

#include "prerequisites.h"
#include "render_common.h"
#include "shader_context.hpp"
#include "utils/Data.hpp"
#include "pugixml/pugixml.hpp"

#include <vector>
#include <map>

namespace brUGE
{
namespace render
{

	class  Material;
	struct MaterialUI;

	//-- Presents data holder of the all materials in the engine. At the beginning of the game all
	//-- needed materials descriptions will be read from the materials.xml file and then used
	//-- to formulate desired material.
	//----------------------------------------------------------------------------------------------
	class Materials : public NonCopyable
	{
	public:

		struct MatShader : public RefCount, public NonCopyable
		{
			MatShader();
			~MatShader();

			bool m_isSkinned;
			bool m_isBumpMaped;
			bool m_isMultipass;

			std::pair<Handle, Handle> m_shader[ShaderContext::SHADER_RENDER_PASS_COUNT];
		};

	public:
		Materials();
		~Materials();

		bool init();
		bool fini();

		Ptr<Material>	createMaterial (const utils::ROData& data);
		Ptr<Material>	createMaterial (const pugi::xml_node& section, MaterialUI* uiDesc);
		bool			createMaterials(std::vector<Ptr<Material>>& out, const utils::ROData& data);

	private:
		typedef std::map<std::string, Ptr<MatShader>> MaterialsMap;

		MaterialsMap m_materials;
	};


	//-- Represents material of the given geometry. It represents only the render vision about
	//-- the material. I.e. render doesn't worry about any kind of the memory management here.
	//-- ToDo: need some reconsideration in the near future.
	//----------------------------------------------------------------------------------------------
	struct RenderFx : public NonCopyable
	{
	public:
		RenderFx()
			:	m_shader(nullptr), m_props(nullptr), m_propsCount(0) { }

		struct SysPropertiesData
		{
			SysPropertiesData()
				:	m_bumpMap(nullptr), m_diffuseMap(nullptr),
					m_alphaRef(1), m_doubleSided(false) { }

			ITexture*	m_bumpMap;
			ITexture*	m_diffuseMap;
			float		m_alphaRef;
			bool		m_doubleSided;
		};

		//-- shader(-s) and vertex declaration.
		Materials::MatShader* m_shader;

		//-- user defined properties.
		IProperty**			  m_props;
		uint				  m_propsCount;

		//-- system auto properties may are used for more then one render pass.
		SysPropertiesData	  m_sysProps;
	};


	//-- Presents render system vision about the material. I.e. it manages the all things
	//-- related to the resources management and loading.
	//----------------------------------------------------------------------------------------------
	class Material : public RefCount, public NonCopyable
	{
		friend class Materials;

	public:
		Material();
		~Material();

		//-- Note: when we add new property to the material we also give it ownership for this
		//--	   property.
		void			addProperty(IProperty* prop);
		const RenderFx* renderFx() const { return &m_fx; }

	private:
		RenderFx					m_fx;
		Properties					m_props;
		Ptr<Materials::MatShader>	m_matShader;

		//-- system resources are shared between more than one render pass.
		Ptr<ITexture>	m_diffuseTex;
		Ptr<ITexture>	m_bumpTex;
		float			m_alphaRef;
		bool			m_doubleSided;
	};


	//-- UI presentation about some shader properties. This interface gives opportunities to modify
	//-- some shader constants directly from the GUI interface if it exists. For example it's
	//-- actively used in the post-processing framework to modify effect's parameters.
	//----------------------------------------------------------------------------------------------
	struct MaterialUI
	{
		struct SliderDesc
		{
			SliderDesc() : m_step(0), m_value(0) { }

			std::string m_name;
			vec2f		m_range;
			float		m_step;
			float		m_value;
		};

		struct CheckBoxDesc
		{
			CheckBoxDesc() : m_value(0) { }

			std::string m_name;
			bool		m_value;
		};

		typedef std::pair<SliderDesc,   NumericProperty<float>*> Slider;
		typedef std::pair<CheckBoxDesc, NumericProperty<float>*> CheckBox;

		std::vector<Slider>   m_sliders;
		std::vector<CheckBox> m_checkBoxes;
	};

} //-- render
} //-- brUGE