/* GFOpenBenchmark: Dhrystone 2.1, its first file (the main loop, Proc_1 to Proc_5). See dhry.h.
 * MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team. */
#include <string.h>
#include "dhry.h"

Rec_Pointer Ptr_Glob, Next_Ptr_Glob;
int Int_Glob;
Boolean Bool_Glob;
char Ch_1_Glob, Ch_2_Glob;
int Arr_1_Glob[50];
int Arr_2_Glob[50][50];

/* Dhrystone's main takes its two records from malloc; here they're the program's own, the same size */
static Rec_Type rec_a, rec_b;

#define structassign(d, s) d = s

static void Proc_3(Rec_Pointer *Ptr_Ref_Par);

static __attribute__((noinline)) void Proc_1(Rec_Pointer Ptr_Val_Par)
{
    Rec_Pointer Next_Record = Ptr_Val_Par->Ptr_Comp;
    structassign(*Ptr_Val_Par->Ptr_Comp, *Ptr_Glob);
    Ptr_Val_Par->variant.var_1.Int_Comp = 5;
    Next_Record->variant.var_1.Int_Comp = Ptr_Val_Par->variant.var_1.Int_Comp;
    Next_Record->Ptr_Comp = Ptr_Val_Par->Ptr_Comp;
    Proc_3(&Next_Record->Ptr_Comp);
    if (Next_Record->Discr == Ident_1) {
        Next_Record->variant.var_1.Int_Comp = 6;
        Proc_6(Ptr_Val_Par->variant.var_1.Enum_Comp, &Next_Record->variant.var_1.Enum_Comp);
        Next_Record->Ptr_Comp = Ptr_Glob->Ptr_Comp;
        Proc_7(Next_Record->variant.var_1.Int_Comp, 10, &Next_Record->variant.var_1.Int_Comp);
    } else {
        structassign(*Ptr_Val_Par, *Ptr_Val_Par->Ptr_Comp);
    }
}

static __attribute__((noinline)) void Proc_2(One_Fifty *Int_Par_Ref)
{
    One_Fifty Int_Loc;
    Enumeration Enum_Loc = Ident_2;
    Int_Loc = *Int_Par_Ref + 10;
    do {
        if (Ch_1_Glob == 'A') {
            Int_Loc -= 1;
            *Int_Par_Ref = Int_Loc - Int_Glob;
            Enum_Loc = Ident_1;
        }
    } while (Enum_Loc != Ident_1);
}

static __attribute__((noinline)) void Proc_3(Rec_Pointer *Ptr_Ref_Par)
{
    if (Ptr_Glob != 0)
        *Ptr_Ref_Par = Ptr_Glob->Ptr_Comp;
    Proc_7(10, Int_Glob, &Ptr_Glob->variant.var_1.Int_Comp);
}

static __attribute__((noinline)) void Proc_4(void)
{
    Boolean Bool_Loc;
    Bool_Loc = Ch_1_Glob == 'A';
    Bool_Glob = Bool_Loc | Bool_Glob;
    Ch_2_Glob = 'B';
}

static __attribute__((noinline)) void Proc_5(void)
{
    Ch_1_Glob = 'A';
    Bool_Glob = 0;
}

