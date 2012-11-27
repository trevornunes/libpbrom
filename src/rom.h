#ifndef _ROM_H_
#define _ROM_H_

/* Rom is a class that provides file path setting and rom selection, iteration, cycling
 * It saves repeating the same type of code on multiple emulators.
 */
using namespace std;
#include <vector>
#include <string>
#include <cstdio>

class Rom {

public:

	typedef enum {
		rom_nes_c = 0,
		rom_gb_c,
		rom_gbc_c,
		rom_gba_c,
		rom_pce_c,
		rom_smd_c,
		rom_lnx_c,
		rom_max_c
	} rom_type_t;

	Rom();

	Rom(rom_type_t rtype);

	bool pathExists(string dpath);

	vector<string> getRomList(void); // get string vector of all roms full path.
	const char *getRomNext(void); // increment up the list
	const char *getRomPrev(void); // decrement down the list

	bool setupSdCard(void); // BB10 phones and SDCARD supporting units. TBC
	void setRomPath(string path);
	string getRomPath() {
		return activeRomPath_m;
	}
	; // get full path of current rom file.
	string getActiveRomName() {
		return activeRom_m;
	}
	; // what is the name of the file minus path ?
	int romCount() {
		return activeRomList_vsm.size();
	}
	; // how many rom files in list ?
	void setActiveRomBad(); // tag this file as being bad, e.g emulator loads but finds it's corrupt.
	bool isBadRom(); // is the current rom index pointing to a 'bad rom' file ?
	void setRomIndex(unsigned int idx); // set the current rom browser to a specific index.

private:
	string emuTypeStr_m;
	string activeRomPath_m;
	string cfgFilePath_m;
	vector<string> activeRomList_vsm;
	vector<string> extensions_vsm;
	vector<unsigned int> badRomList_vim;
	string activeRom_m;
	rom_type_t romType_m;
	unsigned int activeRomIndex_m;

	void sort(void);
	bool extensionIsValid(string ext);
	void saveState(void);
	void loadState(void);
	bool isADir(string dpath);
	void setupPath(string path);
};

#endif /* _ROM_H_ */
