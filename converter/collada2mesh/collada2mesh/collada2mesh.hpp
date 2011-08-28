#pragma once

#include "prerequisites.hpp"
#include "math/math_all.hpp"
#include "utils/Data.hpp"
#include "utils/NonCopyable.hpp"
#include "render/mesh_formats.hpp"

namespace brUGE
{

	void collada2staticmesh(const utils::ROData& iData, utils::WOData& oData);
	void collada2skinnedmesh(const utils::ROData& iData, utils::WOData& oData);
	void collada2animation(const utils::ROData& iData, utils::WOData& oData);

} //-- brUGE