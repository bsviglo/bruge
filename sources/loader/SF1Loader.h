#pragma once

#include "render/render_common.h"

#include <vector>
#include <map>

namespace brUGE
{
namespace render
{

	/**
	* Format:
	* 	- numVertices
	* 	- numFaces
	*  - numTexCoord
	*  + Faces
	* 		- a
	* 		- b
	*      - c
	*  + Texture Coordinates
	* 		- u
	* 		- v
	*  + Texture coordinates for m_vertices in each Face
	* 		- a
	* 		- b
	* 		- c
	* 	+ Vertices
	* 		- x
	* 		- y
	* 		- z
	*/
	class SF1Loader
	{
	public:
		SF1Loader();
		~SF1Loader();

		Ptr<RenderMesh> load(const std::string& fileName);
		uint getTotalSize() const { return m_totalSize; }

	private:

		struct Face
		{
			union
			{
				struct { uint16 v1, v2, v3;	};
				uint16 v[3];
			};

			union
			{
				struct { uint16 t1, t2, t3; };
				uint16 t[3];
			};
		};

		struct TexCoord
		{
			float u,v;
		};

		struct Vertex
		{
			float x,y,z;
		};

		std::vector<Face>		m_faces;
		std::vector<Vertex>		m_vertices;
		std::vector<TexCoord>	m_texCoord;

		std::string		    m_fileName;
		uint				m_totalSize;
	};

} // render
} // brUGE
