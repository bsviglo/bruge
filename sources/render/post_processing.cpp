#include "post_processing.hpp"
#include "os/FileSystem.h"
#include "render_system.hpp"
#include "materials.hpp"

using namespace brUGE;
using namespace brUGE::render;
using namespace brUGE::utils;
using namespace brUGE::os;
using namespace brUGE::math;

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	const char* cfg = "resources/system/post_processing.xml";

	//----------------------------------------------------------------------------------------------
	void displayCheckBox(MaterialUI::CheckBox&, bool)
	{
		//MaterialUI::CheckBoxDesc& d = cb.first;
		//
		//if (imguiCheck(d.m_name.c_str(), &d.m_value, enabled))
		//{
		//	cb.second->set(static_cast<float>(d.m_value));
		//}
	}

	//----------------------------------------------------------------------------------------------
	void displaySlider(MaterialUI::Slider&, bool)
	{
		//MaterialUI::SliderDesc&	d = sl.first;
		//
		//if (imguiSlider(d.m_name.c_str(), &d.m_value, d.m_range.x, d.m_range.y, d.m_step, enabled))
		//{
		//	sl.second->set(d.m_value);
		//}
	}
}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.

namespace brUGE
{
namespace render
{
	//----------------------------------------------------------------------------------------------
	PostProcessing::PostProcessing() : m_curEffectID(-1), m_uiEnabled(false)
	{

	}

	//----------------------------------------------------------------------------------------------
	PostProcessing::~PostProcessing()
	{
		for (auto i = m_effects.begin(); i != m_effects.end(); ++i)
			delete *i;
	}

	//----------------------------------------------------------------------------------------------
	bool PostProcessing::init()
	{
		REGISTER_CONSOLE_MEMBER_VALUE("r_post_process_enable_ui", bool, m_uiEnabled, PostProcessing);

		//-- 1. setup render targets.
		if (!loadCfg())
		{
			return false;
		}

		//-- create UI.
		m_ui.reset(new UI(*this));

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

			if (!(m_fsQuad = rd()->createBuffer(IBuffer::TYPE_VERTEX, vertices, 4, sizeof(VertexXYZUV))))
				return false;

			m_pVB = m_fsQuad.get();
		}

