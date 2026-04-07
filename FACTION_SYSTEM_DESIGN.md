# Fallout Et Tu Megamod -- Faction & Radio System Design

## Core Mechanics

### Radio
- Player gets radio from Seth (Shady Sands) after Tandi rescue quest, or in starting kit
- USE radio = menu: recall waiting party members + summon unlocked factions
- Factions arrive at CAMP location

### Camp System
- Stop on world map = CAMP marker created at that location
- Random encounters count -- if camp falls on random encounter map, camp IS there
- Leave camp, stop elsewhere = camp moves to new location
- Lighter on scrub = campfire = visual camp confirmation
- Factions and caravans travel TO camp

### Inventory Tracking
- EVERY summoned NPC's inventory is tracked ALWAYS (persistent array)
- Barter with summoned NPCs to give them better gear -- they equip it automatically
- Dead NPC = gone forever (tracked as dead, never respawns)

---

## Factions

### VAULT 13 -- Citizens (PARTY recruitment, not radio)
- Any citizen with full sprite set (black-haired female, black-haired male, etc. -- NOT redhead, NOT dark-skinned if missing sprites)
- Recruitment dialog: random movie quotes
  - "Come with me if you want to live"
  - "Take the red pill and I'll show you how deep the rabbit hole goes"
  - "You have my sword." / "And my axe." type references
  - "I volunteer as tribute"
  - etc.
- Recruited citizen joins PLAYER PARTY (full party control)
- Officer already implemented as party member (separate mod)

### VAULT 13 -- Vincent Fix
- Once hired, stays in party permanently (fix vanilla behavior where he leaves)

### SHADY SANDS -- Seth + Militia
- **Unlock:** Rescue Tandi from Raiders, return to Shady Sands
- Seth swears allegiance, gives radio if player doesn't have one
- **Radio summon:** Seth + Shady Sands militia (armed guards)
- Summoning Shady Sands also triggers Hub caravan (see Hub below)

### RAIDERS / KHANS -- War Party
- **Unlock:** Defeat Garl (Khan leader), become new Khan leader
- **Radio summon:** ALL raider NPCs from the Raider camp map
- Script saves every raider NPC on the map and tracks their inventory
- De facto player's personal raider army

### JUNKTOWN -- 4 Possible Factions (mutually exclusive paths)

#### 1. Killian's Law (help Killian)
- **Unlock:** Complete Killian's quest line
- **Radio summon:** Sheriff Lars + all Junktown guards
- Law & order faction

#### 2. Skullz Gang (join Skullz)
- **Unlock:** Join the Skullz
- **Radio summon:** Skullz gang members

#### 3. Gizmo's Crew (side with Gizmo)
- **Unlock:** Help Gizmo, Gizmo dies later
- **Radio summon:** Gizmo's people led by the black lieutenant (no boss)

#### 4. Junktown Militia (independent path)
- **Unlock:** Help various Junktown citizens
- **Radio summon:** Boxer, bartender, hotel owner (if helped her), the guy who was running from the dog, the woman nearby
- Each equipped with the best weapon they can use (based on their skills)

### HUB -- Crimson Caravan (trade faction)
- **Trigger:** Automatically arrives when player summons Shady Sands OR Junktown factions
- Caravan travels to CAMP
- **Barter:** Full stock -- weapons, ammo, supplies, meds
- Trade faction, not combat

### BONEYARD -- Gunrunners (trade + elite gear)
- **Unlock:** Quest line with Gunrunners
- **Radio summon:** ALL living Gunrunners come to camp
- **Barter:** Heavy weapons, POWER ARMOR, best gear in game
- Dead Gunrunner = permanently gone

### BONEYARD -- Blades (combat faction)
- **Unlock:** Quest line with Blades
- **Radio summon:** Blade fighters
- Melee/close combat specialists

### BROTHERHOOD OF STEEL (elite combat + arsenal)
- **Unlock:** BoS quest line (prove yourself, gain trust)
- **Radio summon:** Paladins in Power Armor
- Strongest combat faction in the game
- Barter access to BoS arsenal

### FOLLOWERS OF THE APOCALYPSE (support faction)
- **Unlock:** Quest line with Followers
- **Radio summon:** Medics, scientists
- Support role: healing, science buffs, intelligence

### NECROPOLIS -- Set's Ghouls + Harold
- **Unlock:** Repair the water pump in Necropolis
- Ghouls survive because they have water (super mutants don't massacre them)
- **Radio summon:** Set's ghoul fighters + Harold
- Harold = unique mutated human NPC (iconic Fallout character)
- Harold can join party or be summoned

---

## Radio Menu Flow

```
USE Radio ->
  [1] Recall all waiting party members
  [2] Summon faction: [unlocked factions listed]
      -> Faction arrives at CAMP
      -> If Shady Sands or Junktown summoned: Hub caravan also comes
  [3] Dismiss summoned faction (send them home)
```

---

## Technical Implementation

### GVARs needed (faction unlock flags):
- GVAR_FACTION_SHADY_SANDS (Seth quest complete)
- GVAR_FACTION_KHANS (became Khan leader)
- GVAR_FACTION_JUNKTOWN_KILLIAN (helped Killian)
- GVAR_FACTION_JUNKTOWN_SKULLZ (joined Skullz)
- GVAR_FACTION_JUNKTOWN_GIZMO (sided with Gizmo)
- GVAR_FACTION_JUNKTOWN_MILITIA (helped citizens)
- GVAR_FACTION_GUNRUNNERS (Gunrunner quest)
- GVAR_FACTION_BLADES (Blades quest)
- GVAR_FACTION_BOS (Brotherhood quest)
- GVAR_FACTION_FOLLOWERS (Followers quest)
- GVAR_FACTION_NECROPOLIS (pump repaired)

### Persistent Arrays:
- megamod_faction_npcs: {faction_id: [npc_pid, tile, map, alive]}
- megamod_npc_inventory: {npc_unique_id: [item_pid, count, ...]}
- megamod_camp: {map_index, tile, elevation}
- megamod_waiting_party: [npc_pid, map, tile, ...]

### Scripts:
- gl_radio.ssl -- radio USE handler, faction menu, summon/recall logic
- gl_camp.ssl -- camp marker creation on world map stop
- gl_faction_track.ssl -- NPC inventory + alive/dead tracking
- Modified NPC scripts: Seth, Garl, Lars, Killian, Gizmo NPCs, etc. (add unlock triggers)
- V13 citizen recruitment in gl_megamod_tools.ssl (gamemode handler)

### Implementation Phases:
1. Radio recall (waiting party members) + Camp system
2. V13 citizen recruitment (movie quotes)
3. Shady Sands faction (Seth unlock + militia summon)
4. Raiders/Khans faction
5. Junktown factions (4 paths)
6. Hub caravans (auto-trigger)
7. Boneyard (Gunrunners + Blades)
8. Brotherhood of Steel
9. Followers of the Apocalypse
10. Necropolis (Set's ghouls + Harold)
11. Inventory tracking system
12. Vincent permanent party fix
