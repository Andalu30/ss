# Skyward Sword Decomp: Your Quick Start

Everything is installed and verified on this machine. This page tells you **what you have**, **what each strange word means**, and **how to spend your first hour**.

For the background (why decompilation works, Twilight Princess and Dusklight, recompilers), see `game-decompilation-guide.md`.

---

## 1. What's ready

| Item | Status |
|---|---|
| `ss/`, the decomp repo | Cloned, configured, built |
| Your disc (`SOUE01.wbfs`) | USA **revision 0**, the exact version the project needs ✅ |
| `ss/orig/SOUE01/sys/main.dol` + `files/rels.arc` | Extracted from your disc |
| Build output | **All 637 files rebuilt byte-for-byte identical** to your disc (1 main program + 636 modules) ✅ |
| Branch `shutter-fence-forbiddance` in `ss/` | The worked example from section 4: one new function decompiled, linked and verified (uncommitted) |
| Branch `ship-flood` in `ss/` | The worked example from section 5: a whole actor decompiled to 99.89% (stacked on the branch above, uncommitted) |
| Branch `stage-reverb` in `ss/` | The worked example from section 6: an untouched file decompiled from 0% to 100% and linked (stacked on the branch above, uncommitted) |
| `objdiff` (the diff GUI) | Installed at `~/.local/bin/objdiff` (v3.8.2) |
| `ss/objdiff.json` | Generated, so objdiff will configure itself |

At the time of the build, the project was at **26.8%** matched overall (DOL 45.3%, modules 14.9%).

> Your earlier ISO was **revision 1** (a later print with patches). Its code sits at different addresses, so it can't be used. The WBFS backup is the right one, so keep it.

---

## 2. The concepts, in plain terms

### The game itself

**Wii disc / WBFS / ISO.** An ISO is a raw copy of the disc. WBFS is the same data in the format USB-loader homebrew uses on a Wii. The build tool reads both.

**`SOUE01`.** The game's ID: `S` = Wii game, `OU` = Skyward Sword, `E` = USA, `01` = Nintendo. The **revision** is a separate byte (0 = first print).

**`main.dol`.** The game's main program, like an `.exe`. **DOL** is the Wii's executable format: a small header saying "load these chunks of code and data at these memory addresses."

**REL / `rels.arc`.** A REL works like a **DLL / `.so`**: a module loaded only when needed. Almost every enemy, NPC, object and invisible trigger in SS is its own REL (636 of them), packed into the `rels.arc` archive. This saves memory: the game only loads the actors in the current area.

**Actor.** Anything that "exists" in the game world and runs code each frame: Link, a Bokoblin, a door, a treasure chest, or an invisible trigger zone. File prefixes tell you the kind:
- `d_a_…` = **a**ctor (e.g. `d_a_obj_tbox` = treasure box object)
- `d_t_…` = **t**ag / trigger (invisible logic: timers, area checks, cutscene starters)
- `d_lyt_…` = **l**a**y**ou**t** (2D UI screens)

**Class suffix `_c`, prefixes `d` / `f` / `m` / `s` / `EGG` / `nw4r`.** These are Nintendo's own naming conventions, recovered from symbols left in the game. `d` = this game's code, `f` = the base object framework, `EGG`/`nw4r` = shared Nintendo libraries.

### The CPU

**PowerPC (PPC).** The Wii's processor family. All the assembly you'll read is PPC. **Big-endian**: multi-byte numbers are stored "most significant byte first," the opposite of your PC.

**Assembly / asm.** The human-readable form of machine instructions. You'll mostly see:

| Instruction | Meaning | Rough C equivalent |
|---|---|---|
| `lwz r3, 0x10(r31)` | **l**oad **w**ord: read 4 bytes at address `r31 + 0x10` | `r3 = this->field_0x10;` |
| `stw r0, 0x8(r1)` | **st**ore **w**ord | `*(r1 + 8) = r0;` |
| `li r3, 1` | **l**oad **i**mmediate | `r3 = 1;` |
| `addi r3, r4, 4` | add immediate | `r3 = r4 + 4;` |
| `cmpwi r3, 0` | compare with a number | `if (r3 == 0)…` (sets flags) |
| `beq` / `bne` | branch if equal / not equal | `goto` based on the last compare |
| `bl func` | **b**ranch and **l**ink = call | `func();` |
| `blr` | branch to link register = return | `return;` |
| `lha` / `lhz` | load half-word, signed / unsigned | tells you a field is `s16` vs `u16` |

**Registers.** The CPU's 32 tiny built-in variables, `r0`–`r31`. The calling convention ("who puts what where"):
- `r3`, `r4`, … `r10` = function arguments (in C++ methods `r3` is `this`)
- `r3` = return value
- `r1` = stack pointer
- `r14`–`r31` = "saved" registers. Values that must survive across calls live here, often `r31 = this`.
- `f1`, `f2`… = float registers, used the same way for float args and returns

### The decomp process

**Decompilation (matching).** Writing C++ by hand that compiles to **exactly the same bytes** as the original. Not "similar", but identical.

**Compiler / `mwcc` / CodeWarrior.** The compiler Nintendo's developers used in 2011, Metrowerks CodeWarrior. The project downloads the exact versions. Different compiler means different bytes, so the original one is required.

**wibo.** CodeWarrior is a Windows `.exe`. wibo is a tiny program that lets Linux run it, a bit like a minimal Wine. It's downloaded into `ss/build/tools/`.

**Object file / translation unit (TU) / "unit".** One `.cpp` file compiled = one `.o`. The original game was made of thousands of them, linked together. The decomp works **one unit at a time**.

**Splits (`config/SOUE01/splits.txt`).** The map of where each original `.cpp` file's code and data sit inside `main.dol`. Example: "`mpls.cpp` is `0x80006CE0`–`0x80006E08`."

**Symbols (`config/SOUE01/symbols.txt`).** Names, addresses and sizes of every function and variable. When someone figures out what `fn_80123450` does, they rename it here.

**Mangled names.** C++ squeezes namespaces, classes and argument types into one linker name: `create__28dTgShutterFenceForbiddance_cFv` = `dTgShutterFenceForbiddance_c::create()`. objdiff shows both forms.

**Matching / NonMatching (in `configure.py`).** Every unit is marked:
- `Matching`: our C++ is proven identical, so the build uses it.
- `NonMatching`: still work in progress, so the build links the **original assembly** for that unit instead.

