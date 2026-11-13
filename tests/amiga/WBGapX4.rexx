/* WBGapX4: row X4 of amicradle-teams designs/OPENBENCH-GAPS-20261112.md on GFOpenBench (build plan row 3): a task of
   the desktop's that crashes is caught and started again, and the desktop stays up. In a lab with the task guard
   (AC_TASKGUARD hold, the default) only: it makes GFOpenBench's launcher crash on purpose
   (ENV:GFOpenBench/X4Test and a program named WBGapCrash, desktop/ob_launch.c's x4_test).
     rx WBGapX4.rexx
   MIT, Copyright (c) 2026 AmiCradle Team. */
OPTIONS RESULTS
IF ~SHOW('P', 'OPENBENCH') THEN DO
  SAY 'SKIP X4 no OPENBENCH port: not GFOpenBench'
  EXIT 0
END
ADDRESS COMMAND 'SetEnv GFOpenBench/X4Test 1'
ADDRESS COMMAND 'Copy SYS:T/WBGap/WBGapTest RAM:WBGapCrash QUIET'
ADDRESS COMMAND 'Copy SYS:T/WBGap/WBGapTest RAM:WBGapX4 QUIET'
ADDRESS COMMAND 'Delete RAM:wbgap-order QUIET >NIL:'
ADDRESS OPENBENCH
OPEN 'RAM:WBGapCrash'
ADDRESS COMMAND 'Wait 5'
GETATTR 'application.version'
IF RC ~= 0 THEN DO
  SAY 'FAIL X4 the desktop did not answer after its launcher crashed'
  ADDRESS COMMAND 'UnSetEnv GFOpenBench/X4Test'
  EXIT 5
END
SAY 'INFO X4 the desktop answers after the crash:' RESULT
OPEN 'RAM:WBGapX4'
ADDRESS COMMAND 'Wait 4'
ADDRESS COMMAND 'UnSetEnv GFOpenBench/X4Test'
started = 0
IF OPEN('f', 'RAM:wbgap-order', 'R') THEN DO
  DO WHILE ~EOF('f')
    l = READLN('f')
    IF l = 'start WBGapX4' THEN started = 1
  END
  CALL CLOSE('f')
END
IF started THEN SAY 'PASS X4'
ELSE SAY 'FAIL X4 no program started after the launcher crashed: not started again'
EXIT 0
