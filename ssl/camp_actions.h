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
procedure camp_feed_member(variable member);
procedure camp_give_item(variable pid);
procedure camp_save_map_entry(variable key, variable val, variable arr_name);
procedure camp_clear_map_entry(variable key, variable arr_name);
procedure camp_build_camp;
procedure camp_teardown_camp;
procedure camp_scout_haul;

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

// ------------------------------------------------------------
// Small DRY helpers used by multiple macros.
// ------------------------------------------------------------

// Create one object of `pid` and add to dude inventory. No-op on fail.
procedure camp_give_item(variable pid) begin
    variable obj;
    obj := create_object(pid, 0, 0);
    if (obj != 0) then add_obj_to_inven(dude_obj, obj);
end

// Write (or overwrite) `key` -> `val` in save_array named `arr_name`.
// Creates the map if missing.
procedure camp_save_map_entry(variable key, variable val, variable arr_name) begin
    variable arr;
    arr := load_array(arr_name);
    if (arr == 0) then arr := create_array_map;
    arr[key] := val;
    save_array(arr_name, arr);
end

// Clear `key` (set to 0) in save_array `arr_name`. No-op if absent.
procedure camp_clear_map_entry(variable key, variable arr_name) begin
    variable arr;
    arr := load_array(arr_name);
    if (arr == 0) then return 0;
    arr[key] := 0;
    save_array(arr_name, arr);
    return 1;
end

// ------------------------------------------------------------
// camp_build_camp -- single source of truth for setting up a camp.
//   If party has PID_FIREWOOD (286): consume 1, spawn firepit + 2x light,
//     center = fire tile. Cold camp (no firewood): center = dude tile.
//   Place player bedroll (BED_2 preferred, BED_1 fallback) via 6-dir
//   spiral from center. Place each TEAM_PLAYER NPC bedroll in ring
//   slots; move NPC onto bed + ANIM_fall_back_sf. Set GVAR_PARTY_NO_FOLLOW.
//   Save state: camp_active_map, campfire_map + firepit_obj_map (if fire),
//   campfire_bedrolls, campfire_sleepers, campfire_player_bedroll.
//   Caller must do gfade_out BEFORE, gfade_in AFTER.
//   Returns: 1 if fire was lit, 0 if cold camp.
// ------------------------------------------------------------
procedure camp_build_camp begin
    variable player_tile;
    variable fire_tile;
    variable fire;
    variable fire2;
    variable center_tile;
    variable map_key;
    variable lst;
    variable npc;
    variable idx;
    variable dir;
    variable slot;
    variable ring;
    variable dir_idx;
    variable bedroll_tile;
    variable bedroll;
    variable player_bedroll;
    variable player_bedroll_tile;
    variable bedrolls_arr;
    variable sleepers_arr;
    variable had_sleeper;
    map_key := "" + cur_map_index;
    player_tile := tile_num(dude_obj);
    fire := 0;
    fire_tile := tile_num_in_direction(player_tile, 0, 1);
    if (camp_party_has_pid(286)) then begin
        call camp_party_consume_pid(286);
        fire := create_object(CAMP_PID_FIRE_PIT, fire_tile, dude_elevation);
        if (fire == 0) then begin
            for (dir := 1; dir < 6; dir++) begin
                fire_tile := tile_num_in_direction(player_tile, dir, 1);
                fire := create_object(CAMP_PID_FIRE_PIT, fire_tile, dude_elevation);
                if (fire != 0) then break;
            end
        end
    end
    if (fire != 0) then center_tile := fire_tile;
    else center_tile := player_tile;
    if (fire != 0) then begin
        set_object_data(fire, OBJ_DATA_LIGHT_DISTANCE, 8);
        set_object_data(fire, OBJ_DATA_LIGHT_INTENSITY, 65536);
        fire2 := create_object(CAMP_PID_FIRE_PIT, fire_tile, dude_elevation);
        if (fire2 != 0) then begin
            set_object_data(fire2, OBJ_DATA_LIGHT_DISTANCE, 8);
            set_object_data(fire2, OBJ_DATA_LIGHT_INTENSITY, 65536);
        end
    end
    bedrolls_arr := create_array(0, 4);
    sleepers_arr := create_array(0, 4);
    had_sleeper := 0;
    player_bedroll := 0;
    for (dir := 0; dir < 6; dir++) begin
        player_bedroll_tile := tile_num_in_direction(center_tile, dir, 5);
        player_bedroll := create_object(CAMP_PID_BED_2, player_bedroll_tile, dude_elevation);
        if (player_bedroll == 0) then
            player_bedroll := create_object(CAMP_PID_BED_1, player_bedroll_tile, dude_elevation);
        if (player_bedroll != 0) then break;
    end
    if (player_bedroll != 0) then begin
        resize_array(bedrolls_arr, len_array(bedrolls_arr) + 1);
        bedrolls_arr[len_array(bedrolls_arr) - 1] := player_bedroll;
        call camp_save_map_entry(map_key, player_bedroll, "campfire_player_bedroll");
    end
    idx := 1;
    lst := list_begin(LIST_CRITTERS);
    npc := list_next(lst);
    while (npc) do begin
        if (npc != 0 and npc != dude_obj
            and get_team(npc) == TEAM_PLAYER
            and not(is_critter_dead(npc))) then begin
            slot := idx;
            ring := slot / 6;
            dir_idx := slot % 6;
            bedroll_tile := tile_num_in_direction(center_tile, dir_idx, 5 + ring * 3);
            bedroll := create_object(CAMP_PID_BED_1, bedroll_tile, dude_elevation);
            if (bedroll != 0) then begin
                resize_array(bedrolls_arr, len_array(bedrolls_arr) + 1);
                bedrolls_arr[len_array(bedrolls_arr) - 1] := bedroll;
                move_to(npc, bedroll_tile, dude_elevation);
                reg_anim_begin();
                reg_anim_animate(npc, ANIM_fall_back_sf, -1);
                reg_anim_end();
                resize_array(sleepers_arr, len_array(sleepers_arr) + 1);
                sleepers_arr[len_array(sleepers_arr) - 1] := npc;
                had_sleeper := 1;
                idx := idx + 1;
            end
        end
        npc := list_next(lst);
    end
    list_end(lst);
    if (had_sleeper) then set_global_var(398, 1);
    tile_refresh_display;
    call camp_save_map_entry(map_key, 1, "camp_active_map");
    if (fire != 0) then begin
        call camp_save_map_entry(map_key, fire_tile, "campfire_map");
        call camp_save_map_entry(map_key, fire, "campfire_firepit_obj");
    end
    call camp_save_map_entry(map_key, bedrolls_arr, "campfire_bedrolls");
    call camp_save_map_entry(map_key, sleepers_arr, "campfire_sleepers");
    if (fire != 0) then play_sfx("FLAMETHR");
    if (fire != 0) then return 1;
    return 0;
