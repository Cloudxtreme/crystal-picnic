#ifndef USER_EVENTS_H
#define USER_EVENTS_H

#include "battle_loop.h"

enum User_Event_Type {
	// AI related
	UE_Find_Path = ALLEGRO_GET_EVENT_TYPE('B', 'R', 'Y', 'N'),
	UE_Follow_Edge,
	UE_Rest,
	UE_Stop,
	UE_Direct_Move,
	UE_Seek_On_Platform
};

class User_Event_Data {
public:
	int target_id;
	User_Event_Data() {}
	User_Event_Data(const User_Event_Data &rhs) {
		target_id = rhs.target_id;
	}
};

class UE_Find_Path_Data : public User_Event_Data {
public:
	float dx, dy;
	int platform;
	double end_time;
	float within;
	UE_Find_Path_Data() {
		end_time = 0;
	}
	UE_Find_Path_Data(const UE_Find_Path_Data &rhs) :
		User_Event_Data(rhs)
	{
		dx = rhs.dx;
		dy = rhs.dy;
	}
};

class UE_Follow_Edge_Data : public User_Event_Data {
public:
	float sx, sy;
	float dx, dy;
	int start_platform;
	int end_platform;
	Battle_Loop::Jump_Point_Type jump_type;
	double end_time;
	UE_Follow_Edge_Data() {}
	UE_Follow_Edge_Data(const UE_Follow_Edge_Data &rhs) :
		User_Event_Data(rhs)
	{
		sx = rhs.sx;
		sy = rhs.sy;
		dx = rhs.dx;
		dy = rhs.dy;
		start_platform = rhs.start_platform;
		end_platform = rhs.end_platform;
	}
};

class UE_Rest_Data : public User_Event_Data {
public:
	double countdown;
	bool force;
	UE_Rest_Data() {}
	UE_Rest_Data(const UE_Rest_Data &rhs) :
		User_Event_Data(rhs)
	{
		countdown = rhs.countdown;
	}
};

class UE_Stop_Data : public User_Event_Data {
public:
	UE_Stop_Data() {}
	UE_Stop_Data(const UE_Stop_Data &rhs) :
		User_Event_Data(rhs)
	{
	}
};

class UE_Direct_Move_Data : public User_Event_Data {
public:
	float x, y;
	UE_Direct_Move_Data() {}
	UE_Direct_Move_Data(const UE_Direct_Move_Data &rhs) :
		User_Event_Data(rhs)
	{
		x = rhs.x;
		y = rhs.y;
	}
};

class UE_Seek_On_Platform_Data : public User_Event_Data {
public:
	float x;
	//int platform;
	UE_Seek_On_Platform_Data() {}
	UE_Seek_On_Platform_Data(const UE_Seek_On_Platform_Data &rhs) :
		User_Event_Data(rhs)
	{
		x = rhs.x;
	}
};

#endif // _USER_EVENTS_H