		//-- create rops for drawing.
		{
			RenderOp op;
			op.m_indicesCount = 4;
			op.m_primTopolpgy = PRIM_TOPOLOGY_TRIANGLE_STRIP;
			op.m_VBs		  = &m_pVB;
			op.m_VBCount	  = 1;
			m_rops.push_back(op);
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void PostProcessing::update(float /*dt*/)
	{
		if (m_uiEnabled) m_ui->update();
	}

	//----------------------------------------------------------------------------------------------
	void PostProcessing::draw()
	{
		if (m_curEffectID == -1) return;

		PostEffect& effect = *m_effects[m_curEffectID];

		//-- 2. do post-processing steps.
		for (uint i = 0; i < effect.m_passes.size(); ++i)
		{
			PostEffect::Pass& pass = effect.m_passes[i];

			if (pass.m_enabled)
			{
				if (pass.m_copyBB)
				{
					rd()->copyTexture(rd()->getMainColorRT(), pass.m_rt);
				}
				else
				{
					rd()->setViewPort(0, 0, pass.m_dims.x, pass.m_dims.y);
					rd()->setRenderTarget(pass.m_rt, nullptr);
					if (pass.m_clearRT)
					{
						rd()->clearColorRT(pass.m_rt, Color(0,0,0,0));
					}

					m_rops[0].m_material = pass.m_material->renderFx();
					rs().addImmediateROPs(m_rops);
				}
			}
		}

		rd()->setViewPort(0, 0, rs().screenRes().width, rs().screenRes().height);
		rd()->backToMainFrameBuffer();
	}

	//----------------------------------------------------------------------------------------------
	void PostProcessing::enable(const char* ppName)
	{
		//-- 1. try to find desired post-effect.
		m_curEffectID = -1;
		for (uint i = 0; i < m_effects.size(); ++i)
		{
			if (m_effects[i]->m_name == ppName)
			{
				m_curEffectID = i;
			}
		}

		if (m_curEffectID == -1 && loadPostEffect(ppName))
		{
			m_curEffectID = m_effects.size() - 1;
		}
	}

	//----------------------------------------------------------------------------------------------
	void PostProcessing::disable()
	{
		m_curEffectID = -1;
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
		UI::PostEffectUI ui;

		//-- parse post effect description.
		if (auto name = effectNode.attribute("name"))
		{
			postEffect->m_name = name.value();
		}
		else
		{
			return false;
		}

		//-- parse effect's passes.
		for (auto prop = effectNode.child("pass"); prop; prop = prop.next_sibling("pass"))
		{
			PostEffect::Pass pass;
			UI::PostEffectUI::PassUI passUI;
			if (loadPostEffectPass(prop, pass, &passUI))
			{
				postEffect->m_passes.push_back(pass);
				ui.m_passesUI.push_back(passUI);
			}
			else
			{
				return false;
			}
		}

		//-- add new post-effect.
		m_effects.push_back(postEffect.release());
		m_ui->addPostEffect(ui);

		//-- all ok.
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool PostProcessing::loadPostEffectPass(
		const pugi::xml_node& section, PostEffect::Pass& pass, UI::PostEffectUI::PassUI* passUI)
	{
		PostEffect::Pass out;

		//-- 1. read common properties.
		out.m_name	  = section.attribute("name").value();
		out.m_enabled = section.attribute("enabled").as_bool();
		out.m_clearRT = section.attribute("clearRT").as_bool();
		out.m_copyBB  = section.attribute("copyBB").as_bool();
		
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
					out.m_rt   = rd()->getMainColorRT();
					out.m_dims = vec2ui(rs().screenRes().width, rs().screenRes().height);
				}
				else
				{
					ERROR_MSG("Invalid render target name %s.", name.value());
					return false;
				}
			}

			passUI->m_name = name.value();
		}
		else
		{
			return false;
		}

		//-- if pass is copyBB pass than just exist.
		if (out.m_copyBB)
		{
			out.m_name = "BB copy";
			pass = out;
			return true;
		}

		//-- 3. read material.
		if (auto sec = section.child("material"))
		{
			if (auto mat = rs().materials().createMaterial(sec, &passUI->m_desc))
			{
				out.m_material = mat;
			}
			else
			{
				ERROR_MSG("Can't load pass %s material.", out.m_name.c_str());
				return false;
			}
		}
		else
		{
			return false;
		}

		//-- 4. read states.
		//-- ToDo: implement.

		pass = out;
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool PostProcessing::loadCfg()
	{
		RODataPtr data = FileSystem::instance().readFile(cfg);
		if (!data.get())
		{
			ERROR_MSG("Can't load post processing cfg file.");
			return false;
		}

		pugi::xml_document doc;
		if (!doc.load_buffer(data->ptr(), data->length()))
		{
			ERROR_MSG("Most likely loading file '%s' is corrupted.", cfg);
			return false;
		}

		pugi::xml_node cfgNode = doc.document_element();

		if (auto param = cfgNode.attribute("dir"))
		{
			m_dir = param.value();
		}
		else
		{
			return false;
		}

		if (auto param = cfgNode.child("renderTargets"))
		{
			uint screenWidth  = rs().screenRes().width;
			uint screenHeight = rs().screenRes().height;

			for (auto rt = param.child("rt"); rt; rt = rt.next_sibling("rt"))
			{
				float w = rt.attribute("width").as_float();
				float h = rt.attribute("height").as_float();

				w = w < 0 ? 1.0f / pow(2.0f, abs(w)) : pow(2.0f, abs(w));
				h = h < 0 ? 1.0f / pow(2.0f, abs(h)) : pow(2.0f, abs(h));
				
				const char* name   = rt.attribute("name").value();
				int			width  = w * screenWidth;
				int			height = h * screenHeight;

				ITexture::Desc desc;
				desc.bindFalgs  = ITexture::BIND_RENDER_TARGET | ITexture::BIND_SHADER_RESOURCE;
				desc.format	    = ITexture::FORMAT_RGBA8;
				desc.texType    = ITexture::TYPE_2D;
				desc.width		= width;
				desc.height		= height;

				RTDesc rtDesc;
				rtDesc.m_dims.x = desc.width;
				rtDesc.m_dims.y = desc.height;
				rtDesc.m_rt		= rd()->createTexture(desc, nullptr, 0);

				m_rts[name] = rtDesc;
			}
		}
		else
		{
			return false;
		}
		return true;
	}

