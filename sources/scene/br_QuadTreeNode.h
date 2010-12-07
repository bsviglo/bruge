#ifndef _BR_SCENEQUADTREENODE_H_
#define _BR_SCENEQUADTREENODE_H_

#include "br_Types.h"
#include "br_QuadTree.h"
#include "Math/br_Geometry.h"

#include <vector>

namespace brUGE{

	namespace render{
		class Camera;
	}
	class brStaticObject;

	using namespace render;

	namespace scene{
		
		//
		// 
		//------------------------------------------------------------
		class brQuadTreeNode{
			friend class brQuadTree;
		public:
			brQuadTreeNode();
			~brQuadTreeNode();

			bool addObject(brStaticObject &obj);
			bool buildNode(uint level, brQuadTreeNode *parent, const Rect2d &br);
			void find(const Camera &camera);

		private:
			void _debugRender(float dt, brOGLRender *render);

		protected:
			std::vector<brStaticObject*> objects;
			brQuadTreeNode				*parent;
			brQuadTreeNode				*nodes[4];
			Rect2d					boundingRect;
			bool						isLeaf_;
			bool						isVisibleByCamera_;

			vec3f						position;
			mat3X3						orientation;						
		};

	}/*end namespace sceneGraph*/
}/*end namespace brUGE*/

#endif/*_BR_SCENEQUADTREENODE_H_*/

/**
	* 21/11/2007
		created.
*/