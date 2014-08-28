#ifndef BATTLE_AI_H
#define BATTLE_AI_H

#include <list>

#include "loop.h"
#include "user_events.h"

template <typename T> class Goal_List {
public:
	std::list<T> &get_list() {
		return list;
	}

	void add_data(typename std::list<T>::iterator spot, typename std::list<T>::iterator start, typename std::list<T>::iterator end) {
		list.insert(spot, start, end);
	}

	void add_data(typename std::list<T>::iterator spot, T data) {
		list.insert(spot, data);
	}

	void add_data(T data) {
		add_data(list.end(), data);
	}

	void pop_front() {
		if (list.size() > 0)
			list.erase(list.begin());
	}

	T get_data() {
		return *(list.begin());
	}

	Goal_List<T>()
	{
	}
	
protected:

	std::list<T> list;
};

struct Battle_AI_Action {
	User_Event_Type type;
	User_Event_Data *data;
};

class Battle_Entity;

class Battle_AI : public Loop {
public:
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();
	
	void stop();
	
	bool get_should_attack();
	void set_should_attack(bool should_attack);

	int get_A_PLAYER();
	int get_A_ENEMY();

	void set_aggressiveness(int aggressiveness);

	Battle_AI(Battle_Entity *body);
	virtual ~Battle_AI();

	lua_State *get_lua_state();

protected:
	void process_next_action();
	void decide();
	bool maybe_attack();
	void next_goal();
	void init_lua();
	
	// push events on queue
	void push_rest(double time, bool force);
	void push_stop();

	Battle_Entity *body;

	Battle_AI_Action *current_goal;
	Goal_List<Battle_AI_Action *> goals;

	Battle_Loop *battle_loop;

	bool waiting;

	int aggressiveness; // # of ticks on average between attacks (when possible)
	int missed_attacks; // keep up with aggressiveness

	lua_State *lua_state;

	bool finished_jump;
	bool jumping_down;
	bool double_jumped;
	bool double_jump_stopped; // released button1 in preparation for double jump?
	
	int A_PLAYER;
	int A_ENEMY;
	
	bool should_attack;

	int attack_type;
};

#endif // _BATTLE_AI_H
