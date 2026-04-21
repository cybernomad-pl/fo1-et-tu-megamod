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

// Scenery PIDs (not in standard itempid.h; scenepid.h uses pid_type=2
// encoding `0x02000000 | idx`). Named constants instead of magic
// literals in macro bodies -- single source of truth if PIDs change.
#define CAMP_PID_FIRE_PIT   (33555044)
#define CAMP_PID_BED_1      (33554640)
#define CAMP_PID_BED_2      (33554641)

// ============================================================
// Forward decls for helper procedures defined in this header.
// Each NPC that #includes this file gets its own copy -- no collision
// because procedures are script-local.
// ============================================================
procedure camp_has_fire_here;
procedure camp_active_here;
procedure camp_searched_here;
procedure camp_mark_searched;
procedure camp_party_has_pid(variable pid);
procedure camp_party_consume_pid(variable pid);
procedure camp_grant_barter(variable obj);

// Returns 1 if campfire (actual fire) exists on current map, 0 otherwise.
// Used by shovel handler and by camp_active_here check.
procedure camp_has_fire_here begin
    variable arr;
    variable key;
    key := "" + cur_map_index;
    arr := load_array("campfire_map");
    if (arr == 0) then return 0;
    if (arr[key] > 0) then return 1;
    return 0;
end

// Returns 1 if any camp is set up on current map (WITH or WITHOUT fire).
// Camp exists = bedrolls placed + party sleeping. Fire is optional (needs
// firewood). Dialog Make/Pack gate uses THIS, not camp_has_fire_here.
procedure camp_active_here begin
    variable arr;
    variable key;
    key := "" + cur_map_index;
    arr := load_array("camp_active_map");
    if (arr == 0) then return 0;
    if (arr[key] == 1) then return 1;
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

