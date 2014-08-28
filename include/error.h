#ifndef ERROR_H
#define ERROR_H

#include <string>

class Error {
public:
	std::string get_message();

	Error(std::string msg);
	Error();
private:
	std::string message;
};

#endif