void dhry_run(unsigned long runs, struct dhry_result *r)
{
    One_Fifty Int_1_Loc = 0, Int_2_Loc = 0, Int_3_Loc = 0;
    char Ch_Index;
    Enumeration Enum_Loc = Ident_1;
    Str_30 Str_1_Loc, Str_2_Loc;
    unsigned long Run_Index;

    Next_Ptr_Glob = &rec_b;
    Ptr_Glob = &rec_a;
    memset(&rec_a, 0, sizeof rec_a);
    memset(&rec_b, 0, sizeof rec_b);
    memset(Arr_1_Glob, 0, sizeof Arr_1_Glob);
    memset(Arr_2_Glob, 0, sizeof Arr_2_Glob);
    Int_Glob = 0;
    Bool_Glob = 0;
    Ch_1_Glob = Ch_2_Glob = 0;
    Ptr_Glob->Ptr_Comp = Next_Ptr_Glob;
    Ptr_Glob->Discr = Ident_1;
    Ptr_Glob->variant.var_1.Enum_Comp = Ident_3;
    Ptr_Glob->variant.var_1.Int_Comp = 40;
    strcpy(Ptr_Glob->variant.var_1.Str_Comp, "DHRYSTONE PROGRAM, SOME STRING");
    strcpy(Str_1_Loc, "DHRYSTONE PROGRAM, 1'ST STRING");
    Arr_2_Glob[8][7] = 10;

    for (Run_Index = 1; Run_Index <= runs; ++Run_Index) {
        Proc_5();
        Proc_4();
        Int_1_Loc = 2;
        Int_2_Loc = 3;
        strcpy(Str_2_Loc, "DHRYSTONE PROGRAM, 2'ND STRING");
        Enum_Loc = Ident_2;
        Bool_Glob = !Func_2(Str_1_Loc, Str_2_Loc);
        while (Int_1_Loc < Int_2_Loc) {
            Int_3_Loc = 5 * Int_1_Loc - Int_2_Loc;
            Proc_7(Int_1_Loc, Int_2_Loc, &Int_3_Loc);
            Int_1_Loc += 1;
        }
        Proc_8(Arr_1_Glob, Arr_2_Glob, Int_1_Loc, Int_3_Loc);
        Proc_1(Ptr_Glob);
        for (Ch_Index = 'A'; Ch_Index <= Ch_2_Glob; ++Ch_Index) {
            if (Enum_Loc == Func_1(Ch_Index, 'C')) {
                Proc_6(Ident_1, &Enum_Loc);
                strcpy(Str_2_Loc, "DHRYSTONE PROGRAM, 3'RD STRING");
                Int_2_Loc = (int)Run_Index;
                Int_Glob = (int)Run_Index;
            }
        }
        Int_2_Loc = Int_2_Loc * Int_1_Loc;
        Int_1_Loc = Int_2_Loc / Int_3_Loc;
        Int_2_Loc = 7 * (Int_2_Loc - Int_3_Loc) - Int_1_Loc;
        Proc_2(&Int_1_Loc);
    }
    r->Int_1_Loc = Int_1_Loc;
    r->Int_2_Loc = Int_2_Loc;
    r->Int_3_Loc = Int_3_Loc;
    r->Enum_Loc = Enum_Loc;
    memcpy(r->Str_1_Loc, Str_1_Loc, sizeof r->Str_1_Loc);
    memcpy(r->Str_2_Loc, Str_2_Loc, sizeof r->Str_2_Loc);
}

/* The values Dhrystone 2.1's listing says each should have ("should be: ..."). */
const char *dhry_check(unsigned long runs, const struct dhry_result *r)
{
    static const char some[] = "DHRYSTONE PROGRAM, SOME STRING";
    if (Int_Glob != 5) return "Int_Glob";
    if (Bool_Glob != 1) return "Bool_Glob";
    if (Ch_1_Glob != 'A') return "Ch_1_Glob";
    if (Ch_2_Glob != 'B') return "Ch_2_Glob";
    if (Arr_1_Glob[8] != 7) return "Arr_1_Glob[8]";
    if (Arr_2_Glob[8][7] != (int)runs + 10) return "Arr_2_Glob[8][7]";
    if (Ptr_Glob->Discr != 0) return "Ptr_Glob->Discr";
    if (Ptr_Glob->variant.var_1.Enum_Comp != 2) return "Ptr_Glob->Enum_Comp";
    if (Ptr_Glob->variant.var_1.Int_Comp != 17) return "Ptr_Glob->Int_Comp";
    if (strcmp(Ptr_Glob->variant.var_1.Str_Comp, some)) return "Ptr_Glob->Str_Comp";
    if (Next_Ptr_Glob->Ptr_Comp != Ptr_Glob->Ptr_Comp) return "Next_Ptr_Glob->Ptr_Comp";
    if (Next_Ptr_Glob->Discr != 0) return "Next_Ptr_Glob->Discr";
    if (Next_Ptr_Glob->variant.var_1.Enum_Comp != 1) return "Next_Ptr_Glob->Enum_Comp";
    if (Next_Ptr_Glob->variant.var_1.Int_Comp != 18) return "Next_Ptr_Glob->Int_Comp";
    if (strcmp(Next_Ptr_Glob->variant.var_1.Str_Comp, some)) return "Next_Ptr_Glob->Str_Comp";
    if (r->Int_1_Loc != 5) return "Int_1_Loc";
    if (r->Int_2_Loc != 13) return "Int_2_Loc";
    if (r->Int_3_Loc != 7) return "Int_3_Loc";
    if (r->Enum_Loc != 1) return "Enum_Loc";
    if (strcmp(r->Str_1_Loc, "DHRYSTONE PROGRAM, 1'ST STRING")) return "Str_1_Loc";
    if (strcmp(r->Str_2_Loc, "DHRYSTONE PROGRAM, 2'ND STRING")) return "Str_2_Loc";
    return 0;
}
