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
	//-- Provides not only interface to apply post-effects, but also UI to create and tune them.
	//-- UI was implemented as an independent module. So if doesn't need it we almost not have any
	//-- additional overhead to support this feature.
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

		//-- find post-processing's render targets.
		Ptr<ITexture> find(const char* rtName);

	private:
		//-- render target description.
		struct RTDesc
		{
			Ptr<ITexture> m_rt;
			vec2ui		  m_dims;
		};
		typedef std::map<std::string, RTDesc> RenderTargetMap;

		//-- 
		struct PostEffect
		{
			struct Pass
			{
				Pass() : m_copyBB(false), m_enabled(false), m_clearRT(false), m_rt(nullptr) { }

				bool		  m_copyBB;
				bool		  m_enabled;
				bool		  m_clearRT;
				std::string   m_name;
				ITexture*	  m_rt;
				vec2ui		  m_dims;
				Ptr<Material> m_material;
			};
			typedef std::vector<Pass> Passes;

			std::string	m_name;
			Passes		m_passes;
		};
		typedef std::vector<PostEffect*> PostEffects;

		//--
		//------------------------------------------------------------------------------------------
		class UI : public NonCopyable
		{
		public:
			struct PostEffectUI
			{
				struct PassUI
				{
					PassUI(MaterialUI desc, bool hidden) : m_desc(desc), m_show(hidden) { }
					PassUI() : m_show(false) { }

					bool	    m_show;
					std::string m_name;
					MaterialUI  m_desc;
				};
				typedef std::vector<PassUI> PassesUI;

				PostEffectUI() : m_show(true) { }
				PostEffectUI(bool show) : m_show(show) { }

				bool	 m_show;
				PassesUI m_passesUI;
			};
			typedef std::vector<PostEffectUI> PostEffectsUI;

		public:
			UI(PostProcessing& pp);
			~UI();

			void update();
			void addPostEffect(PostEffectUI& ui);

		private:
			void displaySelectRT();
			void displayEffectsList();

			struct SelectRT
			{
				SelectRT()
					:	m_enabled(false), m_changed(false), m_showBB(false), m_scroll(0),
						m_name(nullptr), m_rt(nullptr), m_ptr(0) { }

				bool  m_enabled;
				bool  m_changed;
				bool  m_showBB;
				int   m_scroll;
				void* m_ptr;

				const char* m_name;
				ITexture*   m_rt;
				vec2ui		m_dims;
			};

			struct EffectsList
			{
				EffectsList() : m_enabled(false), m_scroll(0) { }

				bool m_enabled;
				int  m_scroll;
				std::vector<std::string> m_list;
			};

			int				m_scroll;
			PostProcessing& m_pp;
			PostEffectsUI	m_effectsUI;
			SelectRT		m_selectRT;
			EffectsList		m_effectsList;
		};
		typedef std::unique_ptr<UI> UIPtr;

		//-- make ui friend for Post-Processing.
		friend class UI;

		bool loadCfg();
		bool loadPostEffect(const char* ppName);
		bool loadPostEffectPass(
			const pugi::xml_node& section, PostEffect::Pass& pass, UI::PostEffectUI::PassUI* passUI
			);

	private:
		std::string		m_dir;
		RenderTargetMap	m_rts;
		RenderOps		m_rops;
		Ptr<IBuffer>	m_fsQuad;
		PostEffects		m_effects;
		int				m_curEffectID;

		//-- optional UI interface.
		//-- ToDo: Maybe it will be better to create a separate post-processing editor.
		//--	   But for now it's very expensive in terms of time.
		UIPtr			m_ui;
	};

} //-- render
} //-- brUGE