#ifndef BATTLE_PATHFIND_H
#define BATTLE_PATHFIND_H

#include <deque>
#include <list>

#include "general.h"
#include "battle_loop.h"

struct Battle_Pathfinder_Edge;

struct Battle_Pathfinder_Node {
	struct List_Data {
		Battle_Pathfinder_Edge *edge;
		Battle_Loop::Jump_Point *jump_point;
	}; 
	std::list<List_Data> links;
	float x, y;
	int platform;
	
	float cost_from_start;
	float cost_to_goal;
	float total_cost;
	
	Battle_Pathfinder_Node *parent;
	
	// Need this to override copy constructor below
	Battle_Pathfinder_Node() {}

	// copy constructor and operator= for assignment
	Battle_Pathfinder_Node(const Battle_Pathfinder_Node &rhs) {
		links = rhs.links;
		x = rhs.x;
		y = rhs.y;
		platform = rhs.platform;
		cost_from_start = rhs.cost_from_start;
		cost_to_goal = rhs.cost_to_goal;
		total_cost = rhs.total_cost;
		parent = rhs.parent;
	}
	Battle_Pathfinder_Node &operator=(const Battle_Pathfinder_Node &rhs) {
		links = rhs.links;
		x = rhs.x;
		y = rhs.y;
		platform = rhs.platform;
		cost_from_start = rhs.cost_from_start;
		cost_to_goal = rhs.cost_to_goal;
		total_cost = rhs.total_cost;
		parent = rhs.parent;

		return *this;
	}
	bool operator<(const Battle_Pathfinder_Node &rhs) {
		return total_cost < rhs.total_cost;
	}
};

struct Battle_Pathfinder_Edge {
	Battle_Pathfinder_Node *start;
	Battle_Pathfinder_Node *end;

	// Need this to override copy constructor below
	Battle_Pathfinder_Edge() {}

	// copy constructor and operator= for assignment
	Battle_Pathfinder_Edge(const Battle_Pathfinder_Edge &rhs) {
		start = rhs.start;
		end = rhs.end;
	}
	Battle_Pathfinder_Edge &operator=(const Battle_Pathfinder_Edge &rhs) {
		start = rhs.start;
		end = rhs.end;

		return *this;
	}
};

class Battle_Pathfinder {
public:
	Battle_Pathfinder_Node *find_path(Battle_Pathfinder_Node *dest);

	Battle_Pathfinder(Battle_Pathfinder_Node *initial_state_and_start_node);
	~Battle_Pathfinder();

private:
	Battle_Pathfinder_Node *start_point;
};

#endif // _BATTLE_PATHFIND_H
