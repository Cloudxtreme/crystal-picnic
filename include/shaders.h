#ifndef SHADERS_H
#define SHADERS_H

#include <allegro5/allegro.h>

#include <string>

#include <wrap.h>

namespace Shader
{

void use(Wrap::Shader *shader);
Wrap::Shader *get(std::string name);
void destroy(Wrap::Shader *shader);

}

#endif
