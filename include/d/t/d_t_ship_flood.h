#ifndef D_T_SHIP_FLOOD_H
#define D_T_SHIP_FLOOD_H

#include "d/a/obj/d_a_obj_stage_sink.h"
#include "d/a/obj/d_a_obj_stage_water.h"
#include "d/t/d_tg.h"
#include "m/m_vec.h"
#include "s/s_State.hpp"
#include "toBeSorted/actor_event.h"

/// @brief Raises (or lowers) the water level of the sinking ship stage when Link enters the area.
/// The area is a box of size mScale that follows the stage's movement and tilt.
class dTgShipFlood_c : public dTg_c {
public:
    dTgShipFlood_c() : mStateMgr(*this), mEvent(*this, nullptr) {}
    virtual ~dTgShipFlood_c() {}
    virtual int create() override;
    virtual int doDelete() override;
    virtual int actorExecute() override;
    virtual int draw() override;

    STATE_FUNC_DECLARE(dTgShipFlood_c, Wait);
    STATE_FUNC_DECLARE(dTgShipFlood_c, Stop);

    bool checkPlayerPos(const mVec3_c &playerPos);
    void startRumble();

private:
    /* 0x0FC */ STATE_MGR_DECLARE(dTgShipFlood_c);
    /* 0x138 */ ActorEventRelated mEvent;
    /* 0x188 */ dAcRef_c<dAcOstageSink_c> mStageRef;
    /* 0x194 */ dAcRef_c<dAcOstageWater_c> mWaterRef;
    /* 0x1A0 */ mVec3_c mSpawnPosition;
    /* 0x1AC */ u8 mStopFlag;       ///< Sceneflag that disables the trigger while set
    /* 0x1B0 */ f32 mWaterSpeed;    ///< Speed the water level moves at
    /* 0x1B4 */ f32 mWaterHeight;   ///< Water level to move to
    /* 0x1B8 */ u8 mEventId;        ///< Event to play when triggered (0xFF = none)
    /* 0x1B9 */ u8 mOutputFlag;     ///< Sceneflag set after the event
    /* 0x1BA */ u8 mTriggered;      ///< The flood has been started
    /* 0x1BB */ u8 mRumbleEnabled;  ///< Rumble the controller when the flood starts
    /* 0x1BC */ u8 mHasRumbled;     ///< Rumble already played for the current water level
};

#endif
