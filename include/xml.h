#ifndef XML_H
#define XML_H

#include <allegro5/allegro.h>

#include <cstdio>
#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>

#include "error.h"


class XMLData {
public:
	std::list<XMLData*> &get_nodes();
	XMLData* find(std::string name);
	std::string get_value();
	void write(ALLEGRO_FILE *out, int tab);
	void add(XMLData* node);
	std::string get_name();
	XMLData(std::string name, std::string value);
	XMLData(std::string filename);
	~XMLData();
	bool failed();
private:
	XMLData(std::string name, ALLEGRO_FILE* f);
	std::string readToken();
	int get();
	void unget(int c);
	void seek(long pos);
	void read();
	std::string get_token_name(std::string token);
	ALLEGRO_FILE* file;
	std::string name;
	std::string value;
	std::list<XMLData*> nodes;
	bool debug;
	int ungot;
	bool ungotReady;
};


#endif // XML_HPP
