# Findings

Technical results of the three decompilation exercises, written down so they don't get lost. The code lives on the branches `shutter-fence-forbiddance` → `ship-flood` → `stage-reverb` (stacked, in that order). See `QUICKSTART.md` sections 4–6 for the step-by-step process.

All results are against `SOUE01` revision 0. Every commit was built and checked: `ninja` passes, and `build.sha1` reports 637 × OK.

## Status

| Branch | Unit | Before | After | Linked? |
|---|---|---|---|---|
| `shutter-fence-forbiddance` | `REL/d/t/d_t_shutter_fence_forbiddance` | 4/5 functions | 5/5 (100%) | ✅ `Matching` |
| `ship-flood` | `REL/d/t/d_t_ship_flood` | 54.65% | 99.89% (38/39 functions) | ❌ one function `NONMATCHING` |
| `stage-reverb` | `d/snd/d_snd_stage_reverb` | 0% (no source) | 100% (6/6) | ✅ `Matching` |

## Newly named symbols

| Address / symbol | New name | Evidence |
|---|---|---|
| `fn_80179250` (main, `d_a_door_base`) | `dAcObjDoor_c::getNextDoor(dAcObjDoor_c *)` | returns `node->p_owner` of `door ? door->mDoorRef.next : sDoorList.first` |
| `getStageWaterPtr` (REL `d_a_obj_stage_water`) | `dAcOstageWater_c::GetInstance()` | returns a global pointer, mirrors `dAcOstageSink_c::GetInstance()` |
| `TgShipFlood__init`, `fn_441_550`, `TgShipFlood__update`, `fn_441_5A0` | `create`, `doDelete`, `actorExecute`, `draw` | vtable slot positions |
| `fn_441_8E0` | `dTgShipFlood_c::checkPlayerPos(const mVec3_c &)` | same code and constants as `dTgBarrel_c::checkPlayerPos` |
| `fn_441_B20` | `dTgShipFlood_c::startRumble()` | `dRumble_c::start(sRumblePreset3, 0x11)` guarded by two flags |
| `fn_80366430` … `fn_80366560` | `setStageReverbParam{Field,Dungeon,SilentRealm,Room,Sky}` | fill an `FxReverbStdParam` |
| `fn_803665B0` | `getStageReverbParam(FxReverbStdParam &, s32)` | `switch` over the five presets, `default` = Field |

## Class layouts discovered

### `dTgShipFlood_c` (`TAG_SHIP_FLOOD`)

| Offset | Type | Name | Source |
|---|---|---|---|
| 0x0FC | state manager | `mStateMgr` | |
| 0x138 | `ActorEventRelated` | `mEvent` | |
| 0x188 | `dAcRef_c<dAcOstageSink_c>` | `mStageRef` | |
| 0x194 | `dAcRef_c<dAcOstageWater_c>` | `mWaterRef` | |
| 0x1A0 | `mVec3_c` | `mSpawnPosition` | |
| 0x1AC | `u8` | `mStopFlag` | params bits 0–7 |
| 0x1B0 | `f32` | `mWaterSpeed` | params bits 24–30 |
| 0x1B4 | `f32` | `mWaterHeight` | params bits 8–23 (signed) |
| 0x1B8 | `u8` | `mEventId` | rotation.x low byte (0xFF = none) |
| 0x1B9 | `u8` | `mOutputFlag` | rotation.x high byte |
| 0x1BA | `u8` | `mTriggered` | |
| 0x1BB | `u8` | `mRumbleEnabled` | params bit 31 |
| 0x1BC | `u8` | `mHasRumbled` | |

Behaviour: the trigger box follows the stage sink's position and rotation every frame. When the player enters it, it either schedules event `mEventId` (and, once the event runs, sets scene flag `mOutputFlag`) or directly sets the water's target height and speed. It rumbles once per water-level change. Scene flag `mStopFlag` disables it (state `Stop`).

### `dAcOstageWater_c` (partial)

| Offset | Type | Meaning | Evidence |
|---|---|---|---|
| 0x5EC | state manager | | `lwzu r12, 0x5ec(r3)` before `changeState` |
| 0x644 | `f32` | target height (relative to 0x650) | `Wait`: moves when `pos.y != [0x644] + [0x650]` |
| 0x650 | `f32` | base height | same |
| 0x658 | `f32` | speed | `initializeState_Move`: duration = abs(distance) / [0x658] |
| size 0x660 | | | `li r3, 0x660` in class init |

