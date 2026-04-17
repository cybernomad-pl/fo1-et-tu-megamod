# Lamka Review -- fo1-et-tu-megamod

## gl_crowbar.ssl
- **Feature**: Implements a crowbar forcing mechanic in Fallout 2.
- **Main Functions/Hooks**:
  - `start`: Registers the `useobjon_handler` for the `HOOK_USEOBJON`.
  - `useobjon_handler`: Handles the logic for using a crowbar on locked doo
doors or containers, performing a strength check to determine success.
- **External Dependencies**: Imports from SFall libraries (`sfall/sfall.h`,
(`sfall/sfall.h`, `sfall/define_extra.h`) and uses SFall hooks and function
functions.
- **Config Keys Read**: None explicitly mentioned in the code snippet.
- **Concerns/Red Flags**:
  - The use of hardcoded values for XP success and failure (10 and 3 respec
respectively) might be inconsistent with the game's existing XP system.
  - The strength check is simplistic, using a random roll against the playe
player's ST stat, which may not accurately reflect in-game mechanics.

## gl_general_repair.ssl
- **Feature**: Implements a repair skill for players to convert ammo, disas
disassemble electronics, and upgrade weapons using tools and junk.
- **Main Functions/Hooks**:
  - `repair_skill_handler`: Main handler for the Repair skill on ground ite
items.
  - `try_ammo_conversion`, `try_disassemble_item`, `try_weapon_upgrade`: Ha
Handle specific actions based on item type.
  - `try_dismantle_scenery`: Handles dismantling scenery to obtain junk.
- **External Dependencies**:
  - Imports: `define.h`, `command.h`, `sfall/sfall.h`, `sfall/define_extra.
`sfall/define_extra.h`, `sfall/lib.arrays.h`.
  - Globals: `dude_obj`, `PID_JUNK`, various XP values, `dismantled_scenery
`dismantled_scenery`.
  - SFall Hooks Used: `HOOK_USESKILLON`.
- **Config Keys Read**:
  - `megamod.ini|repair_start|loaded`
  - `megamod.ini|repair|fired`
  - `megamod.ini|repair|target_type`
  - `megamod.ini|repair|target_pid`
  - `megamod.ini|dismantle|pid`
  - `megamod.ini|dismantle|proto`
  - `megamod.ini|dismantle|done`
- **Concerns/Red Flags**:
  - Repair skill check is currently disabled and TODO for balancing.
  - Weapon upgrade modifies all instances globally, which could be risky.
  - Rock dismantling logic might need refinement to ensure usability.

## gl_general_science.ssl
**Summary of Fallout 2 Et Tu mod file:**

- **Feature Implemented:** Allows players to use Science on various objects
objects, gaining observations and XP. Dead critters can be autopsied for cr
creature-specific weak spots.
  
- **Main Functions/Hooks:**
  - `start`: Initializes arrays for tracking examined and autopsied objects
objects, registers the `useskillon_handler` hook.
  - `useskillon_handler`: Handles the Science skill use on various targets,
targets, determining XP gain based on object type and player's Science skil
skill.

- **External Dependencies/Imports/Globals:**
  - Imports from SFall libraries (`sfall/sfall.h`, `sfall/lib.arrays.h`).
  - Uses global variables `examined_objects` and `autopsied_objects`.
  - Registers the `HOOK_USESKILLON`.

- **Config Keys Read:** None explicitly mentioned in the code.

- **Concerns/Red Flags:**
  - The mod relies on SFall-specific functions, which may not be available 
or compatible with other mods.
  - The random observation generation is repetitive and could be improved f
for variety.
  - There's no error handling or fallback mechanism if a target object does
doesn't have the expected properties.

## gl_junk_farmer.ssl
- **Feature**: Implements a repair and rock harvesting system for scenery i
in Fallout 2.
- **Main Functions/Hooks**:
  - `repair_on_scenery_handler`: Handles the repair skill on scenery, disma
dismantling it for parts.
  - `rock_click_handler`: Handles clicking on rock scenery to pick up rocks
rocks.
- **External Dependencies**: Imports from SFall libraries (`sfall/sfall.h`,
(`sfall/sfall.h`, `sfall/define_extra.h`, `sfall/lib.arrays.h`).
- **Config Keys Read**: None explicitly mentioned in the code.
- **Concerns/Red Flags**:
  - The use of global variables like `dismantled_scenery` and `harvested_ro
`harvested_rocks` without proper synchronization could lead to issues in mu
multiplayer environments.
  - The hardcoded list of rock PIDs (`is_rock_scenery`) is not flexible and
and may need updates for new scenery.

## gl_megamod_init.ssl
- **Feature**: Adds a starting kit to the Vault_ Dweller character upon fir
first game load.
- **Main Functions/Hooks**:
  - `start`: Checks if the starting kit has already been given and calls `g
`give_starting_kit` if not.
  - `give_starting_kit`: Adds essential items (lighter, rope, shovel) to th
the player's inventory if they are not already present.
- **External Dependencies/Imports**:
  - SFall libraries (`define.h`, `command.h`, `sfall/sfall.h`, `sfall/defin
`sfall/define_extra.h`).
- **Config Keys Read**: None explicitly mentioned in the code snippet.
- **Concerns/Red Flags**:
  - The mod assumes specific item IDs (`PID_LIGHTER`, etc.) which may chang
change in future versions of Fallout 2.
  - The code does not handle cases where the player might already have some
some items, leading to redundant checks.

## gl_megamod_tools.ssl
**Summary of Fallout 2 Et Tu mod file:**

- **Feature:** Implements a unified handler for `HOOK_USEOBJON` and `HOOK_U
`HOOK_USEOBJ`, providing a single point of control for item interactions.
- **Main Functions/Hooks:**
  - `useobjon_handler`: Handles interactions between items and objects.
  - `useobj_handler`: Handles item usage from the inventory.
  - `gamemode_handler`: Manages game mode-specific logic, such as unlocking
unlocking doors and managing player state.
  - `handle_lighter`, `handle_rope`, `handle_crowbar`, `handle_shovel_on_co
`handle_shovel_on_corpse`, `handle_shovel_dig`, `handle_dismantle_scenery`,
`handle_dismantle_scenery`, etc.: Handle specific item interactions and eff
effects.
- **External Dependencies:**
  - Imports from `sfall` library for game object manipulation, inventory ha
handling, and global variables.
  - Uses SFall hooks (`HOOK_USEOBJON`, `HOOK_USEOBJ`, `HOOK_GAMEMODECHANGE`
`HOOK_GAMEMODECHANGE`) to intercept and handle events.
- **Config Keys Read:** Reads configuration settings from `megamod.ini`.
- **Concerns/Red Flags:**
  - Lack of error handling for invalid inputs or unexpected states.
  - Potential performance issues with frequent calls to `list_begin`, `list
`list_next`, and other list operations.
  - Overuse of global variables and arrays, which can lead to state managem
management challenges.

## gl_mod_faction_travel.ssl
- **Feature**: Respawns summoned faction NPCs on each map enter.
- **Main Functions/Hooks**:
  - `start`: Initializes the mod and registers a hook for game mode changes
changes.
  - `gamemode_handler`: Handles respawning factions when entering a new map
map.
  - `spawn_in_radius`: Spawns NPCs within a radius of 3-5 tiles around the 
player.
  - Faction-specific respawn procedures (`respawn_baldie`, `respawn_brother
`respawn_brotherhood`, etc.).
- **External Dependencies**:
  - Imports from SFall libraries for game object creation, inventory manage
management, and art changes.
  - Uses SFall hooks like `HOOK_GAMEMODECHANGE`.
- **Config Keys Read**: Reads faction data from the "megamod_factions_summo
"megamod_factions_summoned" array in `gl_radio.ssl`.
- **Concerns/Red Flags**:
  - The mod heavily relies on SFall libraries, which may not be compatible 
with all versions of Fallout 2.
  - The respawn logic is hardcoded for specific factions and their equipmen
equipment, limiting flexibility.

## gl_mod_lighter.ssl
- **Feature**: Implements a light-emitting object that follows the player, 
controlled by a lighter item.
- **Main Functions/Hooks**:
  - `useobj_handler`: Handles the use of the lighter item to toggle the lig
light on and off.
  - `map_enter_p_proc`: Creates the light object when entering a map if it'
it's not already created.
  - `map_update_p_proc`: Updates the position of the light object to follow
follow the player.
- **External Dependencies**:
  - Imports from SFall libraries (`sfall/sfall.h`, `sfall/define_extra.h`, 
`sfall/define_lite.h`, `sfall/lib.arrays.h`).
  - Uses SFall hooks for object interaction and global script control.
- **Config Keys Read**: Reads the `lighter_lit` state from `megamod_lighter
`megamod_lighter.ini`.
- **Concerns/Red Flags**:
  - The use of SFall-specific functions and libraries, which may not be com
compatible with other engines or mods.
  - Potential performance issues due to frequent object creation and destru
destruction.

## gl_mod_medic.ssl
- **Feature**: Resurrects dead NPCs using a First Aid skill.
  - **Main Functions/Hooks**: `medic_handler` (handles the use of the First
First Aid skill), `try_resurrect` (attempts to resurrect the target).
  - **External Dependencies**: Imports from SFall libraries (`define.h`, `c
`command.h`, `sfall/sfall.h`, `sfall/define_extra.h`, `sfall/lib.arrays.h`)
`sfall/lib.arrays.h`). Uses global variables and functions.
  - **Config Keys**: Reads "megamod.ini|mod_medic|loaded" (1 if loaded).
  - **Concerns/Red Flags**:
    - Fallback method of destroying the corpse and creating a new one might
might not always be ideal.
    - No filtering for special death animations, which could lead to uninte
unintended resurrections.

- **Feature**: Checks for necessary medic supplies before attempting resurr
resurrection.
  - **Main Functions/Hooks**: `has_medic_supplies` (checks inventory for re
required items).
  - **External Dependencies**: Uses global variables and functions.
  - **Config Keys**: None.
  - **Concerns/Red Flags**:
    - The check might be too strict, requiring multiple Stimpaks even if on
only one is needed.

- **Feature**: Handles the resurrection process, including consuming suppli
supplies, simulating time passing, and creating a new NPC.
  - **Main Functions/Hooks**: `try_resurrect` (handles the actual resurrect
resurrection logic).
  - **External Dependencies**: Uses global variables and functions.
  - **Config Keys**: Reads "megamod.ini|medic|resurrected" (PID of resurrec
resurrected NPCs).
  - **Concerns/Red Flags**:
    - The use of `game_time_advance` for simulating time passing is a worka
workaround since `fade_out/fade_in` are not available in global scripts.
    - The fallback method of destroying the corpse and creating a new one m
might lead to loss of loot or inventory items.

## gl_mod_multitool.ssl
- **Feature:** Implements a multi-tool that searches for and dismantles sce
scenery within 3 tiles, collecting parts based on the type of scenery.
- **Main Functions/Hooks:**
  - `start`: Initializes the mod, loads part counters, registers hooks.
  - `useobj_handler`: Handles the use of the multi-tool, scanning for nearb
nearby scenery and collecting parts.
  - `add_part`: Adds a specified amount of a part to the inventory.
  - `show_parts`: Displays the current inventory of parts.
- **External Dependencies:**
  - Imports from SFall libraries (`define.h`, `command.h`, `sfall/sfall.h`,
`sfall/sfall.h`, `sfall/define_extra.h`, `sfall/lib.arrays.h`).
  - Uses SFall hooks (`HOOK_USEOBJ`).
- **Config Keys Read:** None explicitly mentioned in the code.
- **Concerns/Red Flags:**
  - No dialog is implemented, which could cause engine crashes.
  - The use of hardcoded item IDs and elevation checks might not be robust 
across different game versions or mods.

## gl_mod_recruit.ssl
- **Feature Implementation**: The mod allows players to recruit NPCs by usi
using the Speech skill on them. Recruited NPCs follow the player and fight 
for them. It also includes an armor appearance system that changes based on
on equipped armor.
  
- **Main Functions/Hooks**:
  - `speech_handler`: Handles the recruitment of NPCs when the Speech skill
skill is used.
  - `invenwield_handler`: Updates NPC appearances when items are equipped o
or unequipped, particularly focusing on armor slots.
  - `gamemode_handler`: Refreshes NPC appearances during transitions from i
inventory/loadgame back to gameplay mode.
  - `ondeath_handler`: Protects party members from death by healing them an
and moving them back to the player's tile.

- **External Dependencies**:
  - Imports various SFall libraries for handling game objects, arrays, and 
hooks.
  - Uses SFall-specific functions like `get_sfall_arg`, `set_ini_setting`, 
and `register_hook_proc`.

- **Config Keys Read**: The mod reads from several INI settings stored in "
"megamod.ini", including keys related to recruited NPCs, appearance caching
caching, and flags.

- **Concerns/Red Flags**:
  - The scout handler is commented out, which might indicate it's not fully
fully functional or needed.
  - There are hardcoded lists of NPC IDs that can be recruited, which could
could be improved with a more dynamic system.

## gl_mod_rope.ssl
- **Feature**: Implements a mod that allows players to bind knocked-out cri
critters using ropes.
- **Main Functions/Hooks**:
  - `start`: Registers the `useobjon_handler` for handling object usage int
interactions.
  - `useobjon_handler`: Handles the logic for binding critters with ropes w
when used by the player.
- **External Dependencies/Imports**:
  - Imports from SFall libraries (`define.h`, `command.h`, `sfall/sfall.h`,
`sfall/sfall.h`, `sfall/define_extra.h`, `sfall/lib.arrays.h`).
- **Config Keys Read**: None explicitly mentioned in the code.
- **Concerns/Red Flags**:
  - The mod modifies critter states and team assignments, which could have 
unintended consequences if not tested thoroughly.

## gl_mod_shovel.ssl
- **Feature**: Implements a shovel tool that allows players to dig holes an
and bury corpses.
- **Main Functions/Hooks**:
  - `start`: Initializes the mod, loads saved data, registers hooks for obj
object use.
  - `useobj_handler`: Handles digging holes on valid terrain.
  - `useobjon_handler`: Handles burying corpses on valid terrain.
  - `is_valid_dig_terrain`: Checks if the current terrain is suitable for d
digging.
- **External Dependencies**:
  - Imports from `define.h`, `command.h`, `sfall/sfall.h`, and `sfall/defin
`sfall/define_extra.h`.
  - Uses SFall hooks (`HOOK_USEOBJ` and `HOOK_USEOBJON`).
- **Config Keys**: Reads from "megamod.ini" under the sections "shovel_mod"
"shovel_mod" and "shovel".
- **Concerns/Red Flags**:
  - No error handling for invalid inputs or unexpected states.
  - Potential performance issues with frequent calls to `save_array`.
  - Lack of user feedback on successful actions (e.g., visual confirmation)
confirmation).

## gl_mod_tandi.ssl
- **Feature Implementation**: The mod overwrites Tandi's prototype base FID
FID to apply a new art style and adds logic for unlocking doors based on th
the presence of a water chip.
- **Main Functions/Hooks**:
  - `start`: Initializes the mod, sets up hooks, and logs initialization st
status.
  - `gamemode_handler`: Iterates through all critters on the map, swapping 
Tandi objects to the new art style, and unlocks doors if the player has a w
water chip.
- **External Dependencies**: Imports from SFall libraries (`sfall/sfall.h`,
(`sfall/sfall.h`, `sfall/define_extra.h`, etc.), uses SFall hooks (`HOOK_GA
(`HOOK_GAMEMODECHANGE`), and reads global variables.
- **Config Keys Read**: None explicitly mentioned in the code; it relies on
on SFall's configuration system.
- **Concerns/Red Flags**:
  - The use of direct `OBJ_DATA_FID` writes bypasses metarule caching, whic
which could lead to unexpected behavior or conflicts with other mods.
  - The logic for unlocking doors is hardcoded and may not work in all scen
scenarios (e.g., if Tandi is not present on the map).

## gl_mod_upgrade.ssl
- **Feature Implementation**: Implements weapon upgrades, ammo conversion, 
and item disassembly using the Repair skill on ground items.
- **Main Functions/Hooks**:
  - `upgrade_handler`: Handles the main logic for repairing items based on 
their type (ammo, electronics, weapons).
  - `try_ammo_conversion`, `try_disassemble_item`, `try_weapon_upgrade`: Sp
Specific procedures to handle each type of item repair.
- **External Dependencies**: Imports from SFall libraries (`define.h`, `com
`command.h`, `sfall/sfall.h`, etc.), requiring Multi-Tool or Super Tool Kit
Kit in inventory.
- **Config Keys Read**: None explicitly mentioned; relies on internal game 
state and player actions.
- **Concerns/Red Flags**:
  - Uses global proto mod to change weapon stats, which could affect all in
instances of that weapon type.
  - TODO: Replace with custom PRO files for per-instance upgrades.

## gl_party_hotkeys.ssl
- **Feature**: Implements a hotkey (W) to toggle party wait mode.
- **Main Functions/Hooks**:
  - `keypress_handler`: Handles keypress events and toggles the party wait 
mode.
- **External Dependencies/Imports**:
  - `sfall/sfall.h`, `sfall/define_extra.h`, `sfall/define_lite.h`: For SFa
SFall compatibility and additional definitions.
- **Config Keys Read**: 
  - `megamod.ini|party|wait_mode`: Reads the current state of the party wai
wait mode.
- **Concerns/Red Flags**:
  - The hotkey is set to TAB (DX scancode 15), which might conflict with ot
other functionalities.

## gl_radio.ssl
- **Feature**: The mod implements a radio function to sequentially summon f
factions to aid the player. Each faction is unlocked based on certain game 
progress or global variables.
  
- **Main Functions/Hooks**:
  - `radio_handler`: Handles the radio use action, checks if the roster is 
empty and spawns all unlocked factions, or dismisses them if already presen
present.
  - `refresh_unlocks`: Updates which factions are currently unlocked based 
on global variables.
  - Various spawn procedures (`spawn_baldie_gang`, `spawn_brotherhood`, etc
etc.) to create specific faction members with predefined equipment.

- **External Dependencies**:
  - Uses SFall libraries for global variable access, array manipulation, an
and object creation.
  - Depends on specific FIDs (art numbers) for character sprites and weapon
weapons.
  
- **Config Keys**: Reads from `megamod.ini` for logging purposes.
  
- **Concerns/Red Flags**:
  - Several TODO comments indicate missing or incorrect global variable ref
references that need to be addressed.
  - The mod assumes certain factions are always unlocked, which might not a
align with the game's intended progression.

## gl_rocks.ssl
- **Feature**: Collect Rocks from Rock Scenery  
  - Allows players to interact with rock scenery in the wasteland and colle
collect Rock items.

- **Main Functions/Hooks**:
  - `is_rock_scenery`: Determines if an object is a harvestable rock.
  - `do_harvest_rocks`: Gives rocks to the player and marks them as harvest
harvested.
  - `useanimobj_handler`: Hook that fires when the player clicks scenery on
on the map, triggering the rock collection process.

- **External Dependencies**:
  - Imports from `define.h`, `command.h`, `sfall/sfall.h`, `sfall/define_ex
`sfall/define_extra.h`, and `sfall/lib.arrays.h`.
  - Uses SFall hooks (`HOOK_USEANIMOBJ`).

- **Config Keys**:  
  - `XP_ROCKS`: Experience points given for collecting rocks.
  - `ROCKS_MIN` and `ROCKS_MAX`: Minimum and maximum number of rocks collec
collected per interaction.

- **Concerns/Red Flags**:
  - The code assumes the existence of certain SFall globals and functions, 
which may not be available in all environments.
  - There is no error handling for cases where the player might try to coll
collect rocks during combat.

## gl_rope_tie.ssl
- **Feature**: Implements a rope tie mechanic in Fallout 2, allowing the pl
player to tie down knocked-down critters.
- **Main Functions/Hooks**:
  - `start`: Registers the `useobjon_handler` for the `HOOK_USEOBJON`.
  - `useobjon_handler`: Handles the logic for tying down critters when a ro
rope is used on them.
- **External Dependencies**: Imports from `define.h`, `command.h`, and `sfa
`sfall/sfall.h`. Uses SFall hooks and functions like `register_hook_proc`, 
`get_sfall_arg`, `rm_obj_from_inven`, etc.
- **Config Keys Read**: None explicitly mentioned in the code snippet.
- **Concerns/Red Flags**:
  - The XP value (`XP_ROPE_TIE`) is set to 100, which seems unusually high 
for a simple action like tying someone up.
  - The code does not handle cases where the rope might be used on non-host
non-hostile NPCs or other objects.

## gl_shovel.ssl
- **Feature**: Implements a shovel tool that allows players to dig holes an
and bury corpses.
- **Main Functions/Hooks**:
  - `useobj_handler`: Handles the use of the shovel from the inventory.
  - `useobjon_handler`: Handles the use of the shovel on a target object (s
(specifically, dead critters).
- **External Dependencies**: Imports SFall libraries for game interactions 
(`sfall/sfall.h`, `sfall/define_extra.h`, `sfall/lib.arrays.h`).
- **Config Keys Read**: None explicitly mentioned in the code.
- **Concerns/Red Flags**:
  - The shovel's XP rewards are hardcoded and not configurable.
  - The maximum number of holes per map is hardcoded, which might be too re
restrictive or limiting.
  - There's no error handling for invalid terrain types (e.g., indoors) whe
when digging.

## megamod.ini
- **Feature**: Implements a megamod for Fallout 2 that enhances gameplay th
through various modules.
- **Main Functions/Hooks**:
  - Science: Increases XP rewards for discoveries and introduces rare objec
object bonuses.
  - Repair: Requires tools for repairs, with specific tool IDs defined.
  - Radio: Manages summon cooldowns and limits the number of active allies 
from radio communications.
  - Factions: Manages favor thresholds for different factions, affecting th
their interactions with the player.
- **External Dependencies**:
  - Imports: None explicitly listed in the provided snippet.
  - Globals: Favor thresholds for various factions.
  - SFall Hooks Used: Not specified in the given text.
- **Config Keys Read**: 
  - GeneralScience: `enabled`, `base_xp`, `rare_xp`, `max_discoveries`.
  - GeneralRepair: `enabled`, `require_tools`, `tool_pids`.
  - Radio: `enabled`, `summon_cooldown`, `max_allies`, `scale_with_level`.
  - Factions: Individual faction enablement and favor thresholds.
- **Concerns/Red Flags**:
  - Lack of SFall hooks indicates potential compatibility issues with the o
original game's systems.
  - No explicit mention of external dependencies or imports suggests a clea
clean integration, but further inspection is needed.

## radio_factions.ini
- **Feature**: Implements a mod for Fallout 2 that introduces various facti
factions with unique patrol counts, weapons, armor, and NPCs.
- **Main Functions/Hooks**: Defines faction-specific parameters such as cou
count, weapon, armor, and NPC details.
- **External Dependencies**: Imports data from the game's existing files; n
no specific global imports noted.
- **Config Keys Read**: Reads keys like `basic_count`, `basic_weapon`, `eli
`elite_count`, `elite_weapon`, etc., for each faction.
- **Concerns/Red Flags**: No apparent issues or concerns identified in the 
provided text.

## repair_recipes.ini
- **Feature Implementation**: Implements a system to convert different type
types of ammunition between JHP (Jacketed Hollow Point) and AP (Armor Pierc
Piercing) rounds.
- **Main Functions/Hooks**:
  - Converts 10mm JHP to 10mm AP, 5mm JHP to 5mm AP, .44 Magnum JHP to .44 
FMJ, and Rocket to Explosive Rocket.
  - Adjusts conversion ratios based on the player's skill level.
- **External Dependencies**:
  - Imports ammunition PID values from Fallout 1.
  - Uses SFall hooks for integration with the game engine.
- **Config Keys Read**:
  - `recipe_1` through `recipe_7`: Define individual conversion recipes.
  - `base_ratio`, `skilled_ratio`, `expert_ratio`: Set conversion ratios fo
for different skill levels.
  - `skilled_threshold`, `expert_threshold`: Define skill thresholds for ra
ratio tiers.
- **Concerns/Red Flags**:
  - The mod heavily relies on SFall hooks, which may cause compatibility is
issues with other mods or updates.
  - The high repair percentages required (e.g., 75% for Rocket to Explosive
Explosive Rocket) might be unrealistic in a game setting.

## README.md
- **Feature**: Implements various mods for Fallout Et Tu, including General
General Science, General Repair, and Radio functionalities.
- **Main Functions/Hooks**: Provides science and repair mechanics, faction 
interactions with favor systems, and radio communication to call in allies.
allies.
- **External Dependencies**: Requires Fallout Et Tu v1.15.3735+ and 
sfall 4.4.4+. Uses SFall hooks for integration.
- **Config Keys Read**: Toggles modules via `mods/fo1_megamod/config/megamo
`mods/fo1_megamod/config/megamod.ini`.
- **Concerns/Red Flags**: Work in progress, not playable yet; requires spec
specific versions of Et Tu and SFall; potential compatibility issues with o
other mods.

## FACTION_SYSTEM_DESIGN.md
- **Feature:** Implements a faction and radio system for Fallout 2, enhanci
enhancing player interactions and exploration.
- **Main Functions/Hooks:** Radio menu for recalling waiting party members 
and summoning factions; camp system for creating and moving camps; inventor
inventory tracking for summoned NPCs.
- **External Dependencies:** Uses SFall hooks for script execution and glob
global variables (GVARs) to manage faction unlocks and NPC states.
- **Config Keys:** Reads GVARs for faction unlock flags, persistent arrays 
for NPC and camp data.
- **Concerns/Red Flags:** No specific issues noted; the system appears well
well-documented and modular.

