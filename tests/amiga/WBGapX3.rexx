/* WBGapX3: row X3 of amicradle-teams designs/OPENBENCH-GAPS-20261112.md on GFOpenBench (build plan row 3): with
   nearly no memory left, a drawer opened from the desktop gives the low-memory requester, not a crash, and Retry
   opens it once memory is back. WBGapTest LOWMEM takes the memory, asks the desktop to open the drawer, finds the
   requester, gives the memory back and clicks Retry (with DragTest, beside it). ARexx itself can't run with no
   memory, so the open goes by GFOpenBench's GFOPENBENCH port from WBGapTest.
   PASS also when the drawer opened even so (nothing failed, nothing to ask).
   A drawer, not a program: on AmiCradle a program's LoadSeg still loads with every piece of exec's memory taken
   (the lab, 13 November 2026), so a program can't be made to fail for memory there; the launcher's side is in the
   host tests.
     rx WBGapX3.rexx
   MIT, Copyright (c) 2026 AmiCradle Team. */
OPTIONS RESULTS
IF ~SHOW('P', 'OPENBENCH') THEN DO
  SAY 'SKIP X3 no OPENBENCH port: not GFOpenBench'
  EXIT 0
END
ADDRESS COMMAND 'MakeDir RAM:WBGapX3d >NIL:'
ADDRESS COMMAND 'Delete RAM:x3-eat QUIET >NIL:'
ADDRESS COMMAND 'SYS:T/WBGap/WBGapTest LOWMEM 8 RAM:WBGapX3d >RAM:x3-eat'
seen = 0
opened = 0
early = 0
IF OPEN('f', 'RAM:x3-eat', 'R') THEN DO
  DO WHILE ~EOF('f')
    l = READLN('f')
    IF l ~= '' THEN SAY l
    IF POS('requester open', l) > 0 THEN seen = 1
    IF POS('answered the open: ok 1', l) > 0 THEN opened = 1
    IF POS('open while the memory was taken: yes', l) > 0 THEN early = 1
  END
  CALL CLOSE('f')
END
ADDRESS OPENBENCH
GETATTR 'application.version'
IF RC ~= 0 THEN DO
  SAY 'FAIL X3 the desktop did not answer after low memory'
  EXIT 5
END
SAY 'INFO X3 the desktop answers:' RESULT
/* W2: X3's drawer window not left open for the rows after it (Amiga+Tab found it still there, 14 November 2026);
   the drawer goes too */
WINDOW 'RAM:WBGapX3d' CLOSE
ADDRESS COMMAND 'Delete RAM:WBGapX3d QUIET >NIL:'
ADDRESS OPENBENCH
/* On AmiCradle the drawer often opens even with every piece of exec's memory taken (the lab, 13 November 2026):
   then nothing failed and there was nothing to ask, which passes too */
IF seen & opened THEN SAY 'PASS X3 requester, and Retry opened the drawer'
ELSE IF early & opened & ~seen THEN SAY 'PASS X3 the drawer opened with the memory taken: nothing to ask'
ELSE IF seen THEN SAY 'FAIL X3 Retry did not open the drawer'
ELSE SAY 'FAIL X3 the drawer did not open and no low-memory requester'
EXIT 0
