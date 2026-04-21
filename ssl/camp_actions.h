/*
    camp_actions.h -- INLINE dialog action macros for fo1-et-tu-megamod.

    Vanilla FO1/2 pattern: dialog target procedures execute actions INLINE
    (see ARADESH.ssl:798 aradesh40, RAZLO.ssl:295 razlo06nb, MICHAEL.ssl:961).
    No cross-script messaging.

    MUST be included AFTER command.h (needs get_team, dude_obj, etc.)

    Camp actions (4):
      MAKE    - create campfire + bedrolls, sleep party NPCs (wilderness only)
      PACK    - destroy campfire, wake NPCs, destroy bedrolls
      SEARCH  - merged: firewood + hunt + random loot. ONCE per map.
      IDEAS   - craft recipes from combined party inventory

    NodePartyMain shows Make XOR Pack (contextual), Search (if not yet),
    Ideas (always). See NPC_TEMPLATE comment at bottom.
*/

#ifndef CAMP_ACTIONS_H_MEGAMOD
#define CAMP_ACTIONS_H_MEGAMOD

// Pulls OBJ_DATA_LIGHT_DISTANCE / _INTENSITY and other sfall extras
// needed by the CAMP_DO_MAKE / CAMP_DO_PACK macros.
#include "sfall/define_extra.h"

// ============================================================
// Forward decls for helper procedures defined in this header.
// Each NPC that #includes this file gets its own copy -- no collision
// because procedures are script-local.
// ============================================================
procedure camp_has_fire_here;
procedure camp_searched_here;
procedure camp_mark_searched;
procedure camp_party_has_pid(variable pid);
procedure camp_party_consume_pid(variable pid);

// Returns 1 if campfire exists on current map, 0 otherwise.
procedure camp_has_fire_here begin
    variable arr;
    variable key;
    key := "" + cur_map_index;
    arr := load_array("campfire_map");
    if (arr == 0) then return 0;
    if (arr[key] > 0) then return 1;
    return 0;
end

// Returns 1 if search was already done on current map.
procedure camp_searched_here begin
    variable arr;
    variable key;
    key := "" + cur_map_index;
    arr := load_array("camp_searched");
    if (arr == 0) then return 0;
    if (arr[key] == 1) then return 1;
    return 0;
end

// Marks current map as already searched.
procedure camp_mark_searched begin
    variable arr;
    variable key;
    key := "" + cur_map_index;
    arr := load_array("camp_searched");
    if (arr == 0) then arr := create_array_map;
    arr[key] := 1;
    save_array("camp_searched", arr);
end

procedure camp_party_has_pid(variable pid) begin
    variable lst;
    variable obj;
    if (obj_is_carrying_obj_pid(dude_obj, pid) > 0) then return 1;
    lst := list_begin(LIST_CRITTERS);
    obj := list_next(lst);
    while (obj) do begin
        if (obj != 0 and obj != dude_obj
            and get_team(obj) == TEAM_PLAYER
            and not(is_critter_dead(obj))) then begin
            if (obj_is_carrying_obj_pid(obj, pid) > 0) then begin
                list_end(lst);
                return 1;
            end
        end
        obj := list_next(lst);
    end
    list_end(lst);
    return 0;
end

procedure camp_party_consume_pid(variable pid) begin
    variable lst;
    variable obj;
    variable item;
    variable i;
    i := 0;
    item := inven_ptr(dude_obj, i);
    while (item) do begin
        if (obj_pid(item) == pid) then begin
            rm_obj_from_inven(dude_obj, item);
            destroy_object(item);
            return 1;
        end
        i := i + 1;
        item := inven_ptr(dude_obj, i);
    end
    lst := list_begin(LIST_CRITTERS);
    obj := list_next(lst);
    while (obj) do begin
        if (obj != 0 and obj != dude_obj
            and get_team(obj) == TEAM_PLAYER
            and not(is_critter_dead(obj))) then begin
            i := 0;
            item := inven_ptr(obj, i);
            while (item) do begin
                if (obj_pid(item) == pid) then begin
                    rm_obj_from_inven(obj, item);
                    destroy_object(item);
                    list_end(lst);
                    return 1;
                end
                i := i + 1;
                item := inven_ptr(obj, i);
            end
        end
        obj := list_next(lst);
    end
    list_end(lst);
    return 0;
end

