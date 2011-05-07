#pragma once

#include "prerequisites.hpp"
#include "render_system.hpp"
#include "math/Vector3.hpp"
#include "math/Vector4.hpp"
#include <vector>

namespace brUGE
{
	class Node;

namespace render
{
	class Mesh;
	class Material;

	//----------------------------------------------------------------------------------------------
	class DecalManager : public NonCopyable
	{
	public:
		DecalManager();
		~DecalManager();

		bool init();
		void update(float dt);
		void resolveVisibility();
		uint gatherRenderOps(RenderOps& ops) const;

		void addStaticDecal (const mat4f& orient, const vec3f& scale);
		void addDynamicDecal(const mat4f& orient, const vec3f& scale, const Node* node);

	private:
		struct DecalDesc
		{
			DecalDesc(const vec3f& dir, const vec3f& pos, const vec3f& up, const vec3f& scale)
				: m_dir(dir), m_pos(pos), m_up(up), m_scale(scale) { }

			vec3f m_dir;
			vec3f m_pos;
			vec3f m_up;
			vec3f m_scale;
		};

		struct GPUDecal
		{
			vec4f m_dir;
			vec4f m_pos;
			vec4f m_up;
			vec4f m_scale;
		};

		typedef std::vector<GPUDecal>				GPUDecals;
		typedef std::pair<DecalDesc, const Node*>	DynamicDecalDesc;
		typedef std::vector<DynamicDecalDesc>		DynamicDecalDescs;
		typedef std::vector<DecalDesc>				StaticDecalDescs;

		bool				m_updateStatic;
		bool				m_updateDynamic;

		GPUDecals			m_staticDecalsGPU;
		GPUDecals			m_dynamicDecalsGPU;

		StaticDecalDescs	m_staticDecalDescs;
		DynamicDecalDescs	m_dynamicDecalDescs;

		Ptr<IBuffer>		m_staticTB;
		Ptr<IBuffer>		m_dynamicTB;
		Ptr<Mesh>			m_unitCube;
		Ptr<Material>		m_material;
		RenderOps			m_ROPs;
	};

} //-- render
} //-- brUGE