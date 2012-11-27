/*
 * Simple rom path retrieval and file/path checker. Used by emulators to simplify the repetitive logic

 #include "rom.h"

 1. Create Rom instance and directory setup /misc/roms/smd and setup config.

 Rom romp = Rom( Rom::rom_smd_c );

 2. Initialize/Update the rom listing

 romp.updateRomList(); // read the directory looking for extension types
 // that match the Rom::rom_smd_c type e.g. .smd .gen .bin
 *
 romp.getRomList(); // will return a vector<string> of roms.


 3. Now that the database is stored we can get next/prev,current index

 Now we can browse the rom listing ...

 romp.getNextRom();  // return the next rom full path  will cycle back to start ..
 romp.getActiveRom(); // get the current active rom

 */

#include "rom.h"
#include "pthread.h"
#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <fstream>

static const char *romPath_sd = "/accounts/1000/removable/sdcard/roms";
static const char *romPath_internal = "/accounts/1000/shared/misc/roms";

pthread_mutex_t dir_mutex = PTHREAD_MUTEX_INITIALIZER;
#define DEBUG 1


//*********************************************************
//       name:
//description:
//     params:
//    returns:
//*********************************************************
Rom::Rom(rom_type_t rtype) {
	activeRom_m = "";
	activeRomPath_m = "";
	activeRomList_vsm.clear();
	romType_m = rtype;
	activeRomIndex_m = 0;

	setupPath(romPath_internal);

	cfgFilePath_m = activeRomPath_m + "/" + "pbrom.cfg";

	if (mkdir(romPath_internal, S_IRWXU | S_IRWXG | S_IRWXO | S_ISGID))
		fprintf(stderr,"mkdir: %s\n", strerror(errno));

	if (chmod(romPath_internal, S_IRWXU | S_IRWXG | S_IRWXO | S_ISGID))
		fprintf(stderr,"chmod: %s\n", strerror(errno));

	if (mkdir(activeRomPath_m.c_str(), S_IRWXU | S_IRWXG | S_IRWXO | S_ISGID))
		fprintf(stderr,"mkdir: %s\n", strerror(errno));

	if (chmod(activeRomPath_m.c_str(), S_IRWXU | S_IRWXG | S_IRWXO | S_ISGID))
		fprintf(stderr,"chmod: %s\n", strerror(errno));


	loadState();
    getRomList();

    fprintf(stderr,"rom list ready.");
}


//***********************************************************
//        name: pathExists
// description:  setup rom  extensions,  and active path.
//      params:  path to rom files.
//     returns:  none
//***********************************************************
void Rom::setupPath(string romPath) {
	switch (romType_m) {
	case rom_nes_c:
		emuTypeStr_m = "NES";
		activeRomPath_m = romPath + "/nes";
		extensions_vsm.push_back("nes");
		extensions_vsm.push_back("NES");
		extensions_vsm.push_back("zip");
		extensions_vsm.push_back("ZIP"); // fix this case problem
		break;

	case rom_gb_c:
	case rom_gbc_c:
		emuTypeStr_m = "GameBoy";
		activeRomPath_m = romPath + "/gb";
		extensions_vsm.push_back("gb");
		extensions_vsm.push_back("gbc");
		extensions_vsm.push_back("bin");
		extensions_vsm.push_back("zip");
		break;

	case rom_gba_c:
		emuTypeStr_m = "GameBoyAdvance";
		activeRomPath_m = romPath + "/gba";
		extensions_vsm.push_back("gba");
		extensions_vsm.push_back("GBA");
		extensions_vsm.push_back("zip");
		extensions_vsm.push_back("bin");
		break;

	case rom_pce_c:
		emuTypeStr_m = "PC-Engine";
		activeRomPath_m = romPath + "/pce";
		extensions_vsm.push_back("pce");
		break;

	case rom_smd_c:
		emuTypeStr_m = "MegaDrive";
		activeRomPath_m = romPath + "/smd";
		extensions_vsm.push_back("smd");
		extensions_vsm.push_back("gen");
		extensions_vsm.push_back("bin");
		break;

	case rom_lnx_c:
		emuTypeStr_m = "Lynx";
		activeRomPath_m = romPath + "/lynx";
		extensions_vsm.push_back("lnx");
		extensions_vsm.push_back("zip");
		extensions_vsm.push_back("ZIP"); // fix this case problem
		break;

	default:
		activeRomPath_m = romPath;
		break;
	}

	fprintf(stderr, "%s: activeRomPath_m %s\n", emuTypeStr_m.c_str(),activeRomPath_m.c_str() );
}