// Grant BARTER (trade) flag on proto so "this critter cannot carry anything"
// stops blocking trade. Call from NodeRecruit after party_add.
// Uses named constants CFLG_BARTER (=2, bit 0x02 "can trade with") and
// PROTO_CR_FLAGS (=32, critter flags offset in proto) from sfall/define_extra.h.
// Proto-level change (all instances of this PID get it). Sfall persists
// proto changes across save/load.
procedure camp_grant_barter(variable obj) begin
    variable pid;
    variable cur_flags;
    if (obj == 0) then return;
    pid := obj_pid(obj);
    cur_flags := get_proto_data(pid, PROTO_CR_FLAGS);
    set_proto_data(pid, PROTO_CR_FLAGS, cur_flags bwor CFLG_BARTER);
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
// MAKE CAMP -- EXEC macro (no gates, no Reply, no display_msg).
// Caller (CampMakeExecute trigger) must have already gated:
//   - map_is_encounter
//   - not combat
// Camera: fade_out -> work -> fade_in. Dialog engine closes dialog.
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
    variable c_am_map; \
    variable c_has_firewood; \
    variable c_center_tile; \
    c_player_tile := tile_num(dude_obj); \
    gfade_out(600); \
    game_time_advance(600); \
    c_fire := 0; \
    c_fire_tile := tile_num_in_direction(c_player_tile, 0, 1); \
    c_has_firewood := camp_party_has_pid(286); \
    if (c_has_firewood) then begin \
        call camp_party_consume_pid(286); \
        c_fire := create_object(CAMP_PID_FIRE_PIT, c_fire_tile, dude_elevation); \
        if (c_fire == 0) then begin \
            for (c_dir := 1; c_dir < 6; c_dir++) begin \
                c_fire_tile := tile_num_in_direction(c_player_tile, c_dir, 1); \
                c_fire := create_object(CAMP_PID_FIRE_PIT, c_fire_tile, dude_elevation); \
                if (c_fire != 0) then break; \
            end \
        end \
    end \
    if (c_fire != 0) then c_center_tile := c_fire_tile; \
    else c_center_tile := c_player_tile; \
    if (c_fire != 0) then begin \
        set_object_data(c_fire, OBJ_DATA_LIGHT_DISTANCE, 8); \
        set_object_data(c_fire, OBJ_DATA_LIGHT_INTENSITY, 65536); \
        c_fire2 := create_object(CAMP_PID_FIRE_PIT, c_fire_tile, dude_elevation); \
        if (c_fire2 != 0) then begin \
            set_object_data(c_fire2, OBJ_DATA_LIGHT_DISTANCE, 8); \
            set_object_data(c_fire2, OBJ_DATA_LIGHT_INTENSITY, 65536); \
        end \
    end \
    c_bedrolls_arr := create_array(0, 4); \
    c_sleepers_arr := create_array(0, 4); \
    c_had_sleeper := 0; \
    c_player_bedroll := 0; \
    for (c_dir := 0; c_dir < 6; c_dir++) begin \
        c_player_bedroll_tile := tile_num_in_direction(c_center_tile, c_dir, 5); \
        c_player_bedroll := create_object(CAMP_PID_BED_2, c_player_bedroll_tile, dude_elevation); \
        if (c_player_bedroll == 0) then \
            c_player_bedroll := create_object(CAMP_PID_BED_1, c_player_bedroll_tile, dude_elevation); \
        if (c_player_bedroll != 0) then break; \
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
            c_bedroll_tile := tile_num_in_direction(c_center_tile, c_dir_idx, 5 + c_ring * 3); \
            c_bedroll := create_object(CAMP_PID_BED_1, c_bedroll_tile, dude_elevation); \
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
    c_am_map := load_array("camp_active_map"); \
    if (c_am_map == 0) then c_am_map := create_array_map; \
    c_am_map[c_map_key] := 1; \
    save_array("camp_active_map", c_am_map); \
    if (c_fire != 0) then begin \
        c_cf_map := load_array("campfire_map"); \
        if (c_cf_map == 0) then c_cf_map := create_array_map; \
        c_cf_map[c_map_key] := c_fire_tile; \
        save_array("campfire_map", c_cf_map); \
        c_fp_map := load_array("campfire_firepit_obj"); \
        if (c_fp_map == 0) then c_fp_map := create_array_map; \
        c_fp_map[c_map_key] := c_fire; \
        save_array("campfire_firepit_obj", c_fp_map); \
    end \
    c_bed_map := load_array("campfire_bedrolls"); \
    if (c_bed_map == 0) then c_bed_map := create_array_map; \
    c_bed_map[c_map_key] := c_bedrolls_arr; \
    save_array("campfire_bedrolls", c_bed_map); \
    c_sl_map := load_array("campfire_sleepers"); \
    if (c_sl_map == 0) then c_sl_map := create_array_map; \
    c_sl_map[c_map_key] := c_sleepers_arr; \
    save_array("campfire_sleepers", c_sl_map); \
    if (c_fire != 0) then play_sfx("FLAMETHR"); \
    give_exp_points(15);

// ============================================================
// PACK CAMP -- EXEC macro (no gates, no Reply, no display_msg).
// Caller (CampPackExecute trigger) must have already gated on
// camp_active_here.
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
    variable p_sweep_lst; \
    variable p_sweep_obj; \
    variable p_am_map; \
    p_map_key := "" + cur_map_index; \
    p_am_map := load_array("camp_active_map"); \
    if (p_am_map == 0) then p_am_map := create_array_map; \
    begin \
        gfade_out(300); \
        /* Firepit cleanup (only if camp HAD fire). */ \
        p_cf_map := load_array("campfire_map"); \
        p_fp_map := load_array("campfire_firepit_obj"); \
        if (p_fp_map != 0) then begin \
            p_fire := p_fp_map[p_map_key]; \
            if (p_fire != 0) then destroy_object(p_fire); \
            p_fp_map[p_map_key] := 0; \
            save_array("campfire_firepit_obj", p_fp_map); \
        end \
        /* Sweep LIST_SCENERY for any remaining PID_FIRE_PIT on the */ \
        /* camp tile -- MAKE stacks a second invisible firepit for 2x */ \
        /* brightness (c_fire2), which isn't tracked in save_array. */ \
        /* Without this sweep, Pack leaves a silent light source. */ \
        p_sweep_lst := list_begin(LIST_SCENERY); \
        p_sweep_obj := list_next(p_sweep_lst); \
        while (p_sweep_obj) do begin \
            if (p_sweep_obj != 0 and obj_pid(p_sweep_obj) == CAMP_PID_FIRE_PIT) then begin \
                destroy_object(p_sweep_obj); \
            end \
            p_sweep_obj := list_next(p_sweep_lst); \
        end \
        list_end(p_sweep_lst); \
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
                /* Clear map handle + save BEFORE free to avoid sfall */ \
                /* save_array seeing a freed array reference. */ \
                p_sl_map[p_map_key] := 0; \
                save_array("campfire_sleepers", p_sl_map); \
                free_array(p_sleepers); \
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
                /* Clear + save BEFORE free (sfall safety). */ \
                p_bed_map[p_map_key] := 0; \
                save_array("campfire_bedrolls", p_bed_map); \
                free_array(p_bedrolls); \
            end \
        end \
        p_pb_map := load_array("campfire_player_bedroll"); \
        if (p_pb_map != 0) then begin \
            p_pb_map[p_map_key] := 0; \
            save_array("campfire_player_bedroll", p_pb_map); \
        end \
        if (p_cf_map != 0) then begin \
            p_cf_map[p_map_key] := 0; \
            save_array("campfire_map", p_cf_map); \
        end \
        p_am_map[p_map_key] := 0; \
        save_array("camp_active_map", p_am_map); \
        set_global_var(398, 0); /* GVAR_PARTY_NO_FOLLOW */ \
        tile_refresh_display; \
        gfade_in(300); \
        give_exp_points(10); \
    end

