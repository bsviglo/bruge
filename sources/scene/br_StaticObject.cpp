#include "br_StaticObject.h"
#include "render/br_RenderEntity.h"

namespace brUGE
{

	brStaticObject::brStaticObject()
	{

	}

	brStaticObject::~brStaticObject()
	{

	}

	void brStaticObject::update(float _elapsedTime)
	{
		static float angle = 0.0f;
		angle = 0.004f * _elapsedTime;
	}
	
	const mat4X4 &brStaticObject::getOrientationMatrix() const
	{
		return orientation;
	}

	void brStaticObject::setOrientationMatrix(const mat4X4 &_mat)
	{
		orientation = _mat;
	}

	void brStaticObject::loadRenderEntity(brRenderEntity &_re){
		renderEntity = &_re;
	}

	brRenderEntity *brStaticObject::getRenderEntity() const{
		return renderEntity;
	}
	
}/*end namespace brUGE*/