That's why the build always produces the real game, whatever state the C++ is in.

**"Matched" vs "linked" in the progress output.** *Matched* = the C++ compiles to identical bytes (measured by objdiff). *Linked* = that unit is marked `Matching` and actually used in the final build. Matched code becomes linked once a whole file is complete.

**SHA-1 check (`config/SOUE01/build.sha1`).** After linking, the build hashes the result and compares it with the real game. Passing means the rebuilt game is **provably identical**. It's the project's safety net: if you mark a file `Matching` and you're wrong, the build fails here.

**decomp-toolkit (`dtk`).** The engine behind the build. It reads the disc, cuts the game into units using the splits, disassembles them, and links everything back together.

**ninja.** A fast build runner, like `make`. `configure.py` writes the build rules (`build.ninja`), and `ninja` executes them.

**objdiff.** The tool you'll have open all the time. It shows the **original** asm of a function on the left, **your compiled** asm on the right, and highlights every difference. It rebuilds automatically whenever you save a file.

**Fuzzy match %.** objdiff's score for how close a function is. 100% = identical. 95% usually means the logic is right and you're fighting the compiler.

**decomp.me.** A website for the same diff-and-match loop in the browser. Paste a function, share the link, and others can help.

**`toBeSorted/`.** Code that has been decompiled but whose original file name or location isn't known yet.

---

## 3. Your daily loop

```sh
cd <workdir>/ss
git pull                 # get other contributors' latest work
python3 configure.py     # only needed if configure.py changed
ninja                    # build + verify; prints progress at the end
objdiff &                # open the diff GUI
```

**First time in objdiff:**
1. Click **Settings** next to "Project" → **Project directory → Select** → choose `<workdir>/ss`.
2. It reads `objdiff.json` automatically. The left sidebar lists every unit, colored by match status.
3. Click a unit, then a function. **Left = original, right = yours.**
4. Edit the `.cpp` in your editor and save. objdiff rebuilds and refreshes on its own.

---

## 4. Worked example: decompiling a function from start to finish

This is a real function that had **never been decompiled before**. I took it from a stub to matched, linked, and verified in this repo, on the branch `shutter-fence-forbiddance`. Follow along in objdiff, or just read. Every step below is what you'll do for any function.

**The target:** `dTgShutterFenceForbiddance_c::actorExecute()` in `src/REL/d/t/d_t_shutter_fence_forbiddance.cpp`. The file had 4 of 5 functions matched, and this one was a placeholder at 5.8%:

```cpp
// TODO after finding out more about d_a_obj_door_base
int dTgShutterFenceForbiddance_c::actorExecute() {
    return SUCCEEDED;
}
```

### Step 1 — Read the original assembly

In objdiff, click the unit `d_t_shutter_fence_forbiddance`, then `actorExecute`. The left side is what Nintendo's compiler produced. (Same thing from the terminal: `build/tools/dtk elf disasm build/SOUE01/d_t_shutter_fence_forbiddanceNP/obj/REL/d/t/d_t_shutter_fence_forbiddance.o out.s`.)

Here it is, annotated. Don't try to memorise it; just see how each line maps to an idea.

```asm
# ── prologue: set up a stack frame, save what we'll overwrite ──────────────
stwu  r1, -0x10(r1)      # grow the stack by 16 bytes
mflr  r0                 # copy the return address (link register) into r0...
li    r3, 0x0            #   (r3 = 0 → first argument for the call below = nullptr)
stw   r0, 0x14(r1)       # ...and save it on the stack
stw   r31, 0xc(r1)       # save r31 (callee-saved: we must restore it later)

# ── door = fn_80179250(nullptr) ─────────────────────────────────────────────
bl    fn_80179250        # call it; result comes back in r3
mr    r31, r3            # keep the result in r31 (survives future calls)
b     .check             # jump to the loop condition first → this is a for/while loop

.body:
# ── door->virtualFunction() ─────────────────────────────────────────────────
lwz   r12, 0x60(r31)     # r12 = door's vtable pointer (stored at offset 0x60)
mr    r3, r31            # r3 = door  → this is the "this" pointer for the call
lwz   r12, 0x8c(r12)     # r12 = vtable entry at 0x8C → which virtual function
mtctr r12                # put the function address in the count register
bctrl                    # call it  (= an indirect / virtual call)

# ── door = fn_80179250(door) ────────────────────────────────────────────────
mr    r3, r31            # argument = current door
bl    fn_80179250
mr    r31, r3            # door = result

.check:
cmpwi r31, 0x0           # door == nullptr ?
bne   .body              # if not, loop again

# ── epilogue: return 1 and restore everything ───────────────────────────────
lwz   r31, 0xc(r1)       # restore r31
li    r3, 0x1            # return value = 1   (SUCCEEDED)
lwz   r0, 0x14(r1)       # restore return address
mtlr  r0
addi  r1, r1, 0x10       # shrink the stack back
blr                      # return
```

**Rough pseudocode, before understanding anything else:**

```c
for (x = fn_80179250(NULL); x != NULL; x = fn_80179250(x))
    x->virtual_0x8C();
return 1;
```

Two patterns are worth learning to spot:
- **`b` to the condition, then a backwards `bne`** is how CodeWarrior writes a `for`/`while` loop.
- **`lwz r12, 0x60(rX)` → `lwz r12, N(r12)` → `mtctr` → `bctrl`** is a **virtual function call**. In this engine every object keeps its vtable pointer at offset `0x60` (`include/f/f_base.h` documents it: `/* 0x60 */ // vtable`). `N` tells you which virtual function.

Two unknowns remain: **what is `fn_80179250`**, and **what is virtual slot `0x8C`**?

### Step 2 — Find out what the unknown function is

`fn_…` means "nobody has named this yet." Look it up:

```sh
grep -n 0x80179250 config/SOUE01/symbols.txt
#  fn_80179250 = .text:0x80179250; // type:function size:0x34
```

Then find which original file contains that address in `splits.txt`. It's inside the range for **`d/a/d_a_door_base.cpp`**, the same door code the `TODO` mentioned. That file is still `NonMatching` and has no C++ yet, so I disassembled its original object (`build/SOUE01/obj/d/a/d_a_door_base.o`) and found two small helpers:

