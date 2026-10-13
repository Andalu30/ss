#ifndef D_A_OBJ_DOOR_BASE_H
#define D_A_OBJ_DOOR_BASE_H

#include "d/a/d_a_base.h"
#include "d/a/obj/d_a_obj_base.h"
#include "f/f_list_mg.h"

// Ghidra: ActorDoorBase
//   size:
// non-official name
class dAcObjDoor_c : public dAcObjBase_c {
public:
    dAcObjDoor_c();
    ~dAcObjDoor_c();

    virtual bool canOpen() = 0;
    virtual bool hasVisitedRoom();
    virtual void vt_0x88();
    virtual void vt_0x8C();

    /// @brief Iterates over all doors. Pass nullptr to get the first door.
    static dAcObjDoor_c *getNextDoor(dAcObjDoor_c *door);

protected:
    dAcRef_c<dAcObjDoor_c> mDoorRef;

private:
    static fLiMgBa_c sDoorList;
};

#endif
