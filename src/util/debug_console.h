#include "../graphics/renderstate.h"

#define MAX_STRINGS 10
#define MAX_STRING_LEN 50
extern char console_msgs[MAX_STRINGS][MAX_STRING_LEN];
extern int num_console_msgs;


void console_add_msg(const char *str);

void console_print_all(struct RenderState *renderState);