```asm
fn_80179290:                      # given a door, return the NEXT list node
    cmpwi r3, 0x0
    bne   .notNull
    lwz   r3, DOOR_ACTOR_LIST     # door == NULL → return the list's FIRST node
    blr
.notNull:
    lwz   r3, 0x334(r3)           # else → return door->(field at 0x334)
    blr

fn_80179250:                      # given a door, return the NEXT DOOR
    ...prologue...
    bl    fn_80179290             # node = fn_80179290(door)
    cmpwi r3, 0x0
    bne   .ok
    li    r3, 0x0                 # node == NULL → return NULL
    b     .end
.ok:
    lwz   r3, 0x8(r3)             # else → return node->(field at 0x8)
.end:
    ...epilogue...
```

**Matching offsets to known classes** (this is the detective work):
- The header `include/d/a/obj/d_a_obj_door_base.h` already describes the door class `dAcObjDoor_c`. It has a static `sDoorList` (a list of all doors) and a member `mDoorRef`, a **list node** that links each door into that list.
- List nodes (`include/f/f_list_nd.h`, `c/c_list.h`) look like `{ prev at 0x0, next at 0x4, p_owner at 0x8 }`.
- So `0x334(r3)` = the door's `mDoorRef` (which starts at `0x330`) + 4 = **its `next` pointer**, and `0x8(r3)` = the node's **`p_owner`**, the door that node belongs to.

In plain words, `fn_80179250(door)` means **"give me the door after this one, or the first door if I pass `nullptr`."** It's an iterator over every door in the level.

**And slot `0x8C`?** The same header already lists `dAcObjDoor_c`'s virtual functions, with names based on their slots: `canOpen`, `hasVisitedRoom`, `vt_0x88`, **`vt_0x8C`**. Slot `0x8C` is `vt_0x8C()`. Nobody knows what it does yet, so the placeholder name stays. **That's fine: you don't need to understand everything to match it.**

### Step 3 — Give the function a name (`symbols.txt`)

To call `fn_80179250` from C++, it needs a real C++ name. Naming means editing **one line** in `config/SOUE01/symbols.txt`, using the **mangled** name the compiler will produce:

```diff
-fn_80179250 = .text:0x80179250; // type:function size:0x34
+getNextDoor__12dAcObjDoor_cFP12dAcObjDoor_c = .text:0x80179250; // type:function size:0x34
```

Reading the mangled name: `getNextDoor` + `__` + `12dAcObjDoor_c` (the class, prefixed by its name length, 12) + `F` (function) + `P12dAcObjDoor_c` (parameter: **P**ointer to `dAcObjDoor_c`). In C++ that's `dAcObjDoor_c::getNextDoor(dAcObjDoor_c *)`.