// ============================================================
// SEARCH AREA -- TEST MODE (Borys 2026-04-21): no skill check,
// always returns 2x of each of 9 items. 18 items total. ONE per map.
// Reason: Borys has no inventory to test crafts -- seed all ingredients.
// NOTE: skill-based logic will return in future phase.
// ============================================================
#define CAMP_DO_SEARCH \
    variable s_item; \
    variable s_pass; \
    variable s_has_fire; \
    if (camp_searched_here) then begin \
        Reply("We already picked this place clean. Nothing left out there."); \
    end \
    else begin \
        call camp_mark_searched; \
        s_has_fire := camp_has_fire_here; \
        for (s_pass := 0; s_pass < 2; s_pass++) begin \
            s_item := create_object(286, 0, 0); if (s_item != 0) then add_obj_to_inven(dude_obj, s_item); \
            if (s_has_fire) then begin \
                s_item := create_object(81, 0, 0); if (s_item != 0) then add_obj_to_inven(dude_obj, s_item); \
            end \
            s_item := create_object(19,  0, 0); if (s_item != 0) then add_obj_to_inven(dude_obj, s_item); \
            s_item := create_object(278, 0, 0); if (s_item != 0) then add_obj_to_inven(dude_obj, s_item); \
            s_item := create_object(365, 0, 0); if (s_item != 0) then add_obj_to_inven(dude_obj, s_item); \
            s_item := create_object(271, 0, 0); if (s_item != 0) then add_obj_to_inven(dude_obj, s_item); \
            s_item := create_object(272, 0, 0); if (s_item != 0) then add_obj_to_inven(dude_obj, s_item); \
            s_item := create_object(117, 0, 0); if (s_item != 0) then add_obj_to_inven(dude_obj, s_item); \
            s_item := create_object(320, 0, 0); if (s_item != 0) then add_obj_to_inven(dude_obj, s_item); \
        end \
        give_exp_points(100); \
        if (s_has_fire) then begin \
            Reply("Decent haul. Scrounged some firewood, rocks, flint and flowers. Even caught an iguana or two -- already sizzling on the stick."); \
        end \
        else begin \
            Reply("We poked around. Picked up firewood, rocks, flint, some plants. Found a dead iguana but no fire to cook it on."); \
        end \
    end

// ============================================================
// CRAFT SPEAR -- sharpened pole + knife -> spear.
// Order: product FIRST, consume only if product created (no loss on fail).
// ============================================================
#define CAMP_CRAFT_SPEAR \
    variable cs_item; \
    cs_item := create_object(7, 0, 0); /* PID_SPEAR */ \
    if (cs_item == 0) then begin \
        Reply("Tried to lash the knife to the pole. Something slipped. Spear didn't come out right."); \
    end \
    else begin \
        call camp_party_consume_pid(320); /* sharpened pole */ \
        if (camp_party_has_pid(4)) then call camp_party_consume_pid(4); \
        else call camp_party_consume_pid(236); \
        add_obj_to_inven(dude_obj, cs_item); \
        Reply("There we go. Proper spear now. Knife bound tight to the pole."); \
        give_exp_points(30); \
    end

