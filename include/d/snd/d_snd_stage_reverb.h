#ifndef D_SND_STAGE_REVERB_H
#define D_SND_STAGE_REVERB_H

#include "nw4r/snd/snd_FxReverbStdDpl2.h"

// Reverb presets for the different kinds of stages.
// Selected by dSndStateMgr_c based on the sound stage id (see d_snd_stage_data.h).
// The preset names are based on which stages use them.
enum dSndStageReverbType_e {
    SND_STAGE_REVERB_FIELD = 0,        ///< Outdoor surface areas (F1xx - F4xx)
    SND_STAGE_REVERB_DUNGEON = 1,      ///< Dungeons, boss rooms and caves (Dxxx, Bxxx)
    SND_STAGE_REVERB_SILENT_REALM = 2, ///< Silent Realms (Sxxx) and trials
    SND_STAGE_REVERB_ROOM = 3,         ///< Small interior rooms (F0xxr)
    SND_STAGE_REVERB_SKY = 4,          ///< Skyloft and the Sky (F000, F02x)
};

void setStageReverbParamField(nw4r::snd::detail::FxReverbStdParam &param);
void setStageReverbParamDungeon(nw4r::snd::detail::FxReverbStdParam &param);
void setStageReverbParamSilentRealm(nw4r::snd::detail::FxReverbStdParam &param);
void setStageReverbParamRoom(nw4r::snd::detail::FxReverbStdParam &param);
void setStageReverbParamSky(nw4r::snd::detail::FxReverbStdParam &param);

void getStageReverbParam(nw4r::snd::detail::FxReverbStdParam &param, s32 type);

#endif
