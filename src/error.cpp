#include "crystalpicnic.h"
#include "error.h"

std::string Error::get_message(void)
{
	return message;
}

Error::Error(std::string msg) :
	message(msg)
{
}

Error::Error() :
	message("")
{
}