## Stage reverb presets

`dSndStateMgr_c::onStageOrLayerUpdate()` maps the sound stage ID (enum in `d_snd_stage_data.h`) to an environment type via `fn_80366090`, or forces type 6 when `dScGame_c::currentSpawnInfo.trial` is set. The type then picks a preset:

| Preset | Env. types | Stages | `0x00` | `0x04` | `0x08` | `0x0C` | `0x10` | `0x14` | `0x18` | `0x1C` | `0x20` | `0x24` |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 0 Field (default) | 0 | `F1xx`–`F4xx` | 0.02 | 3.0 | 0.6 | 0.4 | 1.0 | 7 | 0.02 | 4 | 0.0 | 1.0 |
| 1 Dungeon | 1, 4 | `Dxxx`, `Bxxx`, caves, `F402`… | 0.1 | 3.0 | 0.6 | 0.4 | 1.0 | 5 | 0.1 | 5 | 0.0 | 1.0 |
| 2 Silent Realm | 6, trial | `S000`–`S300` | 0.1 | 5.0 | 0.6 | 1.0 | 1.0 | 0 | 0.1 | 5 | 1.0 | 1.0 |
| 3 Room | 3, 5 | `F0xxr` | 0.05 | 0.5 | 0.5 | 0.5 | 1.0 | 1 | 0.05 | 2 | 0.5 | 1.0 |
| 4 Sky | 2 | `F000`, `F02x` | 0.02 | 3.0 | 0.6 | 0.4 | 1.0 | 0 | 0.02 | 4 | 0.0 | 1.0 |

What the fields mean:
- **Proven** from `FxReverbStdDpl2::SetParam`: `0x18` is the maximum pre-delay time (a change triggers reallocation), and `0x00` is the pre-delay time (clamped to `[0, 0x18]`).
- **Likely, not proven**: `0x04` is the reverb tail time. It is clamped ≥ 0, is 0.5 in rooms and 5.0 in the Silent Realms.
- The other fields are unnamed. No reliable public source was found.
- Preset 0 is almost identical to `FxReverbStdParam`'s default constructor.
- `dSndStateMgr_c::setup()` evaluates all 5 presets once to allocate the largest required work buffer.

## Compiler quirks we hit (CodeWarrior, Wii/1.5)

| Symptom in the diff | Cause | Fix |
|---|---|---|
| Stores in a different order | CodeWarrior mostly keeps assignment order | Write assignments in the asm's store order |
| `mRotation.x = 0` loads a zero from `.sdata` (`lwz r0, "@…"`) | Implicit `mAng` construction creates a constant | `mRotation.x.set(0)` |
| `fcmpu` operands swapped | Expression shape | Copy one side into a local first (`f32 targetHeight = …; if (targetHeight != …)`) |
| Float constants accessed through one base register (`addi rX, …rodata.0@l`) | Data pooling on; the original had it off | `#pragma pool_data off` / `on` around the function |
| Default case emitted first in a `switch` | Case bodies are laid out in source order | Write `default:` first |
| Object `.text` < 100% with all functions at 100% | Weak inline copies or alignment gaps in the object | Ignore if the final `build.sha1` check passes |

## Open problems

- **`dTgShipFlood_c::checkPlayerPos`** (99.26%): the original keeps the `0.5f` constant's address in `r30` and multiplies `f31 * cos` directly. Using a literal for `halfCos` reproduces the saved registers but adds `fmr f0, f31`. Using the variable removes the copy but loses `r30`. About 40 variants were tried. Next steps: decomp.me plus the permuter, or ask on the zeldaret Discord.
- **`d_t_ship_slope`** is a near-twin of `d_t_ship_flood`. It's probably the same work, with the same open problem.
- **`fn_80366090`** (stage ID → environment type, 136 bytes in `d_snd_state_mgr`) is understood (see above) but not yet written in C++.

## Environment notes

- Only `SOUE01` **revision 0** works. A revision 1 disc fails with "Hash mismatch" during `SPLIT`.
- `build/tools/dtk vfs cp "<disc>:sys/main.dol" …` extracts files from ISO/WBFS without Dolphin.
- The repo's `.clang-format` (`BinPackArguments: True`) is rejected by newer clang-format versions. Format by hand or use the CI's version.
