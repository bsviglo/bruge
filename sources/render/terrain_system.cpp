#include "terrain_system.hpp"
#include "DebugDrawer.h"
#include "vertex_format.hpp"
#include "os/FileSystem.h"
#include "loader/ResourcesManager.h"
#include "console/Console.h"


using namespace brUGE;
using namespace brUGE::render;
using namespace brUGE::utils;
using namespace brUGE::math;
using namespace brUGE::os;


// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//-- console variables.
	bool g_enableCulling = true;
	bool g_showVisibilityBoxes = false;
	bool g_drawWireframe = false;


	//-- Code to work with normal maps was based on code from Mark J. Kilgard article
	//-- "A Practical and Robust Bump-mapping Technique for Today's GPUs"
	//----------------------------------------------------------------------------------------------
	void convertHeights2Normals(
		std::vector<vec3f>& oNormals, const std::vector<float>& heights,
		uint width, uint height, float scale)
	{
		oNormals.resize(width * height);

		uint offs = 0;

		for (uint i = 0; i < height; ++i)
		{
			for (uint j = 0; j < width; ++j)
			{
				float c  = heights[i * width + j];
				float cx = heights[i * width + (j + 1) % width];
				float cz = heights[((i + 1) % height) * width + j];

				//-- find derivatives.
				float dx = (c - cx) * scale;
				float dz = (c - cz) * scale;

				//-- normalize.
				float len = 1.0f / sqrtf(dx * dx + dz * dz + 1.0f);

				//-- get normal.
				oNormals[offs++].set(-dx * len, dz * len, len);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void filterNormals(
		std::vector<vec3f>& oNormals, const std::vector<vec3f>& iNormals, uint width, uint height
		)
	{
		vec2i offs[9] =
		{
			vec2i(+0, +0),
			vec2i(-1, -1),
			vec2i(+1, +1),
			vec2i(+1, +0),
			vec2i(+0, +1),
			vec2i(-1, +1),
			vec2i(+1, -1),
			vec2i(-1, +0),
			vec2i(+0, -1)
		};

		for (uint i = 0; i < height; ++i)
		{
			for (uint j = 0; j < width; ++j)
			{
				uint  centralIndex = i * width + j;
				vec3f filterNormal;

				for (uint k = 0; k < 9; ++k)
				{
					int index = (i + offs[k].x) * width + (j + offs[k].y);
					
					filterNormal += iNormals[(index > 0 && index < int(height * width)) ? index : centralIndex];
				}
				filterNormal.normalize();

				oNormals[centralIndex] = filterNormal;
			}
		}
	}

	//-- Create index buffer with desired size (xVerts, yVerts) and desired LOD (xStep, yStep).
	//-- xVerts and yVerts present number of vertices per width and height for current LOD.
	//-- xStep and yStep present how many vertices we will skip in this LOD between two grid nodes.
	//-- stride is number of nodes per row terrain's sector at the 0 LOD.
	//----------------------------------------------------------------------------------------------
	Ptr<IBuffer> createSingleStripGrid(
		uint16 xVerts, uint16 yVerts, uint16 xStep, uint16 yStep, uint16 stride)
	{
		uint totalStrips		  = yVerts - 1;
		uint totalIndexesPerStrip = xVerts * 2;

		//-- the total number of indices is equal to the number of strips times the indices used
		//-- per strip plus one degenerate triangle between each strip.
		uint totalIndexes = (totalIndexesPerStrip * totalStrips) + (totalStrips * 2) - 2;

		std::vector<uint16> indices(totalIndexes);

		uint16* index	  = &indices[0];
		uint16  startVert = 0;
		uint16  lineStep  = yStep * stride;

		for (uint j = 0; j < totalStrips; ++j)
		{
			uint32 k = 0;
			uint16 vert = startVert;

			//-- create a strip for this row
			for (k = 0; k < xVerts; ++k)
			{
				*(index++) = vert;
				*(index++) = vert + lineStep;
				vert += xStep;
			}
			startVert += lineStep;

			if (j + 1 < totalStrips)
			{
				//-- add a degenerate to attach to the next row.
				*(index++) = (vert - xStep) + lineStep;
				*(index++) = startVert;
			}
		}

		// finally, use the indices we created above to fill index buffer.
		return rd()->createBuffer(
			IBuffer::TYPE_INDEX,  &indices[0],  indices.size(), sizeof(uint16)
			);
	}

	//----------------------------------------------------------------------------------------------
	class PerTerrainSectorProperty : public IProperty
	{
	public:
		PerTerrainSectorProperty(ShaderContext& sc) : m_sc(sc) { }
		virtual ~PerTerrainSectorProperty() { }

		virtual bool operator () (Handle handle, IShader& shader) const
		{
			return shader.setUniformBlock(
				handle, m_sc.renderOp().m_userData, sizeof(TerrainSystem::PerTerrainSectorProps)
				);
		}

		virtual Handle handle(const char* name, const IShader& shader) const
		{
			return shader.getHandleUniformBlock(name);
		}

	private:
		ShaderContext& m_sc;
	};

}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	TerrainSystem::TerrainSystem()
		:	m_loaded(false),
			m_sectorsCount(0),
			m_sectorSize(0),
			m_sectorVerts(0),
			m_unitsPerCell(1.0f),
			m_heightUnits(1.0f),
			m_tableSize(0)
	{

	}

	//----------------------------------------------------------------------------------------------
	TerrainSystem::~TerrainSystem()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool TerrainSystem::init()
	{
		REGISTER_CONSOLE_VALUE("r_terrain_show_visibility_boxes",	bool, g_showVisibilityBoxes);
		REGISTER_CONSOLE_VALUE("r_terrain_enable_culling",			bool, g_enableCulling);
		REGISTER_CONSOLE_VALUE("r_terrain_draw_wireframe",			bool, g_drawWireframe);
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool TerrainSystem::temporal_hardcoded_load()
	{
		//-- set parameters.
		m_tableSize	   = 1025;
		m_sectorSize   = 64;
		m_sectorsCount = (m_tableSize / m_sectorSize);
		m_sectorVerts  = m_sectorSize + 1;
		m_unitsPerCell = 1.0f;
		m_heightUnits  = 75.0f;

		//-- load terrain material.
		{
			RODataPtr file = FileSystem::instance().readFile("resources/materials/terrain.mtl");
			if (!file || !(m_material = rs().materials().createPipelineMaterial(*file)))
			{
				return false;
			}

			//-- add system terrain properties.
			std::unique_ptr<PerTerrainSectorProperty> prop(
				new PerTerrainSectorProperty(rs().shaderContext())
				);

			m_material->addProperty("cb_PerTerrainSector", prop.release());
		}

		//-- load terrain height map.
		{
			RODataPtr file = FileSystem::instance().readFile("resources/textures/terrain/terrain.raw");
			if (!file)
				return false;

			if (!buildHeightAndNormalTables(*file.get()))
				return false;
		}

		//-- allocate terrain sectors.
		if (!allocateSectors())
			return false;

		//-- build shared VB.
		if (!buildSharedVB())
			return false;

		//-- build indices buffer.
		if (!buildIBs())
			return false;

		m_loaded = true;

		return true;
	}

	/*
	//----------------------------------------------------------------------------------------------
	bool TerrainSystem::load(const pugi::xml_node& section)
	{
		//-- ToDo: implement.
		return true;
	}
	*/

	//----------------------------------------------------------------------------------------------
	uint TerrainSystem::gatherROPs(RenderSystem::EPassType pass, RenderOps& rops, const mat4f& viewPort)
	{
		if (!m_loaded)
		{
			return 0;
		}

		//-- ToDo:
		m_material->rsProps().m_wireframe = g_drawWireframe;

		const RenderFx* fx = m_material->renderFx(rs().shaderPass(pass), false);

		for (auto iter = m_sectors.begin(); iter != m_sectors.end(); ++iter)
		{
			//-- 1. cull frustum against AABB.
			if (g_enableCulling && iter->m_aabb.calculateOutcode(viewPort) != 0)
			{
				continue;
			}
			else if (g_showVisibilityBoxes)
			{
				DebugDrawer::instance().drawAABB(iter->m_aabb, Color(0,0,1,0));
			}

			//-- ToDo:
			//-- set vertex buffers.
			iter->m_VBs[0] = m_sharedVB.get();
			iter->m_VBs[1] = m_uniqueVBs[iter->m_index].get();

			//-- ToDo:
			//-- add ROPs.
			{
				RenderOp rop;

				rop.m_primTopolpgy = PRIM_TOPOLOGY_TRIANGLE_STRIP;
				rop.m_IB		   = m_IBLODs[0].get();
				rop.m_indicesCount = rop.m_IB->getElemCount();
				rop.m_VBs		   = iter->m_VBs;
				rop.m_VBCount	   = 2;
				rop.m_material	   = fx;
				rop.m_userData	   = &iter->m_props;

				rops.push_back(rop);
			}
		}

		return rops.size();
	}

	//----------------------------------------------------------------------------------------------
	bool TerrainSystem::buildHeightAndNormalTables(const ROData& rawData)
	{
		//-- heights map need to be power of two.
		//assert(rawData.length() % 2 == 0);

		uint size = rawData.length() / 2;

		std::vector<float> normalizedHeightTable(size);
		m_heightTable.resize(size);
		m_normalTable.resize(size);

		//-- 1. create heights table.
		for (uint i = 0; i < size; ++i)
		{
			//-- 1. extract height as a byte value.
			uint16 heightAsByte = *static_cast<const uint16*>(rawData.ptr(i*2));

			//-- 2. normalize it and convert to range [-1, 1]
			float heightAsNormalizedFloat = (heightAsByte - 32768) / 65536.0f;

			normalizedHeightTable[i] = 0.5f + 0.5f * heightAsNormalizedFloat;
			
			//-- 3. scale it by desired height units and store in the height table.
			m_heightTable[i] = heightAsNormalizedFloat * m_heightUnits;
		}

		//-- 2. create normals table.
		std::vector<vec3f> normals;
		normals.resize(size);

		convertHeights2Normals(
			m_normalTable, normalizedHeightTable, m_tableSize, m_tableSize, m_tableSize
			);

		filterNormals(normals, m_normalTable, m_tableSize, m_tableSize);
		filterNormals(m_normalTable, normals, m_tableSize, m_tableSize);

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool TerrainSystem::allocateSectors()
	{
		m_sectors.resize(m_sectorsCount * m_sectorsCount);
		m_uniqueVBs.resize(m_sectorsCount * m_sectorsCount);

		//-- The center of the terrain located in the position (0,0,0), so we iterate over the whole
		//-- set of the terrain sectors and started from the far left corner on the XZ plane.
		//-- In the left handed coordinate system it has position (-1,-1).
		float  sectorUnitsSize = m_sectorSize * m_unitsPerCell;

		vec2us farLeftMapCorner(0, 0);
		vec2f  farLeftWorldCorner(
			-1.0f * (m_sectorsCount / 2) * sectorUnitsSize,
			-1.0f * (m_sectorsCount / 2) * sectorUnitsSize
			);

		uint16 index = 0;

		//-- create the sector objects themselves.
		for (uint16 y = 0; y < m_sectorsCount; ++y)
		{
			for (uint16 x = 0; x < m_sectorsCount; ++x)
			{
				//-- world position of the sector on the XZ plane.
				vec2f sectorWorldPos(
					farLeftWorldCorner.x + (x * sectorUnitsSize),
					farLeftWorldCorner.y + (y * sectorUnitsSize)
					);

				//-- world space AABB of the sector, but without y dimension. It will be calculated
				//-- later when we will being create vertex buffer for this sector.
				AABB sectorAABB(
					vec3f(sectorWorldPos.x, 0.0f, sectorWorldPos.y),
					vec3f(sectorWorldPos.x + sectorUnitsSize, 0.0f, sectorWorldPos.y + sectorUnitsSize)
					);

				//-- height map space position of the sector.
				vec2us sectorMapPos(
					farLeftMapCorner.x + x * m_sectorSize,
					farLeftMapCorner.y + y * m_sectorSize
					);

				if (!buildSector(sectorWorldPos, sectorMapPos, sectorAABB, index++))
					return false;
			}
		}

		return true;
	}

	//-- create the vertex buffer shared by the sectors.
	//----------------------------------------------------------------------------------------------
	bool TerrainSystem::buildSharedVB()
	{
		float oneDivSector = 1.0f / (m_sectorVerts - 1);
		vec2f vert(0.0f,0.0f);
		std::vector<VertexXZUV> vertices(m_sectorVerts * m_sectorVerts);

		//-- fill the vertex stream with x,y positions and uv-coordinates. All other data
		//-- (height and surface normals) are stored in the vertex buffers of each terrain section.
		for (uint x = 0; x < m_sectorVerts; ++x)
		{
			vert.set(x * m_unitsPerCell, 0.0f);

			for (uint z = 0; z < m_sectorVerts; ++z)
			{
				VertexXZUV& oVert = vertices[(x * m_sectorVerts) + z];

				oVert.m_xz = vert;
				oVert.m_uv = vec2f(z * oneDivSector, x * oneDivSector);

				vert.y += m_unitsPerCell;
			}
		}

		//-- now that we have built the data, create vertex buffer.
		m_sharedVB = rd()->createBuffer(
			IBuffer::TYPE_VERTEX, &vertices[0], vertices.size(),  sizeof(VertexXZUV)
			);

		return m_sharedVB.isValid();
	}

	//-- create the unique vertex buffer for the desired sector.
	//----------------------------------------------------------------------------------------------
	bool TerrainSystem::buildSector(
		const vec2f& worldXZPos, const vec2us& mapPos, const AABB& aabb, uint16 index)
	{
		//-- container for the unique VB data.
		std::vector<VertexYN> vertices(m_sectorVerts * m_sectorVerts);

		TerrainSector tSector;
		tSector.m_index    = index;
		tSector.m_worldPos = worldXZPos;
		tSector.m_aabb	   = aabb;

		//-- read in the height and normal for each vertex
		for (uint16 x = 0; x < m_sectorVerts; ++x)
		{
			for (uint16 z = 0; z < m_sectorVerts; ++z)
			{
				VertexYN& oVert = vertices[(x * m_sectorVerts) + z];

				//-- create vertex with desired height and normal.
				oVert.m_y	   = readHeight(mapPos.x + x, mapPos.y + z);
				oVert.m_normal = readNormal(mapPos.x + x, mapPos.y + z);

				//-- find the bound of the sector height.
				tSector.m_aabb.m_min.y = min(tSector.m_aabb.m_min.y, oVert.m_y);
				tSector.m_aabb.m_max.y = max(tSector.m_aabb.m_max.y, oVert.m_y);
			}
		}
		
		tSector.m_props.m_texOffset = vec4f(
			mapPos.y / m_sectorSize,
			mapPos.x / m_sectorSize,
			1.0f / m_sectorsCount,
			1.0f / m_sectorsCount
			);
		tSector.m_props.m_posOffset = vec4f(worldXZPos.x, worldXZPos.y, 0, 0);

		m_uniqueVBs[index] = rd()->createBuffer(
			IBuffer::TYPE_VERTEX, &vertices[0], vertices.size(),  sizeof(VertexYN)
			);

		//-- add a new sector to the sectors list.
		m_sectors[index] = tSector;

		return m_uniqueVBs[index].isValid();
	}

	//----------------------------------------------------------------------------------------------
	bool TerrainSystem::buildIBs()
	{
		m_IBLODs[0] = createSingleStripGrid(m_sectorVerts, m_sectorVerts, 1, 1, m_sectorVerts);

		if (!m_IBLODs[0].isValid())
			return false;

		return true;
	}

	//----------------------------------------------------------------------------------------------
	float TerrainSystem::readHeight(uint16 mapX, uint16 mapY)
	{
		if (mapX >= m_tableSize) mapX = m_tableSize - 1;
		if (mapY >= m_tableSize) mapY = m_tableSize - 1;

		return m_heightTable[(mapX * m_tableSize) + mapY];
	}

	//----------------------------------------------------------------------------------------------
	vec3f TerrainSystem::readNormal(uint16 mapX, uint16 mapY)
	{
		if (mapX >= m_tableSize) mapX = m_tableSize - 1;
		if (mapY >= m_tableSize) mapY = m_tableSize - 1;

		return m_normalTable[(mapX * m_tableSize) + mapY];
	}

} //-- render
} //-- brUGE