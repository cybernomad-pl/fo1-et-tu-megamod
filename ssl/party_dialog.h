/*
    party_dialog.h -- shared dialog node procedures for party NPCs.

    DRY architecture: defined ONCE here, 37 NPCs `#include` this header.
    Each NPC's script gets its own compiled copy (SSL can't truly share
    bytecode across scripts), but the SOURCE lives in one file. Any
    change to dialog structure propagates by re-running compile_all.

    MUST be included AFTER command.h / modreact.h (needs Reply, NOption,
    get_team, dude_obj, etc.).

    camp_actions.h is pulled in below -- it provides CAMP_DO_* macros,
    helper procedures (camp_has_fire_here, camp_party_has_pid, etc.),
    and action PIDs.

    NPC-side responsibilities (must exist in each NPC script):
      - talk_p_proc: start_gdialog + gSay_Start + call NodePartyMain + gSay_End
      - NodeRecruit: party_add + set_team + call camp_grant_barter
      - unique NPC dialogue nodes (Theresa01, Agatha08, etc.)
      - NPC-specific NodeCampWakeUp (LIST_SCENERY bedroll handling varies)
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
procedure CampMakeTrigger;
procedure CampMakeExecute;
procedure CampPackTrigger;
procedure CampPackExecute;
procedure CampSearchTrigger;
procedure CampIdeasTrigger;
procedure PartyAmmoTrigger;
procedure PartyFoodTrigger;
procedure PartyWoundedTrigger;
procedure CraftSpearTrigger;
procedure CraftSharpSpearTrigger;
procedure CraftHealingPowderTrigger;
procedure CraftAntidoteTrigger;
procedure CraftMolotovTrigger;

// NodeCampWakeUp is NPC-specific (scenery bed destroy logic may vary) --
// each NPC defines its own. We forward-declare for referencing here.
procedure NodeCampWakeUp;


// ============================================================
// NodePartyMain -- top-level dialog for a party member.
// Contextual: camp options only in wilderness; Pack XOR Make.
// ============================================================
procedure NodePartyMain begin
   Reply_Blank;
   if (global_var(GVAR_PARTY_NO_FOLLOW) == 1 and local_var(13) == 0 and is_critter_prone(self_obj)) then begin
      NOption("Get up and follow me.", NodeCampWakeUp, 4);
   end
   NOption("Tactics.", NodePartyTactics, 4);
   if (map_is_encounter) then begin
      if (camp_active_here) then
         NOption("Pack up camp.", CampPackTrigger, 4);
      else
         NOption("Make camp.", CampMakeTrigger, 4);
      if not(camp_searched_here) then
         NOption("Spread out and search the area for anything useful.", CampSearchTrigger, 4);
   end
   NOption("Anything else on your mind?", NodePartyNeeds, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

// ============================================================
// NodePartyTactics -- Follow/Wait/Holster submenu
// ============================================================
procedure NodePartyTactics begin
   Reply_Blank;
   NOption("Follow me.", NodePartyFollow, 4);
   NOption("Wait here.", NodePartyWait, 4);
   NOption("Holster weapon.", NodePartyHolster, 4);
   NOption("Back.", NodePartyMain, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

// ============================================================
// NodePartyNeeds -- party needs submenu (crafts/ammo/food/wounded)
// ============================================================
procedure NodePartyNeeds begin
   Reply_Blank;
   NOption("Anything on your mind?", CampIdeasTrigger, 4);
   NOption("Redistribute ammo.", PartyAmmoTrigger, 4);
   NOption("Eat and drink.", PartyFoodTrigger, 4);
   NOption("Who is hurt?", PartyWoundedTrigger, 4);
   NOption("Back.", NodePartyMain, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

// ============================================================
// Tactics leaf nodes -- Reply + NOption navigation
// ============================================================
procedure NodePartyFollow begin
   Reply_Blank;
   NOption("Stay close.", NodePartyClose, 4);
   NOption("Keep moderate distance.", NodePartyMedium, 4);
   NOption("Hang back.", NodePartyFar, 4);
   NOption("Back.", NodePartyTactics, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

procedure NodePartyClose begin
   Reply("Right on your shoulder.");
   NOption("Back.", NodePartyTactics, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

procedure NodePartyMedium begin
   Reply("Fine. A few paces back.");
   NOption("Back.", NodePartyTactics, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

procedure NodePartyFar begin
   Reply("I'll hang back. Cover the flank.");
   NOption("Back.", NodePartyTactics, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

procedure NodePartyWait begin
   Reply("Got it. I'll hold this spot.");
   NOption("Back.", NodePartyTactics, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

procedure NodePartyHolster begin
   inven_unwield(self_obj);
   Reply("Putting it away.");
   NOption("Back.", NodePartyTactics, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

procedure NodePartyDone begin
end

// ============================================================
// Camp triggers -- Reply + NOption (confirm pattern)
// Only MAKE/PACK close the dialog (via engine fade inside Execute).
// ============================================================
procedure CampMakeTrigger begin
   if (not(map_is_encounter)) then begin
      Reply("No place to camp here. We need to be out in the wild.");
      NOption("Never mind.", NodePartyMain, 4);
   end
   else if (combat_is_initialized) then begin
      Reply("Bit busy for that right now, boss. Bullets first, blankets later.");
      NOption("Never mind.", NodePartyMain, 4);
   end
   else if (camp_party_has_pid(286)) then begin
      Reply("Good. Let's get a fire going.");
      NOption("We make do.", CampMakeExecute, 4);
      NOption("Never mind.", NodePartyMain, 4);
   end
   else begin
      Reply("Good idea. Shame we have no firewood, though.");
      NOption("We make do.", CampMakeExecute, 4);
      NOption("Never mind.", NodePartyMain, 4);
   end
end

procedure CampMakeExecute begin
   CAMP_DO_MAKE
end

procedure CampPackTrigger begin
   if not(camp_active_here) then begin
      Reply("No camp here to pack up.");
      NOption("Never mind.", NodePartyMain, 4);
   end
   else begin
      Reply("Alright. Let's break camp and get moving.");
      NOption("Let's move.", CampPackExecute, 4);
      NOption("Never mind.", NodePartyMain, 4);
   end
end

procedure CampPackExecute begin
   CAMP_DO_PACK
end

// ============================================================
// SEARCH / AMMO / FOOD / WOUNDED -- macro emits Reply, NOption nav.
// Dialog stays open after action (returns to caller submenu).
// ============================================================
procedure CampSearchTrigger begin
   CAMP_DO_SEARCH
   NOption("Back.", NodePartyMain, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

procedure PartyAmmoTrigger begin
   CAMP_DO_AMMO
   NOption("Back.", NodePartyNeeds, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

procedure PartyFoodTrigger begin
   CAMP_DO_FOOD
   NOption("Back.", NodePartyNeeds, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

procedure PartyWoundedTrigger begin
   CAMP_DO_WOUNDED
   NOption("Back.", NodePartyNeeds, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

// ============================================================
// CampIdeasTrigger -- dialog node, lists available crafts.
// Each Craft*Trigger executes macro + NOption back (no recursion).
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
      NOption("Soak a rag in booze, flick the lighter, Molotov.", CraftMolotovTrigger, 4);
   NOption("Back.", NodePartyNeeds, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

procedure CraftSpearTrigger begin
   CAMP_CRAFT_SPEAR
   NOption("What else?", CampIdeasTrigger, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

procedure CraftSharpSpearTrigger begin
   CAMP_CRAFT_SHARP_SPEAR
   NOption("What else?", CampIdeasTrigger, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

procedure CraftHealingPowderTrigger begin
   CAMP_CRAFT_HEALING_POWDER
   NOption("What else?", CampIdeasTrigger, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

procedure CraftAntidoteTrigger begin
   CAMP_CRAFT_ANTIDOTE
   NOption("What else?", CampIdeasTrigger, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

procedure CraftMolotovTrigger begin
   CAMP_CRAFT_MOLOTOV
   NOption("What else?", CampIdeasTrigger, 4);
   NOption("Nothing right now.", NodePartyDone, 4);
end

#endif
