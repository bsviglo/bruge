#ifndef _BR_STATICOBJECT_H_
#define _BR_STATICOBJECT_H_

#include "br_prerequisites.hpp"
#include "Math/br_MathTypes.h"

namespace brUGE
{

	namespace render
	{
		class brRenderEntity;	
	}

	using namespace math;
	using namespace render;
	
	class brStaticObject
	{
	public:
		brStaticObject();
		virtual ~brStaticObject();

		void update(float _elapsedTime);

		void setOrientationMatrix(const mat4X4 &_mat);
		void loadRenderEntity(brRenderEntity &_re);

		const mat4X4 &getOrientationMatrix() const;
		brRenderEntity *getRenderEntity() const;
	private:
		brRenderEntity   *renderEntity;
		//brSoundEntity	 *soundEntity;
		//brPhysicEntity *physicEntity;

		//vec3f			 position;
		//mat3X3		 orientation;
		mat4X4			 orientation;
	};

}/*end namespace brUGE*/

#endif /*_BR_STATICOBJECT_H_*/