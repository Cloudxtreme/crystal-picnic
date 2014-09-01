#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <allegro5/allegro.h>

#include <map>
#include <string>

#include <wrap.h>

#include "general.h"

class Resource_Manager {
public:
	Wrap::Bitmap *reference_bitmap(std::string filename);
	void release_bitmap(std::string filename);

private:
	struct Resource {
		unsigned int reference_count;
		void *ptr;
	};

	std::map<std::string, Resource *> bitmaps;
};

extern Resource_Manager *resource_manager;


#endif // RESOURCE_MANAGER_H