// ============================================================
// MAKE CAMP -- create campfire + bedrolls in spiral ring.
// Gates: wilderness only (map_is_encounter), no existing camp, not combat.
// Animates party NPCs to lying pose. Sets GVAR_PARTY_NO_FOLLOW.
// ============================================================
#define CAMP_DO_MAKE \
    variable c_player_tile; \
    variable c_fire_tile; \
    variable c_fire; \
    variable c_fire2; \
    variable c_map_key; \
    variable c_lst; \
    variable c_npc; \
    variable c_idx; \
    variable c_bedroll_tile; \
    variable c_bedroll; \
    variable c_dir_idx; \
    variable c_bedrolls_arr; \
    variable c_sleepers_arr; \
    variable c_dir; \
    variable c_player_bedroll_tile; \
    variable c_player_bedroll; \
    variable c_had_sleeper; \
    variable c_ring; \
    variable c_slot; \
    variable c_cf_map; \
    variable c_fp_map; \
    variable c_bed_map; \
    variable c_sl_map; \
    variable c_pb_map; \
    if (not(map_is_encounter)) then begin \
        display_msg("Can't make camp here -- need wilderness."); \
    end \
    else if (combat_is_initialized) then begin \
        display_msg("Can't make camp during combat."); \
    end \
    else begin \
        c_player_tile := tile_num(dude_obj); \
        c_fire_tile := tile_num_in_direction(c_player_tile, 0, 1); \
        gfade_out(600); \
        game_time_advance(600); \
        c_fire := create_object(33555044, c_fire_tile, dude_elevation); /* PID_FIRE_PIT */ \
        if (c_fire == 0) then begin \
            for (c_dir := 1; c_dir < 6; c_dir++) begin \
                c_fire_tile := tile_num_in_direction(c_player_tile, c_dir, 1); \
                c_fire := create_object(33555044, c_fire_tile, dude_elevation); \
                if (c_fire != 0) then break; \
            end \
        end \
        if (c_fire == 0) then begin \
            gfade_in(600); \
            display_msg("No room to make a fire here."); \
        end \
        else begin \
            set_object_data(c_fire, OBJ_DATA_LIGHT_DISTANCE, 8); \
            set_object_data(c_fire, OBJ_DATA_LIGHT_INTENSITY, 65536); \
            c_fire2 := create_object(33555044, c_fire_tile, dude_elevation); \
            if (c_fire2 != 0) then begin \
                set_object_data(c_fire2, OBJ_DATA_LIGHT_DISTANCE, 8); \
                set_object_data(c_fire2, OBJ_DATA_LIGHT_INTENSITY, 65536); \
            end \
            c_bedrolls_arr := create_array(0, 4); \
            c_sleepers_arr := create_array(0, 4); \
            c_had_sleeper := 0; \
            c_player_bedroll_tile := tile_num_in_direction(c_fire_tile, 0, 5); \
            c_player_bedroll := create_object(269, c_player_bedroll_tile, dude_elevation); /* PID_BED_2 */ \
            if (c_player_bedroll == 0) then begin \
                c_player_bedroll := create_object(32, c_player_bedroll_tile, dude_elevation); /* PID_BED_1 */ \
            end \
            if (c_player_bedroll != 0) then begin \
                resize_array(c_bedrolls_arr, len_array(c_bedrolls_arr) + 1); \
                c_bedrolls_arr[len_array(c_bedrolls_arr) - 1] := c_player_bedroll; \
                c_pb_map := load_array("campfire_player_bedroll"); \
                if (c_pb_map == 0) then c_pb_map := create_array_map; \
                c_pb_map["" + cur_map_index] := c_player_bedroll; \
                save_array("campfire_player_bedroll", c_pb_map); \
            end \
            c_idx := 1; \
            c_lst := list_begin(LIST_CRITTERS); \
            c_npc := list_next(c_lst); \
            while (c_npc) do begin \
                if (c_npc != 0 and c_npc != dude_obj \
                    and get_team(c_npc) == TEAM_PLAYER \
                    and not(is_critter_dead(c_npc))) then begin \
                    c_slot := c_idx; \
                    c_ring := c_slot / 6; \
                    c_dir_idx := c_slot % 6; \
                    c_bedroll_tile := tile_num_in_direction(c_fire_tile, c_dir_idx, 5 + c_ring * 3); \
                    c_bedroll := create_object(32, c_bedroll_tile, dude_elevation); /* PID_BED_1 */ \
                    if (c_bedroll != 0) then begin \
                        resize_array(c_bedrolls_arr, len_array(c_bedrolls_arr) + 1); \
                        c_bedrolls_arr[len_array(c_bedrolls_arr) - 1] := c_bedroll; \
                        move_to(c_npc, c_bedroll_tile, dude_elevation); \
                        reg_anim_begin(); \
                        reg_anim_animate(c_npc, ANIM_fall_back_sf, -1); \
                        reg_anim_end(); \
                        resize_array(c_sleepers_arr, len_array(c_sleepers_arr) + 1); \
                        c_sleepers_arr[len_array(c_sleepers_arr) - 1] := c_npc; \
                        c_had_sleeper := 1; \
                        c_idx := c_idx + 1; \
                    end \
                end \
                c_npc := list_next(c_lst); \
            end \
            list_end(c_lst); \
            if (c_had_sleeper) then set_global_var(398, 1); /* GVAR_PARTY_NO_FOLLOW */ \
            tile_refresh_display; \
            gfade_in(600); \
            c_map_key := "" + cur_map_index; \
            c_cf_map := load_array("campfire_map"); \
            if (c_cf_map == 0) then c_cf_map := create_array_map; \
            c_cf_map[c_map_key] := c_fire_tile; \
            save_array("campfire_map", c_cf_map); \
            c_fp_map := load_array("campfire_firepit_obj"); \
            if (c_fp_map == 0) then c_fp_map := create_array_map; \
            c_fp_map[c_map_key] := c_fire; \
            save_array("campfire_firepit_obj", c_fp_map); \
            c_bed_map := load_array("campfire_bedrolls"); \
            if (c_bed_map == 0) then c_bed_map := create_array_map; \
            c_bed_map[c_map_key] := c_bedrolls_arr; \
            save_array("campfire_bedrolls", c_bed_map); \
            c_sl_map := load_array("campfire_sleepers"); \
            if (c_sl_map == 0) then c_sl_map := create_array_map; \
            c_sl_map[c_map_key] := c_sleepers_arr; \
            save_array("campfire_sleepers", c_sl_map); \
            play_sfx("FLAMETHR"); \
            display_msg("You make camp. The fire crackles. The party settles in for the night."); \
            give_exp_points(15); \
        end \
    end

