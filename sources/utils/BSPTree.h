#pragma once

#include "prerequisites.h"
#include "math/Vector2.hpp"
#include "math/Vector3.hpp"
#include "math/Vector4.hpp"
#include "utils/Data.hpp"
#include "utils/Ptr.h"

#include <vector>

#define USE_BSP_SERIALIZATION 1

namespace brUGE
{
namespace utils
{

	//-- ToDo:
	//---------------------------------------------------------------------------------------------
	struct BSPTri
	{
		BSPTri() : data(NULL) { }

		void  split(BSPTri* dest, int& nPos, int& nNeg, const vec4f& plane, const float epsilon) const;
		void  finalize();
		bool  intersects(const vec3f& v0, const vec3f& v1) const;
		bool  isAbove(const vec3f& pos) const;
		float getDistance(const vec3f& pos) const;

		vec4f plane;
		vec4f edgePlanes[3];
		vec3f v[3];
		void* data;
	};

	
	//-- ToDo:
	//---------------------------------------------------------------------------------------------
	struct BSPNode
	{
		BSPNode() : back(NULL), front(NULL) { }
		~BSPNode();

		bool    intersects(const vec3f& v0, const vec3f& v1, const vec3f& dir, float* distance, vec3f* point, const BSPTri** triangle) const;
		BSPTri* intersectsCached(const vec3f& v0, const vec3f& v1, const vec3f& dir) const;

		bool	pushSphere(vec3f& pos, float radius) const;
		void	getDistance(const vec3f& pos, float& minDist) const;

		void	build(std::vector<BSPTri>& tris, int splitCost, int balCost, float epsilon);

#if	USE_BSP_SERIALIZATION
		//-- serialization.
		bool	read (const ROData& data);
		bool	write(WOData& data) const;
#endif

		BSPNode* back;
		BSPNode* front;
		BSPTri   tri;
	};


	//-- ToDo:
	//---------------------------------------------------------------------------------------------
	class BSPTree : public utils::RefCount
	{
	public:
		BSPTree() : top(NULL), cache(NULL) { }
		~BSPTree()
		{
			delete top;
		}
		
		//-- build bsp tree.
		void  addTriangle(const vec3f& v0, const vec3f& v1, const vec3f& v2, void* data = NULL);
		void  build(int splitCost = 3, int balCost = 1, float epsilon = 0.001f);
		
		//-- detect intersection functions.
		bool  intersects(const vec3f& v0, const vec3f& v1, float* distance = NULL, vec3f* point = NULL, const BSPTri** triangle = NULL) const;
		bool  intersectsCached(const vec3f& v0, const vec3f& v1);
		bool  pushSphere(vec3f& pos, float radius) const;
		float getDistance(const vec3f& pos) const;

		bool  isInOpenSpace(const vec3f& pos) const;

#if USE_BSP_SERIALIZATION
		//-- serialization.
		bool load(const ROData& iData);
		bool save(WOData& oData) const;
#endif

	private:
		std::vector<BSPTri> tris;
		BSPNode*			top;
		BSPTri*				cache;

	private:
		BSPTree(const BSPTree&);
		BSPTree& operator = (const BSPTree&);
	};

} // utils
} // brUGE