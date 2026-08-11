# fo1-et-tu-megamod

Megamod for **Fallout Et Tu** (Fallout 1 running on the Fallout 2 engine).
By [cybernomad.pl](https://cybernomad.pl).

A gameplay-expansion layer that turns Fallout 1 into a party-driven survival RPG:
recruit almost anyone, run a camp in the wilderness, control your squad in combat,
craft from what you scavenge, and grow a reputation through small good deeds.

> **Status:** playable / active development. The party, camp, crafting, combat-control
> and NPC systems are deployed and working in-game. Some features are partial (Radio
> factions, biker gang) and there is one open crash under investigation -- see
> [Known issues](#known-issues).

---

## What it is (architecture)

Et Tu is a total conversion that installs into a Fallout 2 folder and creates
`Fallout1in2/`. Everything here is a **layer on top of Et Tu**, shipped as a set of
sfall "mods" loaded via `Fallout1in2/mods/mods_order.txt`. Nothing overwrites the
base game -- last mod in the load order wins.

Our layer (load order, top to bottom):

| Mod folder | What it adds |
|---|---|
| `fo1_mod_party_hotkeys` | Party hold/follow (`F`), GO order (`G`), FID browser |
| `fo1_mod_new_start_plus` | Starting kit + motorcycle-at-start ownership |
| `fo1_mod_outdoorsman` | Camp (firewood-gated) / forage / bury + good deeds + party teleport |
| `fo1_mod_science` | General Science (use the skill on anything) + science crafting |
| `fo1_mod_emergency_radio` | Use a radio to call location/reputation-based reinforcements |
| `fo1_mod_combatcontrol` | Take control of your party members in combat (sfall `gl_partycontrol`) |
| `fo1_mod_tandi` | "Sexy tribal" Tandi (hfprim sprite) + easy main-quest helper |
| `fo1_mod_party_armor_ext` | Party members show the sprite matching their worn armor |
| `fo1_mod_legendary_gear` | Legendary / upgraded gear support |
| `fo1_mod_npc_party` | 68 NPC script overrides -- recruitable NPCs incl. Skulz + Gun Runners |
| `fo1_mod_invasion` | **Live invasion**: towns stay alive, invader mutants hunt civilians, city battles |

Disabled (kept in repo): `fo1_mod_car_encounter` (bike spawn on encounter maps --
duplicate-spawn issues, to be redesigned), `fo1_mod_raiders_garage` (dropped).

Global scripts (`gl_*.int`) auto-load from any active mod's `scripts/` folder and
run every tick / on hooks. NPC scripts (uppercase, e.g. `CURTIS.int`) override the
base critter behaviour so those NPCs gain "Join me", follow, and party-order dialog.

---

## Features

### Party system
- **Recruit almost anyone.** 40+ NPCs get a "Join me." dialog option (`party_add` +
  `set_self_team(TEAM_PLAYER)`) -- including generic **Skulz** gangers (Junktown) and
  **Gun Runners** (Boneyard) for army-building. Some are gated -- e.g. **Tandi** only
  offers to join after she has been kidnapped by the raiders.
- **Follow / hold.** `F` toggles the whole party between FOLLOW and HOLD POSITION.
- **GO order.** `G` sends the whole party running to the hex under your mouse cursor
  (spiral spread around the target); they hold position there. `F` recalls.
- **Garrison.** "Wait for me here until I say otherwise." -- the NPC leaves the party
  (does not travel with you), stays on the map on your side, fights your enemies and
  waits. "Fall in. We're moving out." brings them back. Survives save/load.
- **Party dialog.** Recruited NPCs get party-order options, plus "Let's talk about
  something else." to temporarily drop back into their original (pre-recruit) dialog.
- **Combat control.** With `gl_partycontrol` installed and the sfall `PIDList` left
  empty, `AllowControl()` gates control through `party_member_obj()` -- you drive
  **only actually-recruited** party members in combat, regardless of their proto.
  Configured in `mods/sfall-mods.ini` `[CombatControl] Mode=2`.
- **Armor sprites.** `gl_party_armor_ext` swaps a party member's sprite to match the
  armor they wear (per-FID mapping, not per-character).

### Live invasion (`fo1_mod_invasion`)
- Invasion timers set to **day 1** for every town (`config/fo1_settings.ini`;
  Vault 13 stays off -- its timer ends the game).
- **Towns stay alive.** All vanilla kill paths are neutralized: the
  `invasion_kill_critter` macro is a no-op in our NPC overrides, and the map-script
  mass-kills (`kill_critter_type` lists + `check_invasion_party_waiting`, which
  executed Ian/Tycho/Katja/Tandi/Vasquez by PID) are cut from overridden maps:
  SHADYET, SHADYWST, LAADYTUM, LAGUNRUN, FOLLMAP, LABLADES.
- **Invaders hunt.** Overridden invader scripts (INVADER, SCSUPMUT) make each mutant
  attack the nearest civilian on its own (`attack()` from the critter's script forces
  combat) -- city battles start around you without your input. Your party is never
  auto-targeted. Quest NPCs (Ian, Tandi, Aradesh) are excluded from the hunt.
- The **Hub stays fallen** (by design); a TROY "join or die" escort scene runs there.
- Team repair: any engine-registered party member that lost `TEAM_PLAYER` (vanilla
  script drift) gets it back on map enter.

### Camp & survival (`gl_mod_outdoorsman`)
- **Build a camp** in the wilderness -- **requires firewood**: firewood on the ground
  next to you, or in your pack, + a lighter (`L` or USE firewood).
- **Forage.** `L` with no firewood searches the area (1h game time): an Outdoorsman
  check -- fail finds nothing (retry allowed), success gives firewood, high margins
  add an iguana / flint. One successful forage per map.
- **Rest.** `K` rests 7h at your own camp only (packs the camp on wake). Healing is
  the **natural rate** (Healing Rate per 3h, FO2 mechanics) -- no more free full
  heal. Bed rest is disabled (owned beds planned as a separate mod).
- **Tear down** the camp with the shovel; dropped items stay on the ground.
- **Bury the dead.** Shovel on a corpse buries it; a party order buries all corpses
  around an encounter camp (loot is dropped to the ground first).
- **No item farming on re-camp**: scout+camp (with loot) works once per map; after
  that "Let's set up camp again" rebuilds the camp with no loot.

### Crafting
- **Party dialog crafting only** (the auto-craft hotkey was removed): spear
  (pole + knife), sharpened spear (+flint), healing powder, antidote (Science 50+),
  molotov -- via "Got any ideas we can put together?".
- **Science crafting** (`fo1_mod_science`): use Science on a scorpion tail while
  carrying booze -> antidote (skill check).

### Skills & reputation
- **General Science.** Use the Science skill on anything once per proto -> an
  observation + XP. Dead critters -> autopsy (skill check) revealing weak spots.
- **Good deeds** (`gl_good_deeds`): give food/water to the poor -> XP + local
  reputation.

### New game start (`fo1_mod_new_start_plus`)
- Starting kit: water chip, electronic lockpicks, radio, gang wear (cured leather
  armor, shades, combat knife) + 3 spare leather jackets.
- Motorcycle from the start: sets `GVAR_PLAYER_GOT_CAR` / `GVAR_CAR_CUR_MAP` so the
  bike rides the worldmap with you (requires `GVAR_ENABLE_MOTORCYCLE=1`).

### NPC visuals
- **Tribal sprites**: Curtis (hmwarr), Agatha (hfprim), Tandi (hfprim). The sprite
  is applied once and no longer re-swapped every map enter, so it no longer wipes a
  recruited member's armor sprite.

### Radio factions (partial)
Carry a radio, call in allies from factions you've helped (Brotherhood, Necropolis,
Blades, Shady Sands, Skulz, Harold). Favor-based: help a faction, they send better
people with better gear. **Source present, not fully wired into the active layer yet.**

---

## Hotkeys

| Key | Scancode | Action |
|---|---|---|
| `K` | 37 | Rest 7h at your camp (natural healing rate; packs camp on wake) |
| `G` | 34 | **GO order** -- party runs to the hex under the cursor, then holds |
| `X` | 45 | Quick exit to worldmap (encounter maps, out of combat) |
| `L` | 38 | Lighter; in the wild: camp if firewood (ground/pack), else forage |
| `F` | 33 | Party HOLD / FOLLOW toggle (also recalls after GO) |
| `[` `]` | 26 / 27 | FID browser (cycle a party member's sprite) |

Vanilla Et Tu hotkeys (Character `C`, Inventory `I`, Pipboy `P`, Skilldex `S`,
attack `A`, automap `TAB`) are avoided.

---

## Prerequisites

**Step 0 -- get Fallout Et Tu.** This megamod is a layer ON TOP of Et Tu. Without it
there is no `mods/` folder and nowhere to install anything. Et Tu is a total
conversion and requires BOTH base games installed.

1. **Fallout 1** -- required (content source). Retail / GOG / Steam. Et Tu pulls
   assets from its `MASTER.DAT` / `CRITTER.DAT`.
2. **Fallout 2** -- required (engine). Et Tu installs into this game's folder and
   creates `Fallout1in2/`.
3. **Fallout Et Tu** by the Rotators Collective (free):

| Source | URL |
|---|---|
| **Official -- GitHub releases** | https://github.com/rotators/Fo1in2/releases |
| Repo / docs | https://github.com/rotators/Fo1in2 |
| Mirror -- No Mutants Allowed | https://www.nma-fallout.com/resources/fallout-et-tu.124/ |
| Mirror -- Nexus Mods | https://www.nexusmods.com/fallout2/mods/42 |
| Mirror -- FODev | https://fodev.net/files/fo2/fo1in2.html |

Latest release at time of writing: **v1.16.3771** (ships sfall 4.5). Result:
`<Fallout 2>/Fallout1in2/` with a `mods/` folder -- the install target below.

A full restore procedure (after a fresh Et Tu install) lives in
[`docs/RESTORE-INSTALL.md`](docs/RESTORE-INSTALL.md).

---

## Install

1. Complete the Prerequisites -- `Fallout1in2/mods/` must exist.
2. Copy each `fo1_mod_*` folder into `Fallout1in2/mods/`.
3. Add the mod names to `mods/mods_order.txt` in the load order shown above
   (see [`docs/mods_order-*.txt`](docs/) for a known-good snapshot).
4. Configuration lives in the game's `mods/sfall-mods.ini` and
   `config/fo1_settings.ini` (combat control, motorcycle, etc.). Snapshots are kept
   in [`docs/config-snapshots/`](docs/config-snapshots).

---

## Build & deploy

Scripts are written in sfall SSL and compiled with the sfall **4.4.7** compiler.

- Source: `KLAUDIA/megamod-build/ssl/source/` (`*.ssl`, headers, `SCRIPTS.LST`).
- `gl_*.ssl` -> `out/`. Uppercase NPC `*.ssl` -> **`out/invasion/`** (important --
  the NPC `.int` the game loads come from `out/invasion/`, not `out/`).
- Deploy target: `Fallout1in2/mods/<mod>/scripts/`.
- Repo mirror of compiled scripts: `fo1_megamod/scripts/`.

Every code commit ships an ASCII change diagram in
[`docs/diagrams/`](docs/diagrams) describing the logic/flow that changed.

---

## Known issues

- **Hub maps still call `check_invasion_party_waiting`** -- entering the Hub with
  companions NOT in your party kills them by PID. Will be cut together with the
  planned Loxley/Decker survival override.
- **Junktown map scripts** not yet audited for invasion kills (critter overrides are
  protected; JUNK* maps pending).
- **`gl_pipboytimer` (upstream) has no timer==0 guard** in `check_invasions` --
  do not flip `GVAR_VAULT_13_INVASION_DAYS` from 0 without tracing that chain
  (a 0 timer would read as "already invaded" -> endgame movie).
- **Rope consumed on spear craft** reported once -- no such consumption exists in
  the crafting code; awaiting a repro with details.
- **GO also moves garrisoned NPCs** -- treated as a feature (garrison repositioning
  without rejoin); gate it if unwanted.
- **Crash in Vault 15 on an old save** (under investigation, sfall debug enabled).
- **Radio factions** and **biker gang** are partially implemented (source present,
  not fully wired into the active layer).
- The **talisman** item (control any critter that carries it) is not yet deployed.

See [`docs/V6_ROADMAP.md`](docs/V6_ROADMAP.md) for the full roadmap and status.

---

## License

MIT
