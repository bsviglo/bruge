#include "post_processing.hpp"
#include "os/FileSystem.h"
#include "render_system.hpp"
#include "materials.hpp"
#include "gui/imgui.h"

using namespace brUGE;
using namespace brUGE::utils;
using namespace brUGE::os;
using namespace brUGE::math;

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	PostProcessing::PostProcessing() : m_curEffect(nullptr)
	{

	}

	//----------------------------------------------------------------------------------------------
	PostProcessing::~PostProcessing()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool PostProcessing::init()
	{
		//-- 1. setup render targets.
		{
			ITexture::Desc desc;
			desc.bindFalgs = ITexture::BIND_RENDER_TARGET | ITexture::BIND_SHADER_RESOURCE;
			desc.format	   = ITexture::FORMAT_RGBA8;
			desc.texType   = ITexture::TYPE_2D;
			desc.width	   = rs().screenRes().width;
			desc.height	   = rs().screenRes().height;

			RTDesc rtDesc;
			rtDesc.m_dims.x = rs().screenRes().width;
			rtDesc.m_dims.y = rs().screenRes().height;
			rtDesc.m_rt     = rd()->createTexture(desc, nullptr, 0);
			
			if (!rtDesc.m_rt)
				return false;

			m_rts["BBCopy"] = rtDesc;
		}

		//-- 2. setup full-screen quad.
		{
			VertexXYZUV vertices[4];

			//-- left-top
			vertices[0].m_pos.set(-1.f, 1.f, 0.0f);
			vertices[0].m_tc.set (0.f, 0.f);

			//-- left-bottom		
			vertices[1].m_pos.set(-1.f, -1.f, 0.0f);
			vertices[1].m_tc.set (0.f, 1.f);

			//-- right-top
			vertices[2].m_pos.set(1.f, 1.f, 0.0f);
			vertices[2].m_tc.set (1.f, 0.f);

			//-- right-bottom
			vertices[3].m_pos.set(1.f, -1.f, 0.0f);
			vertices[3].m_tc.set (1.f, 1.f);

			if (!(m_vb = rd()->createBuffer(IBuffer::TYPE_VERTEX, vertices, 4, sizeof(VertexXYZUV))))
				return false;
		}

		//-- create rops for drawing.
		{
			RenderOp op;
			op.m_primTopolpgy = PRIM_TOPOLOGY_TRIANGLE_STRIP;
			op.m_mainVB		  = &*m_vb;
			op.m_indicesCount = 4;

			m_rops.push_back(op);
		}

		//-- 3. setup render states.
		{
			DepthStencilStateDesc dsDesc;
			dsDesc.depthEnable = false;
			m_stateDS = render::rd()->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			m_stateR = render::rd()->createRasterizedState(rDesc);

			BlendStateDesc bDesc;
			m_stateB = render::rd()->createBlendState(bDesc);
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool PostProcessing::fini()
	{
		m_vb.reset();
		m_rts.clear();

		for (auto i = m_effects.begin(); i != m_effects.end(); ++i)
			delete i->second;

		m_effects.clear();

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void PostProcessing::update(float /*dt*/)
	{
		updateUI();
	}

	//----------------------------------------------------------------------------------------------
	void PostProcessing::draw()
	{
		if (!m_curEffect) return;

		//-- ToDo:
		//-- 1. copy back buffer into intermediate texture.
		rd()->copyTexture(rd()->getMainColorRT(), m_rts["BBCopy"].m_rt.get());

		//-- 2. do post-processing steps.
		for (uint i = 0; i < m_curEffect->m_passes.size(); ++i)
		{
			PostEffect::Pass& pass = m_curEffect->m_passes[i];
			if (pass.m_isEnabled)
			{
				rd()->setRenderTarget(pass.m_rt, nullptr);
				//rd()->setViewPort(pass.m_rt.m_dim.x, pass.m_rt.m_dim.y);
				//if (pass.m_clearRT)
				//	rd()->clearColorRT(pass.m_rt.m_rt.get(), Color(0,0,0,0));

				m_rops[0].m_material = pass.m_material->renderFx();

				rs().addImmediateRenderOps(m_rops);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void PostProcessing::enable(const char* ppName)
	{
		//-- 1. try to find desired post-effect.
		auto res = m_effects.find(ppName);
		if (res != m_effects.end())
		{
			m_curEffect = res->second;
		}
		else
		{
			//-- ToDo:
			if (loadPostEffect(ppName))
			{
				m_curEffect = m_effects.find(ppName)->second;
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void PostProcessing::disable()
	{
		m_curEffect = nullptr;
	}

	//----------------------------------------------------------------------------------------------
	Ptr<ITexture> PostProcessing::find(const char* rtName)
	{
		auto res = m_rts.find(rtName);
		if (res != m_rts.end())
		{
			return res->second.m_rt;
		}
		else
		{
			return nullptr;
		}
	}

	//----------------------------------------------------------------------------------------------
	bool PostProcessing::updateUI()
	{
		bool isActive = false;
		uint width	  = rs().screenRes().width;
		uint height   = rs().screenRes().height;
		
		static int scroll = 0;

		if (imguiBeginScrollArea("Post-processing", width-250-10, height-10-450, 250, 450, &scroll))
			isActive = true;

		if (m_curEffect)
		{
			imguiLabel(makeStr("Post-Effect: %s", m_curEffect->m_name.c_str()).c_str());
			imguiIndent();

			for (uint i = 0; i < m_curEffect->m_passes.size(); ++i)
			{
				imguiSeparatorLine();
				PostEffect::Pass& pass = m_curEffect->m_passes[i];
				UIDesc&			  ui   = pass.m_uiDesc;

				if (imguiCheck("enable", pass.m_isEnabled))
				{
					pass.m_isEnabled = !pass.m_isEnabled;
				}
				imguiSeparator();

				//-- update checkboxes.
				for (uint j = 0; j < ui.m_checkBoxes.size(); ++j)
				{
					UIDesc::CheckBoxDesc&	desc = ui.m_checkBoxes[j].first;
					NumericProperty<float>* prop = ui.m_checkBoxes[j].second;

					if (imguiCheck(desc.m_name.c_str(), desc.m_value, pass.m_isEnabled))
					{
						desc.m_value = !desc.m_value;
						prop->set(static_cast<float>(desc.m_value));
					}
				}

				//-- update sliders
				for (uint j = 0; j < ui.m_sliders.size(); ++j)
				{
					UIDesc::SliderDesc&		desc = ui.m_sliders[j].first;
					NumericProperty<float>* prop = ui.m_sliders[j].second;

					if (imguiSlider(desc.m_name.c_str(), &desc.m_value, desc.m_range.x, desc.m_range.y, desc.m_step, pass.m_isEnabled))
					{
						prop->set(desc.m_value);
					}
				}
				imguiSeparatorLine();
			}

			imguiUnindent();
		}
		
		imguiEndScrollArea();

		return isActive;
	}

	//----------------------------------------------------------------------------------------------
	bool PostProcessing::loadPostEffect(const char* ppName)
	{
		RODataPtr data = FileSystem::instance().readFile(
			makeStr("resources/post_processing/%s", ppName).c_str()
			);

		if (!data.get())
		{
			ERROR_MSG("Can't load post processing effect %s.", ppName);
			return false;
		}

		pugi::xml_document doc;
		if (!doc.load_buffer(data->ptr(), data->length()))
		{
			ERROR_MSG("Most likely loading file '%s' is corrupted.", ppName);
			return false;
		}

		pugi::xml_node effectNode = doc.document_element();

		std::unique_ptr<PostEffect> postEffect(new PostEffect);

		//-- parse post effect description.
		if (auto name = effectNode.attribute("name"))
		{
			postEffect->m_name = name.value();
		}
		else
		{
			assert(0);
			return false;
		}

		//-- parse effect's passes.
		for (pugi::xml_node prop = effectNode.child("pass"); prop; prop = prop.next_sibling("pass"))
		{
			PostEffect::Pass pass;
			if (loadPostEffectPass(prop, pass))
			{
				postEffect->m_passes.push_back(pass);
			}
			else
			{
				assert(0);
				return false;
			}
		}

		//-- add new post-effect.
		m_effects[ppName] = postEffect.release();

		//-- all ok.
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool PostProcessing::loadPostEffectPass(const pugi::xml_node& section, PostEffect::Pass& pass)
	{
		PostEffect::Pass out;

		//-- 1. read common properties.
		out.m_name		 = section.attribute("name").value();
		out.m_isEnabled = section.attribute("enabled").as_bool();
		
		//-- 2. read render target name.
		if (auto name = section.attribute("rt"))
		{
			auto res = m_rts.find(name.value());
			if (res != m_rts.end())
			{
				out.m_rt   = res->second.m_rt.get();
				out.m_dims = res->second.m_dims;
			}
			else
			{
				if (std::string(name.value()) == "BB")
				{
					out.m_rt     = rd()->getMainColorRT();
					out.m_dims.x = rs().screenRes().width;
					out.m_dims.y = rs().screenRes().height;
				}
				else
				{
					ERROR_MSG("Invalid render target name %s.", name.value());
					return false;
				}
			}
		}
		else
		{
			assert(0);
			return false;
		}

		//-- 3. read material.
		if (auto sec = section.child("material"))
		{
			Ptr<Material> mat = rs().materials()->createMaterial(sec, &out.m_uiDesc);
			if (mat)
			{
				out.m_material = mat;
			}
			else
			{
				assert(0);
				return false;
			}
		}
		else
		{
			assert(0);
			return false;
		}

		//-- 4. read states.
		//-- ToDo: implement.

		pass = out;
		return true;
	}

} //-- render
} //-- brUGE