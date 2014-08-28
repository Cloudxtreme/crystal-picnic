#include "battle_pathfind.h"

Battle_Pathfinder_Node *Battle_Pathfinder::find_path(Battle_Pathfinder_Node *dest)
{
	std::list<Battle_Pathfinder_Node *> open;
	std::list<Battle_Pathfinder_Node *> closed;

	start_point->cost_from_start = 0;
	start_point->cost_to_goal = start_point->cost_from_start + General::distance(start_point->x, start_point->y, dest->x, dest->y);
	start_point->parent = NULL;

	open.push_back(start_point);

	while (open.size() > 0) {
		Battle_Pathfinder_Node *node = open.front();

		if (node == dest) {
			return dest;
		}

		std::list<Battle_Pathfinder_Node::List_Data>::iterator it;
		for (it = node->links.begin(); it != node->links.end(); it++) {
			Battle_Pathfinder_Node::List_Data &data = *it;
			Battle_Pathfinder_Edge *e = data.edge;

			Battle_Pathfinder_Node *tmp = e->start == node ? e->end : e->start;

			float heuristic = General::distance(tmp->x, tmp->y, dest->x, dest->y);
			float end_node_cost = node->cost_from_start + heuristic;

			Battle_Pathfinder_Node *end_node = NULL;

			bool in_open, in_closed;
			std::list<Battle_Pathfinder_Node *>::iterator it;
			it = std::find(closed.begin(), closed.end(), tmp);
			std::list<Battle_Pathfinder_Node *>::iterator it2;
			it2 = std::find(open.begin(), open.end(), tmp);
			in_closed = it != closed.end();
			in_open = it2 != open.end();

			float end_node_heuristic;

			if (in_closed) {
				end_node = *it;
				if (end_node->cost_from_start <= end_node_cost) {
					continue;
				}
				closed.erase(it);
				end_node_heuristic = end_node->cost_to_goal - end_node->cost_from_start;
			}
			else if (in_open) {
				end_node = *it2;
				if (end_node->cost_from_start <= end_node_cost) {
					continue;
				}
				end_node_heuristic = end_node->cost_to_goal - end_node->cost_from_start;
			}
			else {
				end_node = tmp;
				end_node_heuristic = General::distance(end_node->x, end_node->y, dest->x, dest->y);
			}

			end_node->cost_from_start = end_node_cost;
			end_node->cost_to_goal = end_node_cost + end_node_heuristic;
			end_node->parent = node;
			
			if (!in_open) {
				//open.push_back(end_node);
				std::list<Battle_Pathfinder_Node *>::iterator it = open.begin();
				for (; it != open.end(); it++) {
					if ((*it)->cost_from_start >= end_node->cost_from_start) {
						break;
					}
				}
				open.insert(it, end_node);
			}
		}

		std::list<Battle_Pathfinder_Node *>::iterator it3 = std::find(open.begin(), open.end(), node);
		if (it3 != open.end()) {
			open.erase(it3);
		}

		closed.push_back(node);
	}
	
	return NULL;
}

Battle_Pathfinder::Battle_Pathfinder(Battle_Pathfinder_Node *initial_state_and_start_node) :
	start_point(initial_state_and_start_node)
{
}

Battle_Pathfinder::~Battle_Pathfinder(void)
{
}

