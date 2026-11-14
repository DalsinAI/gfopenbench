/* ob_port: message ports on one signal shared by GFOpenBench's quiet ports (0.1.62; ob_sigplan.h says why). Each
 * one is read by its own code as before (GetMsg until empty) whenever the shared signal comes.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_PORT_H
#define OB_PORT_H

#include <exec/ports.h>

/* A new port for this task on the shared signal (taken with the first one), as CreateMsgPort makes one. NULL when
 * there is no memory or no signal. */
struct MsgPort *ob_port_new(void);
/* The port freed (not public any more, its messages taken by the caller); the signal freed with the last one */
void ob_port_free(struct MsgPort *mp);

#endif
