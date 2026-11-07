# GFOpenBenchmark

One benchmark for an Amiga with a 68040 and its FPU: integer MIPS, MFLOPS, memory bandwidth, a few 2D drawing figures and GL through GFOpenGPU. It does the same fixed work each time, so results compare across days, runtime builds and machines. MIT licence, Dalsin Limited / the AmiCradle Team.

It isn't the desktop. The desktop in this repository is GFOpenBench (`desktop/`). This program is called GFOpenBenchmark so that the two never share a name in `C:` or `SYS:System`.

```
GFOpenBenchmark [CPU] [FPU] [MEM] [2D] [GL] [QUICK] [NOFORBID] [HOST] [LABEL=<text>]
```

- With no test named, it runs all of them. A full run takes about four minutes on AmiCradle.
- `QUICK` makes each run a quarter of a second, to try the program out. Don't compare QUICK figures.
- `NOFORBID` lets other tasks run during the CPU, FPU and memory tests. By default they're held off, as SysInfo and ACInfo do.
- `HOST` prints the AmiCradle host's whole status.
- `LABEL` is carried into the JSON line, for example the lab's name or the runtime's build.

The AmigaDOS stack it gets is enough: Cradle's `amiga_run` gives 256 KB, and GL calls run on ogles3.library's own 1 MB stack.

## The report

The report is plain text. For each test it gives the median of three timed runs, the three runs themselves and how long each lasted.

It opens with the machine:
- the CPU, FPU and caches;
- exec's version and GFOpenUp's version (from its manifest);
- gfopengpu.library and ogles3.library;
- memory and the E clock;
- on AmiCradle, from accontrol.device's host status: the CPU's pace, the runtime's build, the CPU personality, the backend, the motherboard and the ROM standard;
- the GL version and renderer: the host GPU under virgl, or softpipe;
- the compiler and flags it was built with.

Its last line starts with `GFB-JSON `. The rest of that line is one JSON object holding every fact and figure, for comparing and charting. A test that couldn't run is `null` there.

## What it measures

Every run lasts at least 2 seconds. The CPU, FPU and memory tests are timed by the E clock (timer.device), under `Forbid()`. The program first grows the work until one call lasts a quarter of a second. It then sizes each run for 2.2 seconds, and a run that comes out short is made longer and run again.

### CPU

| Line | What it is |
| --- | --- |
| Dhrystone 2.1 | Weicker's Dhrystone 2.1, written again from its published description. Its two files are kept apart (`dhry_1.c`, `dhry_2.c`) and nothing is inlined, as its rules ask. After each run every value its listing gives ("should be") is checked, and the line says WRONG if one differs. The host tests run the same check. |
| Dhrystone MIPS | Dhrystones a second / 1757 (the VAX 11/780's score, 1 DMIPS): the standard scale. |
| SysInfo-scale MIPS | ACInfo SPEED's loop (amicradle-guest `common/sysprobe`, the one GFOpenMonitor's Speed view runs), one second under `Forbid()`, three times. It reports the loop's count / 958. |

The three MIPS figures measure different things, and none of them is the JIT's loop MIPS.

- **SysInfo 4.4.** SysInfo's MIPS is its own Dhrystone-style count divided by 958. Lab t3-sharable measured that on 17 October 2026, against SysInfo's six reference Amigas. SysInfo's loop itself isn't published. The SysInfo-scale line therefore uses ACInfo's loop, which was calibrated to read what SysInfo reads on the same machine. It's comparable with SysInfo, not identical to it.
- **SysInfo on our ROM.** SysInfo 4.4 doesn't start on the open ROM (tools/labbench in amicradle, 24 October 2026), so it can't be run beside this program there.
- **Dhrystone 2.1 against SysInfo's loop.** The two differ in kind. Dhrystone 2.1 copies records and strings and makes the procedure calls, at 1,757 to the VAX. SysInfo's loop does less, at 958.
- **The JIT's loop MIPS.** Figures such as the AC090 review's "1,600 MIPS" count instructions translated in a loop. They're a third thing again.

### FPU

The program is built `-m68040 -m68881` (libm881). Its FPU tests run on the FPU, with no soft-float calls: `build.sh` checks that.

