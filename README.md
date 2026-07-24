# fo1-et-tu-megamod

Megamod for Fallout Et Tu v1.15+ (Fallout 1 on Fallout 2 engine).
By cybernomad.pl

## Modules

### General Science
Use Science on anything, once. Learn something. Get XP.
You're fresh out of Vault 13 -- everything is new.

### General Repair
Use Repair on select items to convert them.
Ammo FMJ -> AP with pliers. Improvise with what you have.

### Radio
Carry a radio. Call in allies from factions you've helped.
They stay until you dismiss them.

Factions:
- Brotherhood of Steel -- power armor patrol, Paladin elite
- Necropolis -- ghoul patrol, Seth the Ranger
- Blades (LA Boneyard) -- knife fighters, leather armor
- Shady Sands -- militia, Tandi
- Skulz (Junktown) -- gang muscle, Vinnie
- Harold -- Harold

Each faction has a favor system. Do them a service, they send better people with better gear.

## Prerequisites

**Step 0 -- get Fallout Et Tu.** This megamod is a layer ON TOP of Et Tu. Without
it there is no `mods/` folder and nowhere to install anything. Et Tu is a total
conversion and requires BOTH base games to be installed.

### 1. Fallout 1 -- required (content source)
Retail / GOG / Steam. Et Tu pulls assets from its `MASTER.DAT` and `CRITTER.DAT`.
Install anywhere -- the Et Tu installer will ask for the path.

### 2. Fallout 2 -- required (engine)
Retail / GOG / Steam. Et Tu installs into this game's folder and creates
`Fallout1in2/`.

### 3. Fallout Et Tu (Fallout 1 in Fallout 2 engine)
By the Rotators Collective. Free.

| Source | URL |
|---|---|
| **Official -- GitHub releases** | https://github.com/rotators/Fo1in2/releases |
| Repo / docs | https://github.com/rotators/Fo1in2 |
| Mirror -- No Mutants Allowed | https://www.nma-fallout.com/resources/fallout-et-tu.124/ |
| Mirror -- Nexus Mods | https://www.nexusmods.com/fallout2/mods/42 |
| Mirror -- FODev | https://fodev.net/files/fo2/fo1in2.html |

Latest release at time of writing: **v1.16.3771** (ships sfall 4.5).

Result: `<Fallout 2>/Fallout1in2/` containing a `mods/` folder. That folder is
the install target for everything below.

### Version note
This megamod's scripts are compiled with the sfall **4.4.7** SSL compiler and
target Et Tu **v1.15.3735+** / sfall 4.4.4+. Et Tu 1.16 ships sfall 4.5, which
is newer than what we build against. sfall is generally backward compatible, but
if upstream changed base scripts, our NPC overrides may need a rebuild
(`compile_all.sh`).

## Install
1. Complete the Prerequisites above -- `Fallout1in2/mods/` must exist
2. Extract `fo1_megamod/` into your Et Tu `mods/` folder
3. Add `fo1_megamod` to `mods/mods_order.txt`
4. Edit `mods/fo1_megamod/config/megamod.ini` to toggle modules

## Status
Work in progress. Not playable yet.

## License
MIT