// ============================================================
// PACK CAMP -- destroy firepit, wake party, destroy bedrolls.
// Gates: must have camp on current map.
// ============================================================
#define CAMP_DO_PACK \
    variable p_map_key; \
    variable p_cf_map; \
    variable p_fp_map; \
    variable p_bed_map; \
    variable p_sl_map; \
    variable p_pb_map; \
    variable p_fire; \
    variable p_bedrolls; \
    variable p_sleepers; \
    variable p_i; \
    variable p_obj; \
    p_map_key := "" + cur_map_index; \
    p_cf_map := load_array("campfire_map"); \
    if (p_cf_map == 0 or p_cf_map[p_map_key] == 0) then begin \
        display_msg("No camp here to pack."); \
    end \
    else begin \
        gfade_out(300); \
        p_fp_map := load_array("campfire_firepit_obj"); \
        if (p_fp_map != 0) then begin \
            p_fire := p_fp_map[p_map_key]; \
            if (p_fire != 0) then destroy_object(p_fire); \
            p_fp_map[p_map_key] := 0; \
            save_array("campfire_firepit_obj", p_fp_map); \
        end \
        p_sl_map := load_array("campfire_sleepers"); \
        if (p_sl_map != 0) then begin \
            p_sleepers := p_sl_map[p_map_key]; \
            if (p_sleepers != 0) then begin \
                for (p_i := 0; p_i < len_array(p_sleepers); p_i++) begin \
                    p_obj := p_sleepers[p_i]; \
                    if (p_obj != 0 and not(is_critter_dead(p_obj))) then begin \
                        reg_anim_begin(); \
                        reg_anim_animate(p_obj, ANIM_back_to_standing, -1); \
                        reg_anim_end(); \
                    end \
                end \
                free_array(p_sleepers); \
                p_sl_map[p_map_key] := 0; \
                save_array("campfire_sleepers", p_sl_map); \
            end \
        end \
        p_bed_map := load_array("campfire_bedrolls"); \
        if (p_bed_map != 0) then begin \
            p_bedrolls := p_bed_map[p_map_key]; \
            if (p_bedrolls != 0) then begin \
                for (p_i := 0; p_i < len_array(p_bedrolls); p_i++) begin \
                    p_obj := p_bedrolls[p_i]; \
                    if (p_obj != 0) then destroy_object(p_obj); \
                end \
                free_array(p_bedrolls); \
                p_bed_map[p_map_key] := 0; \
                save_array("campfire_bedrolls", p_bed_map); \
            end \
        end \
        p_pb_map := load_array("campfire_player_bedroll"); \
        if (p_pb_map != 0) then begin \
            p_pb_map[p_map_key] := 0; \
            save_array("campfire_player_bedroll", p_pb_map); \
        end \
        p_cf_map[p_map_key] := 0; \
        save_array("campfire_map", p_cf_map); \
        set_global_var(398, 0); /* GVAR_PARTY_NO_FOLLOW */ \
        tile_refresh_display; \
        gfade_in(300); \
        display_msg("You shovel dirt onto the fire. The flames die out. Camp packed."); \
        give_exp_points(10); \
    end

