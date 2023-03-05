#include "debug_console.h"
#include "../font/font_ext.h"

char console_msgs[MAX_STRINGS][MAX_STRING_LEN];
int num_console_msgs = 0;

void console_add_msg(const char *str)
{
    int i, j;
    if (num_console_msgs == MAX_STRINGS) {
        // Drop the oldest string by shifting all strings to the left
        for (i = 0; i < num_console_msgs - 1; i++) {
            for (j = 0; j < MAX_STRING_LEN; j++) {
                console_msgs[i][j] = console_msgs[i+1][j];
            }
        }
        num_console_msgs--;
    }
    
    // Add the new string to the end of the array
    for (i = 0; i < MAX_STRING_LEN && str[i] != '\0'; i++) {
        console_msgs[num_console_msgs][i] = str[i];
    }
    console_msgs[num_console_msgs][i] = '\0';
    num_console_msgs++;
}

void console_print_all(struct RenderState *renderState){
    int x = 10;
    int y = 10;
    int i;

    for(i = 0; i < MAX_STRINGS; i++){
        SHOWFONT(&renderState->dl, console_msgs[i], x, y);
        y += 15;
    }
    
}

