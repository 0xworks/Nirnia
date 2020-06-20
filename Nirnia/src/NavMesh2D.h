#pragma once

#include "clipper.hpp"
#include "glm/glm.hpp"

#include <vector>

class NavMesh2D {
public:

	// Add a polygon to the list of walkable areas
	// Polygon is defined by vertices, in counter-clockwise winding order
	// Polygon does not have to be convex
	// After adding a polyon, BakeMesh() must be called before testing IsWalkable(), or requesting a path
	void AddWalkablePolygon(const std::vector<glm::vec2>& walkable);
	// TODO: AddWalkablePolygons(...)

	// Add a polygon to the list of blocked (i.e. non-walkable) areas
	// Polygon is defined by vertices, in counter-clockwise winding order
	// Polygon does not have to be convex
	// After adding a polyon, BakeMesh() must be called before testing IsWalkable(), or requesting a path
	void AddBlockedPolygon(const std::vector<glm::vec2>& blocked);
	// TODO: AddBlockedPolygons(...)

	// Returns true if and only if the point specified by pos is in the walkable area of the mesh
	// TODO: could be made much more efficient (after mesh is "baked")
	bool IsWalkable(const glm::vec2& pos);

	// TODO:
	// "Bakes" a nav mesh.
	// This computes the union of all walkable polygons, minus the intersection with blocked polygons, where
	// each blocked polygon is buffered by the agent size.
	// Then triangulates the result into a "mesh" that can be used for path finding.
	//void BakeMesh(const float agentSize);

	// TODO:
	// Gets a navigable path through a baked mesh from start point to end point
	// GetPath(const vec2 start, const vec2 end, const float agentSize);

private:
	constexpr static float scaling = 100000.0f; // convert glm input (floating point) to scaled integer (for clipper)

	ClipperLib::Paths m_WalkablePolygons;
	ClipperLib::Paths m_BlockedPolygons;
};