	//----------------------------------------------------------------------------------------------
	PostProcessing::UI::UI(PostProcessing& pp) : m_pp(pp), m_scroll(0)
	{
	}

	//----------------------------------------------------------------------------------------------
	PostProcessing::UI::~UI()
	{
	}

	//----------------------------------------------------------------------------------------------
	void PostProcessing::UI::update()
	{
		//uint width	= rs().screenRes().width;
		//uint height = rs().screenRes().height;
		//
		//imguiBeginScrollArea("Post-processing", width-300-10, 10, 300, height-20, &m_scroll);
		//
		//imguiSeparatorLine();
		//imguiIndent(4);
		//if (imguiButton("Load"))
		//{
		//	m_effectsList.m_enabled = !m_effectsList.m_enabled;
		//	if (m_effectsList.m_enabled)
		//	{
		//		m_effectsList.m_list.clear();
		//		FileSystem::instance().getFilesInDir("..\\resources\\post_processing", m_effectsList.m_list);
		//	}
		//}
		//imguiButton("Save");
		//imguiUnindent(4);
		//imguiSeparatorLine();
		//imguiSeparator();
		//
		//if (m_pp.m_curEffectID != -1)
		//{
		//	PostEffect&		effect	 = *m_pp.m_effects[m_pp.m_curEffectID];
		//	PostEffectUI&	effectUI = m_effectsUI[m_pp.m_curEffectID];
		//
		//	imguiIndent();
		//	imguiLabel(("Post-Effect: " + effect.m_name).c_str());
		//
		//	for (uint i = 0; i < effect.m_passes.size(); ++i)
		//	{
		//		PostEffect::Pass&		pass    = effect.m_passes[i];
		//		PostEffectUI::PassUI&	passUI  = effectUI.m_passesUI[i];
		//		bool&					enabled = pass.m_enabled;
		//
		//		imguiSeparatorLine();
		//		imguiCheck("enable", &enabled);
		//		imguiCollapse(makeStr("Pass #%d: %s", i, pass.m_name.c_str()).c_str(), nullptr, &passUI.m_show, enabled);
		//
		//		if (passUI.m_show)
		//		{
		//			imguiIndent();
		//
		//			imguiLabel("Render Target:");
		//			imguiCheck("clear RT", &pass.m_clearRT);
		//			if (imguiButton(passUI.m_name.c_str(), enabled))
		//			{
		//				m_selectRT.m_enabled = !m_selectRT.m_enabled;
		//				m_selectRT.m_showBB  = pass.m_copyBB;
		//				m_selectRT.m_ptr     = &pass;
		//			}
		//			if (m_selectRT.m_changed && m_selectRT.m_ptr == &pass)
		//			{
		//				passUI.m_name = m_selectRT.m_name;
		//				pass.m_rt	  = m_selectRT.m_rt;
		//				pass.m_dims	  = m_selectRT.m_dims;
		//				m_selectRT.m_changed = !m_selectRT.m_changed;
		//			}
		//
		//			imguiSeparator();
		//			imguiLabel("Shader properties:");
		//
		//			//-- update checkboxes.
		//			for (uint j = 0; j < passUI.m_desc.m_checkBoxes.size(); ++j)
		//			{
		//				displayCheckBox(passUI.m_desc.m_checkBoxes[j], enabled);
		//			}
		//
		//			//-- update sliders
		//			for (uint j = 0; j < passUI.m_desc.m_sliders.size(); ++j)
		//			{
		//				displaySlider(passUI.m_desc.m_sliders[j], enabled);
		//			}
		//
		//			//-- update comboboxes
		//			for (uint j = 0; j < passUI.m_desc.m_comboBoxes.size(); ++j)
		//			{
		//				MaterialUI::ComboBox& comboBox = passUI.m_desc.m_comboBoxes[j];
		//				if (!comboBox.first.m_rts)
		//					continue;
		//
		//				imguiLabel(comboBox.first.m_name.c_str());
		//				if (imguiButton(comboBox.first.m_value.c_str(), enabled))
		//				{
		//					m_selectRT.m_enabled = !m_selectRT.m_enabled;
		//					m_selectRT.m_showBB  = true;
		//					m_selectRT.m_ptr	 = &comboBox;
		//				}
		//				if (m_selectRT.m_changed && m_selectRT.m_ptr == &comboBox)
		//				{
		//					comboBox.first.m_value = m_selectRT.m_name;
		//					comboBox.second->set(m_selectRT.m_rt);
		//					m_selectRT.m_changed = !m_selectRT.m_changed;
		//				}
		//			}
		//			imguiUnindent();
		//		}
		//	}
		//	imguiUnindent();
		//}
		//imguiEndScrollArea();
		//
		////-- update select RT box.
		//displaySelectRT();
		//displayEffectsList();
	}

