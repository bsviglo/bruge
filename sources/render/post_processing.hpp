#pragma once

#include "prerequisites.h"
#include "materials.hpp"
#include "vertex_format.hpp"
#include <vector>
#include <map>

namespace brUGE
{
namespace render
{

	//-- Post-processing framework.
	//-- ToDo: document 
	//-- ToDo: needed reconsiderations.
	//----------------------------------------------------------------------------------------------
	class PostProcessing : public NonCopyable
	{
	public:
		PostProcessing();
		~PostProcessing();

		bool init();
		bool fini();

		void update(float dt);
		void draw();

		void enable(const char* ppName);
		void disable();

		Ptr<ITexture> find(const char* rtName);

	private:

		struct RTDesc
		{
			Ptr<ITexture> m_rt;
			vec2ui		  m_dims;
		};
		typedef std::map<std::string, RTDesc> RenderTargetMap;

		struct PostEffect
		{
			struct Pass
			{
				Pass() : m_copyBB(false), m_isEnabled(false), m_clearRT(false) { }

				bool		  m_copyBB;
				bool		  m_isEnabled;
				bool		  m_clearRT;
				std::string   m_name;
				ITexture*	  m_rt;
				vec2ui		  m_dims;
				Ptr<Material> m_material;
				UIDesc		  m_uiDesc;
				//RenderStates m_renderStates;
			};

			std::string			m_name;
			std::vector<Pass>	m_passes;
		};

		bool updateUI();
		bool loadPostEffect(const char* ppName);
		bool loadPostEffectPass(const pugi::xml_node& section, PostEffect::Pass& pass);

	private:

		//-- common render resources.
		RenderOps							m_rops;
		Ptr<IBuffer>						m_vb;
		RenderTargetMap						m_rts;
		std::map<std::string, PostEffect*>	m_effects;
		PostEffect*							m_curEffect;

		DepthStencilStateID					m_stateDS;
		BlendStateID						m_stateB;
		RasterizerStateID					m_stateR;
	};

} //-- render
} //-- brUGE