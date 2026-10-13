# Game Decompilation & Recompilation: A Developer's Guide

*Main case study: **Twilight Princess**, covering the [zeldaret/tp](https://github.com/zeldaret/tp) decompilation and the [Dusklight](https://github.com/TwilitRealm/dusklight) PC port built on it.*
*Work-in-progress example: **Skyward Sword** ([zeldaret/ss](https://github.com/zeldaret/ss)).*
*Recompilation example: [NWiiRecomp](https://github.com/BlackLineInteractive/NWiiRecomp).*

---

## 0. The big picture

Say you want an old console game to run natively on a PC. There are three broad ways to do it:

| Approach | What you do | Result | Example |
|---|---|---|---|
| **Emulation** | Simulate the console's CPU and hardware at runtime. | The original binary runs inside a virtual console. | Dolphin |
| **Decompilation → source port** | Humans rewrite the whole game as readable C/C++, verify it, then port that source. | Real source code you can modify, recompile, and port anywhere. | zeldaret/tp → **Dusklight** |
| **Static recompilation** | A tool mechanically translates the machine code into C/C++ ahead of time, and a runtime fakes the hardware. | A native executable, built mostly automatically, from code nobody can really read. | **NWiiRecomp**, N64Recomp |

This guide covers the second and third approaches. Twilight Princess shows decompilation through to a finished, shipping port. Skyward Sword shows what such a project looks like while it is still in progress, and it is a place where you can contribute. NWiiRecomp shows the recompilation shortcut.

---

# Part I — Decompilation

## 1. The one-sentence version

A **matching decompilation** means writing C/C++ source code by hand that, when compiled with **the same compiler the original developers used**, produces a binary that is **byte-for-byte identical** to the one on the game disc.

Nobody "runs a decompiler and gets the source back." People rewrite the game one function at a time, and an automated diff tells them whether each function is exactly right.

## 2. Why this is possible

Compiling is lossy: names, comments, types, and structure get thrown away. So how can anyone get the source back?

Strictly speaking, they don't. They rely on three facts:

1. **Compilers are deterministic.** The same source with the same compiler version and flags always gives the same machine code.
2. **Old console compilers are fairly simple.** GameCube and Wii games were built with **Metrowerks CodeWarrior** (`mwcc`). Its optimizer is much less aggressive than modern GCC/Clang, so the C++ code maps onto the assembly closely enough that a person can reason backwards from one to the other.
3. **The original compilers can still be obtained.** These projects download the exact CodeWarrior versions automatically. They are Windows `.exe` files, so on Linux/macOS they run through [`wibo`](https://github.com/decompals/wibo), a small Win32 shim.

Put those together and you get a feedback loop:

```
 guess C++ ──► compile with mwcc ──► compare to original asm ──► tweak ──► repeat
                                            │
                                            └── 100% identical? → function "matched" ✅
```

The result is not *the* original source. It is *a* source that compiles to the same thing. Variable names, comments, and file organization are the community's best reconstruction.

## 3. Matching vs. non-matching decomps

| | **Matching** (TP, SS, OoT, SM64, TWW…) | **Non-matching / functional** |
|---|---|---|
| Goal | Byte-identical binary | Code that *behaves* the same |
| Verification | Automatic: diff against original | Testing / playing |
| Compiler | Must be the original one | Anything |
| Effort | Very high | Lower |
| Benefit | Provably correct; a trustworthy base for ports, mods, research | Faster to produce |

Matching is the gold standard because it can be **verified mechanically**. You never have to argue about whether the code is right, because the diff settles it. That guarantee is what makes a port like Dusklight possible. It starts from code that is known to be equivalent to the real game, so any bugs that appear later come from the porting work and not from the decompilation.

## 4. Case study: Twilight Princess ([zeldaret/tp](https://github.com/zeldaret/tp))

**Where it stands:**
- Started in **August 2020**. Its contributors describe it as the largest decompilation project ever completed.
- **The code for the GameCube releases (`GZ2E01`, `GZ2P01`, `GZ2J01`) is fully matching.** Work continues on the Wii versions, the kiosk demo, and even the Nvidia Shield (China) release.
- All of those versions build from **one codebase**. `configure.py` passes `-DVERSION=<n>`, and the code uses `#if VERSION == VERSION_GCN_JPN`-style conditionals wherever the releases differ.
- The README says plainly that the project **"is not, and will not, produce a port."** It only produces the original binary. Porting is someone else's job, which turned out to be Dusklight's (Part II).

### 4.1 Anatomy of a GameCube/Wii game

| Piece | What it is |
|---|---|
| `sys/main.dol` | The main executable. **DOL** is the GameCube/Wii format: a simple header plus a list of code (`.text`) and data (`.data`, `.rodata`, `.bss`…) sections loaded at fixed addresses (`0x80004000`+). |
| `*.rel` files | **Relocatable modules**, which work like DLLs/`.so` files: code loaded on demand and patched to its load address. In TP (and SS) most **actors** (enemies, NPCs, objects, invisible trigger "tags") live in their own REL. |
| Everything else | **Assets**: models, textures, sounds, maps. **These decomp repos contain no assets and no assembly.** You must supply your own legally obtained copy of the game. |

The CPU is a **PowerPC** (Gekko on GameCube, Broadway on Wii: 32-bit, big-endian), so all the assembly you will read is PPC (`lwz`, `stw`, `addi`, `bl`, `blr`…).

### 4.2 The pipeline, step by step

TP, SS, and most GameCube/Wii decomps share the same skeleton, the [dtk-template](https://github.com/encounter/dtk-template). The snippets below come from the SS repo. TP's `config/<VERSION>/` folders use the same format.

**Step 1 — Split the binary into "objects".** The original game was built from hundreds of `.cpp` files, each compiled into a `.o` and then linked together. You work out where each original object starts and ends inside the binary, so they can be rebuilt **one at a time**. That information goes in `splits.txt`:

```
toBeSorted/mpls.cpp:
	.text       start:0x80006CE0 end:0x80006E08 align:16
	.data       start:0x804FD060 end:0x804FD0B8
```

**Step 2 — Recover symbols.** `symbols.txt` names every function and global and records its size and type:

```
__start = .init:0x80004050; // type:function size:0x168 scope:weak
memcpy  = .init:0x800043C4; // type:function size:0x29C scope:global
```

The names come from several sources. Some games accidentally shipped with debug info or map files. Shared libraries (Nintendo's `dolphin`/`revolution` SDK, `JSystem`, the MSL C library) appear in many games, so once one project names a library function, the others can recognize it by its byte pattern. Strings and RTTI left in the binary also help. Finally, people read the code and name things. Names such as `searchBaseByID__10fManager_cF9fBaseID_e` are **CodeWarrior-mangled C++** and have to match exactly for linking to work.

**Step 3 — Disassemble everything.** **[decomp-toolkit (`dtk`)](https://github.com/encounter/decomp-toolkit)** reads the DOL and RELs, cuts them along the splits, and emits one **assembly object** per file, with its **relocations** (which instruction points at which symbol). At this point **the whole game rebuilds from asm and is identical to the original**. That is day zero: 0% C++, and still a working build.

**Step 4 — Replace asm with C++, one object at a time.** `configure.py` lists every object with a status:

```python
Object(Matching,    "toBeSorted/mpls.cpp"),          # use the C++ version
Object(NonMatching, "toBeSorted/file_manager.cpp"),  # WIP: keep linking the original asm
```

The build **always produces a working, identical game**. "Progress" is just the share of bytes that come from C++ instead of extracted asm. For TP's GameCube versions that share is effectively 100%.

**Step 5 — Diff until it matches.** **[objdiff](https://github.com/encounter/objdiff)** places the original function's assembly next to your compiled version and updates live whenever you save:

```
 ORIGINAL                         YOURS
 lwz   r3, 0x4(r31)               lwz   r3, 0x4(r31)
 cmpwi r3, 0                      cmpwi r3, 0
 beq   .L_80123450                bne   .L_80123460      ◄── branch inverted
 li    r4, 1                      li    r4, 0             ◄── different
```

You rearrange your C++ until both sides are identical.

**Step 6 — Clean up.** A function that matches but is unreadable (`field_0x568`, `unk_0x1C`) is only half done. Then comes naming, building correct class layouts in headers, and documenting.

### 4.3 What finished decompiled code looks like

Here is a complete, matching actor from TP, `src/d/actor/d_a_tag_theB_hint.cpp`. It is an invisible trigger that makes Telma's NPC show a hint:

```cpp
int daTagTheBHint_c::create() {
    fopAcM_ct(this, daTagTheBHint_c);
    field_0x568 = std::pow(scale.x * 100.0f,2.0f);   // squared trigger radius
    return cPhs_COMPLEATE_e;
}

int daTagTheBHint_c::execute() {
    if (fopAcM_isSwitch(this, getSwitchNo())) {      // already triggered? remove self
        fopAcM_delete(this);
        return 1;
    }

    if (fopAcM_searchPlayerDistanceXZ2(this) < field_0x568) {   // Link is close enough
        daNpcTheB_c* the_b = (daNpcTheB_c*)fpcM_SearchByName(fpcNm_NPC_THEB_e);
        if (the_b) {
            // !@bug Comparison of s8 (getRoomNo) with 0xFF always evaluates to false
            int roomNo = getRoomNo() == 0xff ? fopAcM_GetRoomNo(this) : getRoomNo();

            the_b->setHintEvent(getMessageNo(), getLinkID(), roomNo);
        }
    }
    return 1;
}
```

Things to notice:

- **It is not pretty, and it isn't meant to be.** `field_0x568` is still an unnamed member at offset `0x568`, because the job was to *match*, and naming comes later.
- **`!@bug`**: the decomp *reproduces the original developers' bug* (a signed `s8` can never equal `0xFF`), because fixing it would change the binary. The bug gets documented instead. Ports can then decide whether to fix it.
- **Naming conventions.** `d` = game code, `f_op`/`f_pc` = Nintendo's actor/process framework, `fopAcM_*` = actor manager helpers, `_c` = class. These come from Nintendo's own code (leftover symbols) and are reused across their games.
- **`g_profile_Tag_TheBHint`** (at the bottom of the real file) is a table that registers the actor with the engine so levels can spawn it by ID.

### 4.4 Why matching is hard: the dark arts

Writing C++ that *behaves* like the asm is the easy part. Writing C++ that *compiles to the exact same instructions* is the hard part.

| Problem | What it looks like | Typical fix |
|---|---|---|
| **Register allocation** | Same logic, but `r30` where the original used `r31`. | Reorder declarations, add or remove a temporary, change scope. |
| **Instruction order** | Two loads swapped. | Reorder statements; sometimes an otherwise pointless temp is needed. |
| **Inlining** | Original had a call; yours got inlined (or the reverse). | Move a function into or out of a header, `inline`, flags. |
| **Types** | `lha` vs `lhz` (signed vs unsigned load). | Fix `s16`/`u16`, `int`/`u32`. Asm reveals types this way. |
| **Branch shape** | `beq` vs `bne`, block order. | Invert the `if`, `switch` vs `if/else`, early `return`. |
| **Data ordering** | Floats/strings in `.rodata` out of order. | Order of first use/definition in the file matters. |
| **Compiler version/flags** | Nothing works no matter what. | That file was built with a different `mwcc` version or flag (`-opt noloop`…). See per-file `extra_cflags`. |
| **Class layout** | Wrong struct offsets everywhere. | Rebuild the header: base classes, vtable, member order/sizes. |

Some functions match in five minutes. Others hold out for months. That is why projects keep `NonMatching` and `Equivalent` (logically correct but not byte-identical) statuses.

---

## 5. A decomp in progress: Skyward Sword ([zeldaret/ss](https://github.com/zeldaret/ss))

TP is where a decomp ends up. **SS is what one looks like in the middle**, and it is a better place to get involved because there is still plenty of work left.

- **Target:** `SOUE01` (USA 1.0, Wii) only. From your disc you need just `sys/main.dol` and `files/rels.arc`, which holds **636 RELs**.
- **Progress** is tracked by the README badges (code %, DOL %, RELs %) on [decomp.dev](https://decomp.dev/zeldaret/ss).
- **Signs of a project under way:**
  - A `toBeSorted/` directory, the honest name for "we know this code exists, but not which original file it came from."
  - `configure.py` mixes `Matching` and `NonMatching` objects.
  - Stubs like this one (`src/REL/d/a/obj/d_a_obj_wood_board.cpp`):
    ```cpp
    STATE_DEFINE(dAcOwoodBoard_c, Wait);
    STATE_DEFINE(dAcOwoodBoard_c, Break);
    void dAcOwoodBoard_c::initializeState_Wait() {}
    void dAcOwoodBoard_c::executeState_Wait() {}
    ...
    ```
    SS actors run on a state machine (`initializeState_X` / `executeState_X` / `finalizeState_X`). Recognizing that pattern lets people stamp out the skeletons first and fill in the bodies later.
- **A finished SS example**: `src/REL/d/t/d_t_timer.cpp`, a "do X within N seconds" puzzle trigger:
  ```cpp
  int dTgTimer_c::actorExecute() {
      bool getFlag = SceneflagManager::sInstance->checkFlag(mRoomID, getCheckSceneflag());
      if (getFlag) {
          if (checkShouldTrigger()) {
              SceneflagManager::sInstance->setFlag(mRoomID, getSetSceneflag());
          } else {
              incrementTimer();
          }
      } else {
          resetTimer();
      }
      return SUCCEEDED;
  }
  u16 dTgTimer_c::getCheckSceneflag() { return mParams >> 0x10 & 0xFF; }
  ```
  Level designers pack settings into a single 32-bit `mParams` (subtype, time, flag to watch, flag to set). Every frame, the timer counts while the watched flag is set and sets the other flag when the time runs out.
- **Engine lineage differs from TP.** SS uses the framework from Nintendo EAD's later Wii games (`f`, `m`, `s`, `EGG`, `nw4r` layers, shared with titles like *New Super Mario Bros. Wii*), so TP knowledge carries over only partly.

**Why does this matter for you?** If SS reaches 100%, the same Dusklight-style port becomes possible for it. Every function matched now moves that closer.

---

# Part II — From decompilation to a port: Dusklight

## 6. What Dusklight is

[Dusklight](https://github.com/TwilitRealm/dusklight) (originally "Dusk", renamed at v1.2) is, in its own words, *"a reverse-engineered reimplementation of Twilight Princess"* built on the TP decomp. It runs natively on **Windows, Linux, macOS, iOS, and Android**, renders through **D3D12 / Vulkan / Metal** (with best-effort D3D11 and OpenGL ES), and adds higher frame rates and resolutions, gyro aiming, texture packs, and mod support (a randomizer and cosmetics ship as mods). As always, **you supply your own game disc**.

The key point: **a matching decomp is not a PC game.** The code still assumes it runs on a GameCube. Turning it into a port takes four main jobs.

## 7. Job 1 — Replace the console with a compatibility layer (Aurora)

Game code doesn't talk to the TV or the controller directly. It calls Nintendo's SDK: **GX** for graphics, **VI** for video output, **PAD** for controllers, **DVD** for the disc, **CARD** for memory-card saves, **OS** for threads/timers. In the decomp those SDK libraries are themselves decompiled (`libs/dolphin/src/gx`, `libs/dolphin/src/pad`…) and poke real hardware registers.

Dusklight swaps them for **[Aurora](https://github.com/encounter/aurora)**, *"a source-level GameCube & Wii compatibility layer"* (originally built for the Metroid Prime port, Metaforce):

```
       ┌────────────────────────────────────────────┐
       │  Twilight Princess game code (from tp)      │  ← mostly unchanged C++
       │  d_a_*.cpp, JSystem, Z2Audio, …             │
       └──────────────┬─────────────────────────────┘
                      │ calls GXBegin(), PADRead(), DVDRead(), CARDWrite()…
       ┌──────────────▼─────────────────────────────┐
       │  Aurora: same function signatures,          │
       │  modern implementations                     │
       │   GX   → WebGPU (Dawn) → D3D12/Vulkan/Metal │
       │   PAD  → SDL gamepads, gyro, keyboard       │
       │   DVD  → reads your ISO/RVZ via nod         │
       │   CARD → Dolphin-compatible .gci saves      │
       └──────────────┬─────────────────────────────┘
                      ▼
               SDL3 app on PC / phone
```

In `CMakeLists.txt` the game is linked against `aurora::gx aurora::pad aurora::dvd aurora::card aurora::vi aurora::os …` instead of the original SDK. The game still "thinks" it is drawing with GX, and Aurora translates those calls into modern GPU work. That translation is also why widescreen, high resolutions, and texture replacement are possible.

## 8. Job 2 — Fix assumptions that only held on the GameCube

The original code was written for one machine: 32-bit, **big-endian** PowerPC, a fixed memory layout, and a single compiler. On a 64-bit little-endian PC, things that "worked" start breaking. Dusklight keeps the original code in place and wraps its changes in preprocessor guards (from `docs/code-conventions.md`):

> When modifying original game code … use `#if TARGET_PC` and keeping the original code in place. Use `#if AVOID_UB` for Undefined Behavior fixes to the original codebase.

The repo has well over 1,700 `TARGET_PC` blocks. Some real examples:

**Endianness.** Game data files are big-endian. Code that points straight into file data has to work on a byte-swapped copy instead (`src/d/d_msg_flow.cpp`):

```cpp
#if TARGET_LITTLE_ENDIAN
            cXyz copy = actor_data->base.position;
            player->setPlayerPosAndAngle(&copy, player->current.angle.y, 0);
#else
            player->setPlayerPosAndAngle(&actor_data->base.position, player->current.angle.y, 0);
#endif
```

**Undefined behaviour that happened to work on the GameCube** (`src/d/d_vibration.cpp`, a modulo by zero):

```cpp
#if AVOID_UB
    if (length == 0)
        index = 0;
#else
    index %= length;
#endif
```

**Buffers sized for the original data.** A string buffer was exactly big enough for the original paths. Adding language folders would overflow it (`src/d/d_s_logo.cpp`):

```cpp
#if AVOID_UB
    static char bmgPath[32];
#else
    static char bmgPath[22];
#endif
```

**Compile-time versions become runtime choices.** The decomp builds one binary per region (`#if VERSION == VERSION_GCN_JPN`). Dusklight is a single program that accepts any disc, so those checks become runtime checks (`src/d/d_meter2_info.cpp`):

```cpp
#if TARGET_PC || VERSION == VERSION_GCN_JPN
            #if TARGET_PC
            if (dusk::version::isRegionJpn())
            #endif
            { /* Shift-JIS text handling */ }
```

**Memory management.** The game allocates through a strict tree of custom heaps by overloading global `operator new`. That doesn't link well on PC, so Dusklight replaces those calls with `JKR_NEW` / `JKR_DELETE` macros.

Many files barely change, though. Diff TP's `d_a_tag_theB_hint.cpp` against Dusklight's copy and the only differences are two macros on the profile tables (`DUSK_CONST`, `DUSK_PROFILE`). The gameplay logic is byte-for-byte the decompiled code.

## 9. Job 3 — Add what a PC player expects

With the source in hand, the port adds things the console never had: a settings UI, frame-rate and resolution options, input remapping, gyro aiming, achievements, autosave, Discord presence, texture packs, and a mod system (`src/dusk/`, `mods/`). Because these live in their own files and `TARGET_PC` blocks, the original game logic stays easy to compare against upstream.

## 10. Job 4 — Stay connected to upstream

Dusklight's guideline: fixes that apply to the original decompilation (naming, docs, cleanup) **go upstream to zeldaret/tp first**. The decomp stays the single source of truth for "what the game does," and the port stays a thin layer on top.

One visible consequence: TP only matches for GameCube, so **even with a Wii disc, Dusklight shows the GameCube HUD**. The port can only be as complete as the decomp under it.

---

# Part III — Static recompilation: NWiiRecomp

## 11. The idea

Decompilation takes years of human work. **Static recompilation** asks a different question: what if a program translated the machine code automatically?

[NWiiRecomp](https://github.com/BlackLineInteractive/NWiiRecomp) *"translates Nintendo Wii/GameCube (`.dol`, `.elf`) executables into native C++ code. The output is a standalone executable that runs natively without instruction-level emulation."* It is inspired by **N64Recomp** (the basis of the *Zelda64Recomp* Majora's Mask PC port) and **PS2Recomp**.

It is *not* decompilation. The output looks like this, a direct transliteration of each PowerPC instruction operating on a fake CPU. Below is an illustrative function in the format NWiiRecomp's emitter produces:

```cpp
void func_80012340(CPUContext& ctx) {
    ctx.gpr[3] = ctx.mmu.read32(ctx.gpr[31] + 4); // lwz r3, 4(r31)
    ctx.gpr[4] = ctx.gpr[3] + 1;                  // addi r4, r3, 1
    ctx.gpr[5] = 0;                               // li r5, 0
    ...
}
```

The registers (`gpr`, `fpr`, condition register…) are fields in a `CPUContext` struct, and memory goes through an emulated MMU. A modern C++ compiler then optimizes this heavily, so it runs far faster than interpreting the instructions one at a time. **It's still the original program, though.** No names, no types, no classes, just a faithful re-expression of the asm.

## 12. The pieces

```
NWiiRecomp/
├── nWiiAnalyzer/  — parse the DOL/ELF, find where every function starts and ends
├── nWiiRecomp/    — translate each function's PPC instructions into C++
├── nWiiRuntime/   — the "fake Wii": hardware, OS services, graphics, input
└── nWiiStudio/    — GUI for inspecting disassembly and memory
```

Workflow:

```bash
./build/nWiiRecomp/nwiirecomp config.toml        # analyze + generate C++ into export/
cd export && cmake -B build && cmake --build build
./export/build/<Game> path/to/extracted-game config.toml
```

**Analyzer.** Recursive disassembly from the entry point, branch analysis for function boundaries, recovery of jump tables and vtable pointers, and signature matching against known SDK functions (using Dolphin's signature database).

**Recompiler.** Covers integer, floating-point, branch, and SPR instructions, including the GameCube's **paired-singles** SIMD. It turns local branches into `goto`s, handles mid-function entry points with a `switch(ctx.pc)`, and splits the output into many small `.cpp` files so modern compilers don't choke on it. Per-game **HLE hooks** let you swap a known function (say, a SDK routine) for a native implementation.

**Runtime.** This is where most of the work goes. Translating the CPU is the easier half. The game still writes to hardware registers and expects real hardware to respond. So the runtime emulates, at register level, the video interface (VI), disc drive (DI), controllers (SI), memory card/RTC (EXI), audio (AI/DSP), interrupts and the decrementer, and the Wii's IOS services (`/dev/fs`, `/dev/es`…), with a virtual disc reading ISO/WBFS/extracted dumps. Graphics are handled by parsing the **GX command FIFO** the game writes and rendering it with NWiiRecomp's own rasterizer and shader generator on OpenGL. There's also an **interpreter fallback** for code that only appears at runtime (code copied into memory, trampolines, streamed overlays), which a static translator can't see in advance.

## 13. Hard problems for recompilers

| Problem | Why it's hard |
|---|---|
| **Finding all the code** | Indirect calls (`bctrl` through function pointers, vtables, jump tables) hide targets. Miss one and the program jumps into nothing. |
| **Code that appears at runtime** | Overlays, RELs, self-copying code. Needs an interpreter or runtime-recompiler fallback. |
| **Hardware semantics** | The game depends on exact GPU/FIFO, DSP, and timing behaviour. This is basically the same work as writing an emulator, minus the CPU. |
| **Enhancements** | Want 60 FPS or widescreen? You're patching anonymous `func_80xxxxxx` code with no structs or names, which is much harder than in a decomp. |

## 14. Status

NWiiRecomp is **early**. As of its README, *Mario Party 7* boots through the Nintendo/Hudson logos into its 3D intro, and *Need for Speed: Hot Pursuit 2* and *Silent Hill: Shattered Memories* are being tested. Its goal is **universality**: one recompiler and runtime for many games, with per-game settings in a TOML file. Compare that to a decomp, which is a multi-year effort for a single game.

---

# Part IV — Putting it together

## 15. Decomp-port vs. recompilation vs. emulation

| | **Emulation** (Dolphin) | **Static recomp** (NWiiRecomp) | **Decomp + port** (TP → Dusklight) |
|---|---|---|---|
| Human effort per game | ~none | Low–medium (config, hooks, fixes) | **Huge** (years) |
| Readable source? | No | No (translated asm) | **Yes** |
| Native performance | No (JIT helps) | Yes | Yes |
| Modding / enhancements | Hacks, patches | Hard (anonymous code) | **Easy**: it's normal C++ |
| New platforms (phones…) | Needs the emulator ported | Needs runtime ported | Just compile it |
| Fidelity risk | Emulator accuracy | Runtime/hardware accuracy | Porting bugs (UB, endianness) |
| Works on many games | Yes | Goal: yes | **One game** |

They also combine well. Recomp projects use decomp symbol maps for names, and decomp projects use emulators (Dolphin) for debugging. The tools and people overlap heavily, with names like encounter (dtk, objdiff, Aurora) appearing across all of them.

## 16. The toolbox

| Tool | Role |
|---|---|
| **[Ghidra](https://ghidra-sre.org/)** (or IDA) | Disassembler/decompiler. Its pseudo-C is a *starting point*, never the answer. SS ships `tools/ghidra_scripts/` to sync symbols; NWiiRecomp has a Ghidra symbol exporter. |
| **[decomp-toolkit](https://github.com/encounter/decomp-toolkit)** | Splits DOL/RELs, extracts asm, handles relocations, builds the project. |
| **[objdiff](https://github.com/encounter/objdiff)** | Side-by-side asm diff with live rebuild. The tool you'll have open all day. |
| **[decomp.me](https://decomp.me)** | Web scratchpad: paste a function's asm, pick the compiler preset, iterate on C in the browser, share the link for help. Has a permuter that randomly mutates code looking for a match. |
| `tools/decompctx.py` | Builds a single preprocessed "context" header to paste into decomp.me. |
| **ninja** + `configure.py` | Build: `python configure.py && ninja`. |
| **wibo** | Runs Windows-only CodeWarrior on Linux/macOS. |
| **Dolphin** | Emulator: extract discs, set breakpoints, inspect memory at runtime. |
| **[Aurora](https://github.com/encounter/aurora)** | SDK compatibility layer for turning a decomp into a port. |

## 17. Getting your hands dirty

**Just want to play / see the result?** Install [Dusklight](https://twilitrealm.dev/install/) with your own TP disc. Then read `src/d/actor/` in its repo next to the same files in zeldaret/tp.

**Want to contribute to a decomp? SS is a good target:**

1. Dump your own `SOUE01` disc. Using Dolphin, go to *Properties → Filesystem → Data Partition → Extract Entire Partition* into `orig/SOUE01`. You only need `sys/main.dol` and `files/rels.arc`.
2. Build:
   ```sh
   git clone https://github.com/zeldaret/ss.git && cd ss
   python configure.py
   ninja
   ```
   A successful build means you've rebuilt Skyward Sword byte-for-byte (checked against `config/SOUE01/build.sha1`).
3. Open **objdiff** on the repo root. Unmatched objects are your to-do list.
4. **Read before you write.** Put finished actors (`src/REL/d/t/`, `src/REL/d/a/obj/`) next to their asm until the mapping feels natural. TP's finished code is also great reading.
5. **Learn just enough PowerPC.** About 30 instructions plus the calling convention: `r3`–`r10` hold arguments, `r3` the return value, `r1` is the stack pointer, `r14`–`r31` are callee-saved, `bl` = call, `blr` = return.
6. **Join the zeldaret Discord** (linked in the README) and ask for a good first target. Small `NonMatching` REL actors are the usual on-ramp.
7. **Match one tiny function.** When objdiff first shows 100%, the topic stops feeling intimidating.

**Curious about recomp?** Build NWiiRecomp (`cmake -B build && cmake --build build`) and point nWiiStudio at a DOL. Reading `nWiiRecomp/src/recompiler.cpp`, where each PPC opcode becomes a line of C++, is the fastest way to understand the technique.

## 18. Mental model to keep

- **Decomp:** you're matching **one function**, inside **one object**, while the rest of the game keeps building from asm. The compiler is your oracle.
- **Port:** the decomp tells you *what the game does*. The port replaces *what the console did* (Aurora) and fixes the assumptions that only held on that console (`TARGET_PC`, `AVOID_UB`).
- **Recomp:** you don't understand the code, you *translate* it, and the hard work moves into faking the hardware convincingly.

## 19. Further reading

- Twilight Princess decomp: https://github.com/zeldaret/tp
- Dusklight: https://github.com/TwilitRealm/dusklight · https://twilitrealm.dev
- Aurora: https://github.com/encounter/aurora
- Skyward Sword decomp: https://github.com/zeldaret/ss · progress: https://decomp.dev/zeldaret/ss
- NWiiRecomp: https://github.com/BlackLineInteractive/NWiiRecomp
- N64Recomp (the recomp approach at maturity): https://github.com/N64Recomp/N64Recomp
- dtk-template (how GC/Wii decomp projects are structured): https://github.com/encounter/dtk-template
- decomp.me: https://decomp.me
- Hardware docs: [YAGCD](https://hitmen.c02.at/files/yagcd/), [WiiBrew](https://wiibrew.org), the IBM PowerPC 750CL User's Manual
- Press coverage of Dusklight: [GamingOnLinux](https://www.gamingonlinux.com/2026/05/dusk-is-a-reverse-engineered-reimplementation-of-the-legend-of-zelda-twilight-princess/page=1/), [Zelda Universe](https://zeldauniverse.net/2026/05/17/twilight-princess-fans-create-a-decompiled-pc-port-called-dusklight/), [feedbagel](https://feedbagel.com/post/dusklight-gc-twilight-princess-decompiled)
