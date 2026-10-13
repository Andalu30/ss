#include "d/snd/d_snd_stage_reverb.h"

#include "nw4r/snd/snd_FxReverbStdDpl2.h"

// field_0x00: pre-delay time (clamped to field_0x18 by FxReverbStdDpl2::SetParam)
// field_0x04: appears to be the reverb tail length (0.5 in rooms, 5.0 in Silent Realms)
// field_0x18: maximum pre-delay time (determines the size of the work buffer)

void setStageReverbParamField(nw4r::snd::detail::FxReverbStdParam &param) {
    param.field_0x00 = 0.02f;
    param.field_0x04 = 3.0f;
    param.field_0x08 = 0.6f;
    param.field_0x0C = 0.4f;
    param.field_0x10 = 1.0f;
    param.field_0x20 = 0.0f;
    param.field_0x24 = 1.0f;
    param.field_0x14 = 7;
    param.field_0x1C = 4;
    param.field_0x18 = 0.02f;
}

void setStageReverbParamDungeon(nw4r::snd::detail::FxReverbStdParam &param) {
    param.field_0x00 = 0.1f;
    param.field_0x04 = 3.0f;
    param.field_0x08 = 0.6f;
    param.field_0x0C = 0.4f;
    param.field_0x10 = 1.0f;
    param.field_0x20 = 0.0f;
    param.field_0x24 = 1.0f;
    param.field_0x14 = 5;
    param.field_0x1C = 5;
    param.field_0x18 = 0.1f;
}

void setStageReverbParamSilentRealm(nw4r::snd::detail::FxReverbStdParam &param) {
    param.field_0x00 = 0.1f;
    param.field_0x04 = 5.0f;
    param.field_0x08 = 0.6f;
    param.field_0x0C = 1.0f;
    param.field_0x10 = 1.0f;
    param.field_0x20 = 1.0f;
    param.field_0x24 = 1.0f;
    param.field_0x14 = 0;
    param.field_0x1C = 5;
    param.field_0x18 = 0.1f;
}

void setStageReverbParamRoom(nw4r::snd::detail::FxReverbStdParam &param) {
    param.field_0x00 = 0.05f;
    param.field_0x04 = 0.5f;
    param.field_0x08 = 0.5f;
    param.field_0x0C = 0.5f;
    param.field_0x10 = 1.0f;
    param.field_0x20 = 0.5f;
    param.field_0x24 = 1.0f;
    param.field_0x14 = 1;
    param.field_0x1C = 2;
    param.field_0x18 = 0.05f;
}

void setStageReverbParamSky(nw4r::snd::detail::FxReverbStdParam &param) {
    param.field_0x00 = 0.02f;
    param.field_0x04 = 3.0f;
    param.field_0x08 = 0.6f;
    param.field_0x0C = 0.4f;
    param.field_0x10 = 1.0f;
    param.field_0x20 = 0.0f;
    param.field_0x24 = 1.0f;
    param.field_0x14 = 0;
    param.field_0x1C = 4;
    param.field_0x18 = 0.02f;
}

void getStageReverbParam(nw4r::snd::detail::FxReverbStdParam &param, s32 type) {
    switch (type) {
        default:                            setStageReverbParamField(param); break;
        case SND_STAGE_REVERB_DUNGEON:      setStageReverbParamDungeon(param); break;
        case SND_STAGE_REVERB_SILENT_REALM: setStageReverbParamSilentRealm(param); break;
        case SND_STAGE_REVERB_ROOM:         setStageReverbParamRoom(param); break;
        case SND_STAGE_REVERB_SKY:          setStageReverbParamSky(param); break;
    }
}
