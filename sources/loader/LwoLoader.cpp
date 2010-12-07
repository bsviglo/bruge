//
// Simple class to load Lightwave LWO files
//
// Author: Alex V. Boreskoff <alexboreskoff@mtu-net.ru>, <steps3d@narod.ru>
//

#include	"Mesh.h"
#include	"MeshNode.h"
#include	"Material.h"
#include	"Data.h"
#include	"LwoLoader.h"

struct	LwoChunk
{
	char	type [4];
	uint32	length;

	bool	isA ( const char * theType ) const
	{
		return strncmp ( type, theType, 4 ) == 0;
	}

	int	getPadding () const
	{
		return (int)(length & 1);
	}
};

struct	LwoSubChunk
{
	char	type [4];
	uint16	length;

	bool	isA ( const char * theType ) const
	{
		return strncmp ( type, theType, 4 ) == 0;
	}

	int	getPadding () const
	{
		return (int)(length & 1);
	}
};

static	uint16	readUint16 ( Data * data )
{
	byte	tmp [2];
	union
	{
		uint16	value;
		byte	bytes [2];
	} buf;

	data -> getBytes ( &tmp, 2 );

	buf.bytes [0] = tmp [1];
	buf.bytes [1] = tmp [0];

	return buf.value;
}


static	int	readUint32 ( Data * data )
{
	byte	tmp [4];
	union
	{
		uint32	value;
		byte	bytes [4];
	} buf;

	data -> getBytes ( &tmp, 4 );

	buf.bytes [0] = tmp [3];
	buf.bytes [1] = tmp [2];
	buf.bytes [2] = tmp [1];
	buf.bytes [3] = tmp [0];

	return buf.value;
}

static	int	readVarInt ( Data * data )
{
	int	value = readUint16 ( data );
	
	if ( (value & 0xFF00) == 0xFF )
		value = ((value & 0xFF) << 16) | readUint16 ( data );
		
	return value;
}

static	float	readFloat ( Data * data )
{
	byte	tmp [4];
	union
	{
		float	value;
		byte	bytes [4];
	} buf;

	data -> getBytes ( &tmp, 4 );

	buf.bytes [0] = tmp [3];
	buf.bytes [1] = tmp [2];
	buf.bytes [2] = tmp [1];
	buf.bytes [3] = tmp [0];

	return buf.value;
}

static	string	readString ( Data * data )
{
	string	str;

	data -> getString ( str, '\0' );
	
	if ( (str.length () +1) & 1 )				// do word align
		data -> seekCur ( 1 );

	return str;
}

static	bool	readChunk ( Data * data, LwoChunk& chunk )
{
	if ( data -> getBytes ( &chunk.type, 4 ) != 4 )
		return false;

	chunk.length = readUint32 ( data );

	return true;
}

static	bool	readSubChunk ( Data * data, LwoSubChunk& chunk )
{
	if ( data -> getBytes ( &chunk.type, 4 ) != 4 )
		return false;

	chunk.length = readUint16 ( data );

	return true;
}

