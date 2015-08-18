#include <map>

#include "bones.h"
#include "xml.h"
#include "collision_detection.h"
#include "animation.h"
#include "engine.h"

namespace Bones {

void load(std::vector<Bone> &bones, int bmp_w, int bmp_h, std::string filename)
{
	ALLEGRO_FILE *f;
	if (engine) {
		f = engine->get_cpa()->load(filename);
	}
	else {
		f = al_fopen(filename.c_str(), "rb");
	}

	if (!f) {
		General::log_message("Error loading bones " + filename);
		return;
	}

	char buf[1000];
	while (1) {
		if (al_fgets(f, buf, 1000) == NULL)
			break;
		Bones::Type bone_type;
		sscanf(buf, "<%d>", (int *)&bone_type);
		if (al_fgets(f, buf, 1000) == NULL)
			break;
	
		std::vector<float> vertices;

		while (1) {
			int x, y, n1, n2;
			if (al_fgets(f, buf, 1000) == NULL)
				break;
			if (sscanf(buf, "\t<%d><x>%d</x><y>%d</y></%d>", &n1, &x, &y, &n2) < 4)
				break;

			vertices.push_back(x);
			vertices.push_back(y);
		}
		
		std::vector<int> splits;
		splits.push_back(vertices.size() / 2);
		float *verts = new float[vertices.size()];
		std::vector< General::Point<float> > outline;

		for (unsigned int i = 0; i < vertices.size() / 2; i++) {
			int index = i*2;
			verts[index] = vertices[index] - bmp_w/2;
			verts[index+1] = vertices[index+1] - bmp_h;
			outline.push_back(General::Point<float>(
				verts[index], verts[index+1]
			));
		}

		std::vector<Triangulate::Triangle> triangles;

		Triangulate::get_triangles(outline, splits, triangles);

		Bone b = Bone(bone_type, outline, triangles, General::Size<int>(bmp_w, bmp_h));
		bones.push_back(b);

		delete[] verts;
	}

	al_fclose(f);
}

void load(
	Animation_Set *anim_set,
	std::string info_filename,
	std::string path_to_files,
	std::map< std::pair<std::string, int>, std::vector<Bone> > &bones)
{
	XMLData *xml = new XMLData(info_filename);
	std::list<XMLData *>::iterator it;
	std::list<XMLData *> nodes = xml->get_nodes();

	std::string sub_bak = anim_set->get_sub_animation_name();

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
		int nframes = atoi(framesS.c_str());
		for (int i = 0; i < nframes; i++) {
			anim_set->set_sub_animation(name, true);
			anim_set->get_current_animation()->set_frame(i);
			int w = anim_set->get_current_animation()->get_current_frame()->get_width();
			int h = anim_set->get_current_animation()->get_current_frame()->get_height();
			std::string filename = path_to_files + "/" + name + "/" + General::itos(i+1) + ".xml";
			std::vector<Bone> bones2;
			load(bones2, w, h, filename);
			std::pair<std::string, int> p;
			p.first = name;
			p.second = i;
			bones[p] = bones2;
		}
	}

	anim_set->set_sub_animation(sub_bak);

	delete xml;
}

General::Size<float> Bone::get_extents(float *out_minx, float *out_miny) const
{
	General::Size<float> extents;
	float min_x = General::BIG_FLOAT;
	float min_y = General::BIG_FLOAT;
	float max_x = General::SMALL_FLOAT;
	float max_y = General::SMALL_FLOAT;
	
	for (size_t i = 0; i < outline.size(); i++) {
		const General::Point<float> &p = outline[i];
		if (p.x < min_x) {
			min_x = p.x;
		}
		if (p.x > max_x) {
			max_x = p.x;
		}
		if (p.y < min_y) {
			min_y = p.y;
		}
		if (p.y > max_y) {
			max_y = p.y;
		}
	}

	if (out_minx) {
		*out_minx = min_x;
	}
	if (out_miny) {
		*out_miny = min_y;
	}
	
	return General::Size<float>(max_x-min_x, max_y-min_y);
}

} // end namespace Bones
