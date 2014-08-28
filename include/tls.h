#ifndef TLS_H
#define TLS_H

namespace TLS
{

struct Thread_Local_State
{
	char current_search_path[1000];
};

void init();
Thread_Local_State *get_state();

} // end namespace TLS

#endif // TLS_H
