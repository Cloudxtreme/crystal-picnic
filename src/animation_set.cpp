#include "crystalpicnic.h"
#include "animation_set.h"
#include "resource_manager.h"

ALLEGRO_DEBUG_CHANNEL("CrystalPicnic")

int Animation_Set::get_length(std::string anim_name)
{
	for (size_t i = 0; i < anims.size(); i++) {
		if (anims[i]->get_name() == anim_name) {
			return anims[i]->get_length();
		}
	}
	
	return -1;
}

void Animation_Set::set_sub_animation(int anim)
{
	curr_anim = anim;
}

int Animation_Set::get_num_animations(void)
{
	return anims.size();
}

Animation *Animation_Set::get_sub_animation(std::string subName)
{
	for (unsigned int i = 0; i < anims.size(); i++) {
		if (anims[i]->get_name() == subName) {
			return anims[i];
		}
	}

	return NULL;
}

/*
 * Returns true if an animation by this name exists
 */
bool Animation_Set::set_sub_animation(std::string subName, bool force)
{
	if (!force) {
		if (!strncmp(anims[curr_anim]->get_name().c_str(), "push", 4) && al_get_time()-push_time < 0.33f) {
			return true;
		}
		else if (!strncmp(subName.c_str(), "push", 4)) {
			push_time = al_get_time();
		}
	}

	std::string sub = prefix + subName;

	for (unsigned int i = 0; i < anims.size(); i++) {
		if (anims[i]->get_name() == sub) {
			curr_anim = i;
			return true;
		}
	}

	// If not found with prefix, looks for it without
	sub = subName;
	
	for (unsigned int i = 0; i < anims.size(); i++) {
		if (anims[i]->get_name() == sub) {
			curr_anim = i;
			return true;
		}
	}

	return false;
}

/*
 * Returns true if an animation by this name exists
 */
bool Animation_Set::check_sub_animation_exists(std::string subName)
{
	std::string sub = prefix + subName;

	for (unsigned int i = 0; i < anims.size(); i++) {
		if (anims[i]->get_name() == sub) {
			return true;
		}
	}

	return false;
}

std::string Animation_Set::get_sub_animation_name(void)
{
	return anims[curr_anim]->get_name();
}


void Animation_Set::set_frame(int frame)
{
	anims[curr_anim]->set_frame(frame);
}


int Animation_Set::get_frame()
{
	return anims[curr_anim]->get_current_frame_num();
}

// Returns how many frames have passed
int Animation_Set::update(int step)
{
	Animation *a = anims[curr_anim];
	return a->update(step);
}

// Returns how many frames have passed
int Animation_Set::update(void)
{
	return update(General::LOGIC_MILLIS);
}

Animation* Animation_Set::get_current_animation(void)
{
	return anims[curr_anim];
}

void Animation_Set::set_prefix(std::string prefix)
{
	this->prefix = prefix;
}

std::string Animation_Set::get_prefix(void)
{
	return prefix;
}

void Animation_Set::reset(void)
{
	if (!strncmp(anims[curr_anim]->get_name().c_str(), "push", 4)) {
		return;
	}
	anims[curr_anim]->reset();
}

bool Animation_Set::load(std::string path, std::string xml_path)
{
	load_path = path;
	load_xml_path = xml_path;
	saved_this = (Animation_Set *)this;

	bool result = load();

	return result;
}


