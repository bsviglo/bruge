#pragma once

#include "render_common.h"
#include "utils/string_utils.h"
#include "utils/Singleton.h"
#include "utils/DynamicLib.h"
#include "render/IShader.h"
#include "render/IRenderDevice.h"
#include "render/state_objects.h"
#include "render/shader_context.hpp"
#include "render/materials.hpp"
#include "render/post_processing.hpp"

#include <memory>
#include <vector>
#include <map>

namespace brUGE
{
namespace render
{

	struct RenderFx;

	//-- Represents the minimum quantum of the engine render system work.
	//----------------------------------------------------------------------------------------------
	struct RenderOp
	{
		RenderOp()
			:	m_primTopolpgy(PRIM_TOPOLOGY_TRIANGLE_LIST), m_mainVB(0), m_tangentVB(0), m_IB(0),
				m_weightsTB(0), m_matrixPaletteCount(0), m_matrixPalette(0), m_indicesCount(0),
				m_instanceTB(nullptr), m_instanceCount(0), m_worldMat(nullptr), m_material(nullptr),
				m_instanceData(nullptr), m_instanceSize(0)
		{ }

		//-- primitive topology of geometry.
		uint16				m_indicesCount;
		EPrimitiveTopology	m_primTopolpgy;
		//-- main data of static mesh.
		IBuffer*			m_IB;
		IBuffer*			m_mainVB;
		IBuffer*			m_tangentVB;
		//-- addition data in case if mesh is animated.
		IBuffer*			m_weightsTB;
		const mat4f*		m_matrixPalette;
		uint				m_matrixPaletteCount;
		//-- material of given sub-mesh.
		const RenderFx*		m_material;
		//-- world transformation.
		const mat4f*		m_worldMat;
		//-- instance data.
		IBuffer*			m_instanceTB;
		const void*			m_instanceData;
		uint16				m_instanceSize;
		uint16				m_instanceCount;
	};
	typedef std::vector<RenderOp> RenderOps;


	//-- The main class of the render system.
	//----------------------------------------------------------------------------------------------
	class RenderSystem : public utils::Singleton<RenderSystem>
	{
	public:
		//-- ToDo: reconsider this representation of the render passes.
		enum EPassType
		{
			PASS_Z_ONLY,	
			PASS_DECAL,
			PASS_LIGHT,
			PASS_SHADOW_CAST,
			PASS_SHADOW_RECEIVE,
			PASS_MAIN_COLOR,
			PASS_POST_PROCESSING,
			PASS_DEBUG_WIRE,
			PASS_DEBUG_SOLID,
			PASS_COUNT
		};

		struct PassDesc
		{
			Ptr<ITexture>		m_rt;
			DepthStencilStateID m_stateDS;
			RasterizerStateID	m_stateR;
			RasterizerStateID	m_stateR_doubleSided;
			BlendStateID		m_stateB;
		};

	public:
		RenderSystem();
		~RenderSystem();

		bool init(ERenderAPIType apiType, HWND hWindow, const VideoMode& videoMode);
		void shutDown();

		bool beginFrame();
		bool endFrame();

		bool beginPass(EPassType type);
		void setCamera(Camera* cam);
		void addRenderOps(const RenderOps& ops);
		bool endPass();

		//-- ToDo:
		void addImmediateRenderOps(const RenderOps& ops);

		const Camera&		camera()		const	 { return *m_camera; }
		ERenderAPIType		gapi()			const	 { return m_renderAPI; }
		ScreenResolution	screenRes()		const	 { return m_screenRes;  }
		Projection			projection()	const	 { return m_projection; }
		IRenderDevice*		device()		const	 { return m_device; }
		ShaderContext*		shaderContext()			 { return &m_shaderContext; }
		Materials*			materials()				 { return &m_materials; }
		PostProcessing*		postProcessing()		 { return &m_postProcessing; }
		ITexture*			depthTexture()			 { return m_passes[PASS_Z_ONLY].m_rt.get(); }
		ITexture*			decalsMask()			 { return m_passes[PASS_DECAL].m_rt.get(); }
		ITexture*			lightsMask()			 { return m_passes[PASS_LIGHT].m_rt.get(); }

		const mat4f&		lastViewProjMat() const				{ return m_lastViewProjMat; }
		const mat4f&		invLastViewProjMat() const			{ return m_invLastViewProjMat; }
		void				lastViewProjMat(const mat4f& vpMat)
		{
			m_lastViewProjMat = vpMat;
			m_invLastViewProjMat = m_lastViewProjMat.getInverted();
		}

	private:
		// console functions.
		int _printGAPIType();

		bool _initPasses();
		bool _finiPasses();
		void _doDraw(RenderOps& ops);

	private:
		VideoMode			m_videoMode;
		Projection			m_projection;
		ScreenResolution	m_screenRes;
		ERenderAPIType		m_renderAPI;
		utils::DynamicLib	m_dynamicLib;
		ShaderContext		m_shaderContext;
		Materials			m_materials;
		PostProcessing		m_postProcessing;

		Camera*				m_camera; //-- ToDo: use render camera instead.
		EPassType			m_pass;
		RenderOps			m_renderOps;

		//-- last view projection matrix.
		mat4f				m_lastViewProjMat;
		mat4f				m_invLastViewProjMat;

		//-- some addition resources for different render passes.
		PassDesc			m_passes[PASS_COUNT];

		IRenderDevice* 		m_device; // deleting performed after closing library *Render.dll
	};

	inline IRenderDevice* rd() { return RenderSystem::instance().device(); }
	inline RenderSystem&  rs() { return RenderSystem::instance(); }

} // render
} // brUGE
