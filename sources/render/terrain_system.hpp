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

		//-- Properties per terrain section.
		struct PerTerrainSectorProps
		{
			vec4f m_posOffset;
			vec4f m_texOffset;
		};

		//-- LOD Bridges types for seamless joining chunks with different LOD.
		//-- Note: The joining process is one-sided. It means that current terrain implementation
		//--	   supports seamless joining only for most detailed LOD to less detailed LOD.
		//------------------------------------------------------------------------------------------
		enum ELODBridges
		{
			LOD_BRIDGE_TOP	  = 1 << 0,
			LOD_BRIDGE_BOTTOM = 1 << 1,
			LOD_BRIDGE_LEFT	  = 1 << 2,
			LOD_BRIDGE_RIGHT  = 1 << 3
		};

	public:
		TerrainSystem();
		~TerrainSystem();

		bool init();
		bool temporal_hardcoded_load();
		//bool load(const pugi::xml_node& section);
		uint gatherROPs(Renderer::EPassType pass, RenderOps& rops, const mat4f& viewPort, const vec3f& camPos);

		//-- ToDo: reconsider interface to physics intercommunications.

	private:
		//-- Useful constants.
		//------------------------------------------------------------------------------------------
		enum
		{
			//-- maximum LOD levels for each terrain chunk.
			CHUNK_LODS_COUNT   = 5,

			//-- maximum number of index buffer for each LOD for seamless transition from one LOD
			//-- to another.
			//-- Note: transition may be only from more detailed to less detailed.
			//--	   Backward transition not allowed.
			CHUNK_LODS_BRIDGES = 16,
		};

		bool  buildHeightAndNormalTables(const utils::ROData& rawData);
		bool  buildIBs();
		bool  buildSharedVB();
		bool  buildSector(const vec2f& worldXZPos, const vec2us& mapPos, const AABB& aabb, uint16 index);
		bool  allocateSectors();
		uint8 generateBridgeMask(const vec2us& chunkPos, uint8 LOD);
		float readHeight(uint16 mapX, uint16 mapY);
		vec3f readNormal(uint16 mapX, uint16 mapY);

	private:

		//-- The minimum quantum of the terrain systemo.
		struct TerrainSector
		{
			TerrainSector() : m_index(0), m_LOD(0), m_chunkPos(0,0), m_worldPos(0,0)
			{
				m_VBs[0] = nullptr;
				m_VBs[1] = nullptr;
			}

			uint16   m_index;    //-- index of the vertex buffer.
			uint8	 m_LOD;		 //-- current LOD for this sector based on the current camera position.
			vec2us	 m_chunkPos; //-- position as indices on the terrain chunk system.
			vec2f    m_worldPos; //-- world pos on the XZ plane.
			AABB     m_aabb;     //-- bounds is in world space.

			//-- render data.
			PerTerrainSectorProps	m_props;
			IBuffer*				m_VBs[2];
		};

		std::vector<TerrainSector>				m_sectors;
		uint16									m_sectorsCount;  //-- count per dimension. I.e. row and column size is equal.
		uint8									m_sectorSize;	//-- size in cells of the sector 1-128
		uint8									m_sectorVerts;   //-- size in vertex's count per sector = m_sectorSize + 1
		float									m_unitsPerCell;  //-- units per sector cell in meters.
		float									m_heightUnits;   //-- units for height value.
		float									m_LODDistances[CHUNK_LODS_COUNT];
		float									m_sectorRadius;  //-- radius of the circumsphere around sector.
		AABB									m_aabb;			//-- the whole terrain AABB.
		
		//-- rendering data.
		EPrimitiveTopology						m_primTopology;
		std::shared_ptr<IBuffer>				m_IBLODs[CHUNK_LODS_COUNT][CHUNK_LODS_BRIDGES];
		std::shared_ptr<IBuffer>				m_sharedVB;
		std::vector<std::shared_ptr<IBuffer>>	m_uniqueVBs;
		std::vector<TerrainSector*>				m_visibleSectors;

		//-- ToDo: terrain data. May be it will be needed for physics.
		uint16									m_tableSize;   //-- size of the table in horizontal and vertical dims.
		std::vector<float>						m_heightTable;
		std::vector<vec3f>						m_normalTable;

		//-- Terrain materials.
		std::shared_ptr<PipelineMaterial>		m_material;
		RenderOps								m_ROPs;

		//-- ToDo:
		bool									m_loaded;
	};

} //-- render
} //-- brUGE