#include "terrain_system.hpp"
#include "DebugDrawer.h"
#include "vertex_format.hpp"
#include "os/FileSystem.h"
#include "loader/ResourcesManager.h"

//-- ToDo: reconsider.
#include "engine/Engine.h"
#include "physics/physic_world.hpp"


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
	bool g_enableLODSystem = true;


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


	//-- The same as createSingleStripGrid but creates triangles list.
	//----------------------------------------------------------------------------------------------
	std::shared_ptr<IBuffer> createSingleListGrid(
		uint16 xVerts, uint16 yVerts, uint16 xStep, uint16 yStep, uint16 stride, uint8 bridges)
	{
		//-- the total number of indices is equal to the grid nodes count i.e. (xVerts - 1) * (yVerts - 1)
		//-- multiplied by 6, because each node consists of the two triangles, each triangle has 3
		//-- vertices.
		uint16 xTris = xVerts - 1;
		uint16 yTris = yVerts - 1;
		uint16 totalIndexes = xTris * yTris * 6;

		std::vector<uint16> indices(totalIndexes);

		uint16* index	  = &indices[0];
		uint16  startVert = 0;
		uint16  lineStep  = yStep * stride;

		for (uint16 i = 0; i < xTris; ++i)
		{
			uint16 vert = startVert;

			for (uint16 j = 0; j < yTris; ++j)
			{
				*(index++) = vert;
				*(index++) = vert + xStep;
				*(index++) = vert + lineStep;

				*(index++) = vert + lineStep;
				*(index++) = vert + xStep;
				*(index++) = vert + lineStep + xStep;

				vert += xStep;
			}
			startVert += lineStep;
		}

		//-- Helpers class to simplify LOD trimer.
		//-- Represent grid of nodes. Each node consists of two triangles. Each triangle of 3 indices.
		//------------------------------------------------------------------------------------------
		struct TrianglesGrid
		{
			struct Node
			{
				struct Tri
				{
					vec3us m_index;
				};

				Tri	m_tries[2];
			};

			TrianglesGrid(uint16* indices, uint16 width, uint16 height)
				:	m_nodes(reinterpret_cast<Node*>(indices)), m_width(width), m_height(height)
			{

			}

			Node&		operator() (uint16 i, uint16 j)		  { return m_nodes[i * m_width + j]; }
			const Node& operator() (uint16 i, uint16 j) const { return m_nodes[i * m_width + j]; }

			uint16 m_width;
			uint16 m_height;
			Node*  m_nodes; //-- array of grid nodes.
		};
		TrianglesGrid triGrid(&indices[0], yTris, xTris);

		//-- trim LOD-es.
		//--
		//-- Triangle indices order.
		//-- 0x means triangle 0, index 0. i.e. m_tries[0].m_index.x
		//--
		//-- 0x		 0y (1y)
		//--  ----------
		//--  |		  /|
		//--  |	    /  |	
		//--  |   /	   |
		//--  | /	   |
		//--  ----------
		//-- 0z (1x)	 1z
		//------------------------------------------------------------------------------------------
		{
			//--trim top corner
			if (bridges & TerrainSystem::LOD_BRIDGE_TOP)
			{
				for (uint16 j = 0; j < triGrid.m_width - 1; j += 2)
				{
					TrianglesGrid::Node& node0 = triGrid(0, j);
					TrianglesGrid::Node& node1 = triGrid(0, j + 1);

					node0.m_tries[0].m_index.y = node0.m_tries[1].m_index.z; //-- revert order triangle.
					node0.m_tries[1].m_index.x = node0.m_tries[0].m_index.x; //-- the big...
					node0.m_tries[1].m_index.y = node1.m_tries[0].m_index.y; //-- triangle.
					node1.m_tries[0].m_index.x = node1.m_tries[0].m_index.z; //-- degenerate
				}
			}
			if (bridges & TerrainSystem::LOD_BRIDGE_BOTTOM)
			{
				for (uint16 j = 1; j < triGrid.m_width; j += 2)
				{
					TrianglesGrid::Node& node0 = triGrid(triGrid.m_height - 1, j);
					TrianglesGrid::Node& node1 = triGrid(triGrid.m_height - 1, j - 1);

					node0.m_tries[1].m_index.x = node0.m_tries[0].m_index.x; //-- revert order triangle.
					node0.m_tries[0].m_index.y = node0.m_tries[1].m_index.z; //-- the big...
					node0.m_tries[0].m_index.z = node1.m_tries[1].m_index.x; //-- triangle.
					node1.m_tries[1].m_index.y = node1.m_tries[1].m_index.z; //-- degenerate
				}
			}
			if (bridges & TerrainSystem::LOD_BRIDGE_LEFT)
			{
				uint16 offset = 0;
				if (bridges & TerrainSystem::LOD_BRIDGE_TOP)
				{
					TrianglesGrid::Node& node0 = triGrid(0, 0);
					TrianglesGrid::Node& node1 = triGrid(1, 0);

					node0.m_tries[0].m_index.z = node1.m_tries[0].m_index.z; //-- the big triangle.
					node1.m_tries[0].m_index.y = node1.m_tries[0].m_index.x; //-- degenerate.

					offset = 2;
				}

				for (uint16 i = offset; i < triGrid.m_height - 1; i += 2)
				{
					TrianglesGrid::Node& node0 = triGrid(i, 0);
					TrianglesGrid::Node& node1 = triGrid(i + 1, 0);

					node0.m_tries[0].m_index.z = node0.m_tries[1].m_index.z; //-- revert order triangle.
					node0.m_tries[1].m_index.y = node0.m_tries[0].m_index.x; //-- the big...
					node0.m_tries[1].m_index.x = node1.m_tries[0].m_index.z; //-- triangle.
					node1.m_tries[0].m_index.x = node1.m_tries[0].m_index.y; //-- degenerate
				}
			}
			if (bridges & TerrainSystem::LOD_BRIDGE_RIGHT)
			{
				uint16 offset = 0;
				if (bridges & TerrainSystem::LOD_BRIDGE_BOTTOM)
				{
					TrianglesGrid::Node& node0 = triGrid(triGrid.m_height - 1, triGrid.m_width - 1);
					TrianglesGrid::Node& node1 = triGrid(triGrid.m_height - 2, triGrid.m_width - 1);

					node0.m_tries[1].m_index.y = node1.m_tries[1].m_index.y; //-- the big triangle.
					node1.m_tries[1].m_index.x = node1.m_tries[1].m_index.z; //-- degenerate.

					offset = 1;
				}

				for (uint16 i = 1; i < triGrid.m_height - offset; i += 2)
				{
					TrianglesGrid::Node& node0 = triGrid(i, triGrid.m_width - 1);
					TrianglesGrid::Node& node1 = triGrid(i - 1, triGrid.m_width - 1);

					node0.m_tries[1].m_index.y = node0.m_tries[0].m_index.x; //-- revert order triangle.
					node0.m_tries[0].m_index.z = node0.m_tries[1].m_index.z; //-- the big...
					node0.m_tries[0].m_index.y = node1.m_tries[1].m_index.y; //-- triangle.
					node1.m_tries[1].m_index.x = node1.m_tries[1].m_index.z; //-- degenerate
				}
			}
		}

		// finally, use the indices we created above to fill index buffer.
		return rd()->createBuffer(
			IBuffer::TYPE_INDEX,  &indices[0],  indices.size(), sizeof(uint16)
			);
	}

	//-- Create index buffer with desired size (xVerts, yVerts) and desired LOD (xStep, yStep).
	//-- xVerts and yVerts present number of vertices per width and height for current LOD.
	//-- xStep and yStep present how many vertices we will skip in this LOD between two grid nodes.
	//-- stride is number of nodes per row terrain's sector at the 0 LOD.
	//-- Warning: trimmer currenly not implemented.
	//-- ToDo: implement LOD trimmer.
	//----------------------------------------------------------------------------------------------
	std::shared_ptr<IBuffer> createSingleStripGrid(
		uint16 xVerts, uint16 yVerts, uint16 xStep, uint16 yStep, uint16 stride, uint8 /*bridges*/)
	{
		uint16 totalStrips		  = yVerts - 1;
		uint16 totalIndexesPerStrip = xVerts * 2;

		//-- the total number of indices is equal to the number of strips times the indices used
		//-- per strip plus one degenerate triangle between each strip.
		uint16 totalIndexes = (totalIndexesPerStrip * totalStrips) + (totalStrips * 2) - 2;

		std::vector<uint16> indices(totalIndexes);

		uint16* index	  = &indices[0];
		uint16  startVert = 0;
		uint16  lineStep  = yStep * stride;

		for (uint16 j = 0; j < totalStrips; ++j)
		{
			uint16 k = 0;
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

		//-- ToDo: implement trimmer.

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
			m_tableSize(0),
			m_primTopology(PRIM_TOPOLOGY_TRIANGLE_STRIP)
	{

	}

	//----------------------------------------------------------------------------------------------
	TerrainSystem::~TerrainSystem()
	{
		Engine::instance().physicsWorld().removeTerrainPhysicsObject();
	}

	//----------------------------------------------------------------------------------------------
	bool TerrainSystem::init()
	{
		REGISTER_CONSOLE_VALUE("r_terrain_show_visibility_boxes",	bool, g_showVisibilityBoxes);
		REGISTER_CONSOLE_VALUE("r_terrain_enable_culling",			bool, g_enableCulling);
		REGISTER_CONSOLE_VALUE("r_terrain_draw_wireframe",			bool, g_drawWireframe);
		REGISTER_CONSOLE_VALUE("r_terrain_enable_LODs",				bool, g_enableLODSystem);
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool TerrainSystem::temporal_hardcoded_load()
	{
		//-- set parameters.
		//-- ToDo: move it to the terrain cfg file.
		{
			m_tableSize	   = 1025;
			m_sectorSize   = 64;
			m_sectorsCount = (m_tableSize / m_sectorSize);
			m_sectorVerts  = m_sectorSize + 1;
			m_unitsPerCell = 1.0f;
			m_heightUnits  = 75.0f;

			//-- calculate radius for sector.
			float halfSize = m_sectorSize * m_unitsPerCell * 0.5f;
			m_sectorRadius = sqrtf(2.0f * halfSize * halfSize);
		
			for (uint i = 0; i < CHUNK_LODS_COUNT; ++i)
			{
				m_LODDistances[i] = halfSize * 2.0f * (i + 1);
			}
		}

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

		//-- build shared VB.
		if (!buildSharedVB())
			return false;

		//-- build indices buffer.
		if (!buildIBs())
			return false;

		//-- allocate terrain sectors.
		if (!allocateSectors())
			return false;

		//-- ToDo: reconsider much more generalized form of terrain<->physics comunication.
		bool success = Engine::instance().physicsWorld().createTerrainPhysicsObject(
			m_tableSize, m_unitsPerCell, &m_heightTable[0], 1.0f,
			m_aabb.m_min.y, m_aabb.m_max.y
			);

		if (!success)
		{
			return false;
		}

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
	uint TerrainSystem::gatherROPs(
		RenderSystem::EPassType pass, RenderOps& rops, const mat4f& viewPort, const vec3f& camPos)
	{
		//-- ToDo:
		if (!m_loaded)
		{
			return 0;
		}

		//-- ToDo:
		m_material->rsProps().m_wireframe = g_drawWireframe;

		const RenderFx* fx = m_material->renderFx(rs().shaderPass(pass), false);

		//-- resolve visibility.
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

			//-- select LOD
			if (g_enableLODSystem)
			{
				float dist	= camPos.flatDist(iter->m_aabb.getCenter());
				iter->m_LOD = CHUNK_LODS_COUNT - 1;

				for (uint i = 0; i < CHUNK_LODS_COUNT; ++i)
				{
					if (dist < m_LODDistances[i])
					{
						iter->m_LOD = i;
						break;
					}
				}
			}
			else
			{
				iter->m_LOD = 0;
			}

			m_visibleSectors.push_back(&*iter);
		}

		//-- prepare ROPs.
		for (auto iter = m_visibleSectors.begin(); iter != m_visibleSectors.end(); ++iter)
		{
			TerrainSector& ts = *(*iter);

			uint8 mask = 0;
			if (g_enableLODSystem)
			{
				mask = generateBridgeMask(ts.m_chunkPos, ts.m_LOD);
			}

			//-- add ROPs.
			{
				RenderOp rop;

				rop.m_primTopolpgy = m_primTopology;
				rop.m_IB		   = m_IBLODs[ts.m_LOD][mask].get();
				rop.m_indicesCount = rop.m_IB->getElemCount();
				rop.m_VBs		   = ts.m_VBs;
				rop.m_VBCount	   = 2;
				rop.m_material	   = fx;
				rop.m_userData	   = &ts.m_props;

				rops.push_back(rop);
			}
		}

		//-- clear visible sectors.
		m_visibleSectors.clear();

		return rops.size();
	}

	//-- Generate bridge mask by comparing our LOD value with all four neighbours.
	//----------------------------------------------------------------------------------------------
	uint8 TerrainSystem::generateBridgeMask(const vec2us& chunkPos, uint8 LOD)
	{
		uint8 bridgeMask = 0;
		int8 offsets[][3] = 
		{
			{-1, +0, LOD_BRIDGE_LEFT   },
			{+1, +0, LOD_BRIDGE_RIGHT  },
			{+0, -1, LOD_BRIDGE_TOP    },
			{+0, +1, LOD_BRIDGE_BOTTOM }
		};

		for (uint i = 0; i < 4; ++i)
		{
			uint8 mask    = offsets[i][2];
			int16 x       = clamp(0, chunkPos.x + offsets[i][0], m_sectorsCount - 1);
			int16 z       = clamp(0, chunkPos.y + offsets[i][1], m_sectorsCount - 1);
			uint8 nextLOD = m_sectors[z * m_sectorsCount + x].m_LOD;

			if (nextLOD - LOD == 1)
			{
				bridgeMask |= mask;
			}
		}
		
		return bridgeMask;
	}

	//----------------------------------------------------------------------------------------------
	bool TerrainSystem::buildHeightAndNormalTables(const ROData& rawData)
	{
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
		for (uint16 z = 0; z < m_sectorsCount; ++z)
		{
			for (uint16 x = 0; x < m_sectorsCount; ++x)
			{
				//-- world position of the sector on the XZ plane.
				vec2f sectorWorldPos(
					farLeftWorldCorner.x + (x * sectorUnitsSize),
					farLeftWorldCorner.y + (z * sectorUnitsSize)
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
					farLeftMapCorner.y + z * m_sectorSize
					);

				if (!buildSector(sectorWorldPos, sectorMapPos, sectorAABB, index++))
					return false;
			}
		}

		//-- calculate the whole terrain AABB.
		for (auto iter = m_sectors.cbegin(); iter != m_sectors.cend(); ++iter)
		{
			m_aabb.combine(iter->m_aabb);
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
		for (uint z = 0; z < m_sectorVerts; ++z)
		{
			vert.set(0.0f, z * m_unitsPerCell);

			for (uint x = 0; x < m_sectorVerts; ++x)
			{
				VertexXZUV& oVert = vertices[(z * m_sectorVerts) + x];

				oVert.m_xz = vert;
				oVert.m_uv = vec2f(x * oneDivSector, z * oneDivSector);

				vert.x += m_unitsPerCell;
			}
		}

		//-- now that we have built the data, create vertex buffer.
		m_sharedVB = rd()->createBuffer(
			IBuffer::TYPE_VERTEX, &vertices[0], vertices.size(),  sizeof(VertexXZUV)
			);

		return m_sharedVB.get() != nullptr;
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
		tSector.m_chunkPos = vec2us(mapPos.x / m_sectorSize, mapPos.y / m_sectorSize);

		//-- read in the height and normal for each vertex
		for (uint16 z = 0; z < m_sectorVerts; ++z)
		{
			for (uint16 x = 0; x < m_sectorVerts; ++x)
			{
				VertexYN& oVert = vertices[(z * m_sectorVerts) + x];

				//-- create vertex with desired height and normal.
				oVert.m_y	   = readHeight(mapPos.x + x, mapPos.y + z);
				oVert.m_normal = readNormal(mapPos.x + x, mapPos.y + z);

				//-- find the bound of the sector height.
				tSector.m_aabb.m_min.y = min(tSector.m_aabb.m_min.y, oVert.m_y);
				tSector.m_aabb.m_max.y = max(tSector.m_aabb.m_max.y, oVert.m_y);
			}
		}
		
		//-- create mask texture offset.
		tSector.m_props.m_texOffset = vec4f(
			mapPos.x / m_sectorSize,
			mapPos.y / m_sectorSize,
			1.0f / m_sectorsCount,
			1.0f / m_sectorsCount
			);
		//-- create terrain XZ offset in world space.
		tSector.m_props.m_posOffset = vec4f(worldXZPos.x, worldXZPos.y, 0, 0);

		m_uniqueVBs[index] = rd()->createBuffer(
			IBuffer::TYPE_VERTEX, &vertices[0], vertices.size(),  sizeof(VertexYN)
			);

		//-- set vertex buffers.
		tSector.m_VBs[0] = m_sharedVB.get();
		tSector.m_VBs[1] = m_uniqueVBs[index].get();

		//-- add a new sector to the sectors list.
		m_sectors[index] = tSector;

		return m_uniqueVBs[index].get() != nullptr;
	}

	//----------------------------------------------------------------------------------------------
	bool TerrainSystem::buildIBs()
	{
		const uint8 vertexSteps[] = { 1, 2, 4, 8, 16 };

		for (uint i = 0; i < CHUNK_LODS_COUNT; ++i)
		{
			uint8 step  = vertexSteps[i];
			uint8 verts = (step == 1) ? m_sectorVerts / step : (m_sectorVerts / step) + 1;

			for (uint j = 0; j < CHUNK_LODS_BRIDGES; ++j)
			{

//-- primitive topology. Currently TRIANGLE_STRIP topology not fully implemented.
#if 0
				m_primTopology = PRIM_TOPOLOGY_TRIANGLE_STRIP;
				m_IBLODs[i][j] = createSingleStripGrid(verts, verts, step, step, m_sectorVerts, j);
#else
				m_primTopology = PRIM_TOPOLOGY_TRIANGLE_LIST;
				m_IBLODs[i][j] = createSingleListGrid(verts, verts, step, step, m_sectorVerts, j);
#endif
				if (!m_IBLODs[i][j])
					return false;
			}
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	float TerrainSystem::readHeight(uint16 mapX, uint16 mapZ)
	{
		if (mapX >= m_tableSize) mapX = m_tableSize - 1;
		if (mapZ >= m_tableSize) mapZ = m_tableSize - 1;

		return m_heightTable[(mapZ * m_tableSize) + mapX];
	}

	//----------------------------------------------------------------------------------------------
	vec3f TerrainSystem::readNormal(uint16 mapX, uint16 mapZ)
	{
		if (mapX >= m_tableSize) mapX = m_tableSize - 1;
		if (mapZ >= m_tableSize) mapZ = m_tableSize - 1;

		return m_normalTable[(mapZ * m_tableSize) + mapX];
	}

} //-- render
} //-- brUGE