# fo1-et-tu-megamod v6 Roadmap -- Issues + Refactor Plan

**Data:** 2026-04-18
**Branch:** draft -- czeka na zatwierdzenie Borysa przed `gh issue create`

## CEL

Dodać: biker gang (motocykl + trunk), stat booster items, firewood mechanic, FID mapping rewrite. Zrefaktorować istniejący kod pod kątem łatwiejszego utrzymania.

---

## STATUS OBECNY (v5.5 + iter 1 v6 deployed)

Pliki zmienione lokalnie, deployed do `Fallout1in2/mods/`, **NIE commitowane jeszcze**.

| Plik | Status | Opis |
|------|--------|------|
| `gl_party_hotkeys.ssl` | deployed | klawisz J zamiast 0 + LOG |
| `gl_mod_new_start_plus.ssl` | deployed | +6 itemów (jacket/shades/knife/3x jacket) |
| `THERESA.ssl` | deployed | Theresa14+24 Join me |
| `OFFICER.ssl` | deployed | Officer01/25/26 Join me |
| `gl_mod_outdoorsman.ssl` | deployed | diagnostic logs |
| `gl_party_armor_ext.ssl` | deployed | diagnostic logs |

---

## ISSUES DRAFT (~19)

### BUGS (4)

#### B1. Czarna mapa przy worldmap exit
- **Priorytet:** CRITICAL (bloker testów)
- **Objaw:** Gracz wychodzi z V13 Entrance → worldmap → czarna mapa, guziki nie działają.
- **Hipotezy (z diagramu `v6-logic-diagnosis.txt`):**
  - H1 [50%] outdoorsman: map_change free_array old_sleepers double-free
  - H2 [20%] party_armor_ext: LIST_CRITTERS na worldmap stale ids
  - H3 [15%] cleanup_pending garbage keys
- **Diagnostyka:** logi w 3 skryptach (deployed), czeka na test Borysa → `outdoorsman.ini`, `party_armor_ext.ini`, `party_hotkeys.ini`
- **Next:** guard `if cur_map_index < 0 return` na wejściu gamemode_handler?

#### B2. FID mapping PER-PID zamiast PER-FID
- **Priorytet:** HIGH
- **Objaw:** Theresa z armor wyświetla zły sprite (ostatnio zjebane). MEDIC leather jacket dostaje HMBLTR (black leather ARMOR) zamiast leather JACKET sprite.
- **Root cause:** `gl_party_armor_ext.ssl` dopasowuje `if (sid == SCRIPT_MEDIC)` -- tylko jedna postać, hardcoded.
- **Fix:** Rewrite per-FID input. Sprawdzać `obj_art_fid(npc)` BASE (bez armor) → mapować do armored wersji tego samego sprite setu. Generic dla wszystkich party NPC.
- **Zależności:** wymaga research w `artfid.h` + `critters.lst` (jakie sprite sety mają armored variants).

#### B3. Theresa team reset po map change
- **Priorytet:** MEDIUM
- **Objaw:** Theresa zwerbowana, wyjście z mapy, wracasz -- Theresa wróciła do TEAM_V13_REBELS (wypada z party).
- **Root cause:** `THERESA.ssl` linia 188: `set_self_team(TEAM_V13_REBELS)` w `map_enter_p_proc` BEZ guardu `if self_team == TEAM_PLAYER return` (który MA Officer linia 140).
- **Fix:** Dodać guard jak w OFFICER.ssl.

#### B4. PID_SPECTACLES (415) marked SPECIAL
- **Priorytet:** LOW (decision)
- **Objaw:** Header komentarz: `DO NOT USE THIS IN YOUR SCRIPTS, THIS IS SPECIAL CASE`.
- **Pytanie do Borysa:** używać (tylko read, żadnych create_object/set_proto_data) czy szukać alternatywy?

---

### FEATURES -- NEW MODS (6)

#### F1. fo1_mod_biker -- motocykl od startu
- **Priorytet:** HIGH
- **Zakres:**
  - NEW GAME: set `GVAR_ENABLE_MOTORCYCLE=1`, `GVAR_QUEST_MOTORCYCLE=20`, `GVAR_PLAYER_GOT_CAR=1`
  - Spawn `PID_DRIVABLE_MOTO1` + `PID_MOTO1_TRUNK` obok gracza w V13 Entrance
  - Trunk loadout: LSW, FN FAL Night Scope, Pancor Jackhammer, .223 Pistol, Desert Eagle Ext Mag, Scoped Hunting Rifle + amunicja + 4x Small Energy Cells
