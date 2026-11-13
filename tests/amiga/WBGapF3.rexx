/* WBGapF3: one task per drawer window (build plan row 3). A drawer opens in a task of its own and ARexx's OPEN
   answers once its window is there; a drawer task that crashes is caught, opened again, and closed for good after
   OB_RESTART_MAX crashes, while the desktop and the other drawers go on. In a lab with the task guard
   (AC_TASKGUARD hold, the default) only: a drawer named WBGapCrash crashes on purpose with ENV:GFOpenBench/X4Test
   set (desktop/ob_dtask.c's x4_test).
     rx WBGapF3.rexx
   MIT, Copyright (c) 2026 AmiCradle Team. */
OPTIONS RESULTS
IF ~SHOW('P', 'OPENBENCH') THEN DO
  SAY 'SKIP F3 no OPENBENCH port: not GFOpenBench'
  EXIT 0
END
ADDRESS COMMAND 'MakeDir RAM:WBGapD1 RAM:WBGapCrash >NIL:'
ADDRESS OPENBENCH
OPEN 'RAM:WBGapD1'
IF RC ~= 0 THEN DO
  SAY 'FAIL F3 OPEN RAM:WBGapD1 RC' RC
  EXIT 5
END
GETATTR 'window.left' NAME 'RAM:WBGapD1'
IF RC ~= 0 THEN DO
  SAY 'FAIL F3 the drawer''s window was not there when OPEN answered'
  EXIT 5
END
GETATTR 'windows.count'
n1 = RESULT
SAY 'INFO F3 windows with RAM:WBGapD1 open:' n1
ADDRESS COMMAND 'Delete T:GFOpenBench-X4.log QUIET >NIL:'
ADDRESS COMMAND 'SetEnv GFOpenBench/X4Test 1'
OPEN 'RAM:WBGapCrash'
ADDRESS COMMAND 'Wait 14'
ADDRESS COMMAND 'UnSetEnv GFOpenBench/X4Test'
GETATTR 'application.version'
IF RC ~= 0 THEN DO
  SAY 'FAIL F3 the desktop did not answer after a drawer task crashed'
  EXIT 5
END
GETATTR 'windows.count'
n2 = RESULT
IF n2 ~= n1 THEN DO
  SAY 'FAIL F3 the crashing drawer was not closed for good: windows' n2 'not' n1
  ADDRESS COMMAND 'Type T:GFOpenBench-X4.log'
END
ELSE SAY 'INFO F3 the crashing drawer closed for good after its restarts'
WINDOW 'RAM:WBGapD1' CLOSE
ADDRESS COMMAND 'Wait 1'
OPEN 'RAM:WBGapD1'
r = RC
GETATTR 'window.left' NAME 'RAM:WBGapD1'
IF r ~= 0 | RC ~= 0 THEN DO
  SAY 'FAIL F3 the other drawer did not close and open again after the crashes'
  EXIT 5
END
WINDOW 'RAM:WBGapD1' CLOSE
IF n2 = n1 THEN SAY 'PASS F3'
EXIT 0
