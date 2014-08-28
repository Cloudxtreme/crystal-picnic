#include "crystalpicnic.h"

int main(int argc, char **argv)
{
	General::argc = argc;
	General::argv = argv;

	Main* m = NULL;

	try {
		m = new Main();
		if (m->init()) {
			m->execute();
			m->shutdown();
		}
	}
	catch (Error e) {
		std::cout << "*** An error occurred *** : " << e.get_message()
			<< std::endl;
	}

	if (cfg.save())
		General::log_message("Configuration saved.");
	else
		General::log_message("Warning: Configuration not saved.");
	
	delete m;
	return 0;
}
