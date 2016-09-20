/*
 *  object.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 20/11/10.
 *  Copyright 2010 OPMORL 2 dev team. All rights reserved.
 *
 */

/* Ok so now we shall use 1 list for the world and one array for the inventory (for letters, it is awesomer) */

#include "opmorl.h"

#define MAX_MIXIN 100
#define MAX_OBJCLASS 100

#define MIXIN(id, compat, descr) add_mixin(&mixin_pointer, id, compat, descr);

#define OBJCLASS(pn, flag, sym) add_objclass(&objclass_pointer, pn, flag, sym);

Mixin mixins_list[MAX_MIXIN];
int nb_mixins;
ObjectClass objclasses_list[MAX_OBJCLASS];
int nb_objclass;

void add_mixin(int *position, Mixin_type id, int compatible_classes, char *desc)
{
    if (*position > MAX_MIXIN) {
        print_to_log("Tried to register too many mixins\n");
        return;
    }

    mixins_list[*position].id = id;
    mixins_list[*position].compatible_classes = compatible_classes;
    mixins_list[*position].descr = desc;
    (*position)++;
}

void init_mixins()
{
    int mixin_pointer = 0;

    MIXIN(MT_AT_CRITICAL, OT_MELEE | OT_WAND, "lucky blows");
    MIXIN(MT_AT_EXP, OT_MELEE | OT_WAND, "quick learning");
    MIXIN(MT_AT_FREEZE, OT_MELEE | OT_WAND, "biting cold");
    MIXIN(MT_AT_LARGE, OT_MELEE | OT_WAND, "prowess");
    MIXIN(MT_AT_MATCH, OT_MELEE | OT_WAND, "consistency");
    MIXIN(MT_AT_RAGE, OT_MELEE | OT_WAND, "last hope");
    MIXIN(MT_AT_SPLIT, OT_MELEE | OT_WAND, "division");
    MIXIN(MT_AT_SMALL, OT_MELEE | OT_WAND, "cowardice");

    MIXIN(MT_DF_BLOWBACK, OT_MELEE | OT_WAND | OT_BODY_ARMOR | OT_HELM, "blowback");
    MIXIN(MT_DF_CRITICAL, OT_MELEE | OT_WAND | OT_BODY_ARMOR | OT_HELM, "calm");
    MIXIN(MT_DF_MELEE, OT_MELEE | OT_WAND | OT_BODY_ARMOR | OT_HELM, "blunt force");
    MIXIN(MT_DF_REFLECTION, OT_MELEE | OT_WAND | OT_BODY_ARMOR | OT_HELM, "reflection");
    MIXIN(MT_DF_SMALL, OT_MELEE | OT_WAND | OT_BODY_ARMOR | OT_HELM, "nontriviality");
    MIXIN(MT_DF_WEAKNESS, OT_MELEE | OT_WAND | OT_BODY_ARMOR | OT_HELM, "uniformity");

    MIXIN(MT_BG_DEX, OT_ALL, "dexterity");
    MIXIN(MT_BG_INT, OT_ALL, "wisdom");
    MIXIN(MT_BG_STR, OT_ALL, "power");
    MIXIN(MT_BG_EMERGPORT, OT_ALL, "vanishing");
    MIXIN(MT_BG_HP, OT_ALL, "health");
    MIXIN(MT_BG_REGEN, OT_ALL, "exercise");
    MIXIN(MT_BG_TELEPATHY, OT_ALL, "ESP");
    MIXIN(MT_BG_ID, OT_ALL, "identification");
    MIXIN(MT_BG_SPEED, OT_ALL, "swiftness");
    MIXIN(MT_BG_WAKE, OT_ALL, "alarm");

    MIXIN(MT_US_DEX, OT_POTION, "nimbleness");
    MIXIN(MT_US_HP, OT_ALL, "healing");
    MIXIN(MT_US_LEVELPORT, OT_ALL, "level change");
    MIXIN(MT_US_MAP, OT_ALL, "level mapping");
    MIXIN(MT_US_MAX_HP, OT_POTION, "great shape");
    MIXIN(MT_US_OPEN, OT_ALL, "opening");
    MIXIN(MT_US_INT, OT_POTION, "smartness");
    MIXIN(MT_US_TP, OT_ALL, "ubiquity");
    MIXIN(MT_US_STR, OT_POTION, "force");
    MIXIN(MT_US_DIG, OT_ALL, "digging");

    nb_mixins = mixin_pointer;

    print_to_log("Registered %d mixins\n", nb_mixins);
}

