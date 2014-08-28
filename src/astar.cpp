#include "crystalpicnic.h"
#include "astar.h"

#include <vector>

#include "map_entity.h"
#include "character_map_entity.h"
#include "character_role.h"
#include "collision_detection.h"

namespace A_Star {

static bool processing;
static ALLEGRO_MUTEX *mutex;

struct Node_Internal {
	Node_Internal *parent;
	Node_Internal *child;
	int triangle_num;
	float cost_so_far;
	float estimated_total_cost;
	
	struct Branches {
		Node_Internal *b[3];
	};
	Branches branches;

	Node_Internal() {
		parent = NULL;
		child = NULL;
		triangle_num = -1;
		cost_so_far = estimated_total_cost = 0;
		branches.b[0] =
		branches.b[1] =
		branches.b[2] = NULL;
	}

	Node_Internal(const Node_Internal &n) {
		parent = n.parent;
		child = n.child;
		triangle_num = n.triangle_num;
		cost_so_far = n.cost_so_far;
		estimated_total_cost = n.estimated_total_cost;
		branches.b[0] = n.branches.b[0];
		branches.b[1] = n.branches.b[1];
		branches.b[2] = n.branches.b[2];
	}

	Node_Internal &operator=(const Node_Internal &rhs) {
		parent = rhs.parent;
		child = rhs.child;
		triangle_num = rhs.triangle_num;
		cost_so_far = rhs.cost_so_far;
		estimated_total_cost = rhs.estimated_total_cost;
		branches.b[0] = rhs.branches.b[0];
		branches.b[1] = rhs.branches.b[1];
		branches.b[2] = rhs.branches.b[2];
		return *this;
	}

