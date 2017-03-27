#pragma once

#include "render_common.h"

#include "render/IShader.h"
#include "render/IRenderDevice.h"
#include "render/state_objects.h"
#include "render/shader_context.hpp"

#include <vector>
#include <unordered_map>

namespace brUGE
{
namespace render
{

	class  Materials;
	class  ShaderContext;
	struct RenderFx;

	//-- ToDo: Make it much more generalized.
	//-- ToDo: Compact it right after generalization.
	//-- Represents the minimum quantum of the engine render system work.
	//------------------------------------------------------------------------------------------------------------------
	struct RenderOp
	{
		RenderOp()
			:	m_primTopolpgy(PRIM_TOPOLOGY_TRIANGLE_LIST), m_VBs(nullptr), m_VBCount(0), m_IB(nullptr),
				m_matrixPaletteCount(0), m_matrixPalette(nullptr), m_startIndex(0), m_baseVertex(0), m_indicesCount(0),
				m_instanceTB(nullptr), m_instanceCount(0), m_worldMat(nullptr), m_material(nullptr),
				m_instanceData(nullptr), m_instanceSize(0), m_userData(nullptr)
		{ }

		//-- primitive topology of geometry.
		uint16				m_startIndex;
		uint16				m_baseVertex;
		uint16				m_indicesCount;
		EPrimitiveTopology	m_primTopolpgy;
		//-- main data of static mesh and terrain.
		IBuffer*			m_IB;
		IBuffer**			m_VBs;
		uint8				m_VBCount;
		//-- addition data in case if mesh is animated.
		const mat4f*		m_matrixPalette;
		uint16				m_matrixPaletteCount;
		//-- material of given sub-mesh.
		const RenderFx*		m_material;
		//-- world transformation.
		const mat4f*		m_worldMat;
		//-- instance data.
		IBuffer*			m_instanceTB;
		const void*			m_instanceData;
		uint16				m_instanceSize;
		uint16				m_instanceCount;
		//-- user data.
		const void*			m_userData;
	};
	typedef std::vector<RenderOp> RenderOps;


	//-- The main class of the render system.
	//------------------------------------------------------------------------------------------------------------------
	class Renderer : public utils::Singleton<Renderer>
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
			PASS_SKY,
			PASS_POST_PROCESSING,
			PASS_DEBUG_WIRE,
			PASS_DEBUG_SOLID,
			PASS_DEBUG_TRANSPARENT,
			PASS_DEBUG_OVERRIDE,
			PASS_COUNT
		};

		struct PassDesc
		{
			std::shared_ptr<ITexture>	m_rt;
			DepthStencilStateID			m_stateDS;
			RasterizerStateID			m_stateR;
			RasterizerStateID			m_stateR_doubleSided;
			RasterizerStateID			m_stateR_wireframe;
			BlendStateID				m_stateB;
		};

	public:
		Renderer();
		~Renderer();

		bool init(ERenderAPIType apiType, HWND hWindow, const VideoMode& videoMode);
		void shutDown();

		bool beginFrame();
		bool endFrame();

		bool beginPass(EPassType type);
		void setCamera(const RenderCamera* cam);
		void addROPs(const RenderOps& ops);
		bool endPass();

		//-- ToDo:
		void addImmediateROPs(const RenderOps& ops);

		const RenderCamera*			camera()		const	 { return m_camera; }
		ERenderAPIType				gapi()			const	 { return m_renderAPI; }
		const RenderStatistics&		statistics()	const	 { return m_device->m_statistics;  }
		ScreenResolution			screenRes()		const	 { return m_screenRes;  }
		IRenderDevice*				device()		const	 { return m_device; }
		ShaderContext&				shaderContext()			 { return *m_shaderContext.get(); }
		Materials&					materials()				 { return *m_materials.get(); }

		//-- ToDo:
		ShaderContext::EPassType	shaderPass(EPassType type) const;

		ITexture*			depthTexture()			 { return m_passes[PASS_Z_ONLY].m_rt.get(); }
		ITexture*			decalsMask()			 { return m_passes[PASS_DECAL].m_rt.get(); }
		ITexture*			lightsMask()			 { return m_passes[PASS_LIGHT].m_rt.get(); }
		ITexture*			shadowsMask()			 { return m_passes[PASS_SHADOW_RECEIVE].m_rt.get(); }

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
		VideoMode							m_videoMode;
		ScreenResolution					m_screenRes;
		ERenderAPIType						m_renderAPI;
		void*								m_renderModuleDLL;
		std::unique_ptr<ShaderContext>		m_shaderContext;
		std::unique_ptr<Materials>			m_materials;

		const RenderCamera*					m_camera;
		EPassType							m_pass;
		RenderOps							m_renderOps;

		//-- last view projection matrix.
		mat4f								m_lastViewProjMat;
		mat4f								m_invLastViewProjMat;

		//-- some addition resources for different render passes.
		std::array<PassDesc, PASS_COUNT>	m_passes;

		IRenderDevice* 						m_device; // deleting performed after closing library *Render.dll
	};

	inline IRenderDevice*	rd() { return Renderer::instance().device(); }
	inline Renderer&		rs() { return Renderer::instance(); }
}
}
