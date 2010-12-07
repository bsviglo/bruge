#include "BSPTree.h"
#include "float.h"

using namespace brUGE::math;

//-- start unnamed namespace.
//-------------------------------------------------------------------------------------------------
namespace
{

	//---------------------------------------------------------------------------------------------
	float planeDistance(const vec4f& plane, const vec3f& point)
	{
		return point.x * plane.x + point.y * plane.y + point.z * plane.z + plane.w;
	}

	//---------------------------------------------------------------------------------------------
	vec3f planeHit(const vec3f& v0, const vec3f& v1, const vec4f& plane)
	{
		vec3f  dir = v1 - v0;
		float  d   = planeDistance(plane, v0);
		vec3f  pos = v0 - dir.scale(d / plane.toVec3().dot(dir));

		return pos;
	}

}
//-------------------------------------------------------------------------------------------------
//-- end unnamed namespace.


namespace brUGE
{
namespace utils
{

	//---------------------------------------------------------------------------------------------
	void BSPTri::split(
		BSPTri* dest, int& nPos, int& nNeg, const vec4f& plane, const float epsilon) const
	{
		float d[3];
		for (uint i = 0; i < 3; ++i)
		{
			d[i] = planeDistance(plane, v[i]);
		}

		int first  = 2;
		int second = 0;
		while (!(d[second] > epsilon && d[first] <= epsilon))
		{
			first = second;
			++second;
		}

		//-- positive triangles.
		nPos = 0;
		vec3f h = planeHit(v[first], v[second], plane);
		do
		{
			first = second;
			++second;
			if (second >= 3) second = 0;

			dest->v[0] = h;
			dest->v[1] = v[first];

			if (d[second] > epsilon)	dest->v[2] = v[second];
			else						dest->v[2] = h = planeHit(v[first], v[second], plane);

			dest->data = data;
			dest->finalize();
			++dest;
			++nPos;
		}
		while (d[second] > epsilon);

		//-- skip zero area triangle.
		if (fabsf(d[second]) <= epsilon)
		{
			first = second;
			++second;
			if (second >= 3) second = 0;
		}

		//-- negative triangles
		nNeg = 0;
		do
		{
			first = second;
			++second;
			if (second >= 3) second = 0;

			dest->v[0] = h;
			dest->v[1] = v[first];

			if (d[second] < -epsilon)	dest->v[2] = v[second];
			else						dest->v[2] = planeHit(v[first], v[second], plane);
			
			dest->data = data;
			dest->finalize();
			++dest;
			++nNeg;
		}
		while (d[second] < -epsilon);
	}

	//---------------------------------------------------------------------------------------------
	void BSPTri::finalize()
	{
		vec3f normal = ((v[1] - v[0]).cross(v[2] - v[0])).getNormalized();
		float offset = -v[0].dot(normal);

		vec3f edgeNormals[3];
		edgeNormals[0] = normal.cross(v[0] - v[2]);
		edgeNormals[1] = normal.cross(v[1] - v[0]);
		edgeNormals[2] = normal.cross(v[2] - v[1]);

		float edgeOffsets[3];
		edgeOffsets[0] = edgeNormals[0].dot(v[0]);
		edgeOffsets[1] = edgeNormals[1].dot(v[1]);
		edgeOffsets[2] = edgeNormals[2].dot(v[2]);

		plane = vec4f(normal, offset);
		edgePlanes[0] = vec4f(edgeNormals[0], -edgeOffsets[0]);
		edgePlanes[1] = vec4f(edgeNormals[1], -edgeOffsets[1]);
		edgePlanes[2] = vec4f(edgeNormals[2], -edgeOffsets[2]);
	}

	//---------------------------------------------------------------------------------------------
	bool BSPTri::intersects(const vec3& v0, const vec3& v1) const
	{
		vec3f dir = v0 - v1;
		float k   = planeDistance(plane, v0) / plane.toVec3().dot(dir);

		if (k < 0 || k > 1) return false;

		vec3f pos = v0 - dir.scale(k);

		for (uint i = 0; i < 3; ++i)
		{
			if (planeDistance(edgePlanes[i], pos) < 0)
				return false;
		}
		return true;
	}

