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
procedure CampPackTrigger;
procedure CampPackExecute;
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
   NOption("About your orders...", NodePartyTactics, 4);
   if (map_is_encounter) then begin
      if (camp_active_here) then
         NOption("Time to break camp.", CampPackTrigger, 4);
      else if not(camp_searched_here) then
         NOption("Let's scout this area and set up for the night.", CampScoutTrigger, 4);
   end
   NOption("Got a minute?", NodePartyNeeds, 4);
   NOption("Later.", NodePartyDone, 4);
end

// ============================================================
// NodePartyTactics -- Follow / Wait / Holster submenu
// ============================================================
procedure NodePartyTactics begin
   Reply_Blank;
   NOption("Follow me.", NodePartyFollow, 4);
   NOption("Hold this spot.", NodePartyWait, 4);
   NOption("Put away your weapon.", NodePartyHolster, 4);
   NOption("On second thought...", NodePartyMain, 4);
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
