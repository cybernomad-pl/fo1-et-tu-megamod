/*
    megamod_debug.h -- centralny wlacznik debug-logow (Borys 2026-08-11).

    Sterowanie BEZ rekompilacji:  mods/sfall-mods.ini
        [Megamod]
        DebugLog=1     <- 1 = loguj (testing.ini + debug_msg), 0/brak = cisza

    Uzycie w skryptach:
      #include "megamod_debug.h"
      #define LOG(s, k, v) if (megamod_dbg_on) then set_ini_setting(...)
      LOGD("tekst")   -- warunkowy debug_msg (ekran/debug.log sfall)

    Stan cache'owany po pierwszym odczycie (zero kosztu ini per tick).
*/
#ifndef MEGAMOD_DEBUG_H
#define MEGAMOD_DEBUG_H

variable mdbg_state := -1;   // -1 = nieodczytane, 0 = off, 1 = on

procedure megamod_dbg_on;
procedure megamod_dbg_on begin
   if (mdbg_state == -1) then begin
      if (get_ini_setting("mods\\sfall-mods.ini|Megamod|DebugLog") == 1) then
         mdbg_state := 1;
      else
         mdbg_state := 0;
   end
   return mdbg_state;
end

#define LOGD(x)   if (megamod_dbg_on) then debug_msg(x)

#endif
