#ifndef ASTAR_H
#define ASTAR_H

#include <list>
#include <vector>
#include <utility>
#include <stack>

#include <allegro5/allegro.h>

#include "general.h"
#include "triangulate.h"
#include "map_entity.h"
#include "bones.h"

class AStar_Character_Role;

namespace A_Star {

struct Navigation_Link {
	Triangulate::Triangle *triangle;
	int triangle_num;
	int edge_start;
};

struct Node {
	int triangle_num;
	int enter_edge;
	int exit_edge;
	bool is_intermediate;
	General::Point<float> intermediate_point;
};

class AStar_Callback_Interface {
public:
	virtual void astar_callback(std::list<Node> *list_nodes) = 0;
	virtual ~AStar_Callback_Interface() {}
};

typedef void (AStar_Callback_Interface::*astar_callback)(std::list<Node> *);

struct A_Star_Info;

struct A_Star_Thread_Info {
	astar_callback cb;
	AStar_Callback_Interface *cb_interface;
	std::vector<Triangulate::Triangle> *mesh;
	Navigation_Link ***nav_links;
	int start_tri;
	int end_tri;
	A_Star_Info *info;
	General::Point<float> start_pos;
	General::Point<float> end_pos;
	std::vector<Bones::Bone> bones;
	bool go;
};

A_Star_Info *create();
void destroy(A_Star_Info *i, bool delete_it = true);
void stop(A_Star_Info *info);

void set_path(
	Map_Entity *calling_entity,
	astar_callback cb,
	std::vector<Triangulate::Triangle> &mesh,
	Navigation_Link ***nav_links,
	General::Point<float> start_pos,
	General::Point<float> end_pos,
	std::vector<Bones::Bone> bones,
	A_Star_Info *info);

void get_nodes(
	std::vector<Triangulate::Triangle> &mesh,
	Navigation_Link ***nav_links,
	General::Point<float> start_pos,
	General::Point<float> end_pos,
	A_Star_Info *info,
	std::list<Node> &nodes,
	std::vector<Bones::Bone> bones);

std::list<Node>::iterator next_intermediate_node(std::list<Node> &nodes, std::list<Node>::iterator start);

General::Point<float> dest_point_from_node(
	std::vector<Triangulate::Triangle> &mesh,
	Navigation_Link ***nav_links,
	std::list<Node> &nodes,
	General::Point<float> astar_dest,
	std::list<Node>::iterator astar_intermediate_node);

void start_area(
	std::vector<Triangulate::Triangle> *mesh,
	Navigation_Link ****nav_links,
	int num_layers
);
void end_area();

General::Point<float> barycenter(Triangulate::Triangle *t);

bool is_processing();
void set_processing(bool processing);
ALLEGRO_MUTEX *get_mutex();

} // end namespace

#endif // ASTAR_H
