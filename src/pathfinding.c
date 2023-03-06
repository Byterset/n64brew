#include <assert.h>

#include "constants.h"
#include "pathfinding.h"
#include "math/vec3d.h"

#ifdef __N64__
#include "mathdef.h"
#else
#include "float.h"
#endif

float Path_distance(Node *a, Node *b)
{
	invariant(a != NULL);
	invariant(b != NULL);
	return Vec3d_distanceTo(&a->position, &b->position);
}

float Path_heuristic(Node *a, Node *b)
{
	return Path_distance(a, b);
}

void Path_initState(Graph *graph,
					PathfindingState *state,
					Node *start,
					Node *end,
					NodeState *nodeStates,
					int nodeStateSize,
					int *result)
{
	int i;
	Node *node;
	NodeState *nodeState;
	state->start = start;
	state->end = end;
	state->nodeStates = nodeStates;
	state->nodeStateSize = nodeStateSize;
	state->result = result;
	state->resultSize = 0;
	for (i = 0, node = graph->nodes, nodeState = state->nodeStates; //
		 i < graph->size;											//
		 i++, node++, nodeState++									//
	)
	{
		nodeState->nodeID = i;
		nodeState->node = node;
		nodeState->reachedViaNode = NULL;
		nodeState->costSoFar = 0.0f;
		nodeState->estimatedTotalCost = 0.0f;
		nodeState->category = UnvisitedNodeStateCategory;
	}
	state->open = 0;
}

void Path_addToOpenList(PathfindingState *state, NodeState *nodeState)
{
	nodeState->category = OpenNodeStateCategory;
	state->open++;
}

NodeState *Path_getSmallestOpenNode(Graph *graph, PathfindingState *state)
{
	int i;
	NodeState *nodeState;
	NodeState *minCostNodeState;

	float minCost = FLT_MAX;
	minCostNodeState = state->nodeStates;
	for (									  //
		i = 0, nodeState = state->nodeStates; //
		i < graph->size;					  //
		i++, nodeState++					  //
	)
	{
		if (nodeState->category == OpenNodeStateCategory)
		{
			if (minCostNodeState == NULL || nodeState->estimatedTotalCost < minCost)
			{
				minCost = nodeState->estimatedTotalCost;
				minCostNodeState = nodeState;
			}
		}
	}
	return minCostNodeState;
}

Node *Path_getNodeByID(Graph *graph, int nodeID)
{
	return graph->nodes + nodeID; // offset into edges
}

EdgeList *Path_getNodeEdgesByID(Graph *graph, int nodeID)
{
	return graph->edges + nodeID; // offset into edges
}

NodeState *Path_getNodeState(PathfindingState *state, int nodeID)
{
	return state->nodeStates + nodeID; // offset into edges
}

int Path_quantizePosition(Graph *graph, Vec3d *position)
{
	int closestNode = 0;
	float closestNodeDist = FLT_MAX;
	float currentNodeDist;
	int i;

	// for now just find closest node to pos
	// could be invalid, like on the other side of a wall :(
	for (i = 0; i < graph->size; ++i)
	{
		currentNodeDist =
			Vec3d_distanceTo(position, &Path_getNodeByID(graph, i)->position);
		if (currentNodeDist < closestNodeDist)
		{
			closestNode = i;
			closestNodeDist = currentNodeDist;
		}
	}
	return closestNode;
}

void Path_reverse(PathfindingState *state)
{
	int temp;
	int start;
	int end;
	start = 0;
	end = state->resultSize - 1;
	while (start < end)
	{
		temp = state->result[start];
		state->result[start] = state->result[end];
		state->result[end] = temp;
		start++;
		end--;
	}
}

// based on https://www.geometrictools.com/GTE/Mathematics/DistPointSegment.h
float Path_getClosestPointParameter(Vec3d *segmentPoint0,
									Vec3d *segmentPoint1,
									Vec3d *point)
{
	float resultSegmentParameter;
	float sqrLength;
	float t;
	Vec3d diff;
	Vec3d direction;

	// The direction vector is not unit length.  The normalization is
	// deferred until it is needed.
	direction = *segmentPoint1;
	Vec3d_sub(&direction, segmentPoint0);
	diff = *point;
	Vec3d_sub(&diff, segmentPoint1);

	t = Vec3d_dot(&direction, &diff);
	if (t >= (float)0)
	{
		resultSegmentParameter = 1;
		// resultSegmentClosest = segmentPoint1;
	}
	else
	{
		diff = *point;
		Vec3d_sub(&diff, segmentPoint0);
		t = Vec3d_dot(&direction, &diff);
		if (t <= (float)0)
		{
			resultSegmentParameter = (float)0;
			// resultSegmentClosest = segmentPoint0;
		}
		else
		{
			sqrLength = Vec3d_dot(&direction, &direction);
			if (sqrLength > (float)0)
			{
				t /= sqrLength;
				resultSegmentParameter = t;
				// resultSegmentClosest = segmentPoint0 + t * direction;
			}
			else
			{
				resultSegmentParameter = (float)0;
				// resultSegmentClosest = segmentPoint0;
			}
		}
	}

	// diff = point - resultSegmentClosest;
	// result.sqrDistance = Dot(diff, diff);
	// result.distance = sqrtf(result.sqrDistance);

	return resultSegmentParameter;
}

