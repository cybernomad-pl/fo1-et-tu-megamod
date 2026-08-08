/*
    party_dialog.h -- shared dialog node procedures for party NPCs.

    DRY: defined ONCE here, 37 NPCs `#include` this header.

    MUST be included AFTER command.h / modreact.h (needs Reply, NOption,
    get_team, dude_obj, etc.). camp_actions.h included below.

    NPC-side responsibilities:
      - talk_p_proc: start_gdialog + gSay_Start + call NodePartyMain + gSay_End
      - NodeRecruit: party_add + set_team + call camp_grant_barter
      - NodeCampWakeUp (LIST_SCENERY bedroll destroy -- per-NPC)
      - unique NPC dialogue nodes
*/

#ifndef PARTY_DIALOG_H_MEGAMOD
#define PARTY_DIALOG_H_MEGAMOD

#include "camp_actions.h"

// ============================================================
// Forward decls -- all shared dialog nodes
// ============================================================
procedure NodePartyMain;
procedure NodePartyTactics;
procedure NodePartyNeeds;
procedure NodePartyFollow;
procedure NodePartyClose;
procedure NodePartyMedium;
procedure NodePartyFar;
procedure NodePartyWait;
procedure NodePartyHolster;
procedure NodePartyDone;
procedure CampScoutTrigger;
procedure CampScoutExecute;
procedure CampRemakeTrigger;
procedure CampRemakeExecute;
procedure NodePartyGarrison;
procedure PartyGarrisonExecute;
procedure PartyRejoinExecute;
// megamod_garrison_here -- zdefiniowana w command.h (uzywana takze
// przez skrypty bez party_dialog.h: IAN, TANDI, VASQUEZ).
procedure CampPackTrigger;
procedure CampPackExecute;
procedure CampBuryTrigger;
procedure CampBuryExecute;
procedure CampIdeasTrigger;
procedure PartyAmmoTrigger;
procedure PartyFoodTrigger;
procedure PartyWoundedTrigger;
procedure CraftSpearTrigger;
procedure CraftSharpSpearTrigger;
procedure CraftHealingPowderTrigger;
procedure CraftAntidoteTrigger;
procedure CraftMolotovTrigger;
procedure NodeCampWakeUp;
procedure NodePartyTalkNormal;
procedure party_wants_normal_talk;


// ============================================================
// NodePartyMain -- top level dialog for a party member.
// Single "scout & set up camp" action (wilderness only) replaces old
// Make + Search. Pack only visible when a camp is already up.
// ============================================================
procedure NodePartyMain begin
   Reply_Blank;
   if (global_var(GVAR_PARTY_NO_FOLLOW) == 1 and local_var(13) == 0 and is_critter_prone(self_obj)) then begin
      NOption("On your feet. Let's go.", NodeCampWakeUp, 4);
   end
   // GARNIZON (Borys 2026-08-08): powrot do party widoczny TYLKO dla
   // garnizonowego NPC (poza party, TEAM_PLAYER, czeka na rozkazy).
   if (megamod_garrison_here) then begin
      NOption("Fall in. We're moving out.", PartyRejoinExecute, 4);
   end
   NOption("About your orders...", NodePartyTactics, 4);
   if (map_is_encounter) then begin
      if (camp_active_here) then begin
         NOption("Time to break camp.", CampPackTrigger, 4);
         NOption("Let's bury the dead around here.", CampBuryTrigger, 4);
      end
      else if not(camp_searched_here) then
         NOption("Let's scout this area and set up for the night.", CampScoutTrigger, 4);
      else
         NOption("Let's set up camp again.", CampRemakeTrigger, 4);
   end
   NOption("Got a minute?", NodePartyNeeds, 4);
   NOption("Relax. Tell me something.", NodePartyTalkNormal, 4);
   NOption("Later.", NodePartyDone, 4);
end

// ============================================================
// NodePartyTactics -- Follow / Wait / Holster submenu
// ============================================================
procedure NodePartyTactics begin
   Reply_Blank;
   NOption("Follow me.", NodePartyFollow, 4);
   NOption("Hold this spot.", NodePartyWait, 4);
   if (not(megamod_garrison_here)) then begin
      NOption("Stay here for good. Hold this ground until I come back.", NodePartyGarrison, 4);
   end
   NOption("Put away your weapon.", NodePartyHolster, 4);
   NOption("On second thought...", NodePartyMain, 4);
   NOption("Later.", NodePartyDone, 4);
end