- **Ryzyko:** `Trunk_Ptr` import timing -- może wymagać timer delay 2s.

#### F2. fo1_mod_biker_gang -- extra motocykle decoration
- **Priorytet:** MEDIUM (zależy od F1)
- **Zakres:**
  - Wszędzie gdzie jest player's motocykl (nie tylko camp) -- spawn extra `PID_DRIVABLE_MOTO1` obok
  - Liczba: `count_party / 3` extra motocykli (co 3 party members +1)
  - Decoration only: bez trunka, gracz nie może "ukraść" (może: inny PID scenery zamiast drivable, lub set_obj_action)
- **Hook:** `map_enter_p_proc` albo `HOOK_GAMEMODECHANGE`
- **Cleanup:** gdy player motocykl znika (destroy_object), extras też
- **Research needed:** czy spawn `PID_DRIVABLE_MOTO1` w random miejscu triggers script? czy script reaguje na use_obj_on?

#### F3. fo1_mod_stat_boosters -- 7 unique items → +3 stat w inv
- **Priorytet:** MEDIUM
- **Matrix finalna (Borys 2026-04-18):**

  | Stat | Item | PID | Bonus |
  |------|------|-----|-------|
  | STR | Boxing Gloves | 292 | +3 |
  | PER | Spectacles | 415 ⚠️ | +3 |
  | END | Angel Eye Dog Tag | 442 | +3 |
  | CHA | Mirror Shades | 433 | +3 |
  | INT | Technical Manual | 228 | +3 |
  | AG | Bio Gel | 440 | +3 |
  | LCK | Talisman | 309 | +3 |

- **Działa na NPC?**
  - TAK: HP_max, AP, carry weight, melee damage bonus, AC, PER sighting range
  - NIE: Skills (wyliczone raz przy character creation, nie dynamic)
- **Mechanizm:** co tick lub HOOK_INVENTORY_MOVE -- sprawdź kto ma jakie itemy, `set_critter_stat(obj, STAT_X, base + bonus)`
- **Reset:** gdy item wychodzi z inv, `set_critter_stat(obj, STAT_X, base)` -- track base w save_array

#### F4. fo1_mod_firewood -- lighter + firewood mechanic
- **Priorytet:** MEDIUM
- **Zakres:**
  - USE lighter w wilderness: sprawdź inv na firewood
  - BRAK firewood: lighter tylko świeci (tak jak teraz)
  - MA firewood: create campfire + consume 1 firewood
- **Zmiana:** refaktor `handle_lighter` w outdoorsman -- dodać firewood check
- **Nowy PID:** brak vanilla "firewood" PID → trzebaby stworzyć custom proto, ALBO użyć istniejącego (PID_WOOD? PID_MONUMENT_CHUNK? junk itemy)
- **Research:** sprawdź czy jest PID_WOOD lub podobny w itempid.h

#### F5. fo1_mod_firewood -- random spawn in wilderness
- **Priorytet:** MEDIUM (zależy od F4)
- **Zakres:**
  - `map_enter_p_proc` lub `HOOK_GAMEMODECHANGE`
  - Jeśli wilderness + brak spawned jeszcze: spawn 3-8 firewood w sensownych miejscach (pod drzewami, przy skałach, w krzakach)
  - Sensowne miejsca: iter LIST_SCENERY, find tree/rock tiles, adjacent empty tile → spawn
- **Ryzyko:** performance (iter scenery na każdej mapie)

#### F6. fo1_mod_mutagenic -- serum → mutant transformation
- **Priorytet:** LATER (po v6 core)
- **Zakres (draft):**
  - Gracz lub NPC pije `PID_MUTAGENIC_SYRUM` (329, już w headerze!)
  - Animation + transformacja → Super Mutant sprite
  - Stats boost (+ST +EN -CHA -INT)
  - Joint work z armor mapping (mutant sprite set osobny)
- **Uwaga:** nie teraz, tylko notka w roadmapie

---

### DONE (iter 1 v6, already deployed -- do commitowania)

- **D1.** Klawisz J zamiast 0 (konflikt party_orders.ini SwitchKey=11)
- **D2.** Theresa14+24 Join me zawsze
- **D3.** Officer01/25/26 Join me zawsze
- **D4.** New start items expansion (combat jacket/shades/knife + 3x leather jacket)
- **D5.** Diagnostic logging w 3 skryptach (outdoorsman, party_armor_ext, party_hotkeys)

---

### REFACTORS (4)

