#include "SceneGraph/br_QuadTreeNode.h"

#include "Math/br_AABoundingBox.h"
#include "SceneGraph/br_StaticObject.h"
#include "Renderer/br_RenderEntity.h"
//#include "Renderer/gl/br_OGLRender.h"
#include "Renderer/br_Camera.h"

namespace brUGE{
	namespace scene{
		
		// 
		//------------------------------------------
		brQuadTreeNode::brQuadTreeNode() : isLeaf_(false), isVisibleByCamera_(false){
			for(uint i=0; i<4; ++i)
				nodes[i] = NULL;
		}

		// 
		//------------------------------------------
		brQuadTreeNode::~brQuadTreeNode(){
			for(uint i=0; i<4; ++i)
				delete nodes[i];
		}

		// 
		//------------------------------------------
		bool brQuadTreeNode::buildNode(uint level, brQuadTreeNode *parent, const Rect2d &br){
			this->parent = parent;
			boundingRect = br;

			//if current node is leaf
			if(level == 0){
				isLeaf_ = true;
				return true;
			}else{
				for(uint i=0; i<4; ++i){
					nodes[i] = new brQuadTreeNode();
				}
			}

			/*
			A = min
			C = max

			^-z(y)
			|
			|
			|
			|--------->x(x)

			A----F----D
			|    |	  | 	
			E----H----L
			|    |    | 
			B----G----C
			*/
			
			vec2f point_A = br.minCorner;
			vec2f point_C = br.maxCorner;
			vec2f point_H = boundingRect.centerPos();
			vec2f point_D = init_vec2	( br.minCorner[0],	br.maxCorner[1]	);
			vec2f point_F = init_vec2	( point_H[0],		br.minCorner[1]	);
			vec2f point_E = init_vec2	( br.minCorner[0],	point_H[1]		);
			vec2f point_L = init_vec2	( br.maxCorner[0],	point_H[1]		);
			vec2f point_B = init_vec2	( br.minCorner[0],	br.maxCorner[1]	);
			vec2f point_G = init_vec2	( point_H[0],		br.maxCorner[1]	);

			nodes[0]->buildNode(level-1 ,this, Rect2d( point_A, point_H	));
			nodes[1]->buildNode(level-1 ,this, Rect2d( point_F, point_L	));
			nodes[2]->buildNode(level-1 ,this, Rect2d( point_E, point_G	));
			nodes[3]->buildNode(level-1 ,this, Rect2d( point_H, point_C	));

			return true;
		}
	
		// 
		//------------------------------------------
		bool brQuadTreeNode::addObject(brStaticObject &obj){

			//TODO: проверка на нахождени€ уже объекта в дереве

			brRenderEntity *re	 = obj.getRenderEntity();
			brAABB aabb = re->getAABB().getTransformed(re->getOrientation());
			
			// 
			// ¬виду того что aabb содержит минимальную точку в верхнем правом углу, а максимальную в нижнем правом,
			// что впринципе соответсвует правосторонней системе координат. то мы также будем поступать и дл€ нашего
			// квад три. :)
			//
			Rect2d aabr = Rect2d(init_vec2(aabb.getMin()[0], aabb.getMin()[2]),
									 init_vec2(aabb.getMax()[0], aabb.getMax()[2]));

			//int flag1 = boundingRect.pointInRect(aabr.minCorner);
			//int flag2 = boundingRect.pointInRect(aabr.maxCorner);

			
			//if (flag1 || flag2){
			if (boundingRect.rectInRect(aabr)){
				if(!isLeaf_){
					for(uint i=0; i<4; ++i)
						nodes[i]->addObject(obj);
				}else{
					objects.push_back(&obj);
				}
			}
			return true;
		}

		// 
		//------------------------------------------
		void brQuadTreeNode::find(const Camera &camera){
			if (camera.frustum().testRect2d(boundingRect)){
				if (!isLeaf_){
					for (uint i=0; i<4; ++i){
						nodes[i]->find(camera);
					}
				}else{
					isVisibleByCamera_ = true;
				}
			}
			
			//TODO: отсечение по области видимости камеры.
			for(size_t i=0; i < objects.size(); ++i){
				objects[i]->getRenderEntity()->setIsCulling(false);
			}
		}
		
		// 
		//------------------------------------------
		void brQuadTreeNode::_debugRender(float dt, brOGLRender *render){
			/*

			^-z(y)
			|
			|
			|
			|--------->x(x)

			vec2f vec;
			vec[0] = -z;
			vec[1] = x;

			a------b
			|	   |
			|	   |
			d------c				
			*/

			//vec3f a = init_vec3(boundingRect.minCorner[1], 0.0f, boundingRect.minCorner[0]);
			//vec3f b = init_vec3(boundingRect.maxCorner[1], 0.0f, boundingRect.minCorner[0]);
			//vec3f c = init_vec3(boundingRect.maxCorner[1], 0.0f, boundingRect.maxCorner[0]);
			//vec3f d = init_vec3(boundingRect.minCorner[1], 0.0f, boundingRect.maxCorner[0]);

			// ≈сли в узле есть объекты, то отрисовывоем его красным цветом.
			float offset = 0.0f;
			brColourF color;
			if (isLeaf_){
				if (objects.size() > 0 && !isVisibleByCamera_){
					color  = color_Blue;
				}else if (isVisibleByCamera_){
					color = color_Red;
					isVisibleByCamera_ = false;
				}else{
					color  = color_Green;
				}

				vec3f a = init_vec3(boundingRect.minCorner[0], offset, boundingRect.minCorner[1]);
				vec3f b = init_vec3(boundingRect.maxCorner[0], offset, boundingRect.minCorner[1]);
				vec3f c = init_vec3(boundingRect.maxCorner[0], offset, boundingRect.maxCorner[1]);
				vec3f d = init_vec3(boundingRect.minCorner[0], offset, boundingRect.maxCorner[1]);
				render->renderQuad(a, b, c, d, color);

				offset = 0.05f;
				
				a = init_vec3(boundingRect.minCorner[0], offset, boundingRect.minCorner[1]);
				b = init_vec3(boundingRect.maxCorner[0], offset, boundingRect.minCorner[1]);
				c = init_vec3(boundingRect.maxCorner[0], offset, boundingRect.maxCorner[1]);
				d = init_vec3(boundingRect.minCorner[0], offset, boundingRect.maxCorner[1]);
				render->wireframe_renderQuad(a, b ,c, d, color_Black);
			}

			if(!isLeaf_){
				for(uint i=0; i<4; i++){
					nodes[i]->_debugRender(dt, render);	
				}
			}
		}

	}/*end namespace sceneGraph*/
}/*end namespace brUGE*/