	//---------------------------------------------------------------------------------------------
	bool BSPTri::isAbove(const vec3f& pos) const
	{
		return (planeDistance(edgePlanes[0], pos) >= 0 &&
				planeDistance(edgePlanes[1], pos) >= 0 &&
				planeDistance(edgePlanes[2], pos) >= 0);
	}

	//---------------------------------------------------------------------------------------------
	float BSPTri::getDistance(const vec3f& pos) const
	{
		uint k = 2;
		for (uint i = 0; i < 3; ++i)
		{
			float dist = planeDistance(edgePlanes[i], pos);
			if (dist < 0)
			{
				//-- project onto the line between the points.
				vec3f dir = v[i] - v[k];
				float c   = dir.dot(pos - v[k]) / dir.dot(dir);

				vec3f d;
				if (c >= 1)
				{
					d = v[i];
				}
				else
				{
					d = v[k];
					if (c > 0)
					{
						d += dir.scale(c);
					}
				}

				return (pos - d).length();
			}

			k = i;
		}

		return fabsf(planeDistance(plane, pos));
	}

	//---------------------------------------------------------------------------------------------
	BSPNode::~BSPNode()
	{
		delete back;
		delete front;
	}

	//---------------------------------------------------------------------------------------------
	bool BSPNode::intersects(
		const vec3f& v0, const vec3f& v1, const vec3f& dir,
		float* distance, vec3f* point, const BSPTri** triangle
		) const
	{
		float d = planeDistance(tri.plane, v0);

		if (d > 0)
		{
			if (front != NULL && front->intersects(v0, v1, dir, distance, point, triangle))
			{
				return true;
			}

			if (planeDistance(tri.plane, v1) < 0)
			{
				float dist = d / tri.plane.toVec3().dot(dir);
				vec3  pos = v0 - dir.scale(dist);

				if (tri.isAbove(pos))
				{
					if (distance) *distance = dist;
					if (point)	  *point	= pos;
					if (triangle) *triangle = &tri;
					return true;
				}

				if (back != NULL && back->intersects(v0, v1, dir, distance, point, triangle))
				{
					return true;
				}
			}
		}
		else
		{
			if (back != NULL && back->intersects(v0, v1, dir, distance, point, triangle))
			{
				return true;
			}

			if (planeDistance(tri.plane, v1) > 0)
			{
				float dist = d / tri.plane.toVec3().dot(dir);
				vec3  pos = v0 - dir.scale(dist);

				if (tri.isAbove(pos))
				{
					if (distance) *distance = dist;
					if (point)	  *point	= pos;
					if (triangle) *triangle = &tri;
					return true;
				}

				if (front != NULL && front->intersects(v0, v1, dir, distance, point, triangle))
				{
					return true;
				}
			}
		}

		return false;
	}

	//---------------------------------------------------------------------------------------------
	BSPTri* BSPNode::intersectsCached(const vec3f& v0, const vec3f& v1, const vec3f& dir) const
	{
		float d = planeDistance(tri.plane, v0);

		if (d > 0)
		{
			if (front != NULL)
			{
				BSPTri* tri = front->intersectsCached(v0, v1, dir);
				if (tri) return tri;
			}

			if (planeDistance(tri.plane, v1) < 0)
			{
				vec3 pos = v0 - dir.scale(d / tri.plane.toVec3().dot(dir));
				if (tri.isAbove(pos))
				{
					return (BSPTri*)&tri;
				}

				if (back != NULL)
				{
					BSPTri *tri = back->intersectsCached(v0, v1, dir);
					if (tri) return tri;
				}
			}
		}
		else
		{
			if (back != NULL)
			{
				BSPTri *tri = back->intersectsCached(v0, v1, dir);
				if (tri) return tri;
			}

			if (planeDistance(tri.plane, v1) > 0)
			{
				vec3 pos = v0 - dir.scale(d / tri.plane.toVec3().dot(dir));
				if (tri.isAbove(pos))
				{
					return (BSPTri*)&tri;
				}

				if (front != NULL)
				{
					BSPTri *tri = front->intersectsCached(v0, v1, dir);
					if (tri) return tri;
				}
			}
		}

		return NULL;
	}

