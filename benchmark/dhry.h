/* GFOpenBenchmark: Dhrystone 2.1 (Reinhold P. Weicker, CACM 1984; version 2.1, 1988), written again
 * from its published description for this program: the same procedures, the same data and the same
 * statements in each, in two files as the original's dhry_1.c and dhry_2.c (so a compiler can't fold
 * the procedures of one into the other's loop). One pass of the main loop is one Dhrystone.
 * Dhrystone MIPS (DMIPS) are Dhrystones a second / 1757, the VAX 11/780's score.
 * Portable C, no Amiga headers: the host tests (tests/host/test_benchmark.c) run it too.
 * MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team. */
#ifndef GFB_DHRY_H
#define GFB_DHRY_H

typedef enum { Ident_1, Ident_2, Ident_3, Ident_4, Ident_5 } Enumeration;
typedef int One_Thirty;
typedef int One_Fifty;
typedef char Capital_Letter;
typedef int Boolean;
typedef char Str_30[31];
typedef int Arr_1_Dim[50];
typedef int Arr_2_Dim[50][50];

typedef struct record {
    struct record *Ptr_Comp;
    Enumeration Discr;
    union {
        struct { Enumeration Enum_Comp; int Int_Comp; char Str_Comp[31]; } var_1;
        struct { Enumeration E_Comp_2; char Str_2_Comp[31]; } var_2;
        struct { char Ch_1_Comp; char Ch_2_Comp; } var_3;
    } variant;
} Rec_Type, *Rec_Pointer;

/* the globals, dhry_1.c's */
extern Rec_Pointer Ptr_Glob, Next_Ptr_Glob;
extern int Int_Glob;
extern Boolean Bool_Glob;
extern char Ch_1_Glob, Ch_2_Glob;
extern int Arr_1_Glob[50];
extern int Arr_2_Glob[50][50];

/* dhry_2.c's */
void Proc_6(Enumeration Enum_Val_Par, Enumeration *Enum_Ref_Par);
void Proc_7(One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val, One_Fifty *Int_Par_Ref);
void Proc_8(Arr_1_Dim Arr_1_Par_Ref, Arr_2_Dim Arr_2_Par_Ref, int Int_1_Par_Val, int Int_2_Par_Val);
Enumeration Func_1(Capital_Letter Ch_1_Par_Val, Capital_Letter Ch_2_Par_Val);
Boolean Func_2(Str_30 Str_1_Par_Ref, Str_30 Str_2_Par_Ref);
Boolean Func_3(Enumeration Enum_Par_Val);

/* What the last run left, for the check Dhrystone's own listing prints. */
struct dhry_result {
    int Int_1_Loc, Int_2_Loc, Int_3_Loc;
    Enumeration Enum_Loc;
    char Str_1_Loc[31], Str_2_Loc[31];
};

#define DHRY_VAX_DPS 1757   /* the VAX 11/780's Dhrystones a second: 1 DMIPS */

/* Sets the data up as Dhrystone's main does, then runs `runs` passes of its loop. */
void dhry_run(unsigned long runs, struct dhry_result *r);
/* 0 when every value Dhrystone 2.1 says it should have after `runs` passes is right; else the
 * first wrong one's name. */
const char *dhry_check(unsigned long runs, const struct dhry_result *r);

#endif