end

// ------------------------------------------------------------
// camp_teardown_camp -- destroy firepit(s), wake sleepers, destroy
// bedrolls, clear all state save_arrays. Caller does gfade_out/in.
// ------------------------------------------------------------
procedure camp_teardown_camp begin
    variable map_key;
    variable fp_map;
    variable fire;
    variable sweep_lst;
    variable sweep_obj;
    variable sl_map;
    variable sleepers;
    variable bed_map;
    variable bedrolls;
    variable i;
    variable obj;
    map_key := "" + cur_map_index;
    // Destroy tracked firepit.
    fp_map := load_array("campfire_firepit_obj");
    if (fp_map != 0) then begin
        fire := fp_map[map_key];
        if (fire != 0) then destroy_object(fire);
    end
    // Also sweep any other PID_FIRE_PIT on map (c_fire2 stacked twins).
    sweep_lst := list_begin(LIST_SCENERY);
    sweep_obj := list_next(sweep_lst);
    while (sweep_obj) do begin
        if (sweep_obj != 0 and obj_pid(sweep_obj) == CAMP_PID_FIRE_PIT) then
            destroy_object(sweep_obj);
        sweep_obj := list_next(sweep_lst);
    end
    list_end(sweep_lst);
    // Wake sleepers.
    sl_map := load_array("campfire_sleepers");
    if (sl_map != 0) then begin
        sleepers := sl_map[map_key];
        if (sleepers != 0) then begin
            for (i := 0; i < len_array(sleepers); i++) begin
                obj := sleepers[i];
                if (obj != 0 and not(is_critter_dead(obj))) then begin
                    reg_anim_begin();
                    reg_anim_animate(obj, ANIM_back_to_standing, -1);
                    reg_anim_end();
                end
            end
            sl_map[map_key] := 0;
            save_array("campfire_sleepers", sl_map);
            free_array(sleepers);
        end
    end
    // Destroy bedrolls.
    bed_map := load_array("campfire_bedrolls");
    if (bed_map != 0) then begin
        bedrolls := bed_map[map_key];
        if (bedrolls != 0) then begin
            for (i := 0; i < len_array(bedrolls); i++) begin
                obj := bedrolls[i];
                if (obj != 0) then destroy_object(obj);
            end
            bed_map[map_key] := 0;
            save_array("campfire_bedrolls", bed_map);
            free_array(bedrolls);
        end
    end
    // Clear map keys on remaining tracked state.
    call camp_clear_map_entry(map_key, "campfire_firepit_obj");
    call camp_clear_map_entry(map_key, "campfire_map");
    call camp_clear_map_entry(map_key, "campfire_player_bedroll");
    call camp_clear_map_entry(map_key, "camp_active_map");
    set_global_var(398, 0);
    tile_refresh_display;
