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

#define MIXIN(id, compat, descr, prob) add_mixin(&mixin_pointer, id, compat, descr, prob, false);
#define MIXIN_UTIL(id, compat, descr, prob) add_mixin(&mixin_pointer, id, compat, descr, prob, true);

#define OBJCLASS(pn, flag, sym, prob) add_objclass(&objclass_pointer, pn, flag, sym, prob);

#define NBUF 200
#define BUF_SIZE 200

Mixin mixins_list[MAX_MIXIN];
int nb_mixins;
ObjectClass objclasses_list[MAX_OBJCLASS];
int nb_objclass;

char buffers[NBUF][BUF_SIZE];
int buffer_index = 0;

void add_mixin(int *position, MixinType id, int compatible_classes, char *desc,
               int prob, bool util)
{
    if (*position > MAX_MIXIN) {
        print_to_log("Tried to register too many mixins\n");
        return;
    }

    mixins_list[*position].id = id;
    mixins_list[*position].compatible_classes = compatible_classes;
    mixins_list[*position].descr = desc;
    mixins_list[*position].prob = prob;
    mixins_list[*position].util = util;
    (*position)++;
}

void init_mixins()
{
    int mixin_pointer = 0;

    MIXIN(MT_AT_CRITICAL, OT_MELEE | OT_WAND | OT_FOOD, "lucky blows", 5);
    MIXIN(MT_AT_EXP, OT_MELEE | OT_WAND | OT_FOOD, "quick learning", 5);
    MIXIN(MT_AT_FREEZE, OT_MELEE | OT_WAND | OT_FOOD, "biting cold", 10);
    MIXIN(MT_AT_LARGE, OT_MELEE | OT_WAND | OT_FOOD, "prowess", 15);
    MIXIN(MT_AT_MATCH, OT_MELEE | OT_WAND | OT_FOOD, "consistency", 20);
    MIXIN(MT_AT_RAGE, OT_MELEE | OT_WAND | OT_FOOD, "last hope", 10);
    MIXIN(MT_AT_SPLIT, OT_MELEE | OT_WAND | OT_FOOD, "division", 20);
    MIXIN(MT_AT_SMALL, OT_MELEE | OT_WAND | OT_FOOD, "cowardice", 20);

    MIXIN(MT_DF_BLOWBACK,
          OT_MELEE | OT_WAND | OT_BODY_ARMOR | OT_HELM | OT_FOOD, "blowback",
          5);
    MIXIN(MT_DF_CRITICAL,
          OT_MELEE | OT_WAND | OT_BODY_ARMOR | OT_HELM | OT_FOOD, "calm", 10);
    MIXIN(MT_DF_MELEE, OT_MELEE | OT_WAND | OT_BODY_ARMOR | OT_HELM | OT_FOOD,
          "blunt force", 20);
    MIXIN(MT_DF_REFLECTION,
          OT_MELEE | OT_WAND | OT_BODY_ARMOR | OT_HELM | OT_FOOD, "reflection",
          20);
    MIXIN(MT_DF_SMALL, OT_MELEE | OT_WAND | OT_BODY_ARMOR | OT_HELM | OT_FOOD,
          "nontriviality", 20);
    MIXIN(MT_DF_WEAKNESS,
          OT_MELEE | OT_WAND | OT_BODY_ARMOR | OT_HELM | OT_FOOD, "uniformity",
          2);

    MIXIN(MT_BG_EMERGPORT, OT_ALL, "vanishing", 10);
    MIXIN(MT_BG_HP, OT_ALL, "health", 10);
    MIXIN(MT_BG_REGEN, OT_ALL, "exercise", 10);
    MIXIN(MT_BG_TELEPATHY, OT_ALL, "ESP", 5);
    MIXIN(MT_BG_ID, OT_ALL, "full knowledge", 2);
    MIXIN(MT_BG_SPEED, OT_ALL, "swiftness", 10);
    MIXIN(MT_BG_WAKE, OT_ALL, "alarm", 5);
    MIXIN(MT_BG_SEE_INV, OT_ALL, "vision", 10);

    MIXIN_UTIL(MT_US_MAX_HP, OT_POTION, "great shape", 5);
    MIXIN_UTIL(MT_US_ENCHANT, OT_POTION, "enchantment", 5);
    MIXIN_UTIL(MT_US_ENLIGHTEN, OT_POTION, "enlightenment", 15);
    MIXIN_UTIL(MT_US_LEVELPORT, OT_POTION | OT_TOOL, "level change", 2);
    MIXIN_UTIL(MT_US_HP, OT_POTION | OT_TOOL, "healing", 15);
    MIXIN_UTIL(MT_US_MAP, OT_POTION | OT_TOOL, "level mapping", 10);
    MIXIN_UTIL(MT_US_OPEN, OT_POTION | OT_TOOL, "opening", 5);
    MIXIN_UTIL(MT_US_TP, OT_POTION | OT_TOOL, "ubiquity", 5);
    MIXIN_UTIL(MT_US_DIG, OT_POTION | OT_TOOL, "digging", 10);
    MIXIN_UTIL(MT_US_CHMC, OT_POTION | OT_TOOL, "change", 4);
    MIXIN_UTIL(MT_US_ID, OT_POTION | OT_TOOL, "identification", 15);

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

int pick_mixin(ObjectClassFlag class_flag, int util_only)
{
    int valid_mixins_probs = 0;
    for (int i = 0; i < nb_mixins; i++) {
        if (mixins_list[i].compatible_classes & class_flag &&
            (!util_only || mixins_list[i].util))
            valid_mixins_probs += mixins_list[i].prob;
    }

    if (valid_mixins_probs == 0) {
        print_to_log("Error: found no valid mixins for object class %d%s\n",
                     class_flag,
                     util_only ? " (util only) " : "");
        return -1;
    }

    int i_chosen_mixin = rand_int(0, valid_mixins_probs - 1);
    int so_far = 0;
    for (int i = 0; i < nb_mixins; i++) {
        if (mixins_list[i].compatible_classes & class_flag &&
            (!util_only || mixins_list[i].util)) {
            if (so_far >= i_chosen_mixin)
                return mixins_list[i].id;
            so_far += mixins_list[i].prob;
        }
    }

    return mixins_list[nb_mixins - 1].id;
}


void
add_objclass(int *objclass_pointer, char *possible_names, ObjectClassFlag flag,
             char symbol, int prob)
{
    if (*objclass_pointer > MAX_OBJCLASS) {
        print_to_log("Tried to register too many object classes\n");
        return;
    }

    objclasses_list[*objclass_pointer].possible_names = possible_names;
    objclasses_list[*objclass_pointer].o_class_flag = flag;
    objclasses_list[*objclass_pointer].symbol = symbol;
    objclasses_list[*objclass_pointer].prob = prob;
    (*objclass_pointer)++;
}


void init_objclass()
{
    int objclass_pointer = 0;

    OBJCLASS("sword,dagger,knife,spear,club,hammer,axe,battle-axe,warhammer,",
             OT_MELEE, '\\', 20);
    OBJCLASS("wand,sceptre,staff,quarterstaff,", OT_WAND, '/', 10);
    OBJCLASS("chain mail,coat,armor,ring mail,", OT_BODY_ARMOR, ']', 20);
    OBJCLASS("helm,cask,helmet,cap,hat,", OT_HELM, '[', 10);
    OBJCLASS("potion,flask,philter,serum,elixir,", OT_POTION, '!', 8);
    OBJCLASS("tool,instrument,device,drive,orb,", OT_TOOL, '(', 10);
    OBJCLASS("morsel,meal,ration,", OT_FOOD, '%', 1);
    OBJCLASS("stack of gold pieces,", OT_MONEY, '$',
             0); // Different probability calculation

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
        print_to_log("Panic: too few commas in possible name descriptor (%s)\n",
                     class->possible_names);
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


char *object_name(Object *obj)
{
    char *buf = buffers[buffer_index];
    char *next;
    buffer_index = (buffer_index + 1) % NBUF;

    if (obj->type->class->o_class_flag == OT_MONEY) {
        snprintf(buf, BUF_SIZE, "pile of %d gold pieces", obj->enchant);
    } else {
        if (obj->type->class->o_class_flag & (OT_WAND | OT_MELEE)) {
            next = buf + snprintf(buf, BUF_SIZE, "+%d ", obj->enchant);
        } else {
            next = buf;
        }
        snprintf(next, BUF_SIZE - (next - buf), "%s %s%s%s%s%s",
                 magic_class_adjectives[obj->type->magic_class],
                 obj->type->base_name,
                 obj->type->mixin1 == -1 ? "" : " of ",
                 obj->type->mixin1 == -1 ? "" :
                 find_mixin(obj->type->mixin1)->descr,
                 obj->type->mixin2 == -1 || !obj->type->mixin2_known ?
                 "" : " and ",
                 obj->type->mixin2 == -1 || !obj->type->mixin2_known ?
                 "" : find_mixin(obj->type->mixin2)->descr);
    }

    return buf;
}


void make_object_classes()
{
    init_mixins();
    init_objclass();

    // Make gold
    ObjectType *gold = &object_types[0];
    ObjectClass *gold_class = find_object_class(OT_MONEY);
    if (!gold_class) {
        print_to_log("Panic: could not find object class for money (%d)\n",
                     OT_MONEY);
        return;
    }
    gold->class = gold_class;
    gold->value = 1;
    gold->mixin1 = MT_NONE;
    gold->mixin2 = MT_NONE;
    gold->magic_class = 0;
    gold->color = CLR_DEFAULT;
    gold->power = 0;

    // Make one melee weapon
    ObjectType *melee_weapon = &object_types[1];
    ObjectClass *melee_class = find_object_class(OT_MELEE);
    if (!melee_class) {
        print_to_log(
                "Panic: could not find object class for melee weapon (%d)\n",
                OT_MELEE);
        return;
    }
    melee_weapon->class = melee_class;
    melee_weapon->power = 8;
    melee_weapon->value = 5;
    melee_weapon->color = CLR_DEFAULT;
    melee_weapon->mixin1 = MT_NONE;
    melee_weapon->mixin2 = MT_NONE;
    melee_weapon->magic_class = rand_int(0, NB_MAGIC_CLASSES - 1);
    strncpy_pad(melee_weapon->base_name, "swiss army knife", MAX_NAME);

    // Make the rest randomly
    for (int i = 2; i < NB_OBJECTS; i++) {
        ObjectClassFlag class_flag = random_object_class();
        ObjectClass *class = find_object_class(class_flag);
        char *name_prefix = pick_name(class);
        ObjectType *ot = &object_types[i];

        ot->class = class;
        ot->magic_class = rand_int(0, NB_MAGIC_CLASSES - 1);
        ot->magic_class_known = false;

        if (ot->class->o_class_flag & (OT_TOOL | OT_POTION))
            ot->mixin1 = pick_mixin(class_flag, true);
        else if (ot->class->o_class_flag == OT_FOOD)
            ot->mixin1 = pick_mixin(class_flag, false);
        else
            ot->mixin1 = rand_int(1, 10) < 3 ? MT_NONE : pick_mixin(class_flag,
                                                                    false);

        ot->mixin2 = (ot->mixin1 != MT_NONE && rand_int(1, 10) < 9) ? MT_NONE
                                                                    : pick_mixin(
                        class_flag, false);
        ot->mixin2_known = false;

        ot->color = CLR_DEFAULT;
        ot->power = ndn(3, 8);
        ot->value =
                ot->power + rand_int(5, 25); // TODO: improve value calculation

        strncpy_pad(ot->base_name, name_prefix, MAX_NAME);
        free(name_prefix);
    }
}

void add_level_objects(int level)
{
    int nb_objects = rand_int(2, 9);

    for (int i = 0; i < nb_objects; i++) {
        Object *obj = malloc(sizeof(Object));
        if (rand_int(1, 100) <= 20) // Money has a 20% chance
            obj->type = &object_types[0];
        else
            obj->type = &object_types[rand_int(1, NB_OBJECTS - 1)];

        obj->cooldown = 0;

        if (obj->type->class->o_class_flag == OT_MONEY)
            obj->enchant = rand_int(10, 50);
        else
            obj->enchant = ndn(8, 3) / 2 - 6;

        if (obj->type->class->o_class_flag == OT_POTION)
            obj->uses_left = rand_int(1, 3);
        else
            obj->uses_left = -1;

        obj->flags = 0;
        if (!find_tile(level, &obj->pos, true, -1)) {
            print_to_log("Could not place object %d on dlvl %d\n", i, level);
            return;
        }
        obj->dlvl = level;
        add_to_linked_list(o_list, (void *) obj);
    }
}

LinkedList *find_objs_at(int dlvl, Coord pos)
{
    LinkedList *ret = new_linked_list();
    LinkedListNode *obj_node = o_list->head;

    if (!obj_node)
        return ret;

    while (obj_node != NULL) {
        Object *obj = (Object *) obj_node->element;
        if (obj->pos.x == pos.x && obj->pos.y == pos.y && obj->dlvl == dlvl)
            add_to_linked_list(ret, obj);

        obj_node = obj_node->next;
    }

    return ret;
}

bool has_mixin(const ObjectType *type, MixinType mixin)
{
    return type->mixin1 == mixin || type->mixin2 == mixin;
}