#pragma once

#include "prerequisites.hpp"
#include "render/game_world.hpp"
#include "utils/Data.hpp"
#include "render/vertex_format.hpp"
#include <vector>

//--------------------------------------------------------------------------------------------------
class EditorGameObj : public brUGE::IGameObj
{
public:
	EditorGameObj();
	virtual ~EditorGameObj();

	//-- serialization functions.
	virtual bool load(const brUGE::utils::ROData& inData, Handle objID, const brUGE::mat4f* orient = NULL);
	virtual void beginUpdate(float dt);

	void showNormals(bool flag)	{ m_showDebugNormals = flag; }

private:
	bool m_showDebugNormals;
	std::vector<brUGE::render::VertexXYZC> m_normals;
};