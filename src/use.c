/*
 *  use.c
 *  OPMORL 2
 *
 *  Created by Matthieu Felix on 26/09/2016.
 *  Copyright 2016 OPMORL 2 dev team. All rights reserved.
 *
 */
#include "opmorl.h"

#define IF_HAS(obj, mix) if (has_mixin((const ObjectType*)obj->type, mix)) \
                                        { found_mixin = true;

int use_object(Object *object)
{
    bool found_mixin = false;

    if (object->type->class->o_class_flag == OT_POTION &&
        object->uses_left <= 0) {
        pline("This %s is exhausted.", object_name(object));
        return 0;
    }

    IF_HAS(object, MT_US_MAX_HP)
        int augment = rand_int(0, 6);

        pline("You feel very healthy!");
        rodney.hp += rodney.hp / rodney.max_hp * (rodney.max_hp + augment);
        rodney.max_hp += augment;
    }
    IF_HAS(object, MT_US_LEVELPORT)
        int new_level = max(min(rand_int(rodney.dlvl - 2, rodney.dlvl + 2), 0),
                            DLVL_MAX - 1);
        // TODO: use a different function
        change_dlvl(new_level, T_STAIRS_DOWN);
    }
    IF_HAS(object, MT_US_DIG)
        pline("In which direction [hjklyubn>]?");
        // TODO: implement
    }
    IF_HAS(object, MT_US_MAP)
        pline("A map coalesces in your mind.");
        for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++)
            for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++)
                visibility_map[rodney.dlvl][i_x][i_y] = TS_UNSEEN;
        recompute_visibility();
    }
    IF_HAS(object, MT_US_HP)
        pline("You regain your health.");
        rodney.hp = rodney.max_hp;
    }
    IF_HAS(object, MT_US_TP)
        pline("The room around you suddenly changes!");
        // Guaranteed to succeed since rodney is on a tile.
        find_tile(rodney.dlvl, &rodney.pos, false, -1);
    }
    IF_HAS(object, MT_US_ENCHANT)
        pline("Choose an object to enchant.");
        LinkedList *inv = array_to_linked_list((void **) rodney.inventory,
                                               INVENTORY_SIZE, false);
        Object *to_use = select_object(inv);
        delete_linked_list(inv);

        if (to_use != NULL) {
            to_use->enchant += rand_int(0, 2);
        } else {
            pline("Never mind.");
        }
    }
    IF_HAS(object, MT_US_ENLIGHTEN)
        pline("You feel very knowledgeable.");
    }

    if (found_mixin) {
        if (object->type->class->o_class_flag == OT_POTION)
            object->uses_left -= 1;
        return 1;
    } else {
        pline("This object can't be used!");
        return 0;
    }
}


int use()
{
    LinkedList *inv = array_to_linked_list((void **) rodney.inventory,
                                           INVENTORY_SIZE, false);
    Object *to_use = select_object(inv);
    delete_linked_list(inv);

    if (to_use == NULL) {
        pline("Never mind.");
        return 0;
    } else return use_object(to_use);
}