end

// ------------------------------------------------------------
// camp_scout_haul -- spawn 2x each of 8 scavenge items to dude.
// Iguana is handled separately (fire-gated).
// ------------------------------------------------------------
procedure camp_scout_haul begin
    variable pass;
    for (pass := 0; pass < 2; pass++) begin
        call camp_give_item(286); // firewood
        call camp_give_item(19);  // rock
        call camp_give_item(278); // flint
        call camp_give_item(365); // spore spike
        call camp_give_item(271); // broc
        call camp_give_item(272); // xander
        call camp_give_item(117); // flower
        call camp_give_item(320); // sharpened pole
    end
end

// Feed a single party member from the shared food/water pool until they
// reach max HP or the pool runs out. Returns an int packing two counters:
//   (food_eaten * 100) + drinks_drunk  -- caller unpacks for reporting.
// HP gain per item: food = 10, drink = 5. Only acts if member is wounded
// (cur < max). Shared basket: pulls from dude + any TEAM_PLAYER inven.
procedure camp_feed_member(variable member) begin
    variable cur;
    variable max_hp;
    variable food_eaten;
    variable drinks_drunk;
    food_eaten := 0;
    drinks_drunk := 0;
    if (member == 0) then return 0;
    if (is_critter_dead(member)) then return 0;
    cur := get_critter_stat(member, STAT_current_hp);
    max_hp := get_critter_stat(member, STAT_max_hp);
    // Food loop: +10 HP each. Priority mutated_fruit -> iguana -> meat.
    while (cur < max_hp and (camp_party_has_pid(71) or camp_party_has_pid(81) or camp_party_has_pid(103))) do begin
        if (camp_party_has_pid(71)) then begin
            call camp_party_consume_pid(71);
        end
        else if (camp_party_has_pid(81)) then begin
            call camp_party_consume_pid(81);
        end
        else begin
            call camp_party_consume_pid(103);
        end
        critter_heal(member, 10);
        food_eaten := food_eaten + 1;
        cur := get_critter_stat(member, STAT_current_hp);
    end
    // Drink loop: +5 HP each. Priority water -> nuka.
    while (cur < max_hp and (camp_party_has_pid(126) or camp_party_has_pid(106))) do begin
        if (camp_party_has_pid(126)) then begin
            call camp_party_consume_pid(126);
        end
        else begin
            call camp_party_consume_pid(106);
        end
        critter_heal(member, 5);
        drinks_drunk := drinks_drunk + 1;
        cur := get_critter_stat(member, STAT_current_hp);
    end
    return (food_eaten * 100) + drinks_drunk;
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
// CAMP MACROS -- thin wrappers around helper procedures.
// Heavy lifting lives in camp_build_camp / camp_teardown_camp /
// camp_scout_haul so the same code isn't duplicated per action.
// ============================================================

// MAKE CAMP -- orphan macro kept for compat; currently unused since
// Scout+Camp merge (Borys spec). May be reintroduced if manual Make
// comes back as separate action.
#define CAMP_DO_MAKE \
    variable c_fire_lit; \
    gfade_out(600); \
    game_time_advance(600); \
    c_fire_lit := camp_build_camp; \
    gfade_in(600); \
    give_exp_points(15);

// PACK CAMP -- tear everything down via helper.
#define CAMP_DO_PACK \
    gfade_out(300); \
    call camp_teardown_camp; \
    gfade_in(300); \
    give_exp_points(10);