// ============================================================
// SEARCH AREA -- merged firewood + hunt + search-loot.
// Outdoorsman skill check (party highest). ONE attempt per map.
// On success: 2 items from merged pool + XP. Failure: nothing + mark used.
// Pool: firewood/iguana/rock/flint/spore spike/broc/xander/flower/sharpened pole.
// ============================================================
#define CAMP_DO_SEARCH \
    variable s_lst; \
    variable s_obj; \
    variable s_sk; \
    variable s_highest; \
    variable s_roll; \
    variable s_pick; \
    variable s_pid; \
    variable s_name; \
    variable s_item; \
    if (camp_searched_here) then begin \
        display_msg("You've already picked this area clean."); \
    end \
    else begin \
        call camp_mark_searched; \
        s_highest := has_skill(dude_obj, SKILL_OUTDOORSMAN); \
        s_lst := list_begin(LIST_CRITTERS); \
        s_obj := list_next(s_lst); \
        while (s_obj) do begin \
            if (s_obj != 0 and s_obj != dude_obj \
                and get_team(s_obj) == TEAM_PLAYER \
                and not(is_critter_dead(s_obj))) then begin \
                s_sk := has_skill(s_obj, SKILL_OUTDOORSMAN); \
                if (s_sk > s_highest) then s_highest := s_sk; \
            end \
            s_obj := list_next(s_lst); \
        end \
        list_end(s_lst); \
        s_roll := roll_vs_skill(dude_obj, SKILL_OUTDOORSMAN, \
            s_highest - has_skill(dude_obj, SKILL_OUTDOORSMAN)); \
        if (not(is_success(s_roll))) then begin \
            display_msg("You comb the area but find nothing useful."); \
        end \
        else begin \
            s_pick := random(0, 8); \
            if      (s_pick == 0) then begin s_pid := 286; s_name := "firewood";            end \
            else if (s_pick == 1) then begin s_pid := 81;  s_name := "an iguana on a stick";end \
            else if (s_pick == 2) then begin s_pid := 19;  s_name := "a rock";              end \
            else if (s_pick == 3) then begin s_pid := 278; s_name := "a flint";             end \
            else if (s_pick == 4) then begin s_pid := 365; s_name := "a spore spike";       end \
            else if (s_pick == 5) then begin s_pid := 271; s_name := "a broc flower";       end \
            else if (s_pick == 6) then begin s_pid := 272; s_name := "a xander root";       end \
            else if (s_pick == 7) then begin s_pid := 117; s_name := "a flower";            end \
            else                       begin s_pid := 320; s_name := "a sharpened pole";    end \
            s_item := create_object(s_pid, 0, 0); \
            if (s_item != 0) then add_obj_to_inven(dude_obj, s_item); \
            give_exp_points(50); \
            display_msg("Combing the area pays off. You bring back " + s_name + " (+50 XP)."); \
        end \
    end

