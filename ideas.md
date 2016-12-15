# Ideas for OPMORL

  * Dungeon forks? With sevral links from the main dungeon to the sub.
  * Make maps a lot denser. The current generation algorithm doesn't
    really make sense for a tower.
  * Beginning with few weapons, make the player focus on armor?
  * Constrain and make the player make choices => convoluted levels
    that take a long time to explore vs pressing food counter
  * Maybe some indications in a native language that the player
    gradually learns?
  * Have a nice administrator name :)


## Immediate todos

  1. Rewrite map generation, rendering, etc. to use a structured
    tiletype system.
  2. Rewrite the map generation algorithm.
  3. Add interactivity in the levels, specifically:
     1. Traps: boulders, trapdoors, turrets, magic traps
     1. Levers that open doors/trapdoors/activate traps
     2. Hidden keys for doors


## Map generation

  * Have the map be full, i.e. look like how people would actually
    design a tower.
  * Collapse some areas to enable better ranged attacks
  * Special levels for atmosphere + special equiment
      * *Barracks* weapons and armor. have turrets in them. Large
      spaces for the lockers. Suite of the sergeant.
      * *Archmage's quarters* magic items. Suite and garden.
      * *Administrator's quarters* Large suite with few rooms and very
      many protections. Pumping control for the last few levels,
      obviating need for special equipment?
      * *Market* Well. Tons of item, mostly food.
      * *Bank* Significantly below the market to force players to
      make a trip as well. Well protected.
  * Standard level types:
      * *Common space* Large rooms with atrium above. Makes ranged
      attacks very useful.
      * *Garden* One very large room with much vegetation. Intermediate,
      trees allow for hiding but there are still large ranges
      * *Offices* Many small rooms
      * *Utility* Tons of pipes and stuff forming a maze.
      * *Reception* A little like offices, larger rooms but still
      convoluted
  * Many types of wall and floor types/dungeon features. Vegetation.
  * Last few levels submerged in water, need special item
  * Have level 3 or 4 always be the barracks or archmage, which makes
    the player pick a class :)
  * Collapsed floors and walls
  
## Monsters
  
  * Mechanical guards (Dwemer-like?)
  * Blind scavenger monsters (degenerate elves?)
  * Animals
  * Magical (clerical and wizardry)
  
## UI

  * Tower map with levels
  * Use all 256 colors
  * Unicode chars for undecyphered Yendori language?
  