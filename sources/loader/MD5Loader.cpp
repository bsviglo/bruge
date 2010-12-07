#include "render/Mesh.hpp"
#include "utils/Data.hpp"
#include "utils/string_utils.h"
#include "math/math_types.h"
#include "math/Vector2.h"
#include "math/Vector3.h"

#include <string>
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
	bool getLine(const ROData& data, std::string& str)
	{
		if (!data.getString(str, '\r'))
			return false;

		str = trim(str);

		return true;
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
}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{
	
	//----------------------------------------------------------------------------------------------
	bool SkinnedMesh::load(const ROData& data)
	{
		string str, cmd, args;
		bool   success = true;

		while (getLine(data, str))
		{
			parseString ( str, cmd, args );
			
			if		(cmd.empty())			{ continue; }
			if	    (cmd == "MD5Version")	{ }
			else if (cmd == "commandline")	{ }
			else if (cmd == "numJoints")	{ m_joints.resize(strToInt(args)); }
			else if (cmd == "joints")		{ success &= loadJoints(data); }
			else if (cmd == "numMeshes")	{ m_submeshes.reserve(strToInt(args)); }
			else if (cmd == "mesh")			{ success &= loadSubMesh(data); }
		}

		return (success && build());
	}

	//----------------------------------------------------------------------------------------------
	bool SkinnedMesh::loadJoints(const ROData& data)
	{
		string	str, cmd, args;
		vec3f	pos, orient;
		int		idx;
		quat	q(0,0,0,0);

		for (uint i = 0; ; ++i)
		{
			//-- loading failed.
			if (!getLine(data, str))
				return false;

			parseString(str, cmd, args);
			
			//-- loading successed.
			if (cmd == "}")
				return true;

			sscanf(args.c_str(), "%d ( %f %f %f ) ( %f %f %f )",
				&idx, &pos.x, &pos.y, &pos.z, &q.x, &q.y, &q.z
				);

			renormalize(q);

			Joint& joint = m_joints[i];

			joint.m_name			 = cmd;
			joint.m_parentIdx		 = idx;
			joint.m_transform.pos	 = pos;
			joint.m_transform.orient = q;
		}
	}

	//----------------------------------------------------------------------------------------------
	bool SkinnedMesh::loadSubMesh(const ROData& data)
	{
		uint idx;
		string str, cmd, args;
		std::auto_ptr<SubMesh> submesh(new SubMesh);

		for (;;)
		{
			//-- loading failed.
			if (!getLine(data, str))
				return false;

			parseString(str, cmd, args);

			if		(cmd.empty())			{ continue; } 
			else if (cmd == "}")			{ break; }
			else if	(cmd == "shader")		{ }
			else if (cmd == "numverts")		{ submesh->vertices.resize(strToInt(args)); }
			else if (cmd == "numtris")		{ submesh->faces.resize(strToInt(args)); }
			else if (cmd == "numweights" )	{ submesh->weights.resize(strToInt(args)); }
			else if (cmd == "vert")
			{
				Vertex vert;
				
				sscanf(args.c_str(), "%d ( %f %f ) %d %d",
					&idx, &vert.texCoord.u, &vert.texCoord.v, &vert.weightIdx, &vert.weightCount
					);

				submesh->vertices[idx] = vert;
			}
			else if (cmd == "tri")
			{
				vec3ui faceUI;
				sscanf(args.c_str(), "%d %d %d %d",	&idx, &faceUI.x, &faceUI.y, &faceUI.z);
				
				vec3us& faceUS = submesh->faces[idx].index;
				faceUS.x = faceUI.x;
				faceUS.y = faceUI.y;
				faceUS.z = faceUI.z;
			}
			else if (cmd == "weight")
			{
				Weight weight;

				sscanf(args.c_str(), "%d %d %f ( %f %f %f )",
					&idx, &weight.joint, &weight.weight, &weight.pos.x, &weight.pos.y, &weight.pos.z
					);

				submesh->weights[idx] = weight;
			}
		}

		//-- release pointer from smart pointer.
		m_submeshes.push_back(submesh.release());
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool Animation::load(const ROData& data)
	{
		std::string	str, cmd, args;
		bool success = true;

		while (getLine(data, str))
		{
			parseString(str, cmd, args);

			if		(cmd.empty())						{ continue; } 
			else if	(cmd == "MD5Version")				{ }
			else if (cmd == "commandline")				{ }
			else if (cmd == "numFrames")				{ m_frameCount = strToInt(args); m_frames.resize(m_frameCount); }
			else if (cmd == "numJoints")				{ m_jointCount = strToInt(args); }
			else if (cmd == "frameRate")				{ m_frameRate = strToInt(args); }
			else if (cmd == "numAnimatedComponents")	{ m_animComponents = strToInt(args); }
			else if (cmd == "hierarchy")				{ success &= loadHierarchy(data); }
			else if (cmd == "bounds")					{ success &= loadBounds(data); }
			else if (cmd == "baseframe")				{ success &= loadBaseFrame(data); }
			else if (cmd == "frame")					{ success &= loadFrame(data, strToInt(args)); }
		}

		if (success)
		{
			m_animTime = (m_frameCount - 1) / (float)m_frameRate;
		}

		return success;
	}
	
	//----------------------------------------------------------------------------------------------
	bool Animation::loadHierarchy(const ROData& data)
	{
		string	str, cmd, args;
		int		parentIdx;

		m_hierarchy.resize(m_jointCount);

		for (uint i = 0; getLine(data, str); )
		{
			parseString(str, cmd, args);

			if (cmd.empty()) continue;
			if (cmd == "}")	 return true;

			sscanf(args.c_str(), "%d %d %d",
				&parentIdx, &m_hierarchy[i].flags, &m_hierarchy[i].startIdx
				);

			++i;
		}

		return true;
	}
	
	//----------------------------------------------------------------------------------------------
	bool Animation::loadBounds(const ROData& data)
	{
		string str, cmd, args;

		m_bounds.resize(m_frameCount);

		for (uint i = 0; getLine(data, str); )
		{
			parseString(str, cmd, args);

			if (cmd.empty())	continue;
			if (cmd == "}")		return true;
			if (cmd == "(")		args = cmd + args;

			AABB& aabb = m_bounds[i++];

			sscanf(args.c_str(), "( %f %f %f ) ( %f %f %f )",
				&aabb.min.x, &aabb.min.y, &aabb.min.z,
				&aabb.max.x, &aabb.max.y, &aabb.max.z
				);
		}
		return true;
	}
	
	//----------------------------------------------------------------------------------------------
	bool Animation::loadBaseFrame(const ROData& data)
	{
		string str, cmd, args;
		vec3f  pos, q;

		m_baseFrame.resize(m_jointCount);

		for (uint i = 0; getLine(data, str); ++i)
		{
			str = trim(str);
			if (str == "}")
				return true;

			sscanf(str.c_str(), "( %f %f %f ) ( %f %f %f )",
				&pos.x, &pos.y, &pos.z, &q.x, &q.y, &q.z
				);

			m_baseFrame[i].pos    = pos;
			m_baseFrame[i].orient = quat(q, 0.0f);
			renormalize(m_baseFrame[i].orient);
		}
		return true;
	}
	
	//----------------------------------------------------------------------------------------------
	bool Animation::loadFrame(const ROData& data, uint idx)
	{
		std::string str, cmd, args;
		FrameData&  frame = m_frames[idx];
		
		frame.resize(m_animComponents);

		for (uint i = 0; getLine(data, str); )
		{
			do
			{
				parseString(str, cmd, args);

				if (cmd.empty())	break;
				if (cmd == "}")		return true;

				sscanf(cmd.c_str(), "%f", &frame[i++]);

				str = args;
			}
			while (args != "");
		}
		return true;
	}

} //-- end brUGE