#### R1. Extract FID constants → `fid_mapping.h`
- Obecnie FID_HMBJMP/HMBLTR/HMBMET/etc. hardcoded w `gl_party_armor_ext.ssl`
- Nowy header `ssl/headers/fid_mapping.h` z:
  - Wszystkie black male FIDs
  - Wszystkie female FIDs (do mapowania Theresa)
  - Standard sprite set tables
- Zależność: blokuje B2 (FID mapping rewrite)

#### R2. party_armor_ext table-driven per-FID mapping
- Zamiast if/else chain:
  ```
  #define MAP_ROW(base, armor_type, target) ...
  static const mapping_table = [
    {HFPUNK,  LEATHER,  HFPUNK_LEATHER},
    {HFPUNK,  METAL,    HFPUNK_METAL},
    ...
  ]
  ```
- SSL nie wspiera static tables, ale można use array_map initialized w start()
- Generic lookup: `lookup_armored_fid(base_fid, armor_type)`

#### R3. outdoorsman section reorganization
- 724 linie w 1 pliku -- trudne do nawigacji
- Refactor: dodać ASCII separators + TOC comment na górze
- Grupowanie procedur logicznie: STATE / CAMPFIRE / BEDROLL / WAKE / CLEANUP
- Extract shared types do `outdoorsman.h` header
- Wymaga: test regression (724 linii = dużo logiki)

#### R4. Common patterns doc
- `last_map_idx` tracking pattern w 3 skryptach (outdoorsman, party_armor_ext, party_hotkeys)
- LOG macro pattern (każdy ma swój ini)
- Nowy dokument `docs/PATTERNS.md` z conventions

---

### INFRASTRUCTURE (2)

#### I1. Auto-sync SSL repo → megamod-build/source
- Aktualnie: manualne `cp` przed `compile_all.sh`
- Fix: skrypt `sync_and_build.sh` w repo który:
  - Znajdzie zmienione SSL (via git diff lub mtime)
  - Kopiuje do build dir
  - Odpala compile
  - Deployuje do game dir
- Zależność: zero, można dodać

#### I2. CI workflow -- compile test
- Opcjonalnie: GitHub Action odpalający compile_all.sh na PR
- Wymaga: compile.exe na Linux runnerze? Może używać Wine lub stałej Win VM
- Priorytet: LOW

---

## GRUPOWANIE -- KOLEJNOŚĆ WYKONANIA

**Faza 1 -- stabilizacja (blokery):**
1. B1 (czarna mapa) -- czekam na ini log od Borysa
2. B3 (Theresa team reset) -- prosty patch, 1 linia
3. D1-D5 commit na main (iter 1 done)

**Faza 2 -- FID mapping (architecture fix):**
4. R1 (fid_mapping.h extract)
5. B2 + R2 (per-FID rewrite w party_armor_ext)

**Faza 3 -- new features:**
6. F3 (stat boosters) -- nowy mod, izolowany
7. F4 + F5 (firewood) -- refactor outdoorsman + nowy mod
8. F1 (motocykl biker) -- research trunk + implement
9. F2 (biker gang extras) -- po F1

**Faza 4 -- cleanup:**
10. R3 (outdoorsman reorganization) -- po F4 refactor (już będzie tknięte)
11. R4 + I1 (docs + build scripts)
12. F6 (mutagenic) -- LATER

**Faza 5 -- post-v6:**
- I2 (CI) opcjonalnie

---

## PYTANIA DO BORYSA PRZED `gh issue create`

1. **Spectacles (415) SPECIAL warning** -- używać mimo tego? (mój mod tylko czyta inv, nie dotyka proto)
2. **Firewood item** -- użyć istniejący PID (który?) czy stworzyć nowy proto? (custom proto wymaga pliku .pro w mods/)
3. **Biker gang decoration** -- drivable PID_DRIVABLE_MOTO1 (gracz może "ukraść") czy szukamy scenery-only PID (może `PID_CAR_WRECK_1` jako martwy moto sprite)?
4. **CHA bonus** -- Mirror Shades +7 czy +3? (wcześniej +7, teraz tabelka ma +3 -- konsystencja?)
5. **Workflow** -- czy chcesz widzieć każdy issue w GH PRZED implementacją, czy mam tworzyć issues + od razu leciec w wykonanie?

---

## PROPOZYCJA MERGE STRATEGY

- **Feature branch per phase**, nie per issue (byłoby ~19 branchy, overhead)
- `wrk1/v6-phase1-stabilization` → PR → merge
- `wrk1/v6-phase2-fid-rewrite` → PR → merge
- `wrk1/v6-phase3-new-features` → PR → merge
- Per-PR: compile test + krótka demo w opisie PR

Alternatywa: per-issue branch, ale wtedy 19 PRs -- dużo overhead review.