	//---------------------------------------------------------------------------------------------
	bool BSPNode::pushSphere(vec3f& pos, const float radius) const
	{
		float d = planeDistance(tri.plane, pos);

		bool pushed = false;
		if (fabsf(d) < radius)
		{
			if (tri.isAbove(pos))
			{
				pos += tri.plane.toVec3().scale(radius - d);
				pushed = true;
			}
		}

		if (front != NULL && d > -radius) pushed |= front->pushSphere(pos, radius);
		if (back  != NULL && d <  radius) pushed |= back ->pushSphere(pos, radius);

		return pushed;
	}

	//---------------------------------------------------------------------------------------------
	void BSPNode::getDistance(const vec3f& pos, float& minDist) const
	{
		float d = planeDistance(tri.plane, pos);

		float dist = tri.getDistance(pos);
		if (dist < minDist)
		{
			minDist = dist;
		}

		if (back && d < minDist)
		{
			back->getDistance(pos, minDist);		
		}

		if (front && -d < minDist)
		{
			front->getDistance(pos, minDist);
		}
	}

#if USE_BSP_SERIALIZATION

	//---------------------------------------------------------------------------------------------
	bool BSPNode::read(const ROData& iData)
	{
		if (!iData.read(tri.v)) return false;
		tri.finalize();
		
		int flags = 0;
		if (!iData.read(flags)) return false;
		
		bool success = true;

		if (flags & 1)
		{
			back = new BSPNode;
			success &= back->read(iData);
		}
		else
		{
			back = NULL;
		}

		if (flags & 2)
		{
			front = new BSPNode;
			success &= front->read(iData);
		}
		else
		{
			front = NULL;
		}

		return success;
	}

	//---------------------------------------------------------------------------------------------
	bool BSPNode::write(WOData& oData) const
	{
		oData.write(tri.v);

		int flags = 0;
		if (back)  flags |= 1;
		if (front) flags |= 2;

		oData.write(flags);

		bool success = true;
		if (back)  success &= back->write(oData);
		if (front) success &= front->write(oData);

		return success;
	}

#endif //-- USE_BSP_SERIALIZATION

	//---------------------------------------------------------------------------------------------
	void BSPNode::build(std::vector<BSPTri>& tris, int splitCost, int balCost, float epsilon)
	{
		uint index    = 0;
		int  minScore = 0x7FFFFFFF;

		for (uint i = 0; i < tris.size(); ++i)
		{
			int score = 0;
			int diff  = 0;

			for (uint k = 0; k < tris.size(); ++k)
			{
				uint neg = 0, pos = 0;
				for (uint j = 0; j < 3; ++j)
				{
					float dist = planeDistance(tris[i].plane, tris[k].v[j]);

					if      (dist < -epsilon) ++neg;
					else if (dist >  epsilon) ++pos;
				}

				if (pos)
				{
					if (neg) score += splitCost;
					else	 ++diff;
				}
				else
				{
					if (neg) --diff;
					else	 ++diff;
				}
			}

			score += balCost * abs(diff);
			if (score < minScore)
			{
				minScore = score;
				index = i;
			}
		}

		tri = tris[index];

		//-- remove index from array.
		tris[index] = tris[tris.size() - 1];
		tris.pop_back();
			
		std::vector<BSPTri> backTris;
		std::vector<BSPTri> frontTris;

		for (uint i = 0; i < tris.size(); ++i)
		{

			uint neg = 0, pos = 0;

			for (uint j = 0; j < 3; ++j)
			{
				float dist = planeDistance(tri.plane, tris[i].v[j]);

				if      (dist < -epsilon) ++neg;
				else if (dist >  epsilon) ++pos;
			}

			if (neg)
			{
				if (pos)
				{
					BSPTri newTris[3];
					int    nPos, nNeg;
					tris[i].split(newTris, nPos, nNeg, tri.plane, epsilon);

					for (int i = 0; i < nPos; ++i)
					{
						frontTris.push_back(newTris[i]);
					}

					for (int i = 0; i < nNeg; ++i)
					{
						backTris.push_back(newTris[nPos + i]);
					}
				}
				else
				{
					backTris.push_back(tris[i]);
				}
			}
			else
			{
				frontTris.push_back(tris[i]);
			}
		}
		tris.clear();

		if (!backTris.empty())
		{
			back = new BSPNode;
			back->build(backTris, splitCost, balCost, epsilon);
		}
		else
		{
			back = NULL;
		}

		if (!frontTris.empty())
		{
			front = new BSPNode;
			front->build(frontTris, splitCost, balCost, epsilon);
		}
		else
		{
			front = NULL;
		}
	}

