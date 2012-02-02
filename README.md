PlaybookRom class in a shared lib. This is to manage rom/file lists in a common way for the emulators I've ported I didn't like replicating similar code so it makes it easier to maintain a rom browsing library that can be enhanced over time.

#include "pbrom.h"

PlaybookRom romz(rom_nes_c);
// sets misc/roms/nes as rom path .. which can be overrided.


romz.updateRomList();
// builds up a list of .nes .NES or .zip files in this directory 
// initializes the navigator index to 0

romz.getNextRom();
// will return the first rom path in the list ...

romz.getActiveRomName();
// returns the current name of the rom as a string.

see the pbrom.h for more methods.