MeshNode * LwoLoader :: load ( Data * data )
{
	LwoChunk				chunk;
	int						numPoints = 0;
	Vertex                * points    = NULL;
	map<string, Material>	surfaces;
	map<int, list<int> >	textures;
	Polys					polys;
	map<int, string>		tags;
	map<int, string>		images;
	Material				mat;

	readChunk ( data, chunk );

	if ( !chunk.isA ( "FORM" ) )
		return NULL;

	readChunk ( data, chunk );

	if ( !chunk.isA ( "LWO2" ) )
		return NULL;

	data -> seekCur ( -4 );

	for ( ; ; )
	{
		if ( !readChunk ( data, chunk ) )
			break;

		if ( chunk.isA ( "PNTS" ) )
			readPoints ( chunk, data, numPoints, points );
		else
		if ( chunk.isA ( "POLS" ) )
			readPolys ( chunk, data, polys );
		else
		if ( chunk.isA ( "VMAP" ) || chunk.isA ( "VMAD" ) )
		{
		
		}
		else
		if ( chunk.isA ( "TAGS" ) )			// tags chunk
			readTags ( chunk, data, tags );
		else
		if ( chunk.isA ( "PTAG" ) )					// read textures assignments for polys
			readPtag ( chunk, data, textures );
		else
		if ( chunk.isA ( "CLIP" ) )
			readClip ( chunk, data, images );
		else
		if ( chunk.isA ( "SURF" ) )
			readSurface ( chunk, data, surfaces );
		else
			data -> seekCur ( chunk.length );
	}

	MeshNode  * root = new MeshNode ( NULL );
											// reassemble all faces with the same surface
	int				  numFaces = 0;
	Face            * faces;
	int				  count = 0;
	int				  poly  = 1;
	
	map<int, list<int> > :: iterator sit = textures.begin ();
	list<int>            :: iterator it;

											// collect polygons by textures (surfaces)
	for ( int i = 1; i <= textures.size (); ++i, ++sit )
	{										// collect faces with texture no i
		string	surfName = tags [i];		// surface name, can be used to extract material from surfaces map
		
		numFaces = 0;
		
											// count triangles, loop through polygon index
		for ( it = sit -> second.begin (); it != sit -> second.end (); ++it )
		{
			int			poly     = *it;		// polygon index
			PolyData&	polyData = polys [poly];
			
			numFaces += polyData.numFaces;
		}

		faces = new Face [numFaces];
		count = 0;

		for ( it = sit -> second.begin (); it != sit -> second.end (); ++it )
		{
			int			poly     = *it;		// polygon index
			PolyData&	polyData = polys [poly];
			
			for ( int j = 0; j < polyData.numFaces; j++ )
				faces [count++] = polyData.faces [j];
					
			delete polyData.faces;
		}

		Mesh * mesh = new Mesh ( surfName.c_str (), points, numPoints, faces, numFaces, true );

		mesh -> computeFaceNormals ();
		mesh -> computeTangents    ();
		mesh -> setMaterial        ( surfaces [surfName] );

		MeshNode :: MeshLink * node = new MeshNode :: MeshLink;

		node -> node     = new MeshNode ( mesh );
		node -> name     = surfName;
		node -> offset   = Vector3D ( 0, 0, 0 );
		node -> matr     = Matrix3D ( 1 );

		root -> attach ( node );
	}
	
	return root;
}

void	LwoLoader :: readPoints ( const LwoChunk& chunk, Data * data, int& numPoints, Vertex *& points ) const
{
	numPoints = chunk.length / 12;
	points    = new Vertex [numPoints];
	
	for ( int i = 0; i < numPoints; i++ )
	{
		points [i].pos.x = readFloat ( data );
		points [i].pos.y = readFloat ( data );
		points [i].pos.z = readFloat ( data );
		points [i].pos.w = 1;
		points [i].color = Vector4D ( 1, 1, 1, 1 );
	}
}

void	LwoLoader :: readPolys ( const LwoChunk& chunk, Data * data, Polys& polys ) const
{
	int			pos = data -> getPos ();
	int			temp [0x400];
	char		face [4];
	int			poly = 0;
	int			i;
	PolyData	polyData;

	if ( chunk.length < 1 )
		return;
		
	data -> getBytes ( face, 4 );
	
	if ( strncmp ( face, "FACE", 4 ) )		// must be some error
		return;
		
	while ( data -> getPos () + 2 < pos + chunk.length )
	{
		int	word  = readUint16 ( data );	// count of points in a polygon
		int	flags = word & 0xFC00;
		int	count = word & 0x03FF;			// count of polygons

		for ( i = 0; i < count; i++ )
			temp [i] = readUint16 ( data );

											// now convert poly into triangle fan of
											// count - 2 triangles
		Face * faces = new Face [count - 2];

		for ( i = 0; i < count - 2; i++ )
		{
			faces [i].index [0] = temp [0];
			faces [i].index [1] = temp [i+1];
			faces [i].index [2] = temp [i+2];
		}

		polyData.numFaces = count - 2;
		polyData.faces    = faces;
		polyData.index    = poly;
		polyData.surface  = 0;				// no surface yet

		polys [poly++] = polyData;
	}
}

