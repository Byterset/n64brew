
#ifndef STAGE_H
#define STAGE_H

#include "../game.h"

typedef struct Stage {
    void (*updateCallback)(void);
} Stage;

Stage Stage_init(Game *game);



#endif /* !STAGE_H_ */