// SCOUT + CAMP -- search area, build camp (with fire if firewood found),
// roll iguana bonus if fire lit. Single dialog action.
#define CAMP_DO_SCOUT_AND_CAMP \
    variable c_fire_lit; \
    variable c_iguanas; \
    variable c_ig; \
    call camp_mark_searched; \
    call camp_scout_haul; \
    gfade_out(600); \
    game_time_advance(600); \
    c_fire_lit := camp_build_camp; \
    if (c_fire_lit) then begin \
        c_iguanas := 1 + random(0, 2); \
        for (c_ig := 0; c_ig < c_iguanas; c_ig++) begin \
            call camp_give_item(81); \
        end \
    end \
    gfade_in(600); \
    give_exp_points(100);


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
// FOOD & DRINK -- SHARED BASKET healing (Borys spec):
//   Every wounded member eats/drinks from the COMMON party pool
//   until they hit max HP, or the basket runs empty. A member with
//   full HP does not consume anything. Whoever carries the food
//   contributes -- dude and all party NPC inventories combined.
// Pools:
//   food (+10 HP): mutated fruit (71), iguana stick (81), meat stick (103)
//   drink (+5 HP): water flask (126), nuka cola (106)
// ============================================================
#define CAMP_DO_FOOD \
    variable f_lst; \
    variable f_obj; \
    variable f_anyone_wounded; \
    variable f_packed; \
    variable f_total_food; \
    variable f_total_drink; \
    f_total_food := 0; \
    f_total_drink := 0; \
    f_anyone_wounded := 0; \
    if (get_critter_stat(dude_obj, STAT_current_hp) < get_critter_stat(dude_obj, STAT_max_hp)) then \
        f_anyone_wounded := 1; \
    if (f_anyone_wounded == 0) then begin \
        f_lst := list_begin(LIST_CRITTERS); \
        f_obj := list_next(f_lst); \
        while (f_obj) do begin \
            if (f_obj != 0 and f_obj != dude_obj \
                and get_team(f_obj) == TEAM_PLAYER \
                and not(is_critter_dead(f_obj)) \
                and get_critter_stat(f_obj, STAT_current_hp) < get_critter_stat(f_obj, STAT_max_hp)) then \
                f_anyone_wounded := 1; \
            f_obj := list_next(f_lst); \
        end \
        list_end(f_lst); \
    end \
    if (f_anyone_wounded == 0) then begin \
        Reply("Everyone's full up. No point wasting rations right now."); \
    end \
    else begin \
        f_packed := camp_feed_member(dude_obj); \
        f_total_food := f_packed / 100; \
        f_total_drink := f_packed - (f_total_food * 100); \
        f_lst := list_begin(LIST_CRITTERS); \
        f_obj := list_next(f_lst); \
        while (f_obj) do begin \
            if (f_obj != 0 and f_obj != dude_obj \
                and get_team(f_obj) == TEAM_PLAYER \
                and not(is_critter_dead(f_obj))) then begin \
                f_packed := camp_feed_member(f_obj); \
                f_total_food := f_total_food + (f_packed / 100); \
                f_total_drink := f_total_drink + (f_packed - ((f_packed / 100) * 100)); \
            end \
            f_obj := list_next(f_lst); \
        end \
        list_end(f_lst); \
        if (f_total_food == 0 and f_total_drink == 0) then begin \
            Reply("Basket's empty. Nothing left to share. Someone should scout for grub."); \
        end \
        else if (f_total_food > 0 and f_total_drink > 0) then begin \
            Reply("Passed the basket around. Everyone took what they needed. Good."); \
        end \
        else if (f_total_food > 0) then begin \
            Reply("Chewed through what we had. Water's running low, though."); \
        end \
        else begin \
            Reply("Drank down what was left. Nothing solid in the basket, though."); \
        end \
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
// Helper: absolute-HP thresholds (Borys spec, not percent):
//   cur == max       -> healthy (not reported)
//   cur >= max/2     -> "wounded"
//   cur >= 10        -> "severely wounded"
//   cur < 10         -> "almost dead"
// Caller must pass current HP + max HP + output string var.
#define CAMP_WOUND_STATUS(cur_var, max_var, status_var) \
    if      (cur_var < 10)            then status_var := "almost dead"; \
    else if (cur_var * 2 <= max_var)  then status_var := "severely wounded"; \
    else                              status_var := "wounded";

#define CAMP_DO_WOUNDED \
    variable w_lst; \
    variable w_obj; \
    variable w_cur; \
    variable w_max; \
    variable w_status; \
    variable w_report; \
    variable w_first; \
    variable w_phrase; \
    w_report := ""; \
    w_first := 1; \
    /* dude first -- perspective of self_obj (the NPC the player talks to) */ \
    /*   dude     -> "You're <status>"                                     */ \
    /*   self_obj -> "I'm <status>"                                        */ \
    /*   other    -> "<Name> is <status>"                                  */ \
    w_cur := get_critter_stat(dude_obj, STAT_current_hp); \
    w_max := get_critter_stat(dude_obj, STAT_max_hp); \
    if (w_cur < w_max) then begin \
        CAMP_WOUND_STATUS(w_cur, w_max, w_status) \
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
                CAMP_WOUND_STATUS(w_cur, w_max, w_status) \
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
        Reply("Nobody's complaining. We're all in one piece."); \
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
