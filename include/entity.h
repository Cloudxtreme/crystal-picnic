#ifndef ENTITY_H
#define ENTITY_H

#include <allegro5/allegro.h>

class Entity
{
public:
	virtual void handle_event(ALLEGRO_EVENT *event) {}
	virtual void logic() {}
	virtual void draw() {}
	virtual void init() {}
	
	int get_id() { return id; }
	void set_id(int id) { this->id = id; }

	void set_delete_me(bool delete_me);
	bool get_delete_me();

	Entity();
	virtual ~Entity() {}

protected:
	int id;
	bool delete_me; // do not delete this variable
};

#endif // ENTITY_H