Mixin *find_mixin(int mixin_id)
{
    for (int i = 0; i < nb_mixins; i++) {
        if (mixins_list[i].id == mixin_id)
            return &mixins_list[i];
    }

    return NULL;
}

int pick_mixin(ObjectClassFlag class_flag)
{
    int valid_mixins = 0;
    for (int i = 0; i < nb_mixins; i++) {
        if (mixins_list[i].compatible_classes & class_flag)
            valid_mixins++;
    }

    if (valid_mixins == 0) {
        print_to_log("Error: found no valid mixins for object class %d\n", class_flag);
        return -1;
    }

    int i_chosen_mixin = rand_int(0, valid_mixins - 1);
    int so_far = 0;
    for (int i = 0; i < nb_mixins; i++) {
        if (mixins_list[i].compatible_classes & class_flag) {
            if (so_far == i_chosen_mixin)
                return mixins_list[i].id;
            so_far++;
        }
    }

    return -1; // Should not reach here
}


void add_objclass(int *objclass_pointer, char *possible_names, ObjectClassFlag flag, char symbol)
{
    if (*objclass_pointer > MAX_OBJCLASS) {
        print_to_log("Tried to register too many object classes\n");
        return;
    }

    objclasses_list[*objclass_pointer].possible_names = possible_names;
    objclasses_list[*objclass_pointer].o_class_flag = flag;
    objclasses_list[*objclass_pointer].symbol = symbol;

    (*objclass_pointer)++;
}


void init_objclass()
{
    int objclass_pointer = 0;

    OBJCLASS("sword,dagger,knife,spear,club,hammer,axe,battle-axe,", OT_MELEE, '\\');
    OBJCLASS("wand,sceptre,staff,quarterstaff,orb,", OT_WAND, '/');
    OBJCLASS("chain mail,coat,armor,", OT_BODY_ARMOR, ']');
    OBJCLASS("helm,cask,helmet,", OT_HELM, '[');
    OBJCLASS("potion,flask,philter,serum,elixir,", OT_POTION, '!');
    OBJCLASS("tool,instrument,device,drive,", OT_TOOL, '(');
    OBJCLASS("morsel,meal,ration,", OT_FOOD, '%');
    OBJCLASS("gold piece,", OT_MONEY, '$');

    nb_objclass = objclass_pointer;

    print_to_log("Registered %d object classes\n", nb_objclass);
}


ObjectClass *find_object_class(ObjectClassFlag flag)
{
    for (int i = 0; i < nb_objclass; i++) {
        if (objclasses_list[i].o_class_flag == flag)
            return &objclasses_list[i];
    }

    return NULL;
}

char *pick_name(ObjectClass *class)
{
    int nb_commas = 0;
    int names_len = (int) strlen(class->possible_names);
    for (int i = 0; i < names_len; i++) {
        if (class->possible_names[i] == ',')
            nb_commas++;
    }

    if (nb_commas < 0) {
        print_to_log("Panic: too few commas in possible name descriptor (%s)\n", class->possible_names);
        return "";
    }

    int name_nb = rand_int(0, nb_commas - 1);
    int found_commas = 0;
    int start = 0, end = 0;

    for (int i = 0; i < names_len; i++) {
        if (class->possible_names[i] == ',') {
            found_commas++;

            if (found_commas == name_nb)
                start = i + 1;
            else if (found_commas > name_nb) {
                end = i + 1;
                break;
            }
        }
    }

    char *ret = malloc(sizeof(char) * (end - start + 1));
    strncpy_pad(ret, class->possible_names + start, (size_t) (end - start));

    return ret;
}


