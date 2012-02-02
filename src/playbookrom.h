#ifndef PLAYBOOKROM_H_
#define PLAYBOOKROM_H_

/* PlaybookRom is a class that provides ROM path setting and rom selection, iteration, cycling
 * It saves repeating the same type of code on multiple emulators.
 */
#include <vector>
#include <string>
using namespace std;
#include <cstdio>



class PlaybookRom {

public:


typedef enum {
		rom_nes_c = 0,
		rom_gba_c = 1,
		rom_pce_c,
		rom_smd_c
}rom_type_t;

PlaybookRom();

PlaybookRom(rom_type_t rtype);

bool           pathExists(string dpath);
vector<string> getRomList(void);
const char    *getRomNext(void);
void           updateRomList(void);
void           setRomPath(string path);
string         getRomPath() { return activeRomPath_m; };
string         getActiveRomName() { return activeRom_m; };
void           setActiveRomBad(void);
bool           isBadRom();

void           setRomIndex(int idx);
private:

 string                  activeRomPath_m;
 vector<string>          activeRomList_vsm;
 vector<unsigned int>    badRomList_vim;
 string                  activeRom_m;
 rom_type_t              romType_m;
 unsigned int            activeRomIndex_m;
};




#endif /* PLAYBOOKROM_H_ */
