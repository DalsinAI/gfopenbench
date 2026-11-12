/* WBGapRexx: rows R1 and R2 of amicradle-teams designs/OPENBENCH-GAPS-20261112.md, the Workbench's
   ARexx port, on GFOpenBench and on OS 3.2.3's Workbench alike. RAM:WBGap must exist (WBGapTest SETUP).
   MIT, Copyright (c) 2026 AmiCradle Team. */
OPTIONS RESULTS
IF ~SHOW('P', 'WORKBENCH') THEN DO
  SAY 'FAIL R1 no WORKBENCH port'
  SAY 'SKIP R2 no WORKBENCH port'
  EXIT 5
END
SAY 'PASS R1'
ADDRESS WORKBENCH
bad = ''
GETATTR 'application.version'
IF RC ~= 0 THEN bad = bad 'GETATTR'
ELSE SAY 'INFO R2 application.version' RESULT
NEWDRAWER 'RAM:WBGap/rexxdir'
IF RC ~= 0 | ~EXISTS('RAM:WBGap/rexxdir') THEN bad = bad 'NEWDRAWER'
RENAME 'RAM:WBGap/rexxdir' 'rexxdir2'
IF RC ~= 0 | ~EXISTS('RAM:WBGap/rexxdir2') THEN bad = bad 'RENAME'
WINDOW 'RAM:WBGap' OPEN
IF RC ~= 0 THEN bad = bad 'WINDOW-OPEN'
GETATTR 'window.icons.all.count' NAME 'RAM:WBGap'
IF RC ~= 0 THEN bad = bad 'GETATTR-ICONS'
ELSE SAY 'INFO R2 RAM:WBGap icons' RESULT
DELETE 'RAM:WBGap/rexxdir2'
IF RC ~= 0 | EXISTS('RAM:WBGap/rexxdir2') THEN bad = bad 'DELETE'
WINDOW 'RAM:WBGap' CLOSE
IF RC ~= 0 THEN bad = bad 'WINDOW-CLOSE'
IF bad = '' THEN SAY 'PASS R2'
ELSE SAY 'FAIL R2 not done:' bad
EXIT 0
