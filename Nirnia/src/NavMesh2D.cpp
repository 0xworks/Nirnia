#include "NavMesh2D.h"

void NavMesh2D::AddWalkablePolygon(const std::vector<glm::vec2>& walkable) {
	ClipperLib::Path path;
	path.reserve(walkable.size());
	for (const auto& vertex : walkable) {
		path.emplace_back(static_cast<ClipperLib::cInt>(vertex.x * scaling), static_cast<ClipperLib::cInt>(vertex.y * scaling));
	}
	m_WalkablePolygons.emplace_back(std::move(path));
}


void NavMesh2D::AddBlockedPolygon(const std::vector<glm::vec2>& blocked) {
	ClipperLib::Path path;
	path.reserve(blocked.size());
	for (const auto& vertex : blocked) {
		path.emplace_back(static_cast<ClipperLib::cInt>(vertex.x * scaling), static_cast<ClipperLib::cInt>(vertex.y * scaling));
	}
	m_BlockedPolygons.emplace_back(std::move(path));
}


//void NavMesh2D::BakeMesh(const float agentSize) {
	// TODO:
	//   combine all walkable polygons
	//   subtract all blocked polygons
	//   triangulate result
	//   add spatial index on triangles (maybe?)
//}


bool NavMesh2D::IsWalkable(const glm::vec2& pos) {
	// Must be in a walkable polygon, and not in any blocked polygon
	// This is the dumbest (but simplest) way to do it.
	// Also, does not account for agent size (we are not buffering the blocked polygons here)
	// TODO: make better (where better == faster, and accounts for agent size)

	bool walkable = false;
	bool blocked = false;
	for (const auto& polygon : m_WalkablePolygons) {
		if (ClipperLib::PointInPolygon(ClipperLib::IntPoint {static_cast<ClipperLib::cInt>(pos.x * scaling), static_cast<ClipperLib::cInt>(pos.y * scaling)}, polygon)) {
			walkable = true;
			break;
		}
	}
	if (walkable) {
		for (const auto& polygon : m_BlockedPolygons) {
			if (ClipperLib::PointInPolygon(ClipperLib::IntPoint {static_cast<ClipperLib::cInt>(pos.x * scaling), static_cast<ClipperLib::cInt>(pos.y * scaling)}, polygon)) {
				blocked = true;
				break;
			}
		}
	}

	return walkable && !blocked;
}
