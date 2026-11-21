/* ol_machine.h: a program's machine settings, pushed before it starts and popped when it ends, through
 * accontrol.device 1.8's ACCMD_MACHINE (gfosdesign gfos/machines/machine-settings; the words: ol_core.h's
 * ol_machine_words). The machine answers before the Amiga's next instruction, so the program's first instruction
 * already runs with them, and they are never switched under it while it runs.
 *
 * Included by gfopenlaunch.library (ollib.c) and GFOpenBench's own starter (desktop/ob_launch.c, when the library isn't
 * there). Needs SysBase. On a machine without accontrol.device 1.8 (a real Amiga, an older AmiCradle) nothing is
 * pushed and the program starts as before.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OL_MACHINE_H
#define OL_MACHINE_H

#include <exec/types.h>
#include <exec/io.h>
#include <exec/ports.h>
#include <proto/exec.h>

#define OLM_ACCMD_MACHINE (CMD_NONSTD + 17)     /* accontrol.h ACCMD_MACHINE */
#define OLM_PUSH 1                              /* ... ACC_MACHINE_PUSH */
#define OLM_POP 2                               /* ... ACC_MACHINE_POP: the id in io_Offset's upper 24 bits */

/* One ACCMD_MACHINE call: io_Actual (a push's id), or 0 when it failed or there is no accontrol.device. */
static ULONG olm_call(ULONG offset, char *buf, ULONG len)
{
    struct MsgPort *port = CreateMsgPort();
    struct IOStdReq *io = port ? (struct IOStdReq *)CreateIORequest(port, sizeof(struct IOStdReq)) : NULL;
    ULONG got = 0;
    if (io) {
        if (!OpenDevice((CONST_STRPTR)"accontrol.device", 0, (struct IORequest *)io, 0)) {
            io->io_Command = OLM_ACCMD_MACHINE;
            io->io_Offset = offset;
            io->io_Data = buf;
            io->io_Length = len;
            if (!DoIO((struct IORequest *)io)) got = (offset & 0xff) == OLM_POP ? 1 : io->io_Actual;
            CloseDevice((struct IORequest *)io);
        }
        DeleteIORequest((struct IORequest *)io);
    }
    if (port) DeleteMsgPort(port);
    return got;
}

/* The words (ol_machine_words's) pushed: the push's id, 0 for none. words is overwritten with the ones refused. */
static ULONG olm_push(char *words, ULONG cap)
{
    return words[0] ? olm_call(OLM_PUSH, words, cap) : 0;
}

/* That push popped (0: nothing pushed) */
static void olm_pop(ULONG id)
{
    if (id) olm_call(OLM_POP | id << 8, NULL, 0);
}

#endif