//***********************************************************
//        name: setupSdCard  TBC
// description: check BB10 SD card existance and configure path
//              for this.
//      params:  none
//     returns:  boolean
//***********************************************************
bool Rom::setupSdCard() {
	if (isADir("/accounts/1000/removable/sdcard")) {
#ifdef DEBUG
		fprintf(stderr,"SDCARD based paths");
#endif
		setupPath( romPath_sd );
		cfgFilePath_m = activeRomPath_m + "/pbrom.cfg";
		mkdir(romPath_sd, 0777);
		mkdir( activeRomPath_m.c_str(), 0777);
		return true;
	}
	return false;
}




//***********************************************************
//        name: extensionIsValid (private)
// description: compare a file extension string with known
//              supported extensions for this 'type' This
//              limits what file extension type are allowed
//              to exist in the 'rom list'.
//      params:   string
//     returns:  boolean
//***********************************************************
bool Rom::extensionIsValid(string ext) {
	unsigned int i = 0;
	for (i = 0; i < extensions_vsm.size(); i++) {
		if (extensions_vsm[i] == ext) {
			return true;
		}
	}

	return false;
}





//***********************************************************
//        name: isADir ( private )
// description: check file path is actually a directory.
//      params:  string
//     returns:  boolean
//***********************************************************
bool Rom::isADir(string dpath) {
	struct stat sb;

	if (stat(dpath.c_str(), &sb) == -1) {
		fprintf(stderr,"error checking '%s'\n", dpath.c_str() );
		perror( dpath.c_str() );
		return false;
	}

	switch (sb.st_mode & S_IFMT) {
	case S_IFBLK:
		printf("block device\n");
		break;

	case S_IFCHR:
		printf("character device\n");
		break;

	case S_IFDIR:
		return true;
		break;

	case S_IFIFO:
		printf("FIFO/pipe\n");
		break;
	case S_IFLNK:
		printf("symlink\n");
		break;
	case S_IFREG:
		printf("regular file\n");
		break;
	case S_IFSOCK:
		printf("socket\n");
		break;
	default:
		printf("unknown?\n");
		break;
	}

	return false;
}

//***********************************************************
//        name: pathExists
// description: check 'string' representing a dir path exists
//      params:   string
//     returns:  boolean
//***********************************************************
bool Rom::pathExists(string dpath) {
	DIR *dp = 0;

	if (!isADir(dpath.c_str())) {
		return false;
	}

	if ((dp = opendir(dpath.c_str())) == NULL) {
		fprintf(stderr,"cannot opendir '%s' %s", dpath.c_str() , strerror(errno) );
		closedir(dp);
		return false;
	}
	else
	{
		closedir(dp);
		return true;
	}
}

void Rom::setRomPath(string dpath) {
	if (pathExists(dpath) == true) {
		cout << "setRomPath " << dpath << " updating rom list ..." << endl;
		activeRomPath_m = dpath;
		getRomList();
	}
}


//*********************************************************
//       name:
//description:
//     params:
//    returns:
//*********************************************************
vector<string> Rom::getRomList(void) {

	DIR* dirp;
	struct dirent* direntp;

//  pthread_mutex_lock( &dir_mutex );
	fprintf(stderr,"getRomList ..\n");

	if (activeRomPath_m == "") {
		activeRomList_vsm.clear();
		return activeRomList_vsm;
	}

	if( activeRomList_vsm.size() )
	    activeRomList_vsm.clear();

	if (!pathExists(activeRomPath_m)) {
		cout << "Directory '%s' not found ...";
		activeRomList_vsm.clear();
		return activeRomList_vsm;
	}

	dirp = opendir(activeRomPath_m.c_str());
	if (dirp != NULL) {
		for (;;) {
			direntp = readdir(dirp);
			if (direntp == NULL)
				break;

			string tmp = direntp->d_name;

			if (strcmp(direntp->d_name, ".") == 0) {
				continue;
			}

			if (strcmp(direntp->d_name, "..") == 0) {
				continue;
			}
			cout << tmp;
			string extension = tmp.substr(tmp.find_last_of(".") + 1);
			if (extensionIsValid(extension) == true) {
				fprintf(stderr,"ROM -> %s\n", direntp->d_name);
				activeRomList_vsm.push_back(direntp->d_name);
			}
			else
			{
				fprintf(stderr,"reject %s\n", direntp->d_name);
			}
		}
	}
	else
	{
		fprintf(stderr,"directory is not valid %s.\n", activeRomPath_m.c_str() );
	}

	if (activeRomList_vsm.size() > 1) {
		sort();
	}

//  pthread_mutex_unlock( &dir_mutex );
	cout << activeRomList_vsm.size() << " files in " << activeRomPath_m;
	return activeRomList_vsm;
}


