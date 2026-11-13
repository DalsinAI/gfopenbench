/* WBGapX3: row X3 of amicradle-teams designs/OPENBENCH-GAPS-20261112.md on GFOpenBench (build plan row 3): with
   nearly no memory left, a program opened from the desktop gives the low-memory requester, not a crash, and Retry
   starts it once memory is back. WBGapTest LOWMEM takes the memory, asks the desktop to open the program, finds the
   requester, gives the memory back and clicks Retry (with DragTest, beside it). ARexx itself can't run with no
   memory, so the open goes by GFOpenBench's GFOPENBENCH port from WBGapTest.
     rx WBGapX3.rexx
   MIT, Copyright (c) 2026 AmiCradle Team. */
OPTIONS RESULTS
IF ~SHOW('P', 'OPENBENCH') THEN DO
  SAY 'SKIP X3 no OPENBENCH port: not GFOpenBench'
  EXIT 0
END
CALL ADDLIB('rexxsupport.library', 0, -30, 0)
ADDRESS COMMAND 'Copy SYS:T/WBGap/WBGapTest RAM:WBGapX3 QUIET'
ADDRESS COMMAND 'Delete RAM:wbgap-order RAM:x3-eat QUIET >NIL:'
ADDRESS COMMAND 'SYS:T/WBGap/WBGapTest LOWMEM 8 RAM:WBGapX3 >RAM:x3-eat'
ADDRESS OPENBENCH
started = 0
DO i = 1 TO 20 WHILE ~started
  CALL DELAY(50)
  IF OPEN('f', 'RAM:wbgap-order', 'R') THEN DO
    DO WHILE ~EOF('f')
      IF READLN('f') = 'start WBGapX3' THEN started = 1
    END
    CALL CLOSE('f')
  END
END
seen = 0
IF OPEN('f', 'RAM:x3-eat', 'R') THEN DO
  DO WHILE ~EOF('f')
    l = READLN('f')
    IF l ~= '' THEN SAY l
    IF POS('requester open', l) > 0 THEN seen = 1
  END
  CALL CLOSE('f')
END
GETATTR 'application.version'
IF RC ~= 0 THEN DO
  SAY 'FAIL X3 the desktop did not answer after low memory'
  EXIT 5
END
SAY 'INFO X3 the desktop answers:' RESULT
IF ~seen THEN SAY 'FAIL X3 no low-memory requester'
ELSE IF ~started THEN SAY 'FAIL X3 Retry did not start the program'
ELSE SAY 'PASS X3'
EXIT 0