// ============================================================
// GARNIZON (Borys 2026-08-08) -- "zostan tu na stale, jak Dogmeat".
// party_remove: NPC wypada z party (nie podrozuje z graczem), team
// ZOSTAJE TEAM_PLAYER (walczy po stronie gracza), follow zablokowany
// przez megamod_garrison_here w party_follow_dude_point (command.h).
// Stan: save_array "garrison", klucz = sfall unique_id (przezywa save).
// ============================================================
procedure NodePartyGarrison begin
   Reply("You want me to hold this ground while you move on? Say the word.");
   NOption("That's an order. I'll be back for you.", PartyGarrisonExecute, 4);
   NOption("Forget it.", NodePartyTactics, 4);
end

procedure PartyGarrisonExecute begin
   variable grsn_arr;
   variable grsn_id;
   grsn_arr := load_array("garrison");
   if (grsn_arr == 0) then begin
      grsn_arr := create_array_map;
   end
   grsn_id := set_unique_id(self_obj);
   grsn_arr[grsn_id] := 1;
   save_array("garrison", grsn_arr);
   party_remove(self_obj);
   Reply("Understood. This ground is mine. Nothing gets past me.");
   NOption("Later.", NodePartyDone, 4);
end

procedure PartyRejoinExecute begin
   variable grsn_arr;
   variable grsn_id;
   grsn_arr := load_array("garrison");
   if (grsn_arr != 0) then begin
      grsn_id := set_unique_id(self_obj);
      grsn_arr[grsn_id] := 0;
      save_array("garrison", grsn_arr);
   end
   party_add(self_obj);
   set_team(self_obj, TEAM_PLAYER);
   Reply("Good to be moving again. Lead on.");
   NOption("Later.", NodePartyDone, 4);
end

// ============================================================
// NodePartyNeeds -- camp actions / party maintenance
// ============================================================
procedure NodePartyNeeds begin
   Reply_Blank;
   NOption("Got any ideas we can put together?", CampIdeasTrigger, 4);
   NOption("Let's check on ammo.", PartyAmmoTrigger, 4);
   NOption("Time to break for rations.", PartyFoodTrigger, 4);
   NOption("How is everyone holding up?", PartyWoundedTrigger, 4);
   NOption("On second thought...", NodePartyMain, 4);
   NOption("Later.", NodePartyDone, 4);
end

// ============================================================
// NodePartyTalkNormal -- flag THIS npc to open its pre-recruitment
// dialog on the NEXT talk (one-shot), then close. Player re-clicks the
// NPC for the normal chat; afterwards it reverts to the party menu
// automatically. NPC stays in the party the whole time. (Borys task 5.)
// Flag store: save_array "party_talk_normal" -> obj handle (session).
// ============================================================
procedure NodePartyTalkNormal begin
   variable arr;
   arr := load_array("party_talk_normal");
   if (arr == 0) then arr := create_array_map;
   arr["obj"] := self_obj;
   save_array("party_talk_normal", arr);
   Reply("Sure -- talk to me again, I'm all yours for a moment.");
   NOption("Alright.", NodePartyDone, 4);
end

// Returns 1 exactly ONCE if this npc was flagged by NodePartyTalkNormal,
// consuming the flag; otherwise 0. Called from each NPC's talk gate:
//   if (self_team == TEAM_PLAYER and not(party_wants_normal_talk)) ...
// SSL 'and' is not short-circuit, so this runs every talk -- safe: it
// only matches (and consumes) when the stored obj == self_obj.
procedure party_wants_normal_talk begin
   variable arr;
   arr := load_array("party_talk_normal");
   if (arr == 0) then return 0;
   if (arr["obj"] != self_obj) then return 0;
   arr["obj"] := 0;
   save_array("party_talk_normal", arr);
   return 1;
end

// ============================================================
// Follow distance submenu + leaf nodes
// ============================================================
procedure NodePartyFollow begin
   Reply_Blank;
   NOption("Stay close.", NodePartyClose, 4);
   NOption("Keep moderate distance.", NodePartyMedium, 4);
   NOption("Hang back and cover the flank.", NodePartyFar, 4);
   NOption("Hold on.", NodePartyTactics, 4);
   NOption("Later.", NodePartyDone, 4);
end

procedure NodePartyClose begin
   Reply("Right on your shoulder.");
   NOption("Hold on.", NodePartyTactics, 4);
   NOption("Later.", NodePartyDone, 4);
end

procedure NodePartyMedium begin
   Reply("Fine. A few paces back.");
   NOption("Hold on.", NodePartyTactics, 4);
   NOption("Later.", NodePartyDone, 4);
end

