#ifndef ACTION_H
#define ACTION_H

typedef int (* Action)(void *);

void action_initialize(void);
int  action_count(void);
void action_add(Action, void *);
void action_process(void);

#endif