void	LwoLoader :: readSurface ( const LwoChunk& chunk, Data * data, map<string, Material>& surfaces ) const
{
	int			end = data -> getPos () + chunk.length;
	LwoSubChunk	subChunk;
	string		surfaceName;
	string		realName;
	string		str;
	Vector4D	color;
	int			eindex;
	int			imageIndex;

	surfaceName = readString ( data );
	realName    = readString ( data );

	while ( data -> getPos () < end )
	{
		if ( !readSubChunk ( data, subChunk ) )
			break;
	
		if ( subChunk.isA ( "COLR" ) )		// color data
		{
			color.x = readFloat  ( data );
			color.y = readFloat  ( data );
			color.z = readFloat  ( data );
			color.w = 1;
			eindex  = readVarInt ( data );
		}
		else
		if ( subChunk.isA ( "BLOK" ) )
		{
											// read subblocks data
			int	blockEnd = data -> getPos () + subChunk.length;
			
			while ( data -> getPos () < blockEnd )
			{
				if ( !readSubChunk ( data, subChunk ) )
					break;
					
				if ( subChunk.isA ( "IMAG" ) )
				{
											// read image index
					imageIndex = readUint16 ( data ) - 1;
				}
				else
					data -> seekCur ( subChunk.length );
			}
		}
		else
			data -> seekCur ( subChunk.length );
	}

	surfaces [surfaceName].color = color;
}

void	LwoLoader :: readTexMapping ( const LwoChunk& chunk, Data * data, bool perPoly ) const
{
	int		start = data -> getPos ();
	int		type  = readUint32 ( data );
	int		dim   = readUint16 ( data );
	string	name  = readString ( data );
	int		pos   = data -> getPos ();			// save current position
	int		numPoints = 0;
	
											// count vmap entries
	for ( ; data -> getPos () <= start + chunk.length; numPoints++ )
	{
		readVarInt ( data );
		
		if ( perPoly )
			readVarInt ( data );
			
		data -> seekCur ( dim * sizeof ( float ) );
	}
	
											// now start reading them
//	data -> seekAbs ( pos );				// restore position
	
	
}

void	LwoLoader :: readClip ( const LwoChunk& chunk, Data * data, map <int, string>& images ) const
{
	LwoSubChunk	subChunk;
	string		imageName;
    int			index = readUint32 ( data );

	if ( !readSubChunk ( data, subChunk ) )
		return;
		
	if ( subChunk.isA ( "STIL" ) )			// still image
		images [index] = readString ( data );
}

void	LwoLoader :: readTags ( const LwoChunk& chunk, Data * data, map <int, string>& tags ) const
{
	int		end = data -> getPos () + chunk.length;
	string	str;
			
	while ( data -> getPos () < end )
	{
		char	ch = (char) data -> getByte ();
				
		if ( ch != '\0' )
			str += ch;
		else
		if ( !str.empty () )
		{
			tags [tags.size () + 1] = str;

			str = "";
		}
	}
}

void	LwoLoader :: readPtag ( const LwoChunk& chunk, Data * data, map<int, list<int> >& textures ) const
{
	char	face [4];

	data -> getBytes ( face, 4 );
	
	if ( strncmp ( face, "SURF", 4 ) )		// must be some error
		return;
		
	int	count = (chunk.length - 4 ) / 4;	// count of poly indices
			
	for ( int i = 1; i <= count; i++ )
	{
		int	poly = readUint16 ( data );		// polygon index
		int	tex  = readUint16 ( data );		// texture index
				
		textures [tex].push_back ( poly );
	}
}