	//---------------------------------------------------------------------------------------------
	void BSPTree::addTriangle(const vec3f& v0, const vec3f& v1, const vec3f& v2, void* data)
	{
		BSPTri tri;

		tri.v[0] = v0;
		tri.v[1] = v1;
		tri.v[2] = v2;
		tri.data = data;

		tri.finalize();

		tris.push_back(tri);
	}

	//---------------------------------------------------------------------------------------------
	void BSPTree::build(int splitCost, int balCost, float epsilon)
	{
		top = new BSPNode;
		top->build(tris, splitCost, balCost, epsilon);
	}

	//---------------------------------------------------------------------------------------------
	bool BSPTree::intersects(
		const vec3f& v0, const vec3f& v1, float* distance, vec3f* point, const BSPTri** triangle) const
	{
		if (top != NULL)	return top->intersects(v0, v1, v1 - v0, distance, point, triangle);
		else				return false;
	}

	//---------------------------------------------------------------------------------------------
	bool BSPTree::intersectsCached(const vec3f& v0, const vec3f& v1)
	{
		if (top != NULL)
		{
			if (cache)
			{
				if (cache->intersects(v0, v1))
				{
					return true;
				}
			}
			cache = top->intersectsCached(v0, v1, v1 - v0);
			return (cache != NULL);
		}

		return false;
	}

	//---------------------------------------------------------------------------------------------
	bool BSPTree::pushSphere(vec3f& pos, float radius) const
	{
		if (top != NULL)	return top->pushSphere(pos, radius);
		else				return false;
	}

	//---------------------------------------------------------------------------------------------
	float BSPTree::getDistance(const vec3f& pos) const
	{
		float dist = FLT_MAX;

		if (top != NULL)
			top->getDistance(pos, dist);

		return dist;
	}

	//---------------------------------------------------------------------------------------------
	bool BSPTree::isInOpenSpace(const vec3f& pos) const
	{
		if (top != NULL)
		{
			BSPNode* node = top;
			for (;;)
			{
				float d = planeDistance(node->tri.plane, pos);

				if (d > 0)
				{
					if (node->front)	node = node->front;
					else				return true;
				}
				else
				{
					if (node->back)		node = node->back;
					else				return false;
				}
			}
		}

		return false;
	}

#if USE_BSP_SERIALIZATION

	//---------------------------------------------------------------------------------------------
	bool BSPTree::load(const ROData& iData)
	{
		if (!iData.length())
			return false;

		delete top;

		top = new BSPNode;
		return top->read(iData);
	}

	//---------------------------------------------------------------------------------------------
	bool BSPTree::save(WOData& oData) const
	{
		if (top == NULL)
			return false;

		return top->write(oData);
	}

#endif //-- USE_BSP_SERIALIZATION

} // utils
} // brUGE