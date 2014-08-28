#include "crystalpicnic.h"
#include "xml.h"

#include <cctype>


std::string XMLData::get_name(void)
{
	return name;
}


std::list<XMLData*> &XMLData::get_nodes(void)
{
	return nodes;
}


XMLData::XMLData(std::string name, ALLEGRO_FILE* f) :
	file(f),
	debug(false),
	ungot(-1),
	ungotReady(false)
{
	this->name = name;
}


XMLData::XMLData(std::string filename) :
	debug(false),
	ungot(-1),
	ungotReady(false)

{
	name = std::string("main");

	if (engine) {
		file = engine->get_cpa()->load(filename);
	}
	else {
		file = al_fopen(filename.c_str(), "rb");
	}
	if (!file) {
		General::log_message("Could not open file ('" + filename + "').");
		return;
	}

	read();

	al_fclose(file);
}


XMLData::XMLData(std::string name, std::string value) :
	debug(false),
	ungot(-1),
	ungotReady(false)
{
	this->name = name;
	this->value = std::string(value);
}


void XMLData::add(XMLData* node)
{
	nodes.push_back(node);
}


XMLData* XMLData::find(std::string name)
{
	std::list<XMLData*>::iterator it = nodes.begin();

	while (it != nodes.end()) {
		if ((*it)->name == name) {
			return *it;
		}
		it++;
	}

	return 0;
}


std::string XMLData::get_value()
{
	return value;
}


XMLData::~XMLData()
{
	std::list<XMLData*>::iterator it = nodes.begin();

	while (it != nodes.end()) {
		XMLData* node = *it;
		delete node;
		it++;
	}

	nodes.clear();
}


std::string XMLData::readToken()
{
	if (al_feof(file)) {
		return "(null)";
	}

	int c;

	/* Skip whitespace */

	for (;;) {
		c = get();
		if (c == EOF) {
			return "(null)";
		}
		if (!isspace(c)) {
			break;
		}
	}

	/* Found tag */

	if (c == '<') {
		if (debug)
			std::cout << "Found tag start/end\n";
		std::string token;
		token += c;
		for (;;) {
			c = get();
			if (c == EOF) {
				break;
			}
			token += c;
			if (c == '>')
				break;
		}
		if (debug)
			std::cout << "Read <token>: " << token << "\n";
		return token;
	}
	/* Found data */
	else {
		std::string data;
		data += c;
		for (;;) {
			c = get();
			if (c == EOF) {
				break;
			}
			if (c == '<') {
				unget(c);
				break;
			}
			data += c;
		}
		if (debug)
			std::cout << "Read data: " << data << "\n";
		return data;
	}

	return "(null)";
}


int XMLData::get()
{
	int c;

	if (ungotReady) {
		c = ungot;
		ungotReady = false;
	}
	else {
		c = al_fgetc(file);
	}

	return c;
}


void XMLData::unget(int c)
{
	ungot = c;
	ungotReady = true;
}


void XMLData::read()
{
	// read until EOF or end token
	
	for (;;) {
		std::string token;
		token = readToken();
		if (token == "(null)" || (!strncmp(token.c_str(), "</", 2))) {
			return;
		}
		if (token.c_str()[0] == '<') {
			if (debug) {
				std::cout << "Reading sub tag\n";
			}
			std::string name = get_token_name(token);
			if (debug)
				std::cout << "Token is " << name.c_str() << "\n";
			XMLData* newdata = new XMLData(name, file);
			newdata->read();
			nodes.push_back(newdata);
		}
		else {
			value += token.c_str();
		}
	}
}


std::string XMLData::get_token_name(std::string token)
{
	std::string s;
	
	for (int i = 1; token.c_str()[i] != '>' && token.c_str()[i]; i++) {
		if (debug)
			std::cout << "Read character " << i << "\n";
		s += token.c_str()[i];
	}

	return s;
}


void XMLData::write(ALLEGRO_FILE *out, int tabs = 0)
{
	char buf[200];

	if (value == "") {
		for (int i = 0; i < tabs; i++) {
			sprintf(buf, "\t");
			al_fputs(out, buf);
		}

		sprintf(buf, "<%s>\n", name.c_str());
		al_fputs(out, buf);

		std::list<XMLData*>::iterator it = nodes.begin();

		while (it != nodes.end()) {
			XMLData* node = dynamic_cast<XMLData*>(*it);
			node->write(out, tabs+1);
			it++;
		}

		for (int i = 0; i < tabs; i++) {
			sprintf(buf, "\t");
			al_fputs(out, buf);
		}

		sprintf(buf, "</%s>\n", name.c_str());
		al_fputs(out, buf);
	}
	else {
		for (int i = 0; i < tabs; i++) {
			sprintf(buf, "\t");
			al_fputs(out, buf);
		}

		sprintf(buf, "<%s>", name.c_str());
		al_fputs(out, buf);
	
		sprintf(buf, "%s", value.c_str());
		al_fputs(out, buf);
		
		std::list<XMLData*>::iterator it = nodes.begin();

		while (it != nodes.end()) {
			XMLData* node = dynamic_cast<XMLData*>(*it);
			node->write(out, tabs+1);
			it++;
		}
		
		sprintf(buf, "</%s>\n", name.c_str());
		al_fputs(out, buf);
	}
}

bool XMLData::failed(void)
{
	return file == NULL;
}
