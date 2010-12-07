#ifndef _BR_SCENEQUADTREE_H_
#define _BR_SCENEQUADTREE_H_

#include "br_Types.h"
#include "Math/br_Geometry.h"
#include "Renderer/ibr_Renderable.h"

namespace brUGE{

	namespace render{
		class Camera;
	}

	using namespace render;
	using namespace math;

	class brStaticObject;

	namespace scene{
		
		class brQuadTreeNode;

		//
		// 
		//------------------------------------------------------------
		class brQuadTree : public ibrRenderable{
		public:
			brQuadTree();
			~brQuadTree();
			
			// 
			//------------------------------------------
			bool buildTree(uint nodeDepth, uint objPerNode, const Rect2d &worldSize);

			// 
			//------------------------------------------
			bool addObject(brStaticObject &object);

			// 
			//------------------------------------------
			void update();

			// 
			//------------------------------------------
			void find(const Camera &camera);
			
			// 
			//------------------------------------------
			bool findPoint(const vec3f &point);

			virtual void render(float dt, brOGLRender *render);	
		private:
			brQuadTreeNode *root_;

			uint		nodeDepth_;
			uint		objectsPerNode_;
			Rect2d	worldSize_;
		};

	}/*end namespace sceneGraph*/
}/*end namespace brUGE*/

#endif/*_BR_SCENEQUADTREE_H_*/