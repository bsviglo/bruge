#pragma once

#include "prerequisites.hpp"
#include "render_system.hpp"
#include "math/Vector3.hpp"
#include "math/Vector4.hpp"
#include "utils/Data.hpp"
#include <vector>

namespace brUGE
{
namespace render
{

	class PipelineMaterial;


	//----------------------------------------------------------------------------------------------
	class TerrainSystem : public NonCopyable
	{
	public:
		struct PerTerrainSectorProps
		{
			vec4f m_posOffset;
			vec4f m_texOffset;
		};

	public:
		TerrainSystem();
		~TerrainSystem();

		bool init();
		bool temporal_hardcoded_load();
		//bool load(const pugi::xml_node& section);
		uint gatherROPs(RenderSystem::EPassType pass, RenderOps& rops, const mat4f& viewPort);

		//-- ToDo: reconsider interface to physics intercommunications.

	private:
		bool  buildHeightAndNormalTables(const utils::ROData& rawData);
		bool  buildIBs();
		bool  buildSharedVB();
		bool  buildSector(const vec2f& worldXZPos, const vec2us& mapPos, const AABB& aabb, uint16 index);
		bool  allocateSectors();
		float readHeight(uint16 mapX, uint16 mapY);
		vec3f readNormal(uint16 mapX, uint16 mapY);

	private:

		enum
		{
			CHUNK_LODS_COUNT = 5
		};

		//-- The minimum quantum of the terrain.
		struct TerrainSector
		{
			TerrainSector() : m_index(0) { m_VBs[0] = nullptr; m_VBs[1] = nullptr; }

			uint16   m_index;
			vec2f    m_worldPos; //-- world pos on the XZ plane.
			AABB     m_aabb;     //-- in world space.

			//-- render data.
			PerTerrainSectorProps	m_props;
			IBuffer*				m_VBs[2];
		};

		std::vector<TerrainSector> m_sectors;
		uint16					   m_sectorsCount;  //-- count per dimension. I.e. row and column size is equal.
		uint8					   m_sectorSize;	//-- size in cells of the sector 1-128
		uint8					   m_sectorVerts;   //-- vertices size per sector m_sectorSize + 1
		float					   m_unitsPerCell;  //-- units per sector cell in meters.
		float					   m_heightUnits;   //-- units for height value.
		
		Ptr<IBuffer>				m_IBLODs[CHUNK_LODS_COUNT];
		Ptr<IBuffer>				m_sharedVB;
		std::vector<Ptr<IBuffer>>	m_uniqueVBs;

		//-- ToDo: terrain data. May be it will be needed for physics.
		uint16						m_tableSize;   //-- size of the table in horizontal and vertical dims.
		std::vector<float>			m_heightTable;
		std::vector<vec3f>			m_normalTable;

		//-- Terrain materials.
		Ptr<PipelineMaterial>		m_material;
		RenderOps					m_ROPs;

		//-- ToDo:
		bool						m_loaded;
	};

} //-- render
} //-- brUGE