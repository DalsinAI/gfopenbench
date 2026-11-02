/* The programs built into GFOpenBench (ob_builtin.h): one BUILTIN(name, entry, stack) a line, each inside the
 * #ifdef that desktop/build.sh defines when it compiles that program in. name is the program's file name
 * (what its path ends in); entry its main() renamed. Keep the lines in alphabetical order. The editors GFOpenPrefs
 * has as pages left with GFOpenBench 0.1.32: their old names open GFOpenPrefs at their page (openbench.c). */
#ifdef OB_HAVE_EXAMPLE
BUILTIN("Example", example_main, 16384)
#endif
#ifdef OB_HAVE_OPENTATA
BUILTIN("GFOpenTata", opentata_main, 32768)           /* Tata's window (gfopentata); Tata's checks are openbench.c's */
#endif
#ifdef OB_HAVE_OPENUP_SETUP
BUILTIN("GFOpenUp-Setup", openup_setup_main, 16384)
#endif