// ============================================================
// CRAFT SHARP SPEAR -- spear + flint -> sharpened spear.
// ============================================================
#define CAMP_CRAFT_SHARP_SPEAR \
    variable css_item; \
    css_item := create_object(280, 0, 0); /* PID_SHARPENED_SPEAR */ \
    if (css_item == 0) then begin \
        Reply("Flint won't stay on. Spear stays plain, but we kept the bits."); \
    end \
    else begin \
        call camp_party_consume_pid(7);   /* spear */ \
        call camp_party_consume_pid(278); /* flint */ \
        add_obj_to_inven(dude_obj, css_item); \
        Reply("Tipped the spear with flint. That'll go through leather armor now."); \
        give_exp_points(40); \
    end

// ============================================================
// CRAFT HEALING POWDER -- broc flower + xander root -> healing powder.
// ============================================================
#define CAMP_CRAFT_HEALING_POWDER \
    variable chp_item; \
    chp_item := create_object(273, 0, 0); /* PID_HEALING_POWDER */ \
    if (chp_item == 0) then begin \
        Reply("The herbs just wouldn't grind together. Weird. We still have them."); \
    end \
    else begin \
        call camp_party_consume_pid(271); /* broc */ \
        call camp_party_consume_pid(272); /* xander */ \
        add_obj_to_inven(dude_obj, chp_item); \
        Reply("Ground the broc and xander into powder. This should help if someone gets hurt."); \
        give_exp_points(30); \
    end

// ============================================================
// CRAFT ANTIDOTE -- scorpion tail + booze + SCIENCE roll.
// Uses party highest SCIENCE. Fail on low skill = ingredients NOT consumed.
// ============================================================
#define CAMP_CRAFT_ANTIDOTE \
    variable ca_item; \
    variable ca_lst; \
    variable ca_obj; \
    variable ca_sci; \
    variable ca_s; \
    variable ca_roll; \
    ca_sci := has_skill(dude_obj, SKILL_SCIENCE); \
    ca_lst := list_begin(LIST_CRITTERS); \
    ca_obj := list_next(ca_lst); \
    while (ca_obj) do begin \
        if (ca_obj != 0 and ca_obj != dude_obj \
            and get_team(ca_obj) == TEAM_PLAYER \
            and not(is_critter_dead(ca_obj))) then begin \
            ca_s := has_skill(ca_obj, SKILL_SCIENCE); \
            if (ca_s > ca_sci) then ca_sci := ca_s; \
        end \
        ca_obj := list_next(ca_lst); \
    end \
    list_end(ca_lst); \
    ca_roll := roll_vs_skill(dude_obj, SKILL_SCIENCE, \
        ca_sci - has_skill(dude_obj, SKILL_SCIENCE)); \
    if (is_success(ca_roll)) then begin \
        ca_item := create_object(49, 0, 0); /* PID_ANTIDOTE */ \
        if (ca_item == 0) then begin \
            Reply("The mixture curdled. Whoever tried to distill it messed it up. We kept the ingredients."); \
        end \
        else begin \
            call camp_party_consume_pid(92);  /* scorpion tail */ \
            call camp_party_consume_pid(125); /* booze */ \
            add_obj_to_inven(dude_obj, ca_item); \
            Reply("Distilled the venom with the booze. Antidote's ready. Bottoms up if anything bites us."); \
            give_exp_points(50); \
        end \
    end \
    else begin \
        Reply("Nobody here knows their chemistry. We saved the tail and the booze for somebody smarter."); \
    end

