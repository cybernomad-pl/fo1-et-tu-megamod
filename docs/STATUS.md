# Project status -- fo1-et-tu-megamod

**As of:** 2026-08-10. This file tracks the *actual* deployed state (README is the
overview; `V6_ROADMAP.md` is the plan). When code and docs disagree, this file is
the tiebreaker for "what is really live right now".

## Environment

- Game: Fallout Et Tu **v1.16.3771** (ships sfall **4.5**), installed at
  `C:\Program Files (x86)\GOG Galaxy\Games\Fallout 2\Fallout1in2\`.
- Our scripts compiled with the sfall **4.4.7** SSL compiler
  (`KLAUDIA/megamod-build/`). Backward-compatible so far.
- Game settings: 1920x1080 (f2_res), combat speed max + player speedup,
  vanilla 4 AP inventory (InventoryAPcosts disabled).
- Invasion timers (`config/fo1_settings.ini`): **all towns = day 1**;
  Vault 13 = 0 (its timer ends the game -- see known issue on the 0-guard).

## Active mod layer (load order)

```
fo1_mod_party_hotkeys      <- F hold/follow, G GO-order, FID browser
fo1_mod_new_start_plus
fo1_mod_outdoorsman        <- v5: firewood-gated camp, forage, natural heal
fo1_mod_science
fo1_mod_emergency_radio
fo1_mod_combatcontrol      <- AllowControl: TEAM_PLAYER + party PID (Victor fix)
fo1_mod_tandi
fo1_mod_party_armor_ext
fo1_mod_legendary_gear
fo1_mod_npc_party          <- 68 NPC .int (66 + GENSKULZ + GUNRNR), wins
fo1_mod_invasion           <- live invasion: 6 map overrides + INVADER/SCSUPMUT
```

Disabled: `fo1_mod_car_encounter` (bike duplicates -- redesign pending),
`fo1_mod_raiders_garage` (dropped), `InventoryAPcosts` (vanilla 4 AP wanted).

## Done and live (2026-08 session on top of the 2026-07 base)

| System | State | Notes |
|---|---|---|
| **Live invasion: Shady Sands** | LIVE, tested | invaders hunt civilians (attack() from INVADER script); Seth & co. fight back |
| **Live invasion: Boneyard** | LIVE, untested | LAADYTUM/LAGUNRUN/FOLLMAP/LABLADES kills cut (65 incl. Gabriel); SCSUPMUT hunts |
| Companion executions cut | LIVE | `check_invasion_party_waiting` (killed Ian/Tycho/Katja/Tandi/Vasquez by PID) cut from 6 maps |
| Quest NPC protection | LIVE | Ian/Tandi/Aradesh excluded from hunt + battle kick |
| **Garrison order** | LIVE | "Wait for me here until I say otherwise." -- out of party, player's side, waits; "Fall in" returns; survives save/load |
| **GO order (`G`)** | LIVE | party runs to `tile_under_cursor`, spiral spread, then holds; F recalls |
| Skulz recruitable (GENSKULZ) | LIVE | vanilla invasion kill_critter also removed |
| Gun Runners recruitable (GUNRNR) | LIVE | faction-hostility guards for recruits |
| Party team repair | LIVE | engine party member without TEAM_PLAYER -> team restored on map enter (fixes save drift) |
| RAIDPRIS (raider prisoner) recruit | LIVE | team reset on load + 2 quest-despawns guarded |
| Outdoorsman v5 | LIVE | camp requires firewood (ground > pack > forage w/ Outdoorsman check, once/map); K = 7h natural-rate heal at own camp only; bed rest & auto-craft removed |
| Re-camp without farming | LIVE | "Set up camp again" after the one scout/loot per map |
| Motorcycle trunk fix | LIVE | trunk 1 hex SE of the bike (Blocking_Cycle hex-drift fixed); encounter spawn disabled |
| Build env repair | DONE | sfall `main.h` restored (GetConfig*), gl_partycontrol compiles in-pipeline again |

Earlier base (2026-07): recruit ~39 NPCs, party dialog + camp actions, combat
control, armor sprites, General Science, good deeds, new-start kit + motorcycle,
Tandi content -- see git history / diagrams.

## Open issues

- **Hub maps** still call `check_invasion_party_waiting` (companion PID kills) --
  cut planned together with the Loxley/Decker survival override (Hub stays fallen
  otherwise, by design; TROY escort scene active).
- **Junktown JUNK\* maps** not audited for invasion kills yet.
- **`gl_pipboytimer` upstream: no ==0 guard** in `check_invasions` -- V13 timer must
  stay untouched until that chain is traced.
- **Rope consumed on spear craft** -- reported once, not found in code; needs repro.
- **GO moves garrisoned NPCs** -- accepted as garrison repositioning; gate if needed.
- **CRASH: Vault 15, old save** -- still open, sfall debug on.

## Next up (task queue)

1. Junktown map audit + Hub Loxley/Decker survival (cut their PIDs from kill lists).
2. Emergency radio: INVASION broadcast when <7 days to invasion.
3. Alternate start: Shady Sands spawn + Overseer intro + bike waiting + day-1 timer.
4. Motorcycle on encounter maps -- redesign from engine behavior up.

## Deferred: MyMod -> megamod migration

Unchanged -- see git history of this file (gl_bedsL, gl_peace/TALISMAN, gl_PTYSHVL,
gl_raiderhunter, gl_reputationfix, gl_rope, gl_zippo, RESPEC, robodog).
