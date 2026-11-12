/* WBGapRexx: rows R1 and R2 of amicradle-teams designs/OPENBENCH-GAPS-20261112.md, the Workbench's
   ARexx port, on GFOpenBench and on OS 3.2.3's Workbench alike. RAM:WBGap must exist (WBGapTest SETUP).
     rx WBGapRexx.rexx                 the WORKBENCH port: rows R1 and R2
     rx WBGapRexx.rexx OPENBENCH       GFOpenBench's own name for it (0.1.50): rows R1b and R2b, SKIP where there
                                       is no such port (OS 3.2.3's Workbench), FAIL on GFOpenBench without it
   MIT, Copyright (c) 2026 AmiCradle Team. */
OPTIONS RESULTS
PARSE UPPER ARG port .
IF port = '' THEN port = 'WORKBENCH'
r1 = 'R1'; r2 = 'R2'
IF port ~= 'WORKBENCH' THEN DO; r1 = 'R1b'; r2 = 'R2b'; END
IF ~SHOW('P', port) THEN DO
  IF port = 'OPENBENCH' & ~SHOW('P', 'OPENBENCH') & SHOW('P', 'WORKBENCH') THEN DO
    ADDRESS WORKBENCH 'GETATTR application.version'
    IF RC = 0 & POS('GFOPENBENCH', UPPER(RESULT)) = 0 THEN DO
      SAY 'SKIP' r1 'no OPENBENCH port: not GFOpenBench'
      SAY 'SKIP' r2 'no OPENBENCH port: not GFOpenBench'
      EXIT 0
    END
  END
  SAY 'FAIL' r1 'no' port 'port'
  SAY 'SKIP' r2 'no' port 'port'
  EXIT 5
END
SAY 'PASS' r1
ADDRESS VALUE port
bad = ''
GETATTR 'application.version'
IF RC ~= 0 THEN bad = bad 'GETATTR'
ELSE SAY 'INFO' r2 'application.version' RESULT
NEWDRAWER 'RAM:WBGap/rexxdir'
IF RC ~= 0 | ~EXISTS('RAM:WBGap/rexxdir') THEN bad = bad 'NEWDRAWER'
RENAME 'RAM:WBGap/rexxdir' 'rexxdir2'
IF RC ~= 0 | ~EXISTS('RAM:WBGap/rexxdir2') THEN bad = bad 'RENAME'
WINDOW 'RAM:WBGap' OPEN
IF RC ~= 0 THEN bad = bad 'WINDOW-OPEN'
GETATTR 'window.icons.all.count' NAME 'RAM:WBGap'
IF RC ~= 0 THEN bad = bad 'GETATTR-ICONS'
ELSE SAY 'INFO' r2 'RAM:WBGap icons' RESULT
DELETE 'RAM:WBGap/rexxdir2'
IF RC ~= 0 | EXISTS('RAM:WBGap/rexxdir2') THEN bad = bad 'DELETE'
WINDOW 'RAM:WBGap' CLOSE
IF RC ~= 0 THEN bad = bad 'WINDOW-CLOSE'
IF bad = '' THEN SAY 'PASS' r2
ELSE SAY 'FAIL' r2 'not done:' bad
EXIT 0