// based on A* implementation from AI For Games
int Path_findAStar(Graph *graph, PathfindingState *state)
{
	// This structure is used to keep track of the
	// information we need for each node.
	NodeState *startNode;
	NodeState *current;
	EdgeList *edges;
	int *reachedViaNodeID;
	float edgeCost;
	int i;
	NodeState *endNode;
	float endNodeCost; // current to end node cost
	float endNodeHeuristic;
	int *result;
	// Initialize the record for the start node.
	startNode = Path_getNodeState(state, state->start->id);
	startNode->estimatedTotalCost = Path_heuristic(state->start, state->end);
	startNode->costSoFar = 0.0f;
	Path_addToOpenList(state, startNode);

	while (state->open > 0)
	{
		// Find the smallest element in the open list (using the
		// estimatedTotalCost).
		current = Path_getSmallestOpenNode(graph, state);

		// If it is the goal node, then terminate.
		if (current->node == state->end)
		{
			break;
		}

		// Otherwise get its outgoing edges.
		edges = Path_getNodeEdgesByID(graph, current->nodeID);

		// Loop through each edge in turn.
		for (										   //
			i = 0, reachedViaNodeID = edges->elements; //
			i < edges->size;						   //
			i++, reachedViaNodeID++					   //
		)
		{
			endNode = Path_getNodeState(state, *reachedViaNodeID);
			edgeCost = Path_distance(current->node, endNode->node);
			// Get the cost estimate for the end node.
			endNodeCost = current->costSoFar + edgeCost;

			// If the node is closed we may have to skip, or remove it
			// from reachedViaNode closed list.
			if (endNode->category == ClosedNodeStateCategory)
			{
				// If we didn’t find a shorter route, skip.
				if (endNode->costSoFar <= endNodeCost)
				{
					continue;
				}
				// Otherwise remove it from the closed list.
				endNode->category = UnvisitedNodeStateCategory;
				// We can use the node’s old cost values to calculate
				// its heuristic without calling the possibly expensive
				// heuristic function.
				endNodeHeuristic = endNode->estimatedTotalCost - endNode->costSoFar;
			}
			else if (endNode->category == OpenNodeStateCategory)
			{
				// Skip if the node is open and we’ve not found a better  route.

				// If our route is no better, then skip.
				if (endNode->costSoFar <= endNodeCost)
				{
					continue;
				}
				// Again, we can calculate its heuristic.
				endNodeHeuristic = edgeCost - endNode->costSoFar;
			}
			else
			{
				// Otherwise we know we’ve got an unvisited node, so init it

				// We’ll need to calculate the heuristic value using
				// the function, since we don’t have an existing record
				// to use.
				endNodeHeuristic = Path_heuristic(endNode->node, state->end);
			}

			// We’re here if we need to update the node. Update the
			// cost, estimate and edge.
			endNode->costSoFar = endNodeCost;
			endNode->reachedViaNode = current->node;
			endNode->estimatedTotalCost = endNodeCost + endNodeHeuristic;

			// And add it to the open list.
			if (endNode->category != OpenNodeStateCategory)
			{
				Path_addToOpenList(state, endNode);
			}
		}
		// We’ve finished looking at the edges for the current
		// node, so add it to the closed list and remove it from the
		// open list.
		if (current->category == OpenNodeStateCategory)
		{
			state->open--;
		}
		current->category = ClosedNodeStateCategory;
	}

	// We’re here if we’ve either found the goal, or if we’ve no more
	// nodes to search, find which.
	if (current->node != state->end)
	{
		// We’ve run out of nodes without finding the goal, so there’s
		// no solution.
		debugPrintf("Pathfinding: ran out of nodes without finding the goal\n");
		return FALSE;
	}
	else
	{
		// Compile the list of edges in the path.

		result = state->result;

		// add end node
		*result = current->nodeID;
		state->resultSize++;
		result++;

		// Work back along the path, accumulating edges.
		while (current->reachedViaNode != NULL)
		{
			current = Path_getNodeState(state, current->reachedViaNode->id);
			// add node
			*result = current->nodeID;
			state->resultSize++;
			result++;
		}

		Path_reverse(state);

		// path is in state->result
		return TRUE;
	}
}
