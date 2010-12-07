//
// Simple class to load Lightwave LWO files
//
// Author: Alex V. Boreskoff <alexboreskoff@mtu-net.ru>, <steps3d@narod.ru>
//

#ifndef	__LWO_LOADER__
#define	__LWO_LOADER__

#include	<list>
#include	<map>
#include	<string>

using namespace std;

class	Data;
class	Mesh;
class	MeshNode;
struct	Face;
struct	LwoChunk;
class	Material;

class	LwoLoader
{
public:
	LwoLoader () {}

	MeshNode * load ( Data * data );

protected:
	struct	PolyData
	{
		int		index;
		int		surface;
		int		numFaces;
		Face  * faces;
	};

	typedef	map<int, PolyData>	Polys;

	void	readPoints     ( const LwoChunk& chunk, Data * data, int& numPoints, Vertex *& points ) const;
	void	readPolys      ( const LwoChunk& chunk, Data * data, Polys& polys ) const;
	void	readSurface    ( const LwoChunk& chunk, Data * data, map<string, Material>& surfaces ) const;
	void	readTexMapping ( const LwoChunk& chunk, Data * data, bool perPoly ) const;
	void	readClip       ( const LwoChunk& chunk, Data * data, map <int, string>& images ) const;
	void	readTags       ( const LwoChunk& chunk, Data * data, map <int, string>& tags ) const;
	void	readPtag       ( const LwoChunk& chunk, Data * data, map<int, list<int> >& textures ) const;
	
};

#endif