bool Animation_Set::load()
{
	std::string filename;
	std::string xml_filename;
	
	if (load_xml_path != "") {
		filename = load_xml_path;
	}
	else {
		filename = load_path;
	}

	xml_filename = "info.xml";

	std::string name_used;

	if (load_xml_path != "") {
		name_used = load_xml_path;
	}
	else {
		name_used = load_path;
	}

	XMLData *xml = new XMLData(name_used + "/" + xml_filename);
	if (!xml || xml->failed()) {
		General::log_message("Error loading xml " + xml_filename + ".");
		return false;
	}

	std::list<XMLData *> &nodes = xml->get_nodes();
	std::list<XMLData *>::iterator it;

	General::log_message("Loaded Animation_Set descriptor " + xml_filename + ".");

	int id = 0;
	
	for (it = nodes.begin(); it != nodes.end(); it++) {
		XMLData *sub = *it;
		XMLData *tmp;
		std::string name = sub->get_name();
		tmp = sub->find("frames");
		std::string framesS;
		if (tmp)
			framesS = tmp->get_value();
		else
			framesS = "1";
		int frames = atoi(framesS.c_str());
		for (int i = 0; i < frames; i++) {
			char fn[500];
			snprintf(fn, 500, "%s/%s/%d.cpi", load_path.c_str(), name.c_str(), i+1);
			Wrap::Bitmap *bmp = resource_manager->reference_bitmap(fn);
			if (!bmp) {
				strcpy(fn, "misc_graphics/interface/x_icon.cpi");
				bmp = resource_manager->reference_bitmap(fn);
			}
			to_delete.push_back(fn);
			bmps.push_back(std::pair<Wrap::Bitmap *, int>(bmp, id++));
		}
	}

	id = 0;

	for (it = nodes.begin(); it != nodes.end(); it++) {
		XMLData *sub = *it;
		XMLData *tmp;
		std::string name = sub->get_name();
		Animation *anim = new Animation(name);

		tmp = sub->find("frames");
		std::string framesS;
		if (tmp)
			framesS = tmp->get_value();
		else
			framesS = "1";
		int frames = atoi(framesS.c_str());
		
		bool looping = true;
		Animation::Loop_Mode loop_mode = Animation::LOOP_NORMAL;
		tmp = sub->find("looping");
		if (tmp) {
			if (tmp->get_value() == "false") {
				looping = false;
			}
		}
		tmp = sub->find("loop_mode");
		if (tmp) {
			if (tmp->get_value() == "pingpong") {
				loop_mode = Animation::LOOP_PINGPONG;
			}
		}
		int loop_start;
		tmp = sub->find("loop_start");
		if (tmp) {
			loop_start = atoi(tmp->get_value().c_str());
		}
		else {
			loop_start = -1;
		}
		std::vector<int> delays;
		XMLData *delayNode = sub->find("delays");
		if (delayNode == NULL) {
			tmp = sub->find("delay");
			if (tmp) {
				int delay = atoi(tmp->get_value().c_str());
				for (int i = 0; i < frames; i++) {
					delays.push_back(delay);
				}
			}
			else {
				delays.push_back(0);
			}
		}
		else {
			XMLData *tmp2;
			for (int i = 0; i < frames; i++) {
				char n[100];
				sprintf(n, "%d", i);
				std::string numS = std::string(n);
				tmp2 = delayNode->find(numS);
				std::string dS = tmp2->get_value();
				int d = atoi(dS.c_str());
				delays.push_back(d);
			}
		}

		for (int i = 0; i < frames; i++) {
			Wrap::Bitmap *subbmp = bmps[id++].first;

			Bitmap *sub_bitmap = new Bitmap(false);
			sub_bitmap->set(subbmp);
			Frame *frame = new Frame(sub_bitmap, delays[i]);
			anim->add_frame(frame);

		}
		anim->set_looping(looping);
		anim->set_loop_mode(loop_mode);
		anim->set_loop_start(loop_start);
		/* Add user defined tags to animation */
		std::list<XMLData *> nodes = sub->get_nodes();
		std::list<XMLData *>::iterator it = nodes.begin();
		std::vector<std::string> tags;
		for (; it != nodes.end(); it++) {
			XMLData *node = *it;
			if (node->get_name() == "tag") {
				tags.push_back(node->get_value());
			}
		}
		anim->set_tags(tags);

		delays.clear();
		anims.push_back(anim);
	}

	delete xml;

	General::log_message("Animation set " + filename + " loaded.");

	return true;
}

void Animation_Set::sync(Animation_Set *sync_to)
{
	set_sub_animation(sync_to->get_sub_animation_name());

	Animation *a1 = get_current_animation();
	Animation *a2 = sync_to->get_current_animation();

	a1->current_frame = a2->current_frame;
	a1->count = a2->count;
	a1->increment = a2->increment;
}

Animation_Set::Animation_Set(void) :
	curr_anim(0),
	prefix(""),
	push_time(0.0)
{
}

Animation_Set::~Animation_Set()
{
	for (unsigned int i = 0; i < anims.size(); i++) {
		delete anims[i];
	}
	anims.clear();

	for (size_t i = 0; i < to_delete.size(); i++) {
		resource_manager->release_bitmap(to_delete[i]);
	}
}