//*********************************************************
//       name:
//description:
//     params:
//    returns:
//*********************************************************
const char *Rom::getRomNext(void) {
	static char romName[128];

	memset(romName, 0, 128);

	if (activeRomList_vsm.size() == 0) {
		fprintf(stderr,"getRomNext: error no games in vecList\n");
		return "";
	}

	if (++activeRomIndex_m >= activeRomList_vsm.size())
		activeRomIndex_m = 0;

	if (activeRomList_vsm.size() == 1)
		activeRomIndex_m = 0;

	if (activeRomIndex_m == activeRomList_vsm.size())
		activeRomIndex_m = 0;

	activeRom_m = activeRomList_vsm[activeRomIndex_m];
	string baseDir = activeRomPath_m + "/"
			+ activeRomList_vsm[activeRomIndex_m];

	sprintf(romName, "%s", baseDir.c_str());

	fprintf(stderr,"%d/%d '%s'\n", activeRomIndex_m, activeRomList_vsm.size(),baseDir.c_str() );
	return romName;
}

//*********************************************************
//       name:
//description:
//     params:
//    returns:
//*********************************************************
const char *Rom::getRomPrev(void) {
	fprintf(stderr,"getRomPrev: %d\n", romType_m);
	static char romName[128];

	memset(romName,0,128);

	if( activeRomList_vsm.size() == 0)
	{
		fprintf(stderr,"getRomPrev: error no games in vecList\n");
		return "";
	}

	if(--activeRomIndex_m < 1)
	activeRomIndex_m = 0;

	if( activeRomList_vsm.size() == 1)
	activeRomIndex_m = 0;

	activeRom_m = activeRomList_vsm[activeRomIndex_m];
	string baseDir = activeRomPath_m + "/" + activeRomList_vsm[activeRomIndex_m];

	sprintf(romName,"%s",baseDir.c_str());

	fprintf(stderr,"getRomPrev: %d/%d '%s'\n", activeRomIndex_m, activeRomList_vsm.size(), baseDir.c_str() );
	return romName;
}



//*********************************************************
//       name:
//description:
//     params:
//    returns:
//*********************************************************
void Rom::setActiveRomBad() {
	badRomList_vim.push_back(activeRomIndex_m);
}



//*********************************************************
//       name:
//description:
//     params:
//    returns:
//*********************************************************
bool Rom::isBadRom() {
	vector<unsigned int>::iterator i;
	for (i = badRomList_vim.begin(); i != badRomList_vim.end(); ++i) {
		if (activeRomIndex_m == *i)
			return true;
	}
	return false;
}



//*********************************************************
//       name:
//description:
//     params:
//    returns:
//*********************************************************
void Rom::sort(void) {
	int swap;
	string temp;

	do {
		swap = 0;
		for (unsigned int count = 0; count < activeRomList_vsm.size() - 1;
				count++) {
			if (activeRomList_vsm.at(count) > activeRomList_vsm.at(count + 1)) {
				temp = activeRomList_vsm.at(count);
				activeRomList_vsm.at(count) = activeRomList_vsm.at(count + 1);
				activeRomList_vsm.at(count + 1) = temp;
				swap = 1;
			}
		}
	} while (swap != 0);
}




//*********************************************************
//       name:
//description:
//     params:
//    returns:
//*********************************************************
void Rom::setRomIndex(unsigned int idx) {
	if (idx <= activeRomList_vsm.size())
		activeRomIndex_m = idx;
	if (activeRomList_vsm.size())
		activeRom_m = activeRomList_vsm[activeRomIndex_m];
}



//*********************************************************
//       name:
//description:
//     params:
//    returns:
//*********************************************************
void Rom::saveState(void) {
	unsigned int i = 0;
	ofstream cfgFile;
	cfgFile.open(cfgFilePath_m.c_str());
	cfgFile << activeRomIndex_m << "\n";
	cfgFile << activeRom_m << "\n";
	for (i = 0; i < activeRomList_vsm.size(); i++) {
		cfgFile << activeRomList_vsm[i] << "\n";
	}
	cfgFile.close();
}



//*********************************************************
//       name:
//description:
//     params:
//    returns:
//*********************************************************
void Rom::loadState(void) {
	ifstream cfgFile;
	string line;

	cfgFile.open(cfgFilePath_m.c_str());

	// 3
	// spaceinvaders.gba
	// blah1.gba
	// ...

	if (cfgFile.is_open()) {

		getline(cfgFile, line);
		//  activeRomIndex_m
		getline(cfgFile, line);
		//  activeRom_m

		while (cfgFile.good()) {
			getline(cfgFile, line);
			cout << line << endl;
			// activeRomList_vsm.push_back(line);
		}
		cfgFile.close();
	}

}
