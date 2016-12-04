#pragma once

#include "prerequisites.hpp"
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
	class  PipelineMaterial;
	struct MaterialUI;

	//-- Presents data holder of the all materials in the engine. At the beginning of the game all
	//-- needed materials descriptions will be read from the materials.xml file and then used
	//-- to formulate desired material.
	//----------------------------------------------------------------------------------------------
	class Materials : public NonCopyable
	{
	public:

		//------------------------------------------------------------------------------------------
		struct PipelineShader
		{
			struct Pass
			{
				Pass()
					:	m_skinned(false), m_bumped(false), m_normal(CONST_INVALID_HANDLE),
						m_instanced(CONST_INVALID_HANDLE), m_vertexDclr(CONST_INVALID_HANDLE) { }

				bool			m_skinned;
				bool			m_bumped;
				Handle			m_normal;
				Handle			m_instanced;
				VertexLayoutID	m_vertexDclr;
			};

			Pass m_passes[ShaderContext::PASS_COUNT];
		};

		typedef std::map<std::string, IProperty*> PropertyMap;

	public:
		Materials();
		~Materials();

		bool								init();

		std::shared_ptr<Material>			createMaterial			(const utils::ROData& data);
		std::shared_ptr<Material>			createMaterial			(const pugi::xml_node& section, MaterialUI* oUI = nullptr);
		bool								createMaterials			(std::vector<std::shared_ptr<Material>>& out, const utils::ROData& data);

		std::shared_ptr<PipelineMaterial>	createPipelineMaterial	(const utils::ROData& data);
		std::shared_ptr<PipelineMaterial>	createPipelineMaterial	(const pugi::xml_node& section, MaterialUI* oUI = nullptr);
		bool								createPipelineMaterials	(std::vector<std::shared_ptr<PipelineMaterial>>& out, const utils::ROData& data);

	private:
		bool loadProps(
			PropertyMap& oPpropsMap, const pugi::xml_node& section
			);

		bool loadRenderStates(
			RenderStateProperties& oRSProps, const pugi::xml_node& section
			);

		bool loadUI(
			MaterialUI& oUI, PropertyMap& propsMap, const pugi::xml_node& section
			);

		bool gatherPropsForShader(
			Properties& oProps, PropertyMap& oPropsMap,
			const IShader& shader, bool warningIfNotExist = false
			);

		typedef std::map<std::string, PipelineShader> PipelineShadersMap;
		PipelineShadersMap m_pipelineShaders;
	};


	//-- Represents material of the given geometry. It represents only the render vision about
	//-- the material. I.e. render doesn't worry about any kind of the memory management here.
	//----------------------------------------------------------------------------------------------
	struct RenderFx : public NonCopyable
	{
	public:
		RenderFx()
			:	m_shader(CONST_INVALID_HANDLE), m_vertexDlcr(CONST_INVALID_HANDLE), m_props(nullptr),
				m_propsCount(0), m_rsProps(nullptr), m_skinned(false), m_bumped(false) { }

		//-- shader and vertex declaration.
		VertexLayoutID			m_vertexDlcr;
		Handle					m_shader;

		//-- user defined properties.
		PropertyPair*			m_props;
		uint16					m_propsCount;

		//-- user defined render state properties.
		RenderStateProperties*	m_rsProps;

		//-- system auto properties may are used for more then one render pass.
		bool					m_skinned;
		bool					m_bumped;
	};


	//----------------------------------------------------------------------------------------------
	class PipelineMaterial : public NonCopyable
	{
		friend class Materials;
		
	public:
		PipelineMaterial();
		~PipelineMaterial();

		void			addProperty	(const char* name, IProperty* prop);
		IProperty*		getProperty (const char* name);
		const RenderFx* renderFx    (ShaderContext::EPassType pass, bool instanced = false);

		//-- ToDo:
		RenderStateProperties& rsProps() { return m_rsProps; }

	private:
		struct Pass
		{
			bool		m_instanced;
			RenderFx	m_normalFx;
			Properties	m_normalProps;
			RenderFx	m_instancedFx;
			Properties	m_instancedProps;
		};
		
		Pass						m_passes[ShaderContext::PASS_COUNT];
		Materials::PipelineShader*	m_shaders;
		Materials::PropertyMap		m_propsMap;
		RenderStateProperties		m_rsProps;
	};


	//-- Presents render system vision about the material. I.e. it manages the all things
	//-- related to the resources management and loading.
	//----------------------------------------------------------------------------------------------
	class Material : public NonCopyable
	{
		friend class Materials;

	public:
		Material();
		~Material();

		//-- Note: when we add new property to the material we also give it ownership for this
		//--	   property.
		void			addProperty	(const char* name, IProperty* prop);
		IProperty*		getProperty (const char* name);
		const RenderFx* renderFx	() const { return &m_fx; }

	private:
		RenderFx				m_fx;
		Properties				m_props;

		RenderStateProperties	m_rsProps;
		Materials::PropertyMap	m_propsMap;
		Handle					m_shader;
		VertexLayoutID			m_vertexDclr;
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

		struct ComboBoxDesc
		{
			std::string m_name;
			std::string m_value;
			bool		m_rts;
		};

		typedef std::pair<SliderDesc,   FloatProperty*>    Slider;
		typedef std::pair<CheckBoxDesc, FloatProperty*>	  CheckBox;
		typedef std::pair<ComboBoxDesc, TextureProperty*> ComboBox;

		std::vector<Slider>   m_sliders;
		std::vector<CheckBox> m_checkBoxes;
		std::vector<ComboBox> m_comboBoxes;
	};

} //-- render
} //-- brUGE