// ============================================================
// ANY IDEAS -- craft recipes from combined party inventory.
// Primitive survival only -- no stimpaks/vanilla medicine.
// Recipes (all consume ingredients from anyone in party):
//   SHARPENED_POLE (320) + KNIFE (4/236) -> SPEAR (7)           [30 XP]
//   SPEAR (7) + FLINT (278)              -> SHARP_SPEAR (280)   [40 XP]
//   BROC (271) + XANDER (272)            -> HEALING_POWDER (273) [30 XP]
//   FIREWOOD (286) + FLINT (278)         -> camp ignite (wilderness) [15 XP]
//   SCORPION_TAIL (92) + BOOZE (125)     -> ANTIDOTE (49, SCIENCE check) [50 XP]
//   BOOZE (125) + LIGHTER (101)          -> MOLOTOV (159), lighter kept  [25 XP]
// ============================================================
#define CAMP_DO_IDEAS \
    variable i_made; \
    variable i_item; \
    variable i_sci; \
    variable i_sci_roll; \
    variable i_lst; \
    variable i_obj; \
    variable i_s; \
    i_made := 0; \
    /* SPEAR */ \
    if (camp_party_has_pid(320) and (camp_party_has_pid(4) or camp_party_has_pid(236))) then begin \
        call camp_party_consume_pid(320); \
        if (camp_party_has_pid(4)) then call camp_party_consume_pid(4); \
        else call camp_party_consume_pid(236); \
        i_item := create_object(7, 0, 0); \
        if (i_item != 0) then add_obj_to_inven(dude_obj, i_item); \
        display_msg("You fashion a spear from a sharpened pole and knife."); \
        give_exp_points(30); \
        i_made := i_made + 1; \
    end \
    /* SHARP_SPEAR */ \
    if (camp_party_has_pid(7) and camp_party_has_pid(278)) then begin \
        call camp_party_consume_pid(7); \
        call camp_party_consume_pid(278); \
        i_item := create_object(280, 0, 0); \
        if (i_item != 0) then add_obj_to_inven(dude_obj, i_item); \
        display_msg("You tip a spear with flint -- sharpened spear."); \
        give_exp_points(40); \
        i_made := i_made + 1; \
    end \
    /* HEALING_POWDER */ \
    if (camp_party_has_pid(271) and camp_party_has_pid(272)) then begin \
        call camp_party_consume_pid(271); \
        call camp_party_consume_pid(272); \
        i_item := create_object(273, 0, 0); \
        if (i_item != 0) then add_obj_to_inven(dude_obj, i_item); \
        display_msg("You grind broc flower and xander root into healing powder."); \
        give_exp_points(30); \
        i_made := i_made + 1; \
    end \
    /* ANTIDOTE */ \
    if (camp_party_has_pid(92) and camp_party_has_pid(125)) then begin \
        i_sci := has_skill(dude_obj, SKILL_SCIENCE); \
        i_lst := list_begin(LIST_CRITTERS); \
        i_obj := list_next(i_lst); \
        while (i_obj) do begin \
            if (i_obj != 0 and i_obj != dude_obj \
                and get_team(i_obj) == TEAM_PLAYER \
                and not(is_critter_dead(i_obj))) then begin \
                i_s := has_skill(i_obj, SKILL_SCIENCE); \
                if (i_s > i_sci) then i_sci := i_s; \
            end \
            i_obj := list_next(i_lst); \
        end \
        list_end(i_lst); \
        i_sci_roll := roll_vs_skill(dude_obj, SKILL_SCIENCE, \
            i_sci - has_skill(dude_obj, SKILL_SCIENCE)); \
        if (is_success(i_sci_roll)) then begin \
            call camp_party_consume_pid(92); \
            call camp_party_consume_pid(125); \
            i_item := create_object(49, 0, 0); \
            if (i_item != 0) then add_obj_to_inven(dude_obj, i_item); \
            display_msg("You distill venom and booze into antidote."); \
            give_exp_points(50); \
            i_made := i_made + 1; \
        end \
        else begin \
            display_msg("Nobody's smart enough to brew antidote from that tail."); \
        end \
    end \
    /* MOLOTOV (consumes booze only, lighter kept) */ \
    if (camp_party_has_pid(125) and camp_party_has_pid(101)) then begin \
        call camp_party_consume_pid(125); \
        i_item := create_object(159, 0, 0); \
        if (i_item != 0) then add_obj_to_inven(dude_obj, i_item); \
        display_msg("You soak a rag in booze and ignite it -- Molotov cocktail."); \
        give_exp_points(25); \
        i_made := i_made + 1; \
    end \
    if (i_made == 0) then display_msg("Nothing obvious to craft with what you've got.");

