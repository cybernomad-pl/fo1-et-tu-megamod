# Custom NPC Ideas (Borys 2026-04-18)

Notatki do przyszłych modów. Nie implementujemy teraz.

---

## 1. Zax (AI) in Robot Body

- **Sprite:** `FID_MAROBO` (16777269) -- art_idx 66 (combat robot / robot bojowy)
- **Concept:** Zax to supercomputer AI z Glow / Sierra Army Depot. Przeniesiony
  do ciała robota bojowego i rekrutowalny jako party member.
- **Dialog hooks:** mógłby komentować technologie, hakować terminale, czytać
  holodyski automatycznie.
- **Questline idea:** znajdź Zaxa w Glow → quest z przeniesieniem core → free floating robot companion.

---

## 2. Gecko Evolution System (Pokemon-style)

Random encounter w wilderness: martwa matka silver gecko + cracked eggs
+ grupa martwych tribal hunters. Gracz może zabrać **1 całe jajo**.
Jajo ewoluuje w party wraz z czasem / walkami / karmieniem.

### Evolution chain

```
 idx | sprite       | stadium        | nazwa robocza
-----|--------------|----------------|--------------------
 58  | MADEGG       | EGG            | Unhatched gecko egg
 67  | (small)      | BABY           | Baby gecko (~hatched)
 68  | MAGCKO       | GOLDEN         | Golden gecko (adult)
 81  | MAFIRE       | FIRE           | Fire gecko (evolved adult)
```

### Mechanika

- **Spawn:** random encounter w desert/wilderness z 30% szansą; fixed layout:
  - Martwa silver gecko matka (MAGKO2, idx 67? check)
  - 3-5 cracked eggs (scenery)
  - 2-4 martwi tribal hunters (NFPRIM/NMWARR)
  - 1 całe jajo (lootable item)
- **Hatching:** w inv gracza tick co N game_hours; po 48h egg → baby.
- **Feeding:** party_member_feed (carry meat, consume N per day).
- **Combat XP:** każda walka z party member → +progress.
- **Evolve triggers:** progress > threshold → baby → golden → fire.

### Pokerelated expansion

Borys wspomniał że inne zwierzęta też mogły by mieć evolution chains:
- Rat → big rat → mole rat
- Scorpion → big scorpion → fire scorpion  
- Brahmin (?)
- Deathclaw baby → adult → baby matriarch

---

## 3. Implementation notes

- **Egg as item:** nowy proto `.pro` file potrzebny (item PID), plus inv art
  (.FRM). Może użyć zapamiętany idx 58 (MADEGG).
- **Party member evolve:** `art_change_fid_num(obj, FID)` po progress check.
- **State tracking:** `save_array("pet_evolution")` per pet obj_id:
  - `stage` (egg/baby/golden/fire)
  - `hatch_time`, `feed_count`, `combat_xp`
- **Dialog:** każde stadium ma inny dialog (baby = bark, golden = growl, fire = fire roar).

---

## 4. Deferred items

- Detailed combat stats per evolution stage
- Art assets for baby gecko (if idx 67 not visible as intended)
- Integration with outdoorsman camp system (pet sleeps near campfire?)

---

## Priorities

1. **v6 stabilise** (current work) -- FID mapping, save/load, camp
2. **v7 stat boosters + firewood** -- #13, #14, #15
3. **v8 biker gang** -- #11, #12
4. **v9 Zax robot + Gecko evolve** -- new custom NPC mod
