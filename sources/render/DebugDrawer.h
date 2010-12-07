#pragma once

#include "render_common.h"
#include "Color.h"
#include "utils/Singleton.h"
#include "math/AABB.h"
#include "math/OBB.hpp"
#include "IBuffer.h"
#include "IShader.h"

#include <vector>

namespace brUGE
{
namespace render
{
	//-- Debug information drawer.
	//-- It gathers drawing information from all its drawing methods and then draw all this
	//-- information uses for that only once draw call.
	//---------------------------------------------------------------------------------------------
	class DebugDrawer : public utils::Singleton<DebugDrawer>
	{
	public:
		DebugDrawer();
		~DebugDrawer() {}

		bool init();
		void destroy();

		void drawLine		(const vec3f& start, const vec3f& end, const Color& color);
		void drawCoordAxis	(const mat4f& orient, float scale = 1.0f);
		
		void drawFrustum	(const Color& color, const mat4f& orient, float	fov, float aspect, float nearDist, float farDist);
		void drawFrustum	(const mat4f& orient, const Projection& projInfo, float aspect);
		void drawFrustum	(const mat4f& orient, const mat4f& projMat);

		void drawAABB		(const AABB& aabb, const Color& color);
		void drawAABB		(const AABB& aabb, const Color& color, const mat4f& transform);

		void drawOBB		(const OBB& obb, const Color& color);
		
		//-- this method performs actual drawing.
		void draw(const mat4f& viewProjMat, float dt);

	private:
		bool _setupRender();
		void _swapBuffers();
		
		//-- console functions.
		int _drawDebugInfo(bool);

	private:
		struct VertDesc
		{
			VertDesc(const vec3f& pos_, const vec4f& color_) : pos(pos_), color(color_) {}

			vec3f pos;
			vec4f color;
		};
		
		bool					m_isEnabled;
		std::vector<VertDesc>	m_vertices;

		Ptr<IBuffer>			m_VB;
		VertexLayoutID			m_VL;
		Ptr<IShader>			m_shader;
		DepthStencilStateID		m_stateDS;
		RasterizerStateID		m_stateR;
	};

} // render
} // brUGE