// ============================================================
// AMMO DISTRIBUTION -- reload party weapons from shared ammo pool.
// Team-oriented: dude speaks to ONE NPC, action runs on WHOLE party.
// For each armed member (incl dude): if weapon can reload from matching
// ammo box anywhere in party pool, consume one box, fill weapon to max.
// ============================================================
#define CAMP_DO_AMMO \
    variable a_lst; \
    variable a_obj; \
    variable a_weapon; \
    variable a_ammo_pid; \
    variable a_current; \
    variable a_max; \
    variable a_reloaded; \
    variable a_plst; \
    variable a_pobj; \
    variable a_item; \
    variable a_i; \
    variable a_found; \
    a_reloaded := 0; \
    /* dude first */ \
    a_weapon := critter_inven_obj(dude_obj, INVEN_TYPE_RIGHT_HAND); \
    if (a_weapon != 0) then begin \
        a_ammo_pid := get_weapon_ammo_pid(a_weapon); \
        if (a_ammo_pid > 0) then begin \
            a_current := get_weapon_ammo_count(a_weapon); \
            a_max := get_proto_data(obj_pid(a_weapon), PROTO_WP_MAG_SIZE); \
            if (a_current < a_max) then begin \
                a_found := 0; \
                a_i := 0; \
                a_item := inven_ptr(dude_obj, a_i); \
                while (a_item and a_found == 0) do begin \
                    if (obj_pid(a_item) == a_ammo_pid) then begin \
                        rm_obj_from_inven(dude_obj, a_item); \
                        destroy_object(a_item); \
                        set_weapon_ammo_count(a_weapon, a_max); \
                        a_found := 1; \
                        a_reloaded := a_reloaded + 1; \
                    end \
                    else begin \
                        a_i := a_i + 1; \
                        a_item := inven_ptr(dude_obj, a_i); \
                    end \
                end \
                if (a_found == 0) then begin \
                    a_plst := list_begin(LIST_CRITTERS); \
                    a_pobj := list_next(a_plst); \
                    while (a_pobj and a_found == 0) do begin \
                        if (a_pobj != 0 and a_pobj != dude_obj \
                            and get_team(a_pobj) == TEAM_PLAYER \
                            and not(is_critter_dead(a_pobj))) then begin \
                            a_i := 0; \
                            a_item := inven_ptr(a_pobj, a_i); \
                            while (a_item and a_found == 0) do begin \
                                if (obj_pid(a_item) == a_ammo_pid) then begin \
                                    rm_obj_from_inven(a_pobj, a_item); \
                                    destroy_object(a_item); \
                                    set_weapon_ammo_count(a_weapon, a_max); \
                                    a_found := 1; \
                                    a_reloaded := a_reloaded + 1; \
                                end \
                                else begin \
                                    a_i := a_i + 1; \
                                    a_item := inven_ptr(a_pobj, a_i); \
                                end \
                            end \
                        end \
                        a_pobj := list_next(a_plst); \
                    end \
                    list_end(a_plst); \
                end \
            end \
        end \
    end \
    /* party NPCs */ \
    a_lst := list_begin(LIST_CRITTERS); \
    a_obj := list_next(a_lst); \
    while (a_obj) do begin \
        if (a_obj != 0 and a_obj != dude_obj \
            and get_team(a_obj) == TEAM_PLAYER \
            and not(is_critter_dead(a_obj))) then begin \
            a_weapon := critter_inven_obj(a_obj, INVEN_TYPE_RIGHT_HAND); \
            if (a_weapon != 0) then begin \
                a_ammo_pid := get_weapon_ammo_pid(a_weapon); \
                if (a_ammo_pid > 0) then begin \
                    a_current := get_weapon_ammo_count(a_weapon); \
                    a_max := get_proto_data(obj_pid(a_weapon), PROTO_WP_MAG_SIZE); \
                    if (a_current < a_max) then begin \
                        a_found := 0; \
                        a_i := 0; \
                        a_item := inven_ptr(dude_obj, a_i); \
                        while (a_item and a_found == 0) do begin \
                            if (obj_pid(a_item) == a_ammo_pid) then begin \
                                rm_obj_from_inven(dude_obj, a_item); \
                                destroy_object(a_item); \
                                set_weapon_ammo_count(a_weapon, a_max); \
                                a_found := 1; \
                                a_reloaded := a_reloaded + 1; \
                            end \
                            else begin \
                                a_i := a_i + 1; \
                                a_item := inven_ptr(dude_obj, a_i); \
                            end \
                        end \
                        if (a_found == 0) then begin \
                            a_plst := list_begin(LIST_CRITTERS); \
                            a_pobj := list_next(a_plst); \
                            while (a_pobj and a_found == 0) do begin \
                                if (a_pobj != 0 and a_pobj != dude_obj \
                                    and get_team(a_pobj) == TEAM_PLAYER \
                                    and not(is_critter_dead(a_pobj))) then begin \
                                    a_i := 0; \
                                    a_item := inven_ptr(a_pobj, a_i); \
                                    while (a_item and a_found == 0) do begin \
                                        if (obj_pid(a_item) == a_ammo_pid) then begin \
                                            rm_obj_from_inven(a_pobj, a_item); \
                                            destroy_object(a_item); \
                                            set_weapon_ammo_count(a_weapon, a_max); \
                                            a_found := 1; \
                                            a_reloaded := a_reloaded + 1; \
                                        end \
                                        else begin \
                                            a_i := a_i + 1; \
                                            a_item := inven_ptr(a_pobj, a_i); \
                                        end \
                                    end \
                                end \
                                a_pobj := list_next(a_plst); \
                            end \
                            list_end(a_plst); \
                        end \
                    end \
                end \
            end \
        end \
        a_obj := list_next(a_lst); \
    end \
    list_end(a_lst); \
    if (a_reloaded == 0) then display_msg("Nobody needs a reload, or no matching ammo to share."); \
    else display_msg("Ammo redistributed. " + a_reloaded + " weapon(s) topped up.");

