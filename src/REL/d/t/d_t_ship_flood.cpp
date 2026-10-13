#include "d/t/d_t_ship_flood.h"

#include "common.h"
#include "d/a/d_a_player.h"
#include "d/a/obj/d_a_obj_stage_sink.h"
#include "d/a/obj/d_a_obj_stage_water.h"
#include "d/d_rumble.h"
#include "d/flag/sceneflag_manager.h"
#include "f/f_base.h"
#include "m/m_angle.h"
#include "m/m_mtx.h"
#include "m/m_vec.h"
#include "toBeSorted/event.h"
#include "toBeSorted/event_manager.h"

SPECIAL_ACTOR_PROFILE(TAG_SHIP_FLOOD, dTgShipFlood_c, fProfile::TAG_SHIP_FLOOD, 0x220, 0, 0);

STATE_DEFINE(dTgShipFlood_c, Wait);
STATE_DEFINE(dTgShipFlood_c, Stop);

int dTgShipFlood_c::create() {
    mEventId = mRotation.x & 0xFF;
    mOutputFlag = (mRotation.x >> 8) & 0xFF;
    mRotation.x.set(0);
    mStopFlag = getFromParams(0, 0xFF);
    mWaterHeight = (s16)getFromParams(8, 0xFFFF);
    mWaterSpeed = (s32)getFromParams(24, 0x7F);
    mRumbleEnabled = getFromParams(31, 0x1);

    mStageRef.link(dAcOstageSink_c::GetInstance());
    mWaterRef.link(dAcOstageWater_c::GetInstance());
    mSpawnPosition = mPosition;

    mStateMgr.changeState(StateID_Wait);
    return SUCCEEDED;
}

int dTgShipFlood_c::doDelete() {
    return SUCCEEDED;
}

int dTgShipFlood_c::actorExecute() {
    mStateMgr.executeState();
    return SUCCEEDED;
}

int dTgShipFlood_c::draw() {
    return SUCCEEDED;
}

void dTgShipFlood_c::initializeState_Wait() {}
void dTgShipFlood_c::executeState_Wait() {
    if (SceneflagManager::sInstance->checkBoolFlag(mRoomID, mStopFlag)) {
        mStateMgr.changeState(StateID_Stop);
        return;
    }

    // Follow the movement of the sinking stage
    dAcOstageSink_c *pStage = mStageRef.get();

    mVec3_c pos = pStage->mPosition;
    mAng3_c rot = pStage->mRotation;

    mVec3_c stageDist = mSpawnPosition - pos;
    mMtx_c m;
    m.transS(pos);
    m.ZXYrotM(rot);
    m.multVecSR(stageDist, mPosition);

    mPosition += pos;

    if (checkPlayerPos(dAcPy_c::GetLink()->mPosition)) {
        dAcOstageWater_c *pWater = mWaterRef.get();
        if (!mTriggered) {
            if (mEventId != 0xFF) {
                // Same trick as in d_t_ship_window
                static volatile u32 FLAGS_1 = 0x00000001;
                static u32 FLAGS_2 = 0x00100001;
                u32 f1 = FLAGS_1;
                u32 f2 = FLAGS_2;
                Event e = Event(mEventId, mRoomID, f2 & ~f1, nullptr, nullptr);
                mEvent.scheduleEvent(e, 0);
            } else {
                pWater->setTargetHeight(mWaterHeight);
                pWater->setSpeed(mWaterSpeed);
                startRumble();
                mTriggered = true;
            }

            if (EventManager::isInEvent(this, nullptr)) {
                SceneflagManager::sInstance->setFlag(mRoomID, mOutputFlag);
                pWater->setTargetHeight(mWaterHeight);
                pWater->setSpeed(mWaterSpeed);
                startRumble();
                mTriggered = true;
            }
        } else {
            pWater->setTargetHeight(mWaterHeight);
            pWater->setSpeed(mWaterSpeed);
            startRumble();
        }
    } else if (mRumbleEnabled) {
        // Someone else changed the water level - allow rumbling again
        f32 targetHeight = mWaterRef.get()->getTargetHeight();
        if (targetHeight != mWaterHeight) {
            mHasRumbled = false;
        }
    }
}
void dTgShipFlood_c::finalizeState_Wait() {}

void dTgShipFlood_c::initializeState_Stop() {}
void dTgShipFlood_c::executeState_Stop() {
    if (!SceneflagManager::sInstance->checkBoolFlag(mRoomID, mStopFlag)) {
        mStateMgr.changeState(StateID_Wait);
    }
}
void dTgShipFlood_c::finalizeState_Stop() {}

// Same logic as dTgBarrel_c::checkPlayerPos, but the original was compiled without constant pooling.
// NONMATCHING: regalloc - the original multiplies by f31 directly where we emit `fmr f0, f31` first.
// Using the 0.5f literal for halfCos gets the saved registers right (r28-r31) but adds this copy.
#pragma pool_data off
bool dTgShipFlood_c::checkPlayerPos(const mVec3_c &playerPos) {
    f32 scale = 0.5f;

    mVec3_c pos = playerPos - (mPosition + (mVec3_c::Ey * mScale.y * scale));
    mAng rot = mStageRef.get()->mRotation.z;

    pos.x = pos.x / mScale.x;
    pos.y = pos.y / mScale.y;
    pos.z = pos.z / mScale.z;

    f32 halfCos = rot.cos() * 0.5f;
    if (rot.cos() != 0.f) {
        scale = mScale.x * ((pos.x * rot.sin()) / rot.cos()) / mScale.y;
    }

    return (
        (pos.x <= halfCos && pos.x >= -halfCos)                            //
        && pos.y <= scale + 0.5f + 1e-6f && pos.y >= scale + -0.5f - 1e-6f //
        && pos.z <= 0.5f && pos.z >= -0.5f
    );
}
#pragma pool_data on

void dTgShipFlood_c::startRumble() {
    if (mRumbleEnabled && !mHasRumbled) {
        dRumble_c::start(dRumble_c::sRumblePreset3, 0x11);
        mHasRumbled = true;
    }
}
