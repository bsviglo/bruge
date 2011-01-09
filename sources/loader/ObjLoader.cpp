#include "ObjLoader.h"
#include "render/Mesh.hpp"
#include "render/IBuffer.h"
#include "utils/Data.hpp"
#include "utils/string_utils.h"
#include "math/math_types.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include <vector>

using namespace std;
using namespace brUGE::render;
using namespace brUGE::utils;
using namespace brUGE::math;

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//----------------------------------------------------------------------------------------------
	string replaceTabs(const string& str)
	{
		int		len = str.length();
		string	s   = str;

		for (int i = 0; i < len; ++i)
			if (s[i] == '\t')
				s[i] = ' ';

		return s;
	}

	//----------------------------------------------------------------------------------------------
	void parseString(const string& str, string& cmd, string& args)
	{
		int	len = str.length();
		int	pos;

		for (pos = 0; pos < len && str[pos] != ' ' && str[pos] != '\t'; ++pos)
			;

		cmd  = str.substr(0, pos);
		args = replaceTabs(trim(str.substr(pos)));
	}

	//----------------------------------------------------------------------------------------------
	int	parseF(const string& str, int* vi, int* ti)
	{
		string arg1, args, s = str;

		for (uint i = 0; ; s = args, ++i)
		{
			parseString(s, arg1, args);

			if (arg1.empty())
				return i;

			if (ti != NULL)		sscanf(arg1.c_str(), "%d/%d", &vi[i], &ti[i]);
			else				sscanf(arg1.c_str(), "%d",    &vi[i]);
		}
	}

	//-- Simple indexator needed to build the indices buffer by gathering only unique vertices from
	//-- the two or more independent data storages. For example vertices, texture coordinates and
	//-- normals arrays and appropriate for them vertex faces, texture coordinate faces and normal
	//-- faces.
	//----------------------------------------------------------------------------------------------
	class Indexator
	{
	public:
		typedef std::map<std::pair<int, int>, uint16> VertexSet;

	public:
		Indexator() : m_indices(0) { }

		uint16 index(const std::pair<int,int>& vert)
		{
			auto iter = m_vertices.find(vert);
			if (iter == m_vertices.end())
			{
				m_vertices[vert] = m_indices;
				return m_indices++;
			}
			return iter->second;
		}

		const VertexSet& vertices() const
		{
			return m_vertices;
		}

	private:
		VertexSet m_vertices;
		uint16    m_indices;
	};

	//----------------------------------------------------------------------------------------------
	void fixFaces(
		std::vector<Mesh::Face>& outFaces,
		std::vector<Mesh::Vertex>& outVertices,
		const std::vector<Mesh::Face>& faces,
		const std::vector<Mesh::Vertex>& vertices,
		const std::vector<Mesh::Face>& texFaces,
		const std::vector<vec2f>& texVertices
		)
	{
		Indexator indexator;
		outFaces.resize(faces.size());

		//-- 1. gather only unique vertices.
		for (uint i = 0; i < faces.size(); ++i)
		{
			for (uint j = 0; j < 3; ++j)
			{
				const int vi = faces   [i].index[j];
				const int ti = texFaces[i].index[j];

				outFaces[i].index[j] = indexator.index(make_pair(vi, ti));
			}
		}
		
		//-- 2. restore vertices by indices in appropriate arrays.
		Mesh::Vertex vert;
		const Indexator::VertexSet& vertexSet = indexator.vertices();
		outVertices.resize(vertexSet.size());
		for (auto i = vertexSet.begin(); i != vertexSet.end(); ++i)
		{
			vert.pos	  = vertices   [i->first.first].pos;
			vert.texCoord = texVertices[i->first.second];

			outVertices[i->second] = vert;
		}
	}

	//----------------------------------------------------------------------------------------------
	Mesh::SubMesh* createSubMesh(
		const vector<Mesh::Vertex>& vertices,
		const vector<Mesh::Face>& faces
		)
	{
		Mesh::SubMesh* submesh = new Mesh::SubMesh();
		submesh->vertices = vertices;
		submesh->faces    = faces;
		return submesh;
	}
		
	typedef vec2f TexCoord;
}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{

	//----------------------------------------------------------------------------------------------
	Ptr<Mesh> ObjLoader::load(const ROData& data)
	{
		Ptr<Mesh>			 out = new Mesh();
		vec3f				 v;
		vec2f				 t;
		int					 vi[30];
		int					 ti[30];
		string				 str, cmd, args;
		string				 mat = "materials/default.mtl";
		bool				 hasTexCoords  = false;
		char				 lastCommand   = '\0';
		vector<Mesh::Vertex> vertices;
		vector<Mesh::Face>   faces;
		vector<Mesh::Face>   texFaces;
		vector<TexCoord>	 texCoords;
		int					 startVertex   = 1;
		int					 startTexCoord = 1;
		int					 startFace     = 1;

		while (data.getString(str, '\n' ))
		{
			str = trim(str);

			//-- skip empty lines and comments.
			if (str.empty() || str[0] == '#')
				continue;

			parseString(str, cmd, args);

			//-- check for start of next mesh.
			if (cmd[0] == 'v' && (lastCommand == 'f' || lastCommand == 'g'))	
			{
				Mesh::SubMesh* submesh = 0;

				if (texFaces.empty())
				{
					submesh = createSubMesh(vertices, faces);
				}
				else
				{
					std::vector<Mesh::Vertex> outVertices;
					std::vector<Mesh::Face>   outFaces;
					fixFaces(outFaces, outVertices, faces, vertices, texFaces, texCoords);

					submesh = createSubMesh(outVertices, outFaces);
				}

				//-- ToDo: try to load material for this submesh.
				{
					RODataPtr mData = os::FileSystem::instance().readFile("resources/" + mat);
					if (!mData.get() || (mData && !submesh->material.load(*mData.get())))
					{
						ERROR_MSG("Can't load submesh material %s.", mat.c_str());
						return 0;
					}
				}

				out->attach(submesh);

				startVertex   += vertices.size();
				startTexCoord += texCoords.size();
				startFace     += faces.size();

				vertices.clear();
				texCoords.clear();
				faces.clear();
				texFaces.clear();
			}

			//-- get vertex.
			if (cmd == "v")
			{
				lastCommand = 'v';
				sscanf(args.c_str(), "%f %f %f", &v.x, &v.y, &v.z);

				{
					Mesh::Vertex vert;
					vert.pos = v;
					vertices.push_back(vert);
				}
			}
			//-- get texcoord.
			else if (cmd == "vt")
			{
				hasTexCoords = true;
				lastCommand = 'v';
				sscanf(args.c_str (), "%f %f", &t.x, &t.y);
				
				{
					TexCoord texCoord;
					texCoord = vec2f(t.x, 1.0f - t.y); //-- ToDo: invert mirror uv along y-axis.
					texCoords.push_back(texCoord);
				}
			}
			//-- get face.
			else if (cmd == "f")
			{											
				lastCommand = 'f';

				//-- check for # of vertices split by spaces into groups
				//-- nn/nn/nn group cannot contain spaces
				int	cnt  = parseF(args, vi, hasTexCoords ? ti : NULL);
				
				//-- number of faces to add.
				int	fcnt = cnt - 2;

				//-- add faces
				for (int i = 0; i < fcnt; ++i)
				{
					Mesh::Face face;
					face.index[0] = vi[i]   - startVertex;
					face.index[1] = vi[i+2] - startVertex; //-- ToDo: was inverted 23 -> 32
					face.index[2] = vi[i+1] - startVertex;
					faces.push_back(face);

					if (hasTexCoords)
					{
						Mesh::Face texFace;
						texFace.index[0] = ti[i]   - startTexCoord;
						texFace.index[1] = ti[i+2] - startTexCoord; //-- ToDo: was inverted 23 -> 32
						texFace.index[2] = ti[i+1] - startTexCoord;
						texFaces.push_back(texFace);
					}
				}
			}
			else if (cmd == "mat_desc")
			{
				mat = args;
			}
		}

		//-- check for last mesh.
		if (!vertices.empty() && !faces.empty())
		{
			Mesh::SubMesh* submesh = 0;

			if (texFaces.empty())
			{
				submesh = createSubMesh(vertices, faces);
			}
			else
			{
				std::vector<Mesh::Vertex> outVertices;
				std::vector<Mesh::Face>   outFaces;
				fixFaces(outFaces, outVertices, faces, vertices, texFaces, texCoords);

				submesh = createSubMesh(outVertices, outFaces);
			}

			//-- ToDo: try to load material for this submesh.
			{
				RODataPtr mData = os::FileSystem::instance().readFile("resources/" + mat);
				if (!mData.get() || (mData && !submesh->material.load(*mData.get())))
				{
					ERROR_MSG("Can't load submesh material %s.", mat.c_str());
					return 0;
				}
			}

			out->attach(submesh);
		}

		if (out->build())	return out;
		else				return 0;
	}

} // end brUGE