#include "EditorObj.hpp"
#include "loader/ResourcesManager.h"
#include "render/DebugDrawer.h"
#include "engine/Engine.h"
#include "render/animation_engine.hpp"
#include "pugixml/pugixml.hpp"

using namespace brUGE;
using namespace brUGE::math;
using namespace brUGE::utils;
using namespace brUGE::render;
using namespace brUGE::physic;

//--------------------------------------------------------------------------------------------------
EditorGameObj::EditorGameObj() : m_showDebugNormals(false)
{

}

//--------------------------------------------------------------------------------------------------
EditorGameObj::~EditorGameObj()
{

}

//--------------------------------------------------------------------------------------------------
bool EditorGameObj::load(const ROData& inData, Handle objID, const mat4f* orient)
{
	bool success = IGameObj::load(inData, objID, orient);

	if (success)
	{
		pugi::xml_document	   doc;
		pugi::xml_parse_result result = doc.load_buffer(inData.ptr(), inData.length());
		
		const char* meshName = doc.document_element().child("render").attribute("file").value();

		ResourcesManager::instance().loadMeshDebugData(meshName, m_normals);

		if (getFileExt(meshName) == "md5mesh")
		{
			Engine::instance().animationEngine().playAnim(m_animCtrl, "death1_pose", true);
		}
	}

	return success;
}

//--------------------------------------------------------------------------------------------------
void EditorGameObj::beginUpdate(float /*dt*/)
{
	if (m_showDebugNormals)
	{
		DebugDrawer::instance().drawLinesList(m_normals);
	}
}
