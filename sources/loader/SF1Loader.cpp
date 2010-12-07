#include "SF1Loader.h"
#include "render/br_Mesh.h"
#include "render/br_Material.h"
#include "math/br_Matrix.h"
#include "math/br_Matrix_ops.h"
#include "utils/br_StringTokenizer.h"
#include "os/br_FileSystem.h"

#include <fstream>
#include <vector>


// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	const char* g_headerStr 	  = "Header MF1 - 0.0.1";
	const char* g_facesStr		  = "Faces";
	const char* g_verticesStr 	  = "Vertices";
	const char* g_texCoordStr 	  = "TexCoord";
	const char* g_faceTexCoordStr = "Textured Faces";


	//------------------------------------------
	template<class VertexType>
	class Indexator
	{
	public:
		Indexator() : m_uiIndex(0) {}
		~Indexator() {}

		uint16 pushVertex(const VertexType& v);
		uint16 getIndex(const VertexType& v) const;
		void getVertices(std::vector<VertexType>& vertices) const;

	private:	
		typedef std::map<VertexType, uint> VertSet;
		VertSet m_vertices;
		uint16	m_uiIndex;
	};

	template<class VertexType>
	uint16 Indexator<VertexType>::pushVertex(const VertexType& v)
	{
		VertSet::iterator i = m_vertices.find(v);
		if (i == m_vertices.end())
		{
			uint16 index = m_uiIndex++;
			m_vertices[v] = index;
			return index;
		}
		return i->second;
	}

	template<class VertexType>
	uint16 Indexator<VertexType>::getIndex(const VertexType& v) const
	{
		VertSet::const_iterator i = m_vertices.find(v);
		if (i != m_vertices.end())  
			return i->second;  
		assert(!"can't find vertex!");
		return 0;
	}

	template<class VertexType>
	void Indexator<VertexType>::getVertices(std::vector<VertexType>& output) const
	{
		output.resize(m_vertices.size());
		for (VertSet::const_iterator iter = m_vertices.begin(); iter != m_vertices.end(); ++iter)
		{
			uint16 ind = iter->second;
			output[ind] = iter->first;
		}
	}
}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{
namespace render
{
	//------------------------------------------
	SF1Loader::SF1Loader() : m_totalSize(0)
	{

	}
	
	//------------------------------------------
	SF1Loader::~SF1Loader()
	{

	}
	
	//------------------------------------------
	Ptr<RenderMesh> SF1Loader::load(const std::string& fileName)
	{
		Ptr<RenderMesh> mesh = new RenderMesh();
		std::ifstream file(fileName.c_str());
		if(!file.is_open())
		{
			ERROR_MSG(" Error opening file: '%s'", fileName.c_str());
			return NULL;
		}

		char tempStr[256];
		std::string str;

		//get header and check file format
		std::getline(file, str, '\n');
		if(str.compare(g_headerStr))
		{
			ERROR_MSG("%s Error bad header. Message: %d", fileName.c_str(), str.compare(g_headerStr));
			return NULL;
		}

		//get texture
		std::getline(file, str, '\n');
		sscanf(str.c_str(), "%s", tempStr);

		// TODO: —делать нормальный загрузчик материалов дл€ моделей.

		/*
		if(brString(tempStr).compare("not_textured")){
			texFileName = tempStr;
			std::vector<brString> texFiles;
			texFiles.push_back(texFileName);
			texFiles.push_back("cyberdemon_local.tga");
			texFiles.push_back("cyberdemon_s.tga");
			mesh->textured = true;
			mesh->material = new brMaterial();
			mesh->material->loadTextures(texFiles, NULL);
			mesh->material->loadShaders("Data/Shaders/specular3.vp",
				"Data/Shaders/specular3.fp");
		}
		*/

		// get number of m_vertices, m_faces and texture coordinates
		std::getline(file, str, '\n');
		
		uint numVertices	= 0;
		uint numFaces		= 0;
		uint numTexCoord	= 0;

		//sscanf ( args.c_str (), "%d %d %f ( %f %f %f )", &index, &joint, &bias, &x, &y, &z );
		sscanf(str.c_str(), "%d %d %d", &numVertices, &numFaces, &numTexCoord);

		//initialization temp arrays
		m_faces.resize(numFaces);
		m_vertices.resize(numVertices);
		m_texCoord.resize(numTexCoord);

		//**************************************************************************//
		//	load m_faces															   	//
		//**************************************************************************//
		std::getline(file, str, '\n');
		if(str.compare(g_facesStr))
		{
			ERROR_MSG("%s Error bad m_faces", fileName.c_str());
			return NULL;
		}

		for(uint i=0; i<numFaces; ++i)
		{
			std::getline(file, str, '\n');
			sscanf(str.c_str(), "%hu %hu %hu", &m_faces[i].v1, &m_faces[i].v2, &m_faces[i].v3);
		}

		//**************************************************************************//
		//	load texture coordinates											   	//
		//**************************************************************************//
		std::getline(file, str, '\n');
		if(str.compare(g_texCoordStr))
		{
			ERROR_MSG("%s Error bad texture coordinate", fileName.c_str());
			return NULL;
		}

		for(uint i=0; i<numTexCoord; ++i)
		{
			std::getline(file, str, '\n');
			sscanf(str.c_str(), "%f %f", &m_texCoord[i].u, &m_texCoord[i].v);
		}


		//	load textured m_faces
		std::getline(file, str, '\n');
		if (str.compare(g_faceTexCoordStr))
		{
			ERROR_MSG("%s Error bad textured m_faces", fileName.c_str());
			return NULL;
		}

		for (uint i=0; i<numFaces; ++i)
		{
			std::getline(file, str, '\n');
			sscanf(str.c_str(), "%hu %hu %hu", &m_faces[i].t1, &m_faces[i].t2, &m_faces[i].t3);
		}

		//	load m_vertices
		std::getline(file, str, '\n');
		if (str.compare(g_verticesStr))
		{
			ERROR_MSG("%s Error bad m_vertices", fileName.c_str());
			return NULL;
		}

		for (uint i=0; i<numVertices; ++i)
		{
			std::getline(file, str, '\n');
			sscanf(str.c_str(), "%f %f %f", &m_vertices[i].x, &m_vertices[i].y, &m_vertices[i].z);
		}

		//**************************************************************************//
		//	Fill mesh structure													   	//
		//**************************************************************************//
		//allocate memory
		mesh->faces.resize(numFaces);	   

		Indexator<Vertex> indexator;
		Vertex vertex;
		Face face;

		for (uint i = 0; i < numFaces; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				Vertex tmpVtx		= m_vertices[m_faces[i].v[j]];
				TexCoord tmpTCrd	= m_texCoord[m_faces[i].t[j]];
				vertex.pos			= init_vec3(tmpVtx.x, tmpVtx.y, tmpVtx.z);
				vertex.texCoord		= init_vec2(tmpTCrd.u, tmpTCrd.v);
				face.index[j] = indexator.pushVertex(vertex);
			}
			mesh->faces.push_back(face);
		}
			
		indexator.getVertices(mesh->vertices);
		mesh->tangents.resize(mesh->vertices.size());

		mesh->buildAABB();
		mesh->buildTangents();
		mesh->optimizeForStrips();
		//mesh->optimizeForList();

		// TODO: Ёто строчку необходимо будет перенести на один уровень выше. “.е. в класс менагера ресурсов.
		mesh->buildBuffers();

		INFO_MSG("------------------------------------------------------------------");
		INFO_MSG("Load mesh from file %s", fileName.c_str());
		INFO_MSG("Vertex: %d, Faces: %d", mesh->vertices.size(), mesh->faces.size());
		INFO_MSG("------------------------------------------------------------------");
		m_totalSize += static_cast<uint>(mesh->vertices.size() * sizeof(Vertex) + mesh->faces.size() * sizeof(Face) / 1024);
		return mesh;
	}

} // render
} // brUGE