// ============================================================
// FOOD & DRINK -- each alive party member eats one food + drinks one
// water-like item from shared inventory. Small HP restore per item.
// FOOD pool: mutated fruit (71), iguana stick (81), meat on stick (103)
// DRINK pool: water flask (126), nuka cola (106)
// Heals: food = +10 HP, drink = +5 HP (per consumed item).
// ============================================================
#define CAMP_DO_FOOD \
    variable f_lst; \
    variable f_obj; \
    variable f_fed; \
    variable f_consumed_food; \
    variable f_consumed_drink; \
    f_fed := 0; \
    f_consumed_food := 0; \
    f_consumed_drink := 0; \
    /* dude */ \
    if (camp_party_has_pid(71) or camp_party_has_pid(81) or camp_party_has_pid(103)) then begin \
        if (camp_party_has_pid(71)) then begin call camp_party_consume_pid(71); end \
        else if (camp_party_has_pid(81)) then begin call camp_party_consume_pid(81); end \
        else begin call camp_party_consume_pid(103); end \
        critter_heal(dude_obj, 10); \
        f_consumed_food := f_consumed_food + 1; \
        f_fed := f_fed + 1; \
    end \
    if (camp_party_has_pid(126) or camp_party_has_pid(106)) then begin \
        if (camp_party_has_pid(126)) then begin call camp_party_consume_pid(126); end \
        else begin call camp_party_consume_pid(106); end \
        critter_heal(dude_obj, 5); \
        f_consumed_drink := f_consumed_drink + 1; \
    end \
    /* party NPCs */ \
    f_lst := list_begin(LIST_CRITTERS); \
    f_obj := list_next(f_lst); \
    while (f_obj) do begin \
        if (f_obj != 0 and f_obj != dude_obj \
            and get_team(f_obj) == TEAM_PLAYER \
            and not(is_critter_dead(f_obj))) then begin \
            if (camp_party_has_pid(71) or camp_party_has_pid(81) or camp_party_has_pid(103)) then begin \
                if (camp_party_has_pid(71)) then begin call camp_party_consume_pid(71); end \
                else if (camp_party_has_pid(81)) then begin call camp_party_consume_pid(81); end \
                else begin call camp_party_consume_pid(103); end \
                critter_heal(f_obj, 10); \
                f_consumed_food := f_consumed_food + 1; \
                f_fed := f_fed + 1; \
            end \
            if (camp_party_has_pid(126) or camp_party_has_pid(106)) then begin \
                if (camp_party_has_pid(126)) then begin call camp_party_consume_pid(126); end \
                else begin call camp_party_consume_pid(106); end \
                critter_heal(f_obj, 5); \
                f_consumed_drink := f_consumed_drink + 1; \
            end \
        end \
        f_obj := list_next(f_lst); \
    end \
    list_end(f_lst); \
    if (f_fed == 0 and f_consumed_drink == 0) then display_msg("Party has no food or water to share."); \
    else display_msg("Party shares rations. Food: " + f_consumed_food + ", drink: " + f_consumed_drink + ".");

