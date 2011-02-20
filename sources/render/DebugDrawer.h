#pragma once

#include "render_common.h"
#include "utils/Singleton.h"
#include "Color.h"
#include "render_system.hpp"
#include <vector>

namespace brUGE
{
namespace render
{
	class Mesh;
	class Font;

	//-- Debug information drawer.
	//-- It gathers drawing information from all its drawing methods and then draw all this
	//-- information as effective as possible.
	//---------------------------------------------------------------------------------------------
	class DebugDrawer : public utils::Singleton<DebugDrawer>
	{
	public:
		DebugDrawer();
		~DebugDrawer() {}

		bool init();
		void destroy();

		//-- wire meshes.
		void drawLine		(const vec3f& start, const vec3f& end, const Color& color);
		void drawCoordAxis	(const mat4f& orient, float scale = 1.0f);
		
		void drawFrustum	(const Color& color, const mat4f& orient, float	fov, float aspect, float nearDist, float farDist);
		void drawFrustum	(const mat4f& orient, const Projection& projInfo, float aspect);
		void drawFrustum	(const mat4f& orient, const mat4f& projMat);

		void drawAABB		(const AABB& aabb, const Color& color);
		void drawAABB		(const AABB& aabb, const Color& color, const mat4f& transform);

		void drawOBB		(const OBB& obb, const Color& color);

		//-- solid meshes.
		void drawBox		(const vec3f& size, const mat4f& world, const Color& color);
		void drawSphere		(float radius, const mat4f& world, const Color& color);
		void drawCylinderX	(float radius, float height, const mat4f& world, const Color& color);
		void drawCylinderY	(float radius, float height, const mat4f& world, const Color& color);
		void drawCylinderZ	(float radius, float height, const mat4f& world, const Color& color);
		void drawCapsuleX	(float radius, float height, const mat4f& world, const Color& color);
		void drawCapsuleY	(float radius, float height, const mat4f& world, const Color& color);
		void drawCapsuleZ	(float radius, float height, const mat4f& world, const Color& color);

		//-- text drawing.
		void drawText2D		(const char* text, const vec3f& pos, const Color& color);
		
		//-- this method performs actual drawing.
		void draw();

	private:
		void _swapBuffers();
		
		//-- console functions.
		int _drawDebugInfo(bool);

	private:
		bool						m_isEnabled;

		//-- wire meshes drawing.
		struct VertDesc
		{
			VertDesc(const vec3f& pos_, const vec4f& color_) : pos(pos_), color(color_) {}

			vec3f pos;
			vec4f color;
		};

		std::vector<VertDesc>		m_vertices;
		Ptr<IBuffer>				m_VB;
		Ptr<Material>				m_wireMaterial;
		RenderOps					m_wireROPs;

		//-- text drawing
		struct TextData
		{
			vec3f		m_pos;
			std::string m_text;
			Color		m_color;
		};
		Ptr<Font>				m_font;
		std::vector<TextData>	m_textDataVec;

		//-- standard solid meshes like (sphere, box, cylinder, ...)
		enum MeshTypes
		{
			MT_BOX,
			MT_SPHERE,
			MT_CYLINDER,
			MT_COUNT
		};

		struct MeshInstance
		{
			MeshInstance(const mat4f& world, const Color& color)
				: m_world(world), m_color(color) { }

			mat4f m_world;
			Color m_color;
		};
		std::vector<MeshInstance>	m_meshCaches[MT_COUNT];
		Ptr<Mesh>					m_meshes[MT_COUNT];
		Ptr<IBuffer>				m_instancingTB;
	};

} // render
} // brUGE