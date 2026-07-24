# Odtworzenie instalki megamodu -- 2026-07-24

Spisane z katalogu gry ZANIM zostal skasowany. Ten plik jest jedynym
zapisem faktycznego ukladu modow (backup `mods_order.txt` byl przestarzaly
-- konczyl sie na starym, scalonym `fo1_megamod`).

## Stan wyjsciowy (potwierdzony na dysku)

- `C:\Program Files (x86)\GOG Galaxy\Games\Fallout`   -- MASTER.DAT, CRITTER.DAT  [OK]
- `C:\Program Files (x86)\GOG Galaxy\Games\Fallout 2` -- FalloutClient.exe, critter.dat, data  [OK]
- `Fallout1in2`  -- **BRAK**, do zainstalowania (konwersja Et Tu)

## KROK 1 -- zainstalowac Fallout Et Tu

Tworzy folder `Fallout1in2` wewnatrz katalogu Fallout 2.
Instalator pyta o sciezke do FO1 (ciagnie content z MASTER.DAT/CRITTER.DAT).
Bez tego kroku nie ma katalogu `mods/`, wiec nie ma gdzie wgrac reszty.

## KROK 2 -- mody AKTYWNE (kolejnosc z mods_order.txt)

Legenda:  [3P] = third-party, do sciagniecia z sieci
          [MY] = nasze, odtwarzalne z repo/zrodel

```
 1. fo1_base                          [3P]  (czesc Et Tu)
 2. fo1_interface                     [3P]
 3. Anim_ImprovedSMutantDeath         [3P]
 4. fo1_alternative_Junktown_Endings  [3P]
 5. fo1_alternative_forcefields       [3P]
 6. fo1_barter_formula                [3P]
 7. fo1_ShadyTrader                   [3P]
 8. fo1_KenjiRearmed                  [3P]
 9. InterfaceUpscaled                 [3P]
10. fo1_footprints                    [3P]
11. fo1_tycho_as_ranger               [3P]
12. fo1_maps_mountains                [3P]
13. fo2tweaks                         [3P]
14. InventoryFilter.dat               [3P]
15. F2MechanicsMiniRework             [3P]
16. fo1_big_party_mod                 [3P + NASZE PRZEROBKI NPC]
17. fo1_mod_party_hotkeys             [MY]
18. fo1_mod_new_start_plus            [MY]
19. fo1_mod_outdoorsman               [MY]
20. fo1_mod_party_armor_ext           [MY]
21. fo1_mod_legendary_gear            [MY]
```

## KROK 3 -- mapowanie: ktory nasz .int gdzie ladowal

Odczytane z zywej instalki przed skasowaniem.

```
fo1_mod_outdoorsman/scripts/     gl_mod_outdoorsman.int
                                 gl_good_deeds.int
                                 gl_party_teleport.int
fo1_mod_party_hotkeys/scripts/   gl_party_hotkeys.int
fo1_mod_party_armor_ext/scripts/ gl_party_armor_ext.int
fo1_mod_legendary_gear/scripts/  gl_mod_legendary_gear.int
fo1_mod_new_start_plus/scripts/  gl_mod_new_start_plus.int
fo1_tycho_as_ranger/scripts/     gl_tycho_ranger.int
```

### Mody OBECNE ale NIEAKTYWNE (byly na dysku, nie ma ich w mods_order.txt)

Lezaly w `mods/` ale silnik ich NIE ladowal. Odtwarzac tylko jesli chcesz
je wlaczyc -- wtedy trzeba je tez dopisac do `mods_order.txt`.

```
fo1_mod_tools/scripts/     gl_megamod_tools.int, gl_mod_lighter.int,
                           gl_mod_multitool.int, gl_mod_rope.int,
                           gl_mod_shovel.int
fo1_mod_medic/scripts/     gl_mod_medic.int
fo1_mod_radio/scripts/     gl_radio.int
fo1_mod_science/scripts/   gl_general_science.int
fo1_mod_tandi/scripts/     gl_mod_tandi.int
fo1_mod_upgrade/scripts/   gl_mod_upgrade.int
fo1_junk_farmer/scripts/   gl_junk_farmer.int
fo1_v15_rework/scripts/    gl_v15_map_enter.int
fo1_new_premades/scripts/  gl_fo1_new_premade.int
fo1_CorpseDrop/scripts/    gl_corpse_drop_ettu.int
```

## KROK 4 -- przywrocenie naszej warstwy (robi Klaudia)

Zrodla: `KLAUDIA/megamod-build/ssl/source` (92 x .ssl) -- kazdy .int
mozna przekompilowac od zera przez `compile_all.sh`.
Repo/GitHub: `cybernomad-pl/fo1-et-tu-megamod` (101 .ssl + 75 .int).

## KROK 5 -- przywrocenie configow

Z `KLAUDIA/fallout-ettu-config/`:
`ddraw.ini`, `f2_res.ini`, `Fallout2.cfg`, `fo1_settings.ini`,
`sfall-mods.ini`, `party_armor.ini`, `Perks.ini`, `Skills.ini`
oraz `mods_order-AKTUALNY-2026-07-24.txt` -> jako `mods/mods_order.txt`

## CZEGO NIE MA I NIE BEDZIE

Save'y. Lezaly w katalogu gry, nie ma ich w koszu ani w AppData.
Przepadly. Nowa gra od zera.
