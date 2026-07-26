# Project status -- fo1-et-tu-megamod

**As of:** 2026-07-26. This file tracks the *actual* deployed state (README is the
overview; `V6_ROADMAP.md` is the plan). When code and docs disagree, this file is
the tiebreaker for "what is really live right now".

## Environment

- Game: Fallout Et Tu **v1.16.3771** (ships sfall **4.5**), installed at
  `C:\Program Files (x86)\GOG Galaxy\Games\Fallout 2\Fallout1in2\`.
- Our scripts compiled with the sfall **4.4.7** SSL compiler
  (`KLAUDIA/megamod-build/`). Backward-compatible so far.
- Note: the game + saves were lost once (deleted folders) and reinstalled fresh.
  All code is triple-safe: disk + local git + GitHub. The megamod as a whole is not
  a single backup -- base game and third-party mods must be re-downloaded (see
  `RESTORE-INSTALL.md`).

## Active mod layer (load order)

```
fo1_mod_party_hotkeys
fo1_mod_new_start_plus
fo1_mod_outdoorsman
fo1_mod_science
fo1_mod_combatcontrol      <- new (2026-07-26)
fo1_mod_tandi              <- new (2026-07-26)
fo1_mod_party_armor_ext
fo1_mod_legendary_gear
fo1_mod_npc_party          <- 66 NPC .int, loaded LAST (wins)
```

## Done and live

| System | State | Notes |
|---|---|---|
| NPC recruit ("Join me") | LIVE | ~39 NPCs, `party_add` + `set_self_team(TEAM_PLAYER)` |
| Party follow / hold (`F`) | LIVE | toggles `GVAR_PARTY_NO_FOLLOW` |
| Party dialog + "talk about something else" | LIVE | temporary drop to original NPC dialog |
| Camp: firewood+lighter build / shovel teardown | LIVE | same camp object both ways |
| Bury corpse (shovel) + bulk bury order | LIVE | deferred-destroy; loot dropped first |
| Bed rest `K` (camp or any bed, 8h, heals) + prompt | LIVE | edge-triggered evening/night prompt |
| Auto-craft `G` (value-priority, chained) | LIVE | moved off `C` (Character screen clash) |
| General Science (use skill, once/proto, autopsy) | LIVE | `fo1_mod_science` |
| Good deeds (food/water -> XP + local rep) | LIVE | `gl_good_deeds` |
| New-start kit + motorcycle ownership | LIVE | `gl_mod_new_start_plus` |
| Party armor sprite swap | LIVE | per-FID (B2 fix) |
| **Combat control** | LIVE | `gl_partycontrol` + empty `PIDList` -> `party_member_obj` gate |
| Tandi tribal sprite (hfprim) | LIVE | `fo1_mod_tandi` (was not deployed before) |
| Curtis/Agatha tribal (hmwarr/hfprim) | LIVE | swap-once guard, no armor wipe |
| Tandi "Join me" gated on kidnapping | LIVE | `HIRELING_STATUS >= 1`, persists after return |

## Bugs fixed this cycle

- **B2 -- FID per-PID -> per-FID.** Party armor sprites map by base FID, generic for
  all party NPCs (not hardcoded per character).
- **B3 -- team reset on map change.** 18 NPCs whose `map_enter_p_proc` reset team +
  teleported home now guard `if (self_team == TEAM_PLAYER) then return;` -- recruited
  NPCs stay in party across maps. (Confirmed on Marcelle, Trish, Sinthia.)
- **Combat control leak.** Was controlling every critter on the map; root cause was
  the missing `gl_partycontrol.int` (sfall 4.5 moved this out of the engine into a
  mod). Deployed it; with an empty `PIDList` control is gated to real party members.
- **FID re-swap wiping armor.** Tribal sprite swaps ran every `map_enter`; now
  applied once / only when not in party.
- **Auto-craft key clash.** `C` opened the Character screen; moved auto-craft to `G`.

## Partial / not wired

- **Radio factions** (`gl_radio` + faction travel): source present, not in the active
  load order yet.
- **Biker gang** (extra motorcycles as decoration): design only (roadmap F2).
- **Stat boosters, mutagenic serum, firewood item chain**: roadmap, not built.

## Open issues

- **CRASH: Vault 15, old save.** Appeared after the combat-control / Tandi / FID
  deploy. Save loads fine (per `sfall-log.txt`); the crash happens during play in
  Vault 15. sfall `[Debugging] Enable=1` is set to capture the next repro. Prime
  suspects: `gl_partycontrol` (foreign pre-compiled binary), `gl_mod_tandi` (freshly
  deployed, iterates critters on every gamemode change), or old-save/global-script
  incompatibility. Not yet root-caused -- no mods were disabled (per request).

## Deferred: MyMod -> megamod migration

`KLAUDIA/OUTPUT/fallout-mods-backup/` (recovered from NordLocker) holds the original
`MyMod` package with features not yet in the megamod source:

- Custom global mods: `gl_bedsL`, `gl_peace` (talisman dispenser), `gl_PTYSHVL`
  (party shovel), `gl_raiderhunter`, `gl_reputationfix`, `gl_rope`, `gl_zippo`.
- Custom items/scripts: `TALISMAN`, `BEDL`, `ROPEL`, `RESPEC` (therapist / stat
  respec), `LIGHTER`, plus modified NPC/map scripts (`DOGMEAT` robodog, `TYCHO`,
  `SHADYWST` = Tandi kidnapping, `SHADYET`, Junktown maps, `ZAX`, `YOURROOM`).

Migration is a task on its own (each needs a `SCRIPTS.LST` entry, header porting from
`sfall_headers\` -> `sfall/`, and a recompile). Planned design notes captured:
- **rope**: sneak -> immobilize (cripple legs+arms + periodic unconscious).
- **raiderhunter**: ears only from bad random-encounter raiders (not Khans), sellable
  at ordinary non-essential Guards in Junktown and the Hub.
- **reputation fix**: must coexist with charity (level 1) + good deeds (level 2).
- **respec/respec2** = the therapist mod.