	bool operator==(const Node_Internal &n) {
		return this->estimated_total_cost == n.estimated_total_cost;
	}
	bool operator<(const Node_Internal &n) {
		return this->estimated_total_cost < n.estimated_total_cost;
	}
};

struct A_Star_Info {
	Node_Internal *path_head;
	Node_Internal *path_tail;
};

static int num_layers_save;
static Navigation_Link ****nav_links_save;
static Node_Internal *find_node(int layer, Node_Internal *node, int looking_for);
static int find_layer(Navigation_Link ***nav_links);
static void reset_nodes(Node_Internal *node);
static Node_Internal **begin;
static std::vector <std::map<int, Node_Internal *> > node_maps;

static void create_hierarchy(
	int layer,
	std::vector<Triangulate::Triangle> &mesh,
	Navigation_Link ***nav_links,
	Node_Internal *this_node
);
void destroy_branches(Node_Internal *node);

void destroy_hierarchy()
{
	for (int i = 0; i < num_layers_save; i++) {
		if (begin[i])
			destroy_branches(begin[i]);
	}
	delete[] begin;
}

static Node_Internal **really_create_hierarchy(
	std::vector<Triangulate::Triangle> *mesh,
	Navigation_Link ****nav_links,
	int num_layers,
	int start_layer,
	int end_layer
)
{
	node_maps.clear();

	for (int i = 0; i < num_layers; i++) {
		node_maps.push_back(std::map<int, Node_Internal *>());
	}

	Node_Internal **st = new Node_Internal*[num_layers];
	for (int i = 0; i < num_layers; i++) {
		st[i] = NULL;
	}
	for (int i = start_layer; i < end_layer; i++) {;
		if (mesh[i].size() > 0) {
			st[i] = new Node_Internal;
			st[i]->triangle_num = 0;
			node_maps[i][0] = st[i];
			create_hierarchy(i, mesh[i], nav_links[i], st[i]);
		}
		else {
			st[i] = NULL;
		}
	}

	return st;
}

static inline void get_common_edge(
	std::vector<Triangulate::Triangle> &mesh,
	Navigation_Link ***nav_links,
	int tri_num_1, int tri_num_2,
	int &edge1, int &edge2)
{
	edge1 = -1;
	edge2 = -1;

	if (nav_links[tri_num_1][0]->triangle_num == tri_num_2) {
		edge1 = 0;
	}
	else if (nav_links[tri_num_1][1]->triangle_num == tri_num_2) {
		edge1 = 1;
	}
	else if (nav_links[tri_num_1][2]->triangle_num == tri_num_2) {
		edge1 = 2;
	}
	if (nav_links[tri_num_2][0]->triangle_num == tri_num_1) {
		edge2 = 0;
	}
	else if (nav_links[tri_num_2][1]->triangle_num == tri_num_1) {
		edge2 = 1;
	}
	else if (nav_links[tri_num_2][2]->triangle_num == tri_num_1) {
		edge2 = 2;
	}
}

General::Point<float> barycenter(Triangulate::Triangle *t)
{
	General::Point<float> c_01((t->points[0].x+t->points[1].x)/2, (t->points[0].y+t->points[1].y)/2);
	General::Point<float> c_12((t->points[1].x+t->points[2].x)/2, (t->points[1].y+t->points[2].y)/2);

	General::Point<float> centroid;

	checkcoll_line_line(&t->points[2], &c_01, &t->points[0], &c_12, &centroid);

	return centroid;
}

float get_cost(
	std::vector<Triangulate::Triangle> &mesh,
	Navigation_Link ***nav_links,
	int tri_num,
	int tri_num2,
	int tri_num_prev
) {
	if (tri_num_prev >= 0) {
		int edge1_a, edge1_b;
		int edge2_a, edge2_b;

		get_common_edge(mesh, nav_links, tri_num, tri_num2, edge1_a, edge1_b);
		get_common_edge(mesh, nav_links, tri_num, tri_num_prev, edge2_a, edge2_b);

		if (edge1_a >= 0 && edge2_a >= 0) {
			Navigation_Link *l1 = nav_links[tri_num][edge1_a];
			Navigation_Link *l3 = nav_links[tri_num][edge2_a];

			General::Point<float> tmp1 = l1->triangle->points[l1->edge_start];
			General::Point<float> tmp2 = l1->triangle->points[(l1->edge_start+1)%3];
			General::Point<float> tmp3 = l3->triangle->points[l3->edge_start];
			General::Point<float> tmp4 = l3->triangle->points[(l3->edge_start+1)%3];

			General::Point<float> center1, center2;
			center1.x = tmp1.x + (tmp2.x - tmp1.x) / 2;
			center1.y = tmp1.y + (tmp2.y - tmp1.y) / 2;
			center2.x = tmp3.x + (tmp4.x - tmp3.x) / 2;
			center2.y = tmp3.y + (tmp4.y - tmp3.y) / 2;

			float cost = General::distance(center1.x, center1.y, center2.x, center2.y);
			return cost;
		}
	}
		
	General::Point<float> tmp;
	General::Point<float> tmp2;

	tmp = barycenter(&mesh[tri_num]);
	tmp2 = barycenter(&mesh[tri_num2]);

	return General::distance(tmp.x, tmp.y, tmp2.x, tmp2.y);
}

static void create_hierarchy(
	int layer,
	std::vector<Triangulate::Triangle> &mesh,
	Navigation_Link ***nav_links,
	Node_Internal *this_node
) {
	std::stack<Node_Internal *> stack;

	stack.push(this_node);

	while (stack.size() > 0) {
		Node_Internal *node = stack.top();
		stack.pop();
		for (int i = 0; i < 3; i++) {
			if (nav_links[node->triangle_num][i]->triangle == NULL) {
				continue;
			}
			if (node->branches.b[i] != NULL) {
				continue;
			}
			
			int seeking = nav_links[node->triangle_num][i]->triangle_num;

			Node_Internal *new_node = find_node(layer, node, seeking);
			bool existed = new_node != NULL;

			if (existed == false) {
				new_node = new Node_Internal;
				new_node->triangle_num = seeking;
				node_maps[layer][seeking] = new_node;
			}

			int matching = -1;
			// find matching edge
			for (int j = 0; j < 3; j++) {
				if (nav_links[new_node->triangle_num][j]->triangle == NULL) {
					continue;
				}
				if (nav_links[new_node->triangle_num][j]->triangle_num == node->triangle_num) {
					matching = j;
					break;
				}
			}

			if (matching != -1) {
				node->branches.b[i] = new_node;
				new_node->branches.b[matching] = node;

				if (existed == false) {
					stack.push(new_node);
				}
			}
			else if (existed == false) {
				delete new_node;
			}
		}
	}
}

struct Possible {
	std::list<Node> path;
	int cost;
};

static std::list<Node> *really_real_astar(A_Star_Thread_Info *ati)
{
	std::vector<Triangulate::Triangle> *mesh = ati->mesh;
	Navigation_Link ***nav_links = ati->nav_links;

	int start_tri = ati->start_tri;

	if (start_tri == -1) {
		// We can find the closest triangle
		float closest = FLT_MAX;
		int closest_i = 0;
		for (size_t i = 0; i < mesh->size(); i++) {
			Triangulate::Triangle &t = (*mesh)[i];
			General::Point<float> p = barycenter(&t);
			float dist = General::distance(p.x, p.y, ati->start_pos.x, ati->start_pos.y);
			if (dist < closest) {
				closest = dist;
				closest_i = i;
			}
		}
		start_tri = closest_i;
	}

	int end_tri = ati->end_tri;
	A_Star_Info *info = ati->info;
	General::Point<float> start_pos = ati->start_pos;
	General::Point<float> end_pos = ati->end_pos;

	std::list<Node_Internal *> open;
	std::list<Node_Internal *> closed;

	int layer = find_layer(nav_links);
	Node_Internal *start = find_node(layer, begin[layer], start_tri);
	reset_nodes(start);
	//start->triangle_num = start_tri;
	start->cost_so_far = 0;
	General::Point<float> st = barycenter(&(*mesh)[start_tri]);
	General::Point<float> en = barycenter(&(*mesh)[end_tri]);
	start->estimated_total_cost =
		General::distance(st.x, st.y, en.x, en.y);
	start->parent = NULL;

	open.push_back(start);

	std::list<Node> *nodes = new std::list<Node>();

	int count = 0;
	Node_Internal *prev_node = NULL;
	int tri_num_prev;
	while (open.size() > 0) {
		if (count != 0) {
			tri_num_prev = prev_node->triangle_num;
		}
		else {
			tri_num_prev = start_tri;
		}
		count++;

		Node_Internal *node = open.front();
		prev_node = node;

		if (node->triangle_num == end_tri) {
			Node_Internal *tmp = node;
			tmp->child = NULL;
			while (tmp->triangle_num != start_tri) {
				tmp->parent->child = tmp;
				tmp = tmp->parent;
			}
			info->path_head = tmp;
			get_nodes(*mesh, nav_links, start_pos, end_pos, info, *nodes, ati->bones);
			break;
		}

		for (int i = 0; i < 3; i++) {	
			Node_Internal *tmp = node->branches.b[i];
			if (!tmp) {
				continue;
			}

			int tri_num1 = node->triangle_num;
			int tri_num2 = tmp->triangle_num;
			float heuristic = get_cost(*ati->mesh, ati->nav_links, tri_num1, tri_num2, tri_num_prev);

			float endNodeCost = node->cost_so_far + heuristic;

			Node_Internal *end_node = NULL;
			bool in_open, in_closed;
			std::list<Node_Internal *>::iterator it;
			it = std::find(closed.begin(), closed.end(), tmp);
			std::list<Node_Internal *>::iterator it2;
			it2 = std::find(open.begin(), open.end(), tmp);
			in_closed = it != closed.end();
			in_open = it2 != open.end();
			float endNodeHeuristic;

			if (in_closed) {
				end_node = *it;
				if (end_node->cost_so_far <= endNodeCost) {
					continue;
				}
				closed.erase(it);
				endNodeHeuristic = end_node->estimated_total_cost - end_node->cost_so_far;
			}
			else if (in_open) {
				end_node = *it2;
				if (end_node->cost_so_far <= endNodeCost) {
					continue;
				}
				endNodeHeuristic = end_node->estimated_total_cost - end_node->cost_so_far;
			}
			else {
				end_node = tmp;
				General::Point<float> st = barycenter(&(*mesh)[end_node->triangle_num]);
				General::Point<float> en = barycenter(&(*mesh)[end_tri]);
				endNodeHeuristic = General::distance(st.x, st.y, en.x, en.y);
			}

			end_node->cost_so_far = endNodeCost;
			end_node->parent = node;
			end_node->estimated_total_cost = endNodeCost + endNodeHeuristic;
			
			if (!in_open) {
				//open.push_back(end_node);
				std::list<Node_Internal *>::iterator it = open.begin();
				for (; it != open.end(); it++) {
					if ((*it)->cost_so_far >= end_node->cost_so_far) {
						break;
					}
				}
				open.insert(it, end_node);
			}
		}

		std::list<Node_Internal *>::iterator it = std::find(open.begin(), open.end(), node);
		if (it != open.end()) {
			open.erase(it);
		}

		closed.push_back(node);
	}
	
	return nodes;
}

void *real_astar(ALLEGRO_THREAD *thread, void *void_ati)
{
	A_Star_Thread_Info *ati = (A_Star_Thread_Info *)void_ati;

	while (!al_get_thread_should_stop(thread)) {
		while (ati->go == false && !al_get_thread_should_stop(thread)) {
			al_rest(0.1);
		}
		ati->go = false;
		if (al_get_thread_should_stop(thread)) {
			break;
		}

		al_lock_mutex(mutex);

		std::list<Node> *nodes = really_real_astar(ati);

		astar_callback cb = ati->cb;
		AStar_Callback_Interface *cb_interface = ati->cb_interface;

		if (nodes->size() > 0) {
			((*cb_interface).*cb)(nodes);
		}
		else {
			delete nodes;
			((*cb_interface).*cb)(NULL);
		}

		al_unlock_mutex(mutex);
		processing = false;
	}

	return NULL;
}

void astar(
	Map_Entity *calling_entity,
	astar_callback cb,
	std::vector<Triangulate::Triangle> &mesh,
	Navigation_Link ***nav_links,
	int start_tri,
	int end_tri,
	A_Star_Info *info,
	General::Point<float> start_pos,
	General::Point<float> end_pos,
	std::vector<Bones::Bone> bones)
{
	ALLEGRO_THREAD *thread;
	A_Star_Thread_Info *ati;

	calling_entity->get_astar_thread_info(&thread, &ati);
	bool thread_exists = thread != NULL;
	if (!thread_exists) {
		ati = new A_Star_Thread_Info;
		ati->go = false;
		thread = al_create_thread(real_astar, (void *)ati);
		al_start_thread(thread);
		calling_entity->set_astar_thread_info(thread, ati);
	}
	
	ati->cb_interface = NULL;
	Character_Map_Entity *character;
	if ((character = dynamic_cast<Character_Map_Entity *>(calling_entity))) {
		Character_Role *role = character->get_role();
		AStar_Callback_Interface *i;
		if ((i = dynamic_cast<AStar_Callback_Interface *>(role))) {
			ati->cb_interface = i; 
		}
	}
	ati->cb = cb;
	ati->mesh = &mesh;
	ati->nav_links = nav_links;
	ati->start_tri = start_tri;
	ati->end_tri = end_tri;
	ati->info = info;
	ati->start_pos = start_pos;
	ati->end_pos = end_pos;
	ati->bones = bones;
	ati->go = true;
}

void stop(A_Star_Info *info)
{
	info->path_head = NULL;
}

static int get_triangle_from_point(
	std::vector<Triangulate::Triangle> &mesh,
	General::Point<float> point)
{
	for (int i = 0; i < (int)mesh.size(); i++) {
		Triangulate::Triangle &t = mesh[i];
		if (checkcoll_point_triangle(&point, &t.points[0], &t.points[1], &t.points[2])) {
			return i;
		}
	}

	return -1;
}

/*
static void closest_point(float x1, float y1, float x2, float y2, float x3, float y3, float *x, float *y)
{
	float dx = x2 - x1;
	float dy = y2 - y1;
	float u = ((x3 - x1) * (x2 - x1) + (y3 - y1) * (y2 - y1)) / (dx*dx + dy*dy);
	*x = x1 + u * (x2 - x1);
	*y = y1 + u * (y2 - y1);
}
*/

struct My_Line {
	General::Point<float> a;
	General::Point<float> b;
};

static std::vector< std::list<My_Line> > boundaries;

static void real_find_intermediates(
	int layer,
	std::list<Node>::iterator begin_search_iterator,
	std::vector<Triangulate::Triangle> &mesh,
	General::Point<float> start_pos,
	General::Point<float> end_pos,
	std::vector<Bones::Bone> bones)
{
	std::list<Node>::iterator it = begin_search_iterator;
	General::Point<float> curr_pos = start_pos;

	while (true) {
		std::list<Node>::iterator end_it;
		int count = 0;
		end_it = it;

		while ((*end_it).exit_edge != -1) {
			end_it++;
			count++;
		}

		while (true) {
			Node &end_n = *end_it;
			
			General::Point<float> center;

			if (end_n.exit_edge == -1) {
				center = end_pos;
			}
			else {
				center = barycenter(&mesh[end_n.triangle_num]);
			}

			if (count == 0) {
				end_n.is_intermediate = true;
				end_n.intermediate_point = end_pos;
				break;
			}

			bool collided = false;

			// First check for direct line to end
			float dx = center.x - curr_pos.x;
			float dy = center.y - curr_pos.y;
			float a = atan2(dy, dx);
			float dist = sqrt(dx*dx + dy*dy);
			std::list<My_Line>::iterator it2;
			for (it2 = boundaries[layer].begin(); it2 != boundaries[layer].end(); it2++) {
				My_Line &l = *it2;
				for (size_t bone = 0; bone < bones.size(); bone++) {
					// quick proximity check
					General::Size<float> extents = bones[bone].get_extents();
					float max = MAX(extents.w, extents.h);
					float min = MIN(extents.w, extents.h);
					float d = General::circle_line_distance(
						curr_pos.x, curr_pos.y,
						l.a.x, l.a.y, l.b.x, l.b.y
					);
					if (d > (max+dist)+10) {
						continue;
					}
					std::vector< General::Point<float> > &outline = bones[bone].get_outline();
					for (size_t point = 0; point < outline.size(); point++) {
						General::Point<float> p1 = outline[point];
						General::Point<float> p2 = outline[(point+1) % outline.size()];
						for (int i = 0; i < dist; i += (min-0.5f)) {
							General::Point<float> p3(
								p1.x + (curr_pos.x + cos(a) * i),
								p1.y + (curr_pos.y + sin(a) * i) + General::BOTTOM_SPRITE_PADDING
							);
							General::Point<float> p4(
								p2.x + (curr_pos.x + cos(a) * i),
								p2.y + (curr_pos.y + sin(a) * i) + General::BOTTOM_SPRITE_PADDING
							);
							if (checkcoll_line_line(&p3, &p4, &l.a, &l.b, NULL)) {
								collided = true;
								break;
							}
						}
						if (collided) break;
					}
					if (collided) break;
				}
				if (collided) break;
			}

			if (!collided) {
				end_n.is_intermediate = true;
				end_n.intermediate_point = center;
				it = end_it;
				curr_pos = center;
	
				break;
			}

			end_it--;
			if (end_it == it) {
				if ((*end_it).exit_edge != -1) {
					end_it++;
				}

				end_n = *end_it;

				General::Point<float> tmp1 = mesh[end_n.triangle_num].points[end_n.exit_edge];
				General::Point<float> tmp2 = mesh[end_n.triangle_num].points[(end_n.exit_edge+1)%3];
				center.x = (tmp1.x + tmp2.x) / 2;
				center.y = (tmp1.y + tmp2.y) / 2;

				end_n.is_intermediate = true;
				end_n.intermediate_point = center;
				it = end_it;
				curr_pos = center;

				break;
			}
		}

		if ((*end_it).exit_edge == -1) {
			break;
		}
	}
}

static void find_intermediates(
	int layer,
	std::list<Node>::iterator begin_search_iterator,
	std::vector<Triangulate::Triangle> &mesh,
	General::Point<float> start_pos,
	General::Point<float> end_pos,
	std::vector<Bones::Bone> bones)
{
	real_find_intermediates(layer, begin_search_iterator, mesh, start_pos, end_pos, bones);
}

A_Star_Info *create(void)
{
	A_Star_Info *info = new A_Star_Info;
	info->path_head = info->path_tail = 0;
	return info;
}

void destroy_branches(Node_Internal *node)
{
	std::stack<Node_Internal *> stack;
	std::list<Node_Internal *> pushed;
	
	stack.push(node);
	pushed.push_back(node);
	
	while (stack.size() > 0) {
		Node_Internal *n = stack.top();
		stack.pop();

		for (int i = 0; i < 3; i++) {
			if (n->branches.b[i]) {
				if (std::find(pushed.begin(), pushed.end(), n->branches.b[i]) == pushed.end())
				{
					stack.push(n->branches.b[i]);
					pushed.push_back(n->branches.b[i]);
				}
			}
		}

		delete n;
	}
}

void destroy(A_Star_Info *info, bool delete_it)
{
	if (info->path_tail) {
		info->path_tail = NULL;
	}

	if (delete_it)
		delete info;
}

void set_path(
	Map_Entity *calling_entity,
	astar_callback cb,
	std::vector<Triangulate::Triangle> &mesh,
	Navigation_Link ***nav_links,
	General::Point<float> start_pos,
	General::Point<float> end_pos,
	std::vector<Bones::Bone> bones,
	A_Star_Info *info)
{
	int start_tri = get_triangle_from_point(mesh, start_pos);
	int end_tri = get_triangle_from_point(mesh, end_pos);

	astar(calling_entity, cb, mesh, nav_links, start_tri, end_tri, info, start_pos, end_pos, bones);

}

void get_nodes(
	std::vector<Triangulate::Triangle> &mesh,
	Navigation_Link ***nav_links,
	General::Point<float> start_pos,
	General::Point<float> end_pos,
	A_Star_Info *info,
	std::list<Node> &nodes,
	std::vector<Bones::Bone> bones
) {
	int last_enter_edge = -1;

	Node_Internal *n = info->path_head;
	do {
		Node node;
		node.is_intermediate = false;
		node.triangle_num = n->triangle_num;
		node.enter_edge = last_enter_edge;
		if (!n->child) {
			node.exit_edge = -1;
		}
		else {
			Node_Internal *n2 = n->child;
			get_common_edge(mesh, nav_links, n->triangle_num, n2->triangle_num, node.exit_edge, last_enter_edge);
		}

		nodes.push_back(node);

		n = n->child;
	} while (n);

	int layer = find_layer(nav_links);

	find_intermediates(
		layer,
		nodes.begin(),
		mesh,
		start_pos,
		end_pos,
		bones
	);
}

std::list<Node>::iterator next_intermediate_node(std::list<Node> &nodes, std::list<Node>::iterator start)
{
	std::list<Node>::iterator it = start;

	for (; it != nodes.end(); it++) {
		if ((*it).is_intermediate) {
			break;
		}
	}

	return it;
}

General::Point<float> dest_point_from_node(
	std::vector<Triangulate::Triangle> &mesh,
	Navigation_Link ***nav_links,
	std::list<Node> &nodes,
	General::Point<float> astar_dest,
	std::list<Node>::iterator astar_intermediate_node)
{
	if (astar_intermediate_node == nodes.end()) {
		return astar_dest;
	}

	return (*astar_intermediate_node).intermediate_point;
}

/* Create the whole hierarchy of nodes */
void start_area(
	std::vector<Triangulate::Triangle> *mesh,
	Navigation_Link ****nav_links,
	int num_layers
) {
	mutex = al_create_mutex();
	begin = really_create_hierarchy(mesh, nav_links, num_layers, 0, num_layers);
	num_layers_save = num_layers;
	nav_links_save = nav_links;

	for (int layer = 0; layer < num_layers; layer++) {
		boundaries.push_back(std::list<My_Line>());
		for (int i = 0; i < (int)mesh[layer].size(); i++) {
			Triangulate::Triangle &t = mesh[layer][i];
			for (int j = 0; j < 3; j++) {
				int j2 = (j+1) % 3;
				My_Line l;
				l.a = t.points[j];
				l.b = t.points[j2];
				// If this line already exists in boundaries, don't add it
				std::list<My_Line>::iterator it;
				bool found = false;
				for (it = boundaries[layer].begin(); it != boundaries[layer].end(); it++) {
					My_Line &l2 = *it;
					if ((l2.a == l.a && l2.b == l.b) || (l2.b == l.a && l2.a == l.b)) {
						found = true;
						boundaries[layer].erase(it);
						break;
					}
				}
				if (found) {
					continue;
				}
				boundaries[layer].push_back(l);
			}
		}
	}
}

void end_area(void)
{
	destroy_hierarchy();
	boundaries.clear();
	al_destroy_mutex(mutex);
}

static int find_layer(Navigation_Link ***nav_links)
{
	for (int i = 0; i < num_layers_save; i++) {
		if (nav_links_save[i] == nav_links)
			return i;
	}

	return 0;
}

static Node_Internal *find_node(int layer, Node_Internal *_node, int looking_for)
{
	if (node_maps[layer].find(looking_for) != node_maps[layer].end()) {
		return node_maps[layer][looking_for];
	}

	return NULL;
}

static void reset_nodes(Node_Internal *_node)
{
	std::map<Node_Internal *, bool> reset;
	std::stack<Node_Internal *> stack;
	stack.push(_node);

	while (stack.size() > 0) {
		Node_Internal *node = stack.top();
		stack.pop();
		node->parent = NULL;
		node->child = NULL;
		node->cost_so_far = node->estimated_total_cost = 0;
		reset[node] = true;

		for (int i = 0; i < 3; i++) {
			if (node->branches.b[i] == NULL)
				continue;
			if (reset.find(node->branches.b[i]) != reset.end())
				continue;
			stack.push(node->branches.b[i]);
		}
	}
}

bool is_processing()
{
	return processing;
}

void set_processing(bool proc)
{
	processing = proc;
}

ALLEGRO_MUTEX *get_mutex()
{
	return mutex;
}

} // end namespace