procedure NodePartyFar begin
   Reply("I'll hang back. Cover the flank.");
   NOption("Hold on.", NodePartyTactics, 4);
   NOption("Later.", NodePartyDone, 4);
end

procedure NodePartyWait begin
   Reply("Got it. I'll hold this spot.");
   NOption("Hold on.", NodePartyTactics, 4);
   NOption("Later.", NodePartyDone, 4);
end

procedure NodePartyHolster begin
   inven_unwield(self_obj);
   Reply("Putting it away.");
   NOption("Hold on.", NodePartyTactics, 4);
   NOption("Later.", NodePartyDone, 4);
end

procedure NodePartyDone begin
end

// ============================================================
// SCOUT & CAMP -- merged Make + Search.
// Reply + NOption confirm. On execute: search returns items, and
// if firewood was found, fire is lit + iguanas (random, gated by
// firewood count). Bedrolls always set up. Fire is optional.
// ============================================================
procedure CampScoutTrigger begin
   if (not(map_is_encounter)) then begin
      Reply("No place to camp here. Too exposed, too close to trouble. We need the wild.");
      NOption("Forget it.", NodePartyMain, 4);
   end
   else if (combat_is_initialized) then begin
      Reply("Bit busy for that right now, boss. Bullets first, blankets later.");
      NOption("Forget it.", NodePartyMain, 4);
   end
   else begin
      Reply("Alright. We'll fan out, see what we can scrape together, then set up bedrolls. If we find wood, we'll get a fire going.");
      NOption("Do it.", CampScoutExecute, 4);
      NOption("Forget it.", NodePartyMain, 4);
   end
end

procedure CampScoutExecute begin
   CAMP_DO_SCOUT_AND_CAMP
end

// ============================================================
// RE-CAMP -- area already scavenged (camp_searched_here == 1): build the
// camp again WITHOUT the scout haul / iguana bonus. Borys 2026-08-03:
// second camp on the same map must NOT farm items.
// ============================================================
procedure CampRemakeTrigger begin
   if (not(map_is_encounter)) then begin
      Reply("No place to camp here. Too exposed, too close to trouble. We need the wild.");
      NOption("Forget it.", NodePartyMain, 4);
   end
   else if (combat_is_initialized) then begin
      Reply("Bit busy for that right now, boss. Bullets first, blankets later.");
      NOption("Forget it.", NodePartyMain, 4);
   end
   else begin
      Reply("We've already picked this place clean, but we can roll out the bedrolls again.");
      NOption("Do it.", CampRemakeExecute, 4);
      NOption("Forget it.", NodePartyMain, 4);
   end
end

procedure CampRemakeExecute begin
   CAMP_DO_MAKE
end

// ============================================================
// PACK CAMP -- Reply + NOption confirm. Execute tears down + fade.
// ============================================================
procedure CampPackTrigger begin
   if not(camp_active_here) then begin
      Reply("No camp here to pack up.");
      NOption("Forget it.", NodePartyMain, 4);
   end
   else begin
      Reply("Alright. Let's break camp and get moving.");
      NOption("Let's move.", CampPackExecute, 4);
      NOption("Forget it.", NodePartyMain, 4);
   end
end

procedure CampPackExecute begin
   CAMP_DO_PACK
end

// ============================================================
// BURY THE DEAD (Borys task 4) -- party order, camp active on an encounter
// map. Buries EVERY corpse on the map. Loot is NOT destroyed: each corpse's
// belongings are dropped onto its tile (floor) before the body is removed,
// so the player must still pick them up.
// ============================================================
procedure CampBuryTrigger begin
   if (combat_is_initialized) then begin
      Reply("Not while there's still shooting, boss.");
      NOption("Forget it.", NodePartyMain, 4);
   end
   else begin
      Reply("Grim work, but alright. We'll put them under. Whatever they were carrying stays on the ground.");
      NOption("Do it.", CampBuryExecute, 4);
      NOption("Forget it.", NodePartyMain, 4);
   end
end