ObjectClassFlag random_object_class()
{
    int r = rand_int(0, NB_OT - 2); // Never return gold
    switch (r) {
    case 0:
        return OT_HELM;
    case 1:
        return OT_BODY_ARMOR;
    case 2:
        return OT_MELEE;
    case 3:
        return OT_WAND;
    case 4:
        return OT_POTION;
    case 5:
        return OT_FOOD;
    case 6:
        return OT_TOOL;
    default:
        return OT_MELEE; // Should not happen.
    }
}


void make_objects()
{
    init_mixins();
    init_objclass();

    // Make gold
    ObjectType *gold = &object_types[0];
    ObjectClass *gold_class = find_object_class(OT_MONEY);
    if (!gold_class) {
        print_to_log("Panic: could not find object class for money (%d)\n", OT_MONEY);
        return;
    }
    gold->class = gold_class;
    strcpy(gold->name, "gold piece");
    gold->value = 1;
    gold->mixin1 = -1;
    gold->mixin2 = -1;
    gold->magic_class = -1;
    gold->color = CLR_DEFAULT;
    gold->power = 0;

    // Make one melee weapon
    ObjectType *melee_weapon = &object_types[1];
    ObjectClass *melee_class = find_object_class(OT_MELEE);
    if (!melee_class) {
        print_to_log("Panic: could not find object class for melee (%d)\n", OT_MELEE);
        return;
    }
    melee_weapon->class = melee_class;
    strncpy(melee_weapon->name, "sword of the beginner", MAX_NAME);
    melee_weapon->power = 5;
    melee_weapon->value = 5;
    melee_weapon->color = CLR_DEFAULT;
    melee_weapon->mixin1 = -1;
    melee_weapon->mixin2 = -1;
    melee_weapon->magic_class = rand_int(0, NB_MAGIC_CLASSES - 1);

    // Make the rest randomly
    for (int i = 2; i < NB_OBJECTS; i++) {
        ObjectClassFlag class_flag = random_object_class();
        ObjectClass *class = find_object_class(class_flag);
        char *name_prefix = pick_name(class);

        object_types[i].class = class;
        object_types[i].magic_class = rand_int(0, NB_MAGIC_CLASSES - 1);

        object_types[i].mixin1 = pick_mixin(class_flag);
        object_types[i].mixin2 = pick_mixin(class_flag);

        object_types[i].color = CLR_DEFAULT;
        object_types[i].power = rand_int(0, 100);
        object_types[i].value = object_types[i].power + (rand_int(0, 20) - 10);

        char *mixin1desc = find_mixin(object_types[i].mixin1)->descr;
        char *mixin2desc = find_mixin(object_types[i].mixin2)->descr;
        snprintf(object_types[i].name, MAX_NAME, "%s %s of %s and %s", magic_class_names[object_types[i].magic_class],
                 name_prefix, mixin1desc, mixin2desc);
        free(name_prefix);
    }
}

void add_level_objects(int level)
{
    int nb_objects = rand_int(2, 9);

    for (int i = 0; i < nb_objects; i++) {
        Object *obj = malloc(sizeof(Object));
        ObjectType *type = &object_types[rand_int(0, NB_OBJECTS - 1)];

        obj->amount = 1;
        obj->cooldown = 0;
        obj->flags = 0;
        obj->name[0] = '\0';
        obj->type = type;
        if (!find_floor_tile(level, &obj->posx, &obj->posy, T_WALKABLE, true)) {
            print_to_log("Could not place object %d on level %d\n", i, level);
            return;
        }
        obj->level = level;
        add_to_linked_list(o_list, (void *) obj);
    }
}

LinkedList *find_objs_at(int x, int y, int level)
{
    LinkedList *ret = new_linked_list();
    LinkedListNode *obj_node = o_list->head;

    if (!obj_node)
        return NULL; /* Be careful with that one ! */

    while (obj_node != NULL) {
        Object *obj = (Object *) obj_node->element;
        if (obj->posx == x && obj->posy == y && obj->level == level)
            add_to_linked_list(ret, obj);

        obj_node = obj_node->next;
    }

    return ret;
}