// ============================================================
// WOUNDED CHECK -- report who's hurt + auto-apply healing powder
// (273) or stimpak (40) if available in party pool. Heals +30 HP
// per healing powder, +20 per stimpak. Iterates until no more HP
// missing or no more heal items.
// ============================================================
#define CAMP_DO_WOUNDED \
    variable w_lst; \
    variable w_obj; \
    variable w_cur; \
    variable w_max; \
    variable w_wounded; \
    variable w_healed; \
    variable w_name; \
    variable w_missing; \
    variable w_report; \
    w_wounded := 0; \
    w_healed := 0; \
    w_report := ""; \
    /* dude */ \
    w_cur := get_critter_stat(dude_obj, STAT_current_hp); \
    w_max := get_critter_stat(dude_obj, STAT_max_hp); \
    if (w_cur < w_max) then begin \
        w_missing := w_max - w_cur; \
        w_report := w_report + "You: " + w_cur + "/" + w_max + ". "; \
        w_wounded := w_wounded + 1; \
        while (w_cur < w_max and (camp_party_has_pid(273) or camp_party_has_pid(40))) do begin \
            if (camp_party_has_pid(273)) then begin \
                call camp_party_consume_pid(273); \
                critter_heal(dude_obj, 30); \
            end \
            else begin \
                call camp_party_consume_pid(40); \
                critter_heal(dude_obj, 20); \
            end \
            w_healed := w_healed + 1; \
            w_cur := get_critter_stat(dude_obj, STAT_current_hp); \
        end \
    end \
    /* party NPCs */ \
    w_lst := list_begin(LIST_CRITTERS); \
    w_obj := list_next(w_lst); \
    while (w_obj) do begin \
        if (w_obj != 0 and w_obj != dude_obj \
            and get_team(w_obj) == TEAM_PLAYER \
            and not(is_critter_dead(w_obj))) then begin \
            w_cur := get_critter_stat(w_obj, STAT_current_hp); \
            w_max := get_critter_stat(w_obj, STAT_max_hp); \
            if (w_cur < w_max) then begin \
                w_name := obj_name(w_obj); \
                w_report := w_report + w_name + ": " + w_cur + "/" + w_max + ". "; \
                w_wounded := w_wounded + 1; \
                while (w_cur < w_max and (camp_party_has_pid(273) or camp_party_has_pid(40))) do begin \
                    if (camp_party_has_pid(273)) then begin \
                        call camp_party_consume_pid(273); \
                        critter_heal(w_obj, 30); \
                    end \
                    else begin \
                        call camp_party_consume_pid(40); \
                        critter_heal(w_obj, 20); \
                    end \
                    w_healed := w_healed + 1; \
                    w_cur := get_critter_stat(w_obj, STAT_current_hp); \
                end \
            end \
        end \
        w_obj := list_next(w_lst); \
    end \
    list_end(w_lst); \
    if (w_wounded == 0) then display_msg("Nobody's hurt -- party in good shape."); \
    else if (w_healed == 0) then display_msg("Wounded: " + w_report + "(no healing items available)"); \
    else display_msg("Treated " + w_healed + " wound(s). " + w_report);

/* ============================================================
   NPC TEMPLATE -- NodePartyMain contextual pattern:

   procedure NodePartyMain begin
      Reply_Blank;
      NOption("Follow me.", NodePartyFollow, 4);
      NOption("Wait here.", NodePartyWait, 4);
      NOption("Holster weapon.", NodePartyHolster, 4);

      // Camp/search options -- only in wilderness (random encounter maps)
      if (map_is_encounter) then begin
         if (camp_has_fire_here) then
            NOption("Pack up camp.", CampPackTrigger, 4);
         else
            NOption("Make camp.", CampMakeTrigger, 4);
         if not(camp_searched_here) then
            NOption("Spread out and search the area for anything useful.",
                    CampSearchTrigger, 4);
      end

      // Team-ops -- available anywhere (town/wilderness/vault)
      NOption("Anything on your mind?", CampIdeasTrigger, 4);
      NOption("Redistribute ammo.", PartyAmmoTrigger, 4);
      NOption("Eat and drink.", PartyFoodTrigger, 4);
      NOption("Who's hurt?", PartyWoundedTrigger, 4);

      NOption("[Done]", NodePartyDone, 4);
   end

   procedure CampMakeTrigger begin CAMP_DO_MAKE end
   procedure CampPackTrigger begin CAMP_DO_PACK end
   procedure CampSearchTrigger begin CAMP_DO_SEARCH end
   procedure CampIdeasTrigger begin CAMP_DO_IDEAS end
   procedure PartyAmmoTrigger begin CAMP_DO_AMMO end
   procedure PartyFoodTrigger begin CAMP_DO_FOOD end
   procedure PartyWoundedTrigger begin CAMP_DO_WOUNDED end
   ============================================================ */

#endif
