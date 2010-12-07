#include "SceneGraph/br_QuadTree.h"
#include "SceneGraph/br_QuadTreeNode.h"

#include "Math/br_Geometry.h"

namespace brUGE{
	namespace scene{
		
		//------------------------------------------
		brQuadTree::brQuadTree(){
			root_ = new brQuadTreeNode();
		}

		//------------------------------------------
		brQuadTree::~brQuadTree(){
			delete root_;
		}

		//------------------------------------------
		bool brQuadTree::buildTree(uint nodeDepth, uint objPerNode, const Rect2d &worldSize){
			nodeDepth_		= nodeDepth;
			objectsPerNode_ = objPerNode;
			worldSize_		= worldSize;

			root_->buildNode(nodeDepth, static_cast<brQuadTreeNode*>(0), worldSize);
			return true;
		}

		//------------------------------------------
		bool brQuadTree::addObject(brStaticObject &object){
			root_->addObject(object);			
			return true;
		}

		//------------------------------------------
		void brQuadTree::update(){

		}
		
		//------------------------------------------
	    void brQuadTree::find(const Camera &camera){
			root_->find(camera);
		}

		//------------------------------------------
		void brQuadTree::render(float dt, brOGLRender *render){
			root_->_debugRender(dt, render);
		}	

	}/*end namespace sceneGraph*/
}/*end namespace brUGE*/