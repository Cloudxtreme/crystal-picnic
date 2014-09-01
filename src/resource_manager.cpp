#include "resource_manager.h"

Resource_Manager *resource_manager;

Wrap::Bitmap *Resource_Manager::reference_bitmap(std::string filename)
{
	std::map<std::string, Resource *>::iterator it =
		bitmaps.find(filename);
	
	if (it == bitmaps.end()) {
		Resource *r = new Resource;
		r->reference_count = 1;
		Wrap::Bitmap *b = Wrap::load_bitmap(filename);
		if (b) {
			r->ptr = b;
			bitmaps[filename] = r;
			return b;
		}
		else {
			delete r;
			return NULL;
		}
	}
	else {
		std::pair<std::string, Resource *> p = *it;
		Resource *r = p.second;
		r->reference_count++;
		return (Wrap::Bitmap *)r->ptr;
	}
}

void Resource_Manager::release_bitmap(std::string filename)
{
	std::map<std::string, Resource *>::iterator it =
		bitmaps.find(filename);
	
	if (it == bitmaps.end()) {
		return;
	}
	
	std::pair<std::string, Resource *> p = *it;

	Resource *r = p.second;

	r->reference_count--;
	if (r->reference_count <= 0) {
		Wrap::Bitmap *b = (Wrap::Bitmap *)r->ptr;
		if (b) {
			Wrap::destroy_bitmap(b);
		}
		delete r;
		bitmaps.erase(it);
	}
}

