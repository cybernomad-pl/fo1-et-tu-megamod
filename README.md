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
| `fo1_mod_party_hotkeys` | Party hold/follow toggle, FID browser, party-PID dump |
| `fo1_mod_new_start_plus` | Starting kit + motorcycle-at-start ownership |
| `fo1_mod_outdoorsman` | Camp / bedroll / bury / bed-rest / auto-craft + good deeds + party teleport |
| `fo1_mod_science` | General Science (use the skill on anything) + science crafting |
| `fo1_mod_combatcontrol` | Take control of your party members in combat (sfall `gl_partycontrol`) |
| `fo1_mod_tandi` | "Sexy tribal" Tandi (hfprim sprite) + easy main-quest helper |
| `fo1_mod_party_armor_ext` | Party members show the sprite matching their worn armor |
| `fo1_mod_legendary_gear` | Legendary / upgraded gear support |
| `fo1_mod_npc_party` | 66 NPC script overrides -- makes ~39 NPCs recruitable |

Global scripts (`gl_*.int`) auto-load from any active mod's `scripts/` folder and
run every tick / on hooks. NPC scripts (uppercase, e.g. `CURTIS.int`) override the
base critter behaviour so those NPCs gain "Join me", follow, and party-order dialog.

---

## Features

### Party system
- **Recruit almost anyone.** ~39 NPCs get a "Join me." dialog option (`party_add` +
  `set_self_team(TEAM_PLAYER)`). Some are gated -- e.g. **Tandi** only offers to join
  after she has been kidnapped by the raiders (and the option then persists).
- **Follow / hold.** `F` toggles the whole party between FOLLOW and HOLD POSITION.
- **Party dialog.** Recruited NPCs get party-order options, plus "Let's talk about
  something else." to temporarily drop back into their original (pre-recruit) dialog.
- **Combat control.** With `gl_partycontrol` installed and the sfall `PIDList` left
  empty, `AllowControl()` gates control through `party_member_obj()` -- you drive
  **only actually-recruited** party members in combat, regardless of their proto.
  Configured in `mods/sfall-mods.ini` `[CombatControl] Mode=2`.
- **Armor sprites.** `gl_party_armor_ext` swaps a party member's sprite to match the
  armor they wear (per-FID mapping, not per-character).

### Camp & survival (`gl_mod_outdoorsman`)
- **Build a camp** in the wilderness: use **firewood + lighter** to light a campfire.
- **Rest.** `K` rests -- full camp rest in the wild (packs the camp on wake), or
  sleep at any adjacent bed/mattress/bedroll anywhere (8h, heals the party). An
  on-screen prompt appears when you're hurt, it's evening/night, and a bed is near.
- **Tear down** the camp with the shovel; dropped items stay on the ground.
- **Bury the dead.** Shovel on a corpse buries it; a party order buries all corpses
  around an encounter camp (loot is dropped to the ground first).

### Crafting
- **Auto-craft** (`G`): makes everything craftable from your pack, highest in-game
  value first, max of each, chaining intermediate results. Recipes include spear
  (pole + **plain knife only**), sharpened spear (+flint), healing powder, antidote
  (Science 50+), molotov.
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
| `K` | 37 | Rest (camp in the wild, or any nearby bed) |
| `G` | 34 | Auto-craft everything possible |
| `X` | 45 | Quick exit to worldmap (encounter maps, out of combat) |
| `L` | 38 | Lighter toggle |
| `F` | 33 | Party HOLD / FOLLOW toggle |
| `[` `]` | 26 / 27 | FID browser (cycle a party member's sprite) |
| `;` | 39 | **temp/diagnostic** -- dump party protos to `party_pids.ini` |

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

- **Crash in Vault 15 on an old save** (under investigation). Appeared during the
  combat-control / Tandi / FID work. sfall debug logging is enabled to capture the
  next repro; likely suspects are the newly-added global scripts (`gl_partycontrol`,
  `gl_mod_tandi`) or old-save incompatibility.
- **Radio factions** and **biker gang** are partially implemented (source present,
  not fully wired into the active layer).
- The **talisman** item (control any critter that carries it) is not yet deployed --
  it needs a `SCRIPTS.LST` entry and a recompile of `gl_peace` / `TALISMAN` against
  our headers. Tracked as part of the MyMod -> megamod migration.

See [`docs/V6_ROADMAP.md`](docs/V6_ROADMAP.md) for the full roadmap and status.

---

## License

MIT