| Line | What it is |
| --- | --- |
| Whetstone | Curnow and Wichmann's Whetstone in double precision, with the module weights of the usual C translation, in MWIPS (one loop is 100,000 Whetstone instructions). Modules 7 and 11 call `sin`, `cos`, `atan`, `exp`, `log` and `sqrt` from libm881. The integer and branch modules' variables are volatile, so a compiler can't work those loops out at build time. |
| LINPACK 100 | A 100x100 double system solved by LU with partial pivoting (matgen, dgefa, dgesl, rolled BLAS). It's counted as 2/3 n³ + 2 n² flops a solve, in MFLOPS. Each run's answer is checked: x is all ones, and the normalised residual is of order 1. The host gives 1.67 and the 68040 1.81, because of its extended-precision registers. |
| DAXPY | y = y + a x on 1,000 doubles, at 2 flops an element. |
| Single multiply / divide (C) | Four chains of `float` multiplies or divides, which GCC emits as the 68040's FSMUL/FSDIV forms. |
| FSGLMUL.X / FSGLDIV.X | The same four chains, written in assembly with the FSGLMUL and FSGLDIV instructions themselves. |

### Memory

Two 1 MiB blocks are allocated in Fast RAM, then in Chip RAM. On each:
- a copy by a plain long-word loop;
- a copy by exec's `CopyMemQuick`, the system's own;
- a fill;
- a read.

The figures are MB/s, in MiB (1,048,576 bytes) a second.

### 2D

`RectFill` 128x128, `ClipBlit` 128x128 and `Text` of 32 characters, drawn in a window on the Workbench screen through graphics.library. Each run ends with `WaitBlit`. Other tasks run meanwhile.

### GL

GL goes through **ogles3.library**, GFOpenGPU's OpenGL ES 3.1 face, by its dispatch table, as gfopenrtg's OGLES3Check does. The context draws in the library's own 640x480 window, with no VSYNC. There are five scenes:

| Line | What it draws |
| --- | --- |
| Present | Empty frames (`glClear` and `aglSwapBuffers`): a frame's fixed cost |
| Fill | 16 full-screen quads a frame, blended, in a flat colour (Mpixels/s) |
| Triangles | A static mesh of 130,050 triangles in buffer objects, drawn 4 times a frame (Mtris/s) |
| Shader | One full-screen quad of a 128-step Mandelbrot at a fixed place (frames/s) |
| Compute | An ES 3.1 compute dispatch of 1,048,576 invocations, 64 multiply-adds each. A step is 16 dispatches and a `glFinish` (dispatches/s) |

**GL is timed by the host's clock, not the E clock.** In hyper mode the guest's clock falls behind real time while the CPU waits on a host call. Afterwards it catches up at most 2 ms a slice (a1200native.c). So a GL-bound loop timed by the E clock reads several times too fast: a quick GL run took 33 s by the host's clock and 24 s by the E clock.

On AmiCradle the battery clock (MSM6242B) reads the host's own time, in whole seconds. Each GL run therefore:
1. works up to the turn of a second;
2. counts steps until two more seconds have turned (one with QUICK);
3. ends with `glFinish`.

Without a battery clock it falls back to the E clock, and says so. The summary gives the whole run's length both ways, as a check.

## Building

```
STOVE=~/AmiCradle/stoves/os32-gcc16-open6 GUEST=../amicradle-guest ORTG=../gfopenrtg sh benchmark/build.sh [OUT_DIR]
```

Or use Cradle's `stove_build`, with target `benchmark` and stove `os32-gcc16-open6`. Its `build-amiga.sh` fetches the parts it needs at main:
- amicradle-guest's `common/sysprobe` and accontrol.device's header;
- gfopenrtg's `library/ogles3`, whose `gen_ogles3.py` makes ogles3.library's slot numbers.

The top-level `build.sh` builds the program too. `tests/host/run.sh` runs the portable kernels on the host: Dhrystone's checks, LINPACK's answer, and finite results from the rest.

## Comparing runs

- Compare like with like: the same runtime build, the same instance or lab, hyper pace with the JIT on, and a quiet PC.
- On AmiCradle, the PC's load moves the CPU figures a lot (tools/labbench in amicradle). Note the host's `uptime` beside each run.
- The baseline of 7 November 2026 is in the AmiCradle project's notes, as `salvor/Benchmark-Baseline-20261107.md`.