// ============================================================
// CRAFT MOLOTOV -- booze + lighter -> molotov cocktail. Lighter kept.
// ============================================================
#define CAMP_CRAFT_MOLOTOV \
    variable cm_item; \
    cm_item := create_object(159, 0, 0); /* PID_MOLOTOV_COCKTAIL */ \
    if (cm_item == 0) then begin \
        Reply("Lighter won't catch. Damn thing. Booze is fine though."); \
    end \
    else begin \
        call camp_party_consume_pid(125); /* booze (lighter kept) */ \
        add_obj_to_inven(dude_obj, cm_item); \
        Reply("Soaked the rag in booze, flicked the lighter. One Molotov, ready to throw."); \
        give_exp_points(25); \
    end


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
    if (a_reloaded == 0) then begin \
        Reply("Everyone's already topped off. Or we don't have the right rounds."); \
    end \
    else if (a_reloaded == 1) then begin \
        Reply("Shuffled the ammo around. Topped off one weapon."); \
    end \
    else begin \
        Reply("Shuffled the ammo around. " + a_reloaded + " weapons ready to go."); \
    end

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
    if (f_fed == 0 and f_consumed_drink == 0) then begin \
        Reply("Rations are gone. We'll have to find something, or go hungry."); \
    end \
    else if (f_fed > 0 and f_consumed_drink > 0) then begin \
        Reply("Passed around some grub and a canteen. We're good for a bit."); \
    end \
    else if (f_fed > 0) then begin \
        Reply("Ate what we had. Water's short though -- hope we find some soon."); \
    end \
    else begin \
        Reply("Drank what was left. Nothing solid to eat, but at least we're not parched."); \
    end

// ============================================================
// WOUNDED CHECK -- report who's hurt + auto-apply healing powder
// (273) or stimpak (40) if available in party pool. Heals +30 HP
// per healing powder, +20 per stimpak. Iterates until no more HP
// missing or no more heal items.
// ============================================================
// ============================================================
// WOUNDED CHECK -- status inspect only. NO healing, NO HP numbers.
// Fuzzy descriptors like Fallout's look_at status: perfect / scratched /
// hurting / near death. Report delivered as DIALOG REPLY (not float
// display_msg) so it feels like asking the NPC for a sitrep.
// ============================================================
// Helper macro: map ratio int (current*100/max) to fuzzy text.
// Output var must be named status_var.
#define CAMP_WOUND_STATUS(ratio_var, status_var) \
    if      (ratio_var >= 85) then status_var := "in good shape"; \
    else if (ratio_var >= 60) then status_var := "a little roughed up"; \
    else if (ratio_var >= 35) then status_var := "hurting pretty bad"; \
    else if (ratio_var >= 10) then status_var := "bleeding heavily"; \
    else                      status_var := "barely conscious";

#define CAMP_DO_WOUNDED \
    variable w_lst; \
    variable w_obj; \
    variable w_cur; \
    variable w_max; \
    variable w_ratio; \
    variable w_status; \
    variable w_report; \
    variable w_first; \
    variable w_phrase; \
    w_report := ""; \
    w_first := 1; \
    /* dude first -- perspective of self_obj (the NPC we're talking to): */ \
    /*   dude     -> "You're <status>"                                    */ \
    /*   self_obj -> "I'm <status>"                                       */ \
    /*   other    -> "<Name> is <status>"                                 */ \
    w_cur := get_critter_stat(dude_obj, STAT_current_hp); \
    w_max := get_critter_stat(dude_obj, STAT_max_hp); \
    if (w_cur < w_max) then begin \
        w_ratio := (w_cur * 100) / w_max; \
        CAMP_WOUND_STATUS(w_ratio, w_status) \
        w_report := "You're " + w_status; \
        w_first := 0; \
    end \
    w_lst := list_begin(LIST_CRITTERS); \
    w_obj := list_next(w_lst); \
    while (w_obj) do begin \
        if (w_obj != 0 and w_obj != dude_obj \
            and get_team(w_obj) == TEAM_PLAYER \
            and not(is_critter_dead(w_obj))) then begin \
            w_cur := get_critter_stat(w_obj, STAT_current_hp); \
            w_max := get_critter_stat(w_obj, STAT_max_hp); \
            if (w_cur < w_max) then begin \
                w_ratio := (w_cur * 100) / w_max; \
                CAMP_WOUND_STATUS(w_ratio, w_status) \
                if (w_obj == self_obj) then w_phrase := "I'm " + w_status; \
                else w_phrase := obj_name(w_obj) + " is " + w_status; \
                if (w_first) then begin \
                    w_report := w_phrase; \
                    w_first := 0; \
                end \
                else w_report := w_report + ". " + w_phrase; \
            end \
        end \
        w_obj := list_next(w_lst); \
    end \
    list_end(w_lst); \
    if (w_report == "") then begin \
        Reply("Nobody's complaining. We're all in good shape."); \
    end \
    else begin \
        Reply(w_report + "."); \
    end

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
