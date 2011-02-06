#pragma once

#include "render_common.h"
#include "shader_context.hpp"
#include "utils/Data.hpp"

#include <vector>
#include <map>

namespace brUGE
{
namespace render
{

	class Material;

	//-- Presents data holder of the all materials in the engine. At the beginning of the game all
	//-- needed materials descriptions will be readed from the materials.xml file and then used
	//-- to formulate desired material.
	//----------------------------------------------------------------------------------------------
	class Materials
	{
	private:
		//-- make non-copyable.
		Materials(const Materials&);
		Materials& operator = (const Materials&);

	public:
		struct RenderMat
		{
			RenderMat();

			bool   m_isSkinned;
			bool   m_isBumpMaped;
			bool   m_isMultipass;

			Handle m_vertDecl;
			Handle m_shaders[ShaderContext::SHADER_RENDER_PASS_COUNT];
		};

	public:
		Materials();
		~Materials();

		bool init();
		bool fini();

		Ptr<Material> createMaterial(const utils::ROData& data);

	private:
		typedef std::map<std::string, RenderMat> MaterialsMap;

		MaterialsMap m_materials;
	};


	//-- Represents material of the given geometry. It represents only the render vision about
	//-- the material. I.e. render doesn't worry about any kind of the memory management here.
	//-- ToDo: need some reconsideration in the near future.
	//----------------------------------------------------------------------------------------------
	struct RenderFx
	{
	private:
		//-- make non-copyable.
		RenderFx(const RenderFx&);
		RenderFx& operator = (const RenderFx&);

	public:

		RenderFx()
			:	m_shader(nullptr), m_props(nullptr), m_propsCount(0) { }

		struct SysProperties
		{
			SysProperties() : m_bumpMap(nullptr), m_diffuseMap(nullptr) { }

			ITexture*	m_bumpMap;
			ITexture*	m_diffuseMap;
		};

		//-- shader(-s) and vertex declaration.
		Materials::RenderMat* m_shader;

		//-- user defined properties.
		IProperty**			  m_props;
		uint				  m_propsCount;

		//-- system auto properties may are used for more then one render pass.
		SysProperties		  m_sysProps;
	};


	//-- Presents system render system vision about the material. I.e. it manages the all things
	//-- related to the resources management and loading.
	//----------------------------------------------------------------------------------------------
	class Material : public utils::RefCount
	{
		friend class Materials;

	private:
		//-- make non-copyable.
		Material(const Material&);
		Material& operator = (const Material&);

	public:
		Material();
		~Material();

		const RenderFx* renderFx() const { return &m_fx; }

	private:
		RenderFx		m_fx;

		Properties		m_props;

		Ptr<ITexture>	m_diffuseTex;
		Ptr<ITexture>	m_bumpTex;
	};

} //-- render
} //-- brUGE