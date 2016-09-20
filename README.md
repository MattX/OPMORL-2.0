# OVER POWERFUL MOBILE ROGUELIKE 2.0

OPMORL 2.0 intends to be a full-featured Rogue-like written in C using the ncurses library.

## Game overview

The object of the game is to kill the big boss on the final level.

## Objects

### Object generation

Objects in OPMORL 2 are not completely predefined. At the beginning of every game, a number of objects will be formed
by combining object classes, magic classes, and effects.

Object classes define what the object is and how it can be used. The following object classes are planned to be
available:

 * Potions
 * Armor (helm and body armor, effectively two armor slots)
 * Wands
 * Melee weapons
 * Tools
 * Food
 
Experience is calculated by magic class, not by type of weapon. It is usually beneficial to have a set that's as
homogeneous as possible in terms of magic classes. Magic classes are also used for weakness/strength determination. 
There are 5 magic classes per game; their names are generated at the beginning of the game. One of these classes
has no weakness and no resistance, two have one each and two have two weaknesses and two resistances.
Object mixins are revealed only if the player has sufficient magic class experience.

Finally, mixins define define any special effects of the objects. Effect mixins provide long-lasting background effects,
and attack, defense and use mixins produce special results when the object is used to attack, when an attack is
received, and when the object is used, respectively. An object can have at most 2 mixins.

100 items types will be available on each game.

### Monster generation

Monsters also have a magic class, used to determine weakness/resistance.
Contrary to items, they are predefined.

### Combat

## TODO

1. Inventory management (mostly complete)
2. Raytracing algorithm for LOS and combat
3. Monster generation and movement
4. Combat implementation
5. Object frequency tweaking, allow single or no mixin.

## Mixins reference

### Background mixins

 * Stat modification for Int/Str/Dex
 * HP or HP regeneration modification
 * Speed modification
 * Wake monsters
 * Telepathy
 * Full object identification
 * Teleport on low health

### Attack mixins
 
 * Damage against large/small monsters
 * Split small monsters
 * Critical hit chance
 * Attack bonus for matching armor+weapon
 * Freezing
 * Exp augmentation
 * More attack on low health
 
### Defense mixins

 * Immunity against small attacks
 * Reduce critical hit
 * Cancel magic class weakness
 * Reflect damage
 * Reflect rays / ranged projectiles
 * Reflect melee damage

### Use mixins

 * Teleport control
 * Levelport
 * Reveal map
 * Regain health
 * Augment stat
 * (Open locked door) -> waiting for doors
