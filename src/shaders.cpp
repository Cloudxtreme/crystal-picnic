#include <allegro5/allegro.h>

#ifdef ALLEGRO_WINDOWS
#include <allegro5/allegro_direct3d.h>
#endif

#include <vector>

#include <wrap.h>

#include "shaders.h"
#include "general.h"
#include "engine.h"

struct MyShader {
	Wrap::Shader *shader;
	std::string name;
};

static std::vector<MyShader> shaders;

ALLEGRO_SHADER *my_create_shader(std::string vertex_source, std::string pixel_source)
{
	ALLEGRO_SHADER *s;

#ifdef ALLEGRO_WINDOWS
	if (al_get_display_flags(engine->get_display()) & ALLEGRO_DIRECT3D) {
		s = al_create_shader(ALLEGRO_SHADER_HLSL);
		const char *technique =
			"technique TECH\n"
			"{\n"
			"	pass p1\n"
			"	{\n"
			"		VertexShader = compile vs_2_0 vs_main();\n"
			"		PixelShader = compile ps_2_0 ps_main();\n"
			"	}\n"
			"}\n";
		static char buf[10000];
		snprintf(buf, sizeof(buf), "%s", pixel_source.c_str());
		pixel_source = buf;
		int len = strlen(buf);
		snprintf(buf+len, sizeof(buf)-len, technique);
	}
	else
#endif
	{
		s = al_create_shader(ALLEGRO_SHADER_GLSL);
#if defined ALLEGRO_RASPBERRYPI || defined ALLEGRO_IPHONE || defined ALLEGRO_ANDROID
		const char *version = "#version 100\n";
#else
		const char *version = "#version 120\n";
#endif
		static char vbuf[10000];
		static char pbuf[10000];
		snprintf(vbuf, sizeof(vbuf), "%s%s", version, vertex_source.c_str());
		vertex_source = vbuf;
		snprintf(pbuf, sizeof(pbuf), "%s%s", version, pixel_source.c_str());
		pixel_source = pbuf;
	}

	printf("- vert -\n%s\n-\n", vertex_source.c_str());
	printf("- pix -\n%s\n-\n", pixel_source.c_str());

	if (!al_attach_shader_source(
		s,
		ALLEGRO_VERTEX_SHADER,
		vertex_source.c_str()
	)) {
		General::log_message(std::string("Shader error: ") + al_get_shader_log(s));
	}

	if (!al_attach_shader_source(
		s,
		ALLEGRO_PIXEL_SHADER,
		pixel_source.c_str()
	)) {
		General::log_message(std::string("Shader error: ") + al_get_shader_log(s));
	}

	if (!al_build_shader(s)) {
		General::log_message(std::string("Shader error: ") + al_get_shader_log(s));
	}

	return s;
}

Wrap::Shader *load_shader(std::string name)
{
	int sz;
	std::string dir;

#ifdef ALLEGRO_WINDOWS
	if (al_get_display_flags(engine->get_display()) & ALLEGRO_DIRECT3D) {
		dir = "hlsl";
	}
	else
#endif
	dir = "glsl";

	unsigned char *buf1 = General::slurp("shaders/" + dir + "/default_vertex.txt", &sz, true, false);
	if (!buf1) {
		return NULL;
	}
	unsigned char *buf2 = General::slurp("shaders/" + dir + "/" + name + ".txt", &sz, true, false);
	if (!buf2) {
		delete[] buf1;
		return NULL;
	}

	Wrap::Shader *s = Wrap::create_shader((const char *)buf1, (const char *)buf2);

	delete[] buf1;
	delete[] buf2;

	return s;
}

namespace Shader
{

void use(Wrap::Shader *shader)
{
	if (shader == NULL) {
		al_use_shader(NULL);
		return;
	}

	al_use_shader(shader->shader);
#ifdef ALLEGRO_WINDOWS
	if (al_get_display_flags(engine->get_display()) & ALLEGRO_DIRECT3D) {
		ALLEGRO_BITMAP *target = al_get_target_bitmap();
		al_set_shader_float("target_w", al_get_bitmap_width(target));
		al_set_shader_float("target_h", al_get_bitmap_height(target));
	}
#endif
}

Wrap::Shader *get(std::string name)
{
	size_t sz = shaders.size();
	for (size_t i = 0; i < sz; i++) {
		if (shaders[i].name == name) {
			return shaders[i].shader;
		}
	}

	MyShader s;
	s.shader = load_shader(name);

	if (!s.shader->shader) {
		Wrap::destroy_shader(s.shader);
		return NULL;
	}

	s.name = name;
	shaders.push_back(s);

	return s.shader;
}

void destroy(Wrap::Shader *shader)
{
	size_t sz = shaders.size();
	for (size_t i = 0; i < sz; i++) {
		if (shaders[i].shader == shader) {
			Wrap::destroy_shader(shaders[i].shader);
			shaders.erase(shaders.begin() + i);
			break;
		}
	}
}

} // end namespace