On the next build, `dtk` renames that symbol everywhere in the extracted assembly, so the original game asm and your C++ refer to the same name. (The name `getNextDoor` is our invention. Nintendo's real name is lost. Pick something descriptive.)

### Step 4 — Declare it in the header

`include/d/a/obj/d_a_obj_door_base.h`:

```cpp
    virtual void vt_0x8C();

    /// @brief Iterates over all doors. Pass nullptr to get the first door.
    static dAcObjDoor_c *getNextDoor(dAcObjDoor_c *door);
```

It's **`static`** because the assembly calls it with the door as a normal argument in `r3`, and it never uses an implicit `this`. A static member function and a normal member function generate identical machine code here. We could have chosen either. A static method matches how it's used ("iterate starting from X"), and the mangled name in step 3 must match the choice.

We do **not** need to write `getNextDoor`'s body. Its file (`d_a_door_base.cpp`) is still `NonMatching`, so the build keeps linking the original assembly for it. You can decompile a caller before its callees.

### Step 5 — Write the C++

`src/REL/d/t/d_t_shutter_fence_forbiddance.cpp`. Add the include, and translate the pseudocode:

```cpp
#include "d/a/obj/d_a_obj_door_base.h"
...
int dTgShutterFenceForbiddance_c::actorExecute() {
    for (dAcObjDoor_c *door = dAcObjDoor_c::getNextDoor(nullptr); door != nullptr;
         door = dAcObjDoor_c::getNextDoor(door)) {
        door->vt_0x8C();
    }
    return SUCCEEDED;
}
```

Save. objdiff rebuilds and shows **100%** on the first try. It went smoothly because the function is simple and the loop shape in the C++ mirrors the asm exactly. On harder functions this is where the trial-and-error starts (see the "dark arts" table in the long guide).

**What does it do in the game?** Every frame, this invisible trigger tells **every door in the area** to run `vt_0x8C()`. Given the name *shutter fence **forbiddance*** (it has a sibling, `d_t_shutter_fence_permission`), a good guess is that it keeps shutter-fence doors locked while you're inside some zone. That's a hypothesis to confirm later by reading the door classes. Writing hypotheses down in comments is how the code becomes documented over time.

### Step 6 — Check that the *whole file* matches, not just the function

objdiff's report for the unit after the change:

| | Before | After |
|---|---|---|
| Functions | 4 / 5 | **5 / 5** ✅ |
| Code bytes | partial | **380 / 380** ✅ |
| Data bytes | 136 / 136 | 136 / 136 ✅ |
| `.text` section | — | 98.96% ⚠️ |

**Why not 100% for `.text`?** My compiled object contains an extra **weak** function, `dTg_c::~dTg_c()` (the parent class's destructor). The compiler emits a copy of such inline functions into *every* file that needs them and marks them "weak". The linker keeps one copy and throws away the rest. The original file had already lost its copy, so the layouts differ inside the object file but **not in the final linked module**. Other finished `d_t_*` files have the same pattern. The real test is the next step.

### Step 7 — Mark it `Matching` and let the hash check decide

In `configure.py`:

```diff
     Rel(
-        NonMatching,
+        Matching,
         "d_t_shutter_fence_forbiddance",
         "REL/d/t/d_t_shutter_fence_forbiddance.cpp",
     ),
```

```sh
python3 configure.py && ninja
```

The build now links **your C++** into `d_t_shutter_fence_forbiddanceNP.rel` instead of the original asm, then hashes everything:

```
[4/8] CHECK config/SOUE01/build.sha1      → 637 × OK
  All: 26.80% matched, 15.74% linked (1732 / 3088 files)    ← was 1731
  Modules: 14.87% matched, 5.84% linked (1238 / 1933 files) ← was 1237
```

**All 637 files still identical to your disc.** This module of Skyward Sword is now built entirely from C++ source.

### The complete change

```
 config/SOUE01/symbols.txt                     | 2 +-   (named the function)
 include/d/a/obj/d_a_obj_door_base.h           | 3 +++  (declared it)
 src/REL/d/t/d_t_shutter_fence_forbiddance.cpp | 6 +++++-  (the decompiled code)
 configure.py                                  | 2 +-   (NonMatching → Matching)
```

Ten lines. That's what a typical small decomp contribution looks like. See `git diff main` in `ss/`.

### The method, in one list

1. **Read the asm** and write rough pseudocode. Spot the patterns: prologue/epilogue, loops, virtual calls.
2. **Resolve the unknowns.** Look up `fn_…` in `symbols.txt` and `splits.txt`, read callees' asm, and match offsets against classes in `include/`.
3. **Name things** in `symbols.txt` (mangled), and **declare** them in headers.
4. **Write the C++** to mirror the asm's shape. Iterate in objdiff until it's 100%.
5. **Check the whole unit.** If every function and the data match, mark it `Matching`.
6. **`ninja`.** The SHA-1 check is the final judge.

### Your turn

Practice on the same kind of target:
- `d_t_shutter_fence_permission`, the sibling trigger. Compare it with this one.
- `d_t_defeat_boss`: 9 of 10 functions, 99.9% overall.
- `d_t_beetle`: 8 of 9 functions.
- `main/d/d_main`: 6 of 7 functions, 99.7%.

The near-100% ones are close because someone got stuck on the last details, so they're harder than they look. They're still very good for reading.

---

## 5. Worked example 2: decompiling a whole actor (`d_t_ship_flood`)

The first example was one 96-byte function. This one is a **complete actor**: 8 unknown functions (about 1.8 KB of code), a class layout nobody had mapped, two other actors to research, matrix and trigonometry math, cutscene events and controller rumble. It's on the branch `ship-flood` (stacked on `shutter-fence-forbiddance`, uncommitted).

**Result: 54.65% → 99.89%.** 38 of 39 functions are byte-identical and all data matches. The last function is at 99.26%, held back by a single register copy. The full game still rebuilds identically (637 × OK). The file stays `NonMatching` until that last instruction is solved. The final part of this section explains why that's a normal outcome.

### What the actor does

`TAG_SHIP_FLOOD` is an invisible box used together with a **moving, tilting stage** (`dAcOstageSink_c`, "stage sink"). Going by the `ship_` names, that's a ship that sinks and tilts; which level exists is an inference from names, not something the code says. When Link walks into the box, the water level rises (or drops). In detail:

- The box **follows the ship**. Every frame it recomputes its position from the ship's position and rotation, so it stays in place on deck even while the ship tilts.
- On entering, it either plays a **cutscene** (an "event") or directly tells the **water actor** the new target height and speed.
- After the cutscene it sets a **scene flag** (a saved "this happened" bit), so the rest of the level can react.
- Optionally it **rumbles the Wii Remote** once, and allows the rumble again if something else changes the water level.
- A second scene flag acts as an **off switch** (state `Stop`).

Everything above was deduced from the assembly. None of it was known before.

### Step 1 — Take inventory

Start with the module's `config/SOUE01/rels/d_t_ship_floodNP/symbols.txt` and objdiff's report. They show what exists and what doesn't:

| Symbol | Size | What we learn later |
|---|---|---|
| `TgShipFlood__init` | 0xF0 | `create()`: reads parameters |
| `fn_441_550`, `fn_441_5A0` | 8 each | `doDelete()`, `draw()`: just `return 1` |
| `TgShipFlood__update` | 0x30 | `actorExecute()`: runs the state machine |
| `executeState_Wait` | 0x270 | the main logic |
| `executeState_Stop` | 0x74 | re-enables itself when the flag clears |
| `fn_441_8E0` | 0x234 | "is Link in my box?" |
| `fn_441_B20` | 0x58 | rumble, once |
| constructor, destructor, class init | — | partial: depend on the class layout |

**How do we know `fn_441_550` is `doDelete`?** From the **vtable**. The original's data section contains the class's virtual function table: a list of function pointers in a fixed order inherited from the base classes. Comparing it slot by slot with a known vtable (e.g. `dTgShipWindow_c`'s) shows `TgShipFlood__init` in the `create` slot, `fn_441_550` in `doDelete`, `fn_441_5A0` in `draw`, and `TgShipFlood__update` in `actorExecute`. The vtable tells you which virtual functions a class overrides.

### Step 2 — Read `create()` and map the class layout

```asm
lwz   r8, 0x4(r3)          # r8  = mParams           (offset 0x4 is documented in f_base.h)
lha   r10, 0xb8(r3)        # r10 = mRotation.x       (0xB8, from d_a_base.h)
extrwi r4, r8, 16, 8       # r4  = (mParams >> 8) & 0xFFFF
extsh r4, r4               #       ...as a signed 16-bit number
xoris r6, r4, 0x8000       # ┐
stw   r6, 0xc(r1)          # │ the classic CodeWarrior int→float conversion:
lfd   f0, 0x8(r1)          # │ build the double 0x43300000_xxxxxxxx on the stack
fsubs f1, f0, f2           # ┘ and subtract the magic constant (rodata 0x43300000_80000000)
extrwi r0, r8, 7, 1        # (mParams >> 24) & 0x7F  → also converted to float
srwi  r0, r8, 31           # mParams >> 31           → 1 bit
stb   r10, 0x1b8(r3)       # field_0x1B8 = rotation.x & 0xFF
stb   r9,  0x1b9(r3)       # field_0x1B9 = (rotation.x >> 8) & 0xFF
sth   r6,  0xb8(r3)        # mRotation.x = 0
stb   r8,  0x1ac(r3)       # field_0x1AC = mParams & 0xFF
stfs  f1,  0x1b4(r3)       # field_0x1B4 = (float)(s16)(mParams >> 8)
stfs  f0,  0x1b0(r3)       # field_0x1B0 = (float)((mParams >> 24) & 0x7F)
stb   r0,  0x1bb(r3)       # field_0x1BB = mParams >> 31
bl    GetInstance__15dAcOstageSink_cFv
addi  r3, r31, 0x188 ; bl link__9fLiNdBa_cFP7fBase_c   # 0x188: a reference to the sinking stage
bl    getStageWaterPtr
addi  r3, r31, 0x194 ; bl link__9fLiNdBa_cFP7fBase_c   # 0x194: a reference to the water
lfs ... 0xc0/0xc4/0xc8 → stfs 0x1a0/0x1a4/0x1a8        # 0x1A0: copy of the start position
lwz r12, 0xfc(r31) ; lwz r12, 0x18(r12) ; bctrl        # mStateMgr (0xFC) .changeState(Wait)
```

**Things to learn from this:**
- **Actor parameters.** Level designers configure each placed actor with a 32-bit `mParams` word and its rotation values, and the code unpacks them with shifts and masks. Bit fields map directly to `extrwi` / `srwi`.
- **The `0x43300000` trick.** PowerPC has no "int to float" instruction, so CodeWarrior builds a special double on the stack. Whenever you see `xoris …, 0x8000` + `lfd` + `fsubs`, the source just had `(f32)someInt`.
- **`link__9fLiNdBa_c`** is the method of an **actor reference** (`dAcRef_c<T>`). It's a safe pointer to another actor that is automatically cleared if that actor is deleted. Two of them, at `0x188` and `0x194`.

Collecting every offset gives the class layout:

```
0x0FC  state manager            (known: every state-machine actor has one here)
0x138  ActorEventRelated        (seen later: scheduleEvent__17ActorEventRelated on r31+0x138)
0x188  ref → dAcOstageSink_c    (the sinking ship)
0x194  ref → dAcOstageWater_c   (the water)
0x1A0  mVec3_c start position
0x1AC  u8   stop flag            0x1B0  f32 water speed     0x1B4  f32 water height
0x1B8  u8   event id             0x1B9  u8  output flag     0x1BA  u8  triggered
0x1BB  u8   rumble enabled       0x1BC  u8  has rumbled
```

### Step 3 — Find a solved sibling

Before decoding the 624-byte `executeState_Wait` by hand, **look for similar code that's already decompiled**. Nintendo's programmers copy-pasted a lot, so the same code often appears in several actors.

`dTgBarrel_c` (`src/REL/d/t/d_t_barrel.cpp`) is a **matched** trigger on the same ship stage. It references the same `dAcOstageSink_c`, has the same `Wait`/`Stop` states, and has a `checkPlayerPos` function. Comparing the assembly:

- The first half of our `executeState_Wait` (`PSMTXTrans` → `ZXYrotM` → `PSMTXMultVecSR` → add position) is **identical** to barrel's "follow the stage" code.
- `fn_441_8E0` uses **the same constants** as barrel's `checkPlayerPos`: `0.5f`, `-0.5f`, `1/256` (angle conversion), and `1e-6f` (bytes `0x358637BD`), with the same `Ey * scale.y` and the same `sin/cos` of the ship's Z rotation. It's the same function.

That turned the two hardest functions into copy-paste.

### Step 4 — Research the other actors involved

Two unknowns remain: **`getStageWaterPtr`**, which lives in a *different* module (the water actor's REL), and **what the water actor's fields at `0x644` and `0x658` mean**, since our trigger writes to them.

Disassembling the water actor (`build/SOUE01/d_a_obj_stage_waterNP/obj/.../d_a_obj_stage_water.o`) answers both:

```asm
getStageWaterPtr:                # just returns a global pointer to the water actor
    lis r3, STAGE_WATER_PTR@ha
    lwz r3, STAGE_WATER_PTR@l(r3)
    blr

dAcOstageWater_c::executeState_Wait:
    lfs f1, 0x644(r3)            # field_0x644
    lfs f0, 0x650(r3)            # field_0x650
    lfs f2, 0xc4(r3)             # mPosition.y
    fadds f0, f1, f0
    fcmpu cr0, f2, f0            # if (mPosition.y != field_0x644 + field_0x650)
    beqlr
    ... changeState(Move)        #     start moving

dAcOstageWater_c::initializeState_Move:
    ... fabs ... fdivs f1, f6, f1   # duration = |distance| / field_0x658
```

So `0x644` = **target height** (relative to a base height at `0x650`), and `0x658` = **speed**. Same pattern as before: `getStageWaterPtr` is the water's singleton accessor, mirroring `dAcOstageSink_c::GetInstance()`.

`fn_441_B20` turned out to be the simplest function of all:

```asm
lbz r0, 0x1bb(r3) ; cmpwi ; beq end      # if (mRumbleEnabled
lbz r0, 0x1bc(r3) ; cmpwi ; bne end      #     && !mHasRumbled) {
lis r3, sRumblePreset3__9dRumble_c@ha    #   dRumble_c::start(sRumblePreset3, 0x11);
li  r4, 0x11 ; bl start__9dRumble_c…
li  r0, 1 ; stb r0, 0x1bc(r31)           #   mHasRumbled = true; }
```

### Step 5 — Write it all down

1. **Name the symbols** (`symbols.txt` of both modules): `create__14dTgShipFlood_cFv`, `doDelete…`, `actorExecute…`, `draw…`, `checkPlayerPos__14dTgShipFlood_cFRC7mVec3_c`, `startRumble__14dTgShipFlood_cFv`, `StateID_Wait__14dTgShipFlood_c`, `StateID_Stop…`, `__vt__14dTgShipFlood_c`, and in the water module `GetInstance__16dAcOstageWater_cFv`.
2. **Header for the trigger** (`include/d/t/d_t_ship_flood.h`): every member at its offset, with `/* 0x1AC */`-style comments. That's the repo convention, and it lets you check the layout at a glance.
3. **Header for the water** (`include/d/a/obj/d_a_obj_stage_water.h`): the fields we understood, with **padding arrays** for the bytes we don't, e.g. `u8 _0x330[0x5EC - 0x330];`. Padding is honest: "something is here, we don't know what yet."
4. **The C++** (`src/REL/d/t/d_t_ship_flood.cpp`): `create()` translated from Step 2, the barrel code adapted, and the flood logic:

```cpp
if (checkPlayerPos(dAcPy_c::GetLink()->mPosition)) {
    dAcOstageWater_c *pWater = mWaterRef.get();
    if (!mTriggered) {
        if (mEventId != 0xFF) {
            ... schedule cutscene mEventId ...
        } else {
            pWater->setTargetHeight(mWaterHeight);
            pWater->setSpeed(mWaterSpeed);
            startRumble();
            mTriggered = true;
        }
        if (EventManager::isInEvent(this, nullptr)) {      // cutscene is playing
            SceneflagManager::sInstance->setFlag(mRoomID, mOutputFlag);
            ... same three lines ...
            mTriggered = true;
        }
    } else {
        ... keep the water at our height ...
    }
} else if (mRumbleEnabled) {
    f32 targetHeight = mWaterRef.get()->getTargetHeight();
    if (targetHeight != mWaterHeight) {
        mHasRumbled = false;    // someone else moved the water: allow rumble again
    }
}
```

The first compile scored **97.26%**, with 36 of 39 functions identical, including the constructor, destructor and class init. Those matched "for free" once the member types and offsets were right, because the compiler generates them from the class definition.

### Step 6 — The dark arts: chasing the last 3%

This is where most decomp time goes. To iterate quickly I used a small script that disassembles both objects and diffs one function, ignoring label names. (objdiff's GUI shows the same thing.) Each fix below was found by **forming a hypothesis from the diff, then testing it**:

**`create()`: 74% → 100%.** Two problems:
- *Store order.* The original writes `0x1B8`, `0x1B9`, the rotation, then `0x1AC`… CodeWarrior keeps stores roughly in source order, so **reordering the statements** to match fixed it.
- *`mRotation.x = 0` loaded a zero from memory* (`lwz r0, "@24621"`) instead of `li r6, 0`. Assigning through the `mAng` class's constructor created a hidden constant. Writing **`mRotation.x.set(0)`** produced the plain `li`.

**`executeState_Wait`: 99.94% → 100%.** One `fcmpu cr0, f1, f0` had its operands in the opposite order. Five rewrites of the same comparison were tested automatically. The winner introduces a local variable:

```cpp
f32 targetHeight = mWaterRef.get()->getTargetHeight();   // ← this local changes
if (targetHeight != mWaterHeight) {                      //   which register holds which value
```

That's the same logic but different code generation. Small, "pointless" locals like this are everywhere in matching decomps.

**`checkPlayerPos`: 91.96% → 99.26%.** Barrel's code compiled differently here, which led to two findings:
- *Constant pooling.* My build loaded all float constants through **one base register** (`addi r31, …rodata.0`), while the original loaded each constant from its own address. That's a compiler optimization called **data pooling**, and the original file evidently had it off for this function. The repo already uses the fix in other files: `#pragma pool_data off` around the function. 92% → 99%+.
- *One register copy* (`fmr f0, f31`), not yet solved. The original keeps the address of the `0.5f` constant in a saved register (`r30`), but never needs a copy. Using a literal `0.5f` for `halfCos` reproduced the saved register but added the copy. Using the `scale` variable removed the copy but lost the saved register. About 40 variants were tried (statement order, `const`, separate variables, ternaries, compound assignment); none produced both behaviours at once.

### Step 7 — Knowing when to stop

A file can only be marked `Matching` when **every** function is byte-identical. So `d_t_ship_flood` stays `NonMatching`, with a comment marking the function:

```cpp
// NONMATCHING: regalloc - the original multiplies by f31 directly where we emit `fmr f0, f31` first.
```

The work is still fully useful:
- The **38 matched functions count toward the project's "matched" progress** (objdiff measures per function). The overall number moved from 26.80% to 26.82%.
- The actor is now **documented and readable**: anyone (or a future PC port) can see exactly how the flooding works.
- The water actor's header gained **real field names**, which helps whoever decompiles it next.
- Someone else can finish the last instruction. The normal routes are **decomp.me** (share the function; others can try, and it has a "permuter" that mutates the code automatically for hours) or asking on the Discord. A pull request with a `NONMATCHING` note is a normal, welcome contribution.

### What made this one tractable

| Technique | Where it paid off |
|---|---|
| Read the **vtable** to identify virtual functions | Named 4 functions without reading their code |
| Map every **offset** → write the header first | Constructor, destructor and class init matched for free |
| **Find a solved sibling** | Two hardest functions became copy-paste |
| Read the **other actors'** code | Discovered what the water fields mean |
| **Automate** the diff + variant testing | Dozens of experiments in minutes |
| Know the **compiler quirks** (int→float trick, pooling, store order) | Every fix in Step 6 |
| Know when to **stop and document** | One instruction shouldn't block 38 functions of progress |

### Your turn

`d_t_ship_slope` is a near-twin of this file. It has the same structure, the same unknown functions (`executeState_Wait` 728 bytes, an identical-looking 564-byte box check), and the same `NONMATCHING` mystery to solve. With this walkthrough you can probably take it from ~52% to ~99% in an evening, and if you crack the `fmr` puzzle there, the fix will likely apply to both files.

---

## 6. Worked example 3: the fire test, code nobody has ever touched (`d_snd_stage_reverb`)

The first two examples had help: a header that already existed, a `TODO` pointing the way, a matched sibling to copy from. This one has **none of that**. The challenge was: *pick code that is completely new, with nothing that looks like it and nothing to base the work on.*

**Result: 0% → 100%, linked.** Six functions decompiled from nothing, the file marked `Matching`, the whole game still byte-identical (637 × OK). Along the way the code turned out to be **how Skyward Sword picks the echo (reverb) for each area**. Branch `stage-reverb`, stacked on `ship-flood`, uncommitted.

### Step 0 — Choosing a truly untouched target

"Low percentage" isn't the same as untouched. The stub actors from earlier sit at 10–20% only because the auto-generated class registration matches; their real logic is at 0%. What I looked for was stricter:

1. **No `.cpp` file at all.** objdiff reports these with no compiled code (`code None / 408`), so the build links 100% original assembly.
2. **Game code, not library code.** 185 units have no source, but most are Nintendo SDK, Bluetooth or debugger libraries (`revolution/…`, `TRK_…`, `nw4r/…`), and other projects have decompiled those for other games. They "look like" existing work, so they're excluded.
3. **No header, no references, no family.** For each remaining candidate I grepped the whole repo for its name.

`d/snd/d_snd_stage_reverb.cpp` passed every filter: no source, no header, **zero mentions anywhere**, and its six functions had no names, just addresses (`fn_80366430`…`fn_803665B0`). Even the file name was someone's guess.

### Step 1 — Look at the shape before the details

Don't start reading line 1 of function 1. First look at the **whole file** and its **data**:

```asm
fn_80366430:                         # five functions that all look like this…
    lfs   f5, lbl_8057D450@sda21(r0) #   load constants
    li    r4, 0x7                    #   load small integers
    ...
    stfs  f5, 0x0(r3)                #   store them into a struct passed in r3
    stfs  f4, 0x4(r3)
    ...                              #   offsets 0x0 to 0x24 → a 0x28-byte struct
    stw   r4, 0x14(r3)               #   two int fields (0x14 and 0x1C), the rest float
    blr

fn_803665B0:                         # …and one that picks between them
    cmpwi r4, 0x1 ; beq → b fn_80366480
    cmpwi r4, 0x2 ; beq → b fn_803664D0
    cmpwi r4, 0x3 ; beq → b fn_80366520
    cmpwi r4, 0x4 ; beq → b fn_80366560
    b fn_80366430                    # anything else
```

Decoding the constants (hex → float): `0x3CA3D70A` = 0.02, `0x40400000` = 3.0, `0x3F19999A` = 0.6, `0x3ECCCCCD` = 0.4, `0x3F800000` = 1.0, `0x00000000` = 0.0, `0x3DCCCCCD` = 0.1, `0x40A00000` = 5.0, `0x3D4CCCCD` = 0.05, `0x3F000000` = 0.5. *(Tip: `python3 -c "import struct;print(struct.unpack('>f',bytes.fromhex('3CA3D70A')))"`)*

Without understanding anything yet, the shape alone says: **five "fill in a preset" functions, and a `switch` that chooses one by number.** Two details are worth noticing:
- `b` instead of `bl` in the switch is a **tail call**. The function ends by jumping into another function instead of calling it and returning. That's what the compiler does when a call is the very last thing a function does.
- Preset 0 is used for 0 **and every unknown value**: that's a `default:` case.

### Step 2 — Identify the type from its shape

A struct of exactly `f32 ×5, int, f32, int, f32, f32` is very specific. Search the headers for it. The file name said "reverb", and the sound headers include `nw4r::snd::FxReverbStdDpl2` (Nintendo's standard reverb effect). Its parameter struct:

```cpp
struct FxReverbStdParam {
    f32 field_0x00; f32 field_0x04; f32 field_0x08; f32 field_0x0C; f32 field_0x10;
    int field_0x14; f32 field_0x18; int field_0x1C; f32 field_0x20; f32 field_0x24;
};
```

**A perfect match.** So these functions fill in reverb settings. The fields are still unnamed in the repo. Reading the library's `SetParam` (where each field ends up) proves two of them:
- `field_0x18` is compared with its previous value and can trigger a **reallocation of the work buffer**, so it's the **maximum pre-delay time** (the delay buffer is sized from it).
- `field_0x00` is **clamped to `[0, field_0x18]`**, so it's the **pre-delay time** itself.

I searched online for official names for the rest and found nothing reliable, so **they stay unnamed**. A wrong name is worse than no name, because people trust names. Matching doesn't need names at all.

### Step 3 — Follow the callers: who uses this, and with what?

Nothing in the source referred to these functions by name, so I disassembled **all 1155 objects of the main executable** and grepped for the function names:

```sh
find build/SOUE01/obj -name '*.o' | xargs -P 16 -I{} sh -c 'build/tools/dtk elf disasm {} out/$(echo {} | tr / _).s'
grep -l fn_803665B0 out/*.s      # → only d_snd_state_mgr
```

Two callers, both in `dSndStateMgr_c` (the sound state manager):

1. **`setup()`**, already decompiled, with a placeholder: `extern "C" void fn_803665B0(FxReverbStdParam &, int);` That confirms the signature (struct reference + int). The loop is interesting:
   ```cpp
   for (int i = 0; i < 5; i++) {        // try every preset…
       getStageReverbParam(param, i);
       mFx.SetParam(param);
       max = MAX(max, mFx.GetRequiredMemSize());   // …to find the biggest buffer needed
   }
   pHeap->alloc(max, 4);                // allocate once, at boot
   ```
   So the game allocates reverb memory **once**, big enough for any preset. That avoids allocating memory when changing areas.
2. **`restoreEffects()`**, still assembly, passes the state manager's field `0x48C`. Following *who writes `0x48C`* leads to `onStageOrLayerUpdate()`, which runs on every area load:
   ```
   environment type (field 0x3C):  1,4 → preset 1    6 → preset 2    3,5 → preset 3    2 → preset 4    else → preset 0
   ```
   And `0x3C` comes from `fn_80366090(soundStageId)`, a function mapping **ranges of sound stage IDs** to environment types, or is forced to 6 when `dScGame_c::currentSpawnInfo.trial` is set.

### Step 4 — Turn numbers into meaning

The sound stage IDs are an enum that already exists (`d_snd_stage_data.h`, `SND_STAGE_F100`…). Grouping the enum by the ID ranges in `fn_80366090`:

| Preset | Used for (stage names) | Reading of the names | `field_0x04` |
|---|---|---|---|
| 0 | `F100`…`F406` | outdoor surface areas | 3.0 |
| 1 | `D000`…`D301`, `B100`…`B400`, some caves | dungeons and boss arenas | 3.0 |
| 2 | `S000`, `S100`, `S200`, `S300` + `trial` flag | **Silent Realms** | **5.0** |
| 3 | `F001r`, `F002r`… (the `r` suffix) | small **interior rooms** | **0.5** |
| 4 | `F000`, `F020`, `F021`, `F023` | Skyloft and the Sky | 3.0 |

The stage-name letters (F = field, D = dungeon, B = boss, S = Silent Realm, `r` = room) follow the naming visible in the enum itself. `field_0x04` is clamped to ≥ 0 by the library and is **0.5 in small rooms, 5.0 in the Silent Realms, 3.0 elsewhere**, which is exactly how you'd tune a reverb's **tail length**. Strong evidence, but not proof, so it went into a comment ("appears to be"), not into a field name.

One more discovery: the library's default constructor for `FxReverbStdParam` holds almost exactly **preset 0's values**, so the outdoor preset is essentially "Nintendo's SDK defaults".

### Step 5 — Write it

- **New header** `include/d/snd/d_snd_stage_reverb.h`: an enum for the five presets (named after where they're used) and the six function declarations.
- **New source** `src/d/snd/d_snd_stage_reverb.cpp`: five preset functions assigning the ten fields, plus the `switch`.
- **Names in `symbols.txt`.** This time I **compiled first, then copied the mangled names out of my own object file** (`dtk elf disasm` lists them), e.g. `getStageReverbParam__FRQ44nw4r3snd6detail16FxReverbStdParaml`. No hand-mangling, no typos.
- **The caller.** Replaced the `extern "C" fn_803665B0` placeholder in `d_snd_state_mgr.cpp` with the real header, then checked that `setup()` there **still matches 100%**. When you touch someone else's file, verify you didn't break it.

**First compile: 0% → 78.53%.** All 10 constants landed in the right order (data 100%), meaning the assignments came first in the same order Nintendo's did.

### Step 6 — The last 21%

Only two kinds of differences, both about **order**:

**Store order in the presets.** Same instructions, but the original writes the fields as `0x00, 0x04, 0x08, 0x0C, 0x10, 0x20, 0x24, 0x14, 0x1C, 0x18`, and mine in struct order. I tested four candidate orders automatically on preset 0:

```
0 diff lines :: asm store order        ← winner
2 diff lines :: floats then ints
4 diff lines :: 0x18 last
6 diff lines :: ints first
```

Writing the assignments in exactly the order the asm stores them matched all five presets. (CodeWarrior mostly keeps stores in source order, so the asm order *is* the source order. Nintendo's programmer just didn't write them in struct order.)

**Block order in the `switch`.** The original places the `default` call right after the comparisons, and mine placed it last. CodeWarrior emits case bodies **in source order**, so the original source had **`default:` written first**:

```cpp
switch (type) {
    default:                            setStageReverbParamField(param); break;
    case SND_STAGE_REVERB_DUNGEON:      setStageReverbParamDungeon(param); break;
    ...
```

Unusual style, but that's what the compiler output says. **6/6 functions at 100%.**

### Step 7 — Link it

`configure.py`: `Object(NonMatching, …)` → `Object(Matching, "d/snd/d_snd_stage_reverb.cpp")`, then `ninja`:

```
CHECK config/SOUE01/build.sha1  → 637 × OK
DOL: 45.30% matched, 31.10% linked (495 / 1155 files)   ← was 494
```

The main executable now builds this file from C++.

### How to approach code with nothing to base it on

This is the general method, in the order I used it:

1. **Shape first.** Look at all functions and the data before reading any one in detail. Count them, compare their sizes, spot repetition. "Five similar functions + one switch" was obvious before reading a single field.
2. **Decode the data.** Floats in hex, strings, tables. Constants are often the most meaningful thing in the file (0.5 vs 5.0 told the story here).
3. **Identify types by their layout.** An offset pattern like `f32×5, int, f32, int, f32×2` is a fingerprint. Search the headers for it.
4. **Follow the callers, then their data.** Disassemble everything and grep. The callers explain *when* and *why*. Following the data (`0x48C` → `0x3C` → `fn_80366090` → stage enum) explains *what for*.
5. **Separate proof from guesses.** Name only what the code proves (pre-delay fields). Put strong hints in comments (`appears to be`). Leave the rest unnamed.
6. **Write the simplest C++ that does the same thing, then let the compiler tell you the rest.** First compile: 78%. The diff then showed only ordering, and each fix was one hypothesis plus one test.
7. **Check what you touched.** Re-verify any other file you changed (`setup()` stayed at 100%), and let the final hash decide.

### Your turn

The function that maps stage IDs to environment types, **`fn_80366090`** (136 bytes, in `d_snd_state_mgr`), is the natural next step. You already know what it does from Step 3. It's a chain of range comparisons returning 0–6, perfect for practising `switch` and `if/else` layout. Name it, write it, and make it match.

---

## 7. When you match something

1. objdiff shows **100%** for the function. 🎉
2. If **every** function in the unit is at 100%, change `NonMatching` to `Matching` for that file in `configure.py`.
3. Run `ninja`. The SHA-1 check must still say OK. If it fails, something in the file (often data, not code) still differs. Change it back to `NonMatching` and look again.
4. Format with `clang-format` (the repo has a `.clang-format`), commit on a branch, and open a PR to `zeldaret/ss`. Your system clang-format currently rejects the repo's config (`BinPackArguments: True`, a version mismatch), so format by hand (120 columns, 4-space indent) or install the version the project's CI uses.
5. Before your first PR, say hi on the zeldaret Discord (linked in the README) and ask which files are free, so you don't duplicate someone's work.

---

## 8. Troubleshooting

| Symptom | Cause / fix |
|---|---|
| `Hash mismatch` during `SPLIT` | Wrong disc files in `orig/SOUE01/`. Re-extract from `SOUE01.wbfs` (see below). |
| `CHECK` fails at the end of `ninja` | A file marked `Matching` isn't actually identical. Switch it back to `NonMatching`. |
| objdiff shows nothing / no units | Project directory not set, or `ninja` hasn't run yet (there's no `objdiff.json`). |
| Want a clean slate | `rm -rf build && python3 configure.py && ninja` (it re-downloads the tools). |

**Re-extracting the game files** (this is what I ran; no Dolphin needed):

```sh
cd <workdir>/ss
build/tools/dtk vfs cp ../SOUE01.wbfs:sys/main.dol   orig/SOUE01/sys/main.dol
build/tools/dtk vfs cp ../SOUE01.wbfs:files/rels.arc orig/SOUE01/files/rels.arc
sha1sum orig/SOUE01/sys/main.dol   # must be 450a6806f46d59dcf8278db08e06f94865a4b18a
```

---

## 9. Where things are

```
DecompsTest/
├── SOUE01.wbfs                  your disc backup (rev 0) — keep it, never commit it
├── game-decompilation-guide.md  the long-form background guide
├── QUICKSTART.md                this file
└── ss/
    ├── configure.py             list of all units + Matching/NonMatching status
    ├── config/SOUE01/           splits.txt, symbols.txt, build.sha1, rels/
    ├── src/                     the C++ you edit
    │   ├── d/                   game code in main.dol
    │   └── REL/d/{a,t,lyt}/     actors, triggers, UI in RELs
    ├── include/                 headers: class layouts live here
    ├── orig/SOUE01/             files extracted from your disc (git-ignored)
    ├── build/SOUE01/            compiled output + report.json
    ├── build/tools/             dtk, wibo, objdiff-cli (auto-downloaded)
    └── tools/ghidra_scripts/    sync names with Ghidra, if you use it
```
