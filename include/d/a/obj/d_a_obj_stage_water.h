#ifndef D_A_OBJ_STAGE_WATER_H
#define D_A_OBJ_STAGE_WATER_H

#include "d/a/obj/d_a_obj_base.h"
#include "s/s_State.hpp"

class dAcOstageWater_c : public dAcObjBase_c {
public:
    dAcOstageWater_c() : mStateMgr(*this) {}
    virtual ~dAcOstageWater_c() {}

    STATE_FUNC_DECLARE(dAcOstageWater_c, Wait);
    STATE_FUNC_DECLARE(dAcOstageWater_c, Move);

    static dAcOstageWater_c *GetInstance();

    void setTargetHeight(f32 height) {
        mTargetHeight = height;
    }
    void setSpeed(f32 speed) {
        mSpeed = speed;
    }
    f32 getTargetHeight() const {
        return mTargetHeight;
    }

private:
    /* 0x330 */ u8 _0x330[0x5EC - 0x330];
    /* 0x5EC */ STATE_MGR_DECLARE(dAcOstageWater_c);
    /* 0x628 */ u8 _0x628[0x644 - 0x628];
    /* 0x644 */ f32 mTargetHeight; ///< Water level to move to, relative to mBaseHeight
    /* 0x648 */ u8 _0x648[0x650 - 0x648];
    /* 0x650 */ f32 mBaseHeight;
    /* 0x654 */ u8 _0x654[0x658 - 0x654];
    /* 0x658 */ f32 mSpeed; ///< How fast the water level moves
    /* 0x65C */ u8 _0x65C[0x660 - 0x65C];
};

#endif