procedure CampBuryExecute begin
   variable lst;
   variable obj;
   variable arr;
   variable k;
   variable corpse;
   variable item;
   variable t;
   variable elev;
   variable cnt;
   variable guard;

   // Pass 1: collect dead critters. Never destroy while iterating the
   // engine's LIST_CRITTERS (stale-pointer crash) -- collect, then act.
   arr := create_array(0, 4);
   lst := list_begin(LIST_CRITTERS);
   obj := list_next(lst);
   while (obj) do begin
      if (obj != 0) then begin
         if (obj != dude_obj) then begin
            if (is_critter_dead(obj)) then begin
               resize_array(arr, len_array(arr) + 1);
               arr[len_array(arr) - 1] := obj;
            end
         end
      end
      obj := list_next(lst);
   end
   list_end(lst);

   if (len_array(arr) == 0) then begin
      free_array(arr);
      Reply("Nothing here to bury.");
      NOption("Right.", NodePartyMain, 4);
   end
   else begin
      gfade_out(300);
      cnt := 0;
      for (k := 0; k < len_array(arr); k++) begin
         corpse := arr[k];
         if (corpse != 0) then begin
            t := tile_num(corpse);
            elev := elevation(corpse);
            // Drop this corpse's loot onto its ground tile before removing
            // it (destroy_object would take the loot with it otherwise).
            guard := 0;
            item := inven_ptr(corpse, 0);
            while (item and guard < 100) do begin
               rm_obj_from_inven(corpse, item);
               move_to(item, t, elev);
               guard := guard + 1;
               item := inven_ptr(corpse, 0);
            end
            destroy_object(corpse);
            cnt := cnt + 1;
         end
      end
      free_array(arr);
      tile_refresh_display;
      gfade_in(300);
      give_exp_points(10);
      display_msg("The party buries the dead. Their belongings are left on the ground.");
   end
end

// ============================================================
// AMMO / FOOD / WOUNDED -- macro emits Reply, NOption navigation.
// Dialog stays open after action (returns to caller submenu).
// ============================================================
procedure PartyAmmoTrigger begin
   CAMP_DO_AMMO
   NOption("Hold on.", NodePartyNeeds, 4);
   NOption("Later.", NodePartyDone, 4);
end

procedure PartyFoodTrigger begin
   CAMP_DO_FOOD
   NOption("Hold on.", NodePartyNeeds, 4);
   NOption("Later.", NodePartyDone, 4);
end

procedure PartyWoundedTrigger begin
   CAMP_DO_WOUNDED
   NOption("Hold on.", NodePartyNeeds, 4);
   NOption("Later.", NodePartyDone, 4);
end

// ============================================================
// CampIdeasTrigger -- crafts submenu (only shows what's possible).
// Craft*Trigger executes macro + NOption back (re-enters this node).
// ============================================================
procedure CampIdeasTrigger begin
   Reply_Blank;
   if (camp_party_has_pid(320) and (camp_party_has_pid(4) or camp_party_has_pid(236))) then
      NOption("Fashion a spear from a sharpened pole and knife.", CraftSpearTrigger, 4);
   if (camp_party_has_pid(7) and camp_party_has_pid(278)) then
      NOption("Tip a spear with flint for a sharpened spear.", CraftSharpSpearTrigger, 4);
   if (camp_party_has_pid(271) and camp_party_has_pid(272)) then
      NOption("Grind broc and xander into healing powder.", CraftHealingPowderTrigger, 4);
   if (camp_party_has_pid(92) and camp_party_has_pid(125)) then
      NOption("Distill a scorpion tail and booze into antidote.", CraftAntidoteTrigger, 4);
   if (camp_party_has_pid(125) and camp_party_has_pid(101)) then
      NOption("Soak a rag in booze, flick the lighter. Molotov.", CraftMolotovTrigger, 4);
   NOption("Hold on.", NodePartyNeeds, 4);
   NOption("Later.", NodePartyDone, 4);
end

procedure CraftSpearTrigger begin
   CAMP_CRAFT_SPEAR
   NOption("Anything else we can make?", CampIdeasTrigger, 4);
   NOption("Later.", NodePartyDone, 4);
end

procedure CraftSharpSpearTrigger begin
   CAMP_CRAFT_SHARP_SPEAR
   NOption("Anything else we can make?", CampIdeasTrigger, 4);
   NOption("Later.", NodePartyDone, 4);
end

procedure CraftHealingPowderTrigger begin
   CAMP_CRAFT_HEALING_POWDER
   NOption("Anything else we can make?", CampIdeasTrigger, 4);
   NOption("Later.", NodePartyDone, 4);
end

procedure CraftAntidoteTrigger begin
   CAMP_CRAFT_ANTIDOTE
   NOption("Anything else we can make?", CampIdeasTrigger, 4);
   NOption("Later.", NodePartyDone, 4);
end

procedure CraftMolotovTrigger begin
   CAMP_CRAFT_MOLOTOV
   NOption("Anything else we can make?", CampIdeasTrigger, 4);
   NOption("Later.", NodePartyDone, 4);
end

#endif