	//----------------------------------------------------------------------------------------------
	void PostProcessing::UI::addPostEffect(PostEffectUI& ui)
	{
		m_effectsUI.push_back(ui);
	}

	//----------------------------------------------------------------------------------------------
	void PostProcessing::UI::displaySelectRT()
	{
		//m_selectRT.m_changed = false;
		//
		//if (m_selectRT.m_enabled)
		//{
		//	uint width	= rs().screenRes().width;
		//	uint height = rs().screenRes().height;
		//
		//	imguiBeginScrollArea("Render Targets", width-520, height-10-250, 200, 250, &m_selectRT.m_scroll);
		//	if (!m_selectRT.m_showBB && imguiItem("BB"))
		//	{
		//		m_selectRT.m_name = "BB";
		//		m_selectRT.m_rt	  = rd()->getMainColorRT();
		//		m_selectRT.m_dims = vec2ui(rs().screenRes().width, rs().screenRes().height);
		//
		//		m_selectRT.m_enabled = false;
		//		m_selectRT.m_changed = true;
		//	}
		//	for (auto i = m_pp.m_rts.begin(); i != m_pp.m_rts.end(); ++i)
		//	{
		//		if (imguiItem(i->first.c_str()))
		//		{
		//			m_selectRT.m_name = i->first.c_str();
		//			m_selectRT.m_rt	  = i->second.m_rt.get();
		//			m_selectRT.m_dims = i->second.m_dims;
		//
		//			m_selectRT.m_enabled = false;
		//			m_selectRT.m_changed = true;
		//		}
		//	}
		//	imguiEndScrollArea();
		//}
	}

	//----------------------------------------------------------------------------------------------
	void PostProcessing::UI::displayEffectsList()
	{
		//if (m_effectsList.m_enabled)
		//{
		//	uint width	= rs().screenRes().width;
		//	uint height = rs().screenRes().height;
		//
		//	imguiBeginScrollArea("Effects", width-520, height-10-250, 200, 250, &m_effectsList.m_scroll);
		//
		//	for (uint i = 0; i < m_effectsList.m_list.size(); ++i)
		//	{
		//		const char* name = m_effectsList.m_list[i].c_str();
		//
		//		if (imguiItem(name))
		//		{
		//			m_pp.enable(name);
		//			m_effectsList.m_enabled = false;
		//		}
		//	}
		//	imguiEndScrollArea();
		//}
	}

} //-- render
} //-- brUGE