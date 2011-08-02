#include "CursorCamera.hpp"

namespace brUGE
{
	//----------------------------------------------------------------------------------------------
	CursorCamera::CursorCamera(const render::Projection& projection)
		:	m_source(nullptr), m_target(nullptr), Camera(projection)
	{

	}

	//----------------------------------------------------------------------------------------------
	CursorCamera::~CursorCamera()
	{

	}

	//----------------------------------------------------------------------------------------------
	void CursorCamera::update(bool /*updateInput*/, float /*dt*/)
	{
		//-- waiting correct initialization.
		if (!m_source || !m_target)
			return;

		const vec3f& pos = m_source->applyToOrigin();
		const vec3f& dir = m_target->applyToUnitAxis(mat4f::Z_AXIS);
		const vec3f& up  = m_target->applyToUnitAxis(mat4f::Y_AXIS);

		setLookAt(pos, dir, up);
	}
	
	//----------------------------------------------------------------------------------------------
	void CursorCamera::updateMouse(float /*dx*/, float /*dy*/, float /*dz*/)
	{

	}

} //-- brUGE