/*
 * playbook_utils.cpp
 *
 *  Created on: Jan 10, 2012
 *      Author: tnunes
 */


#include "playbookrom.h"
#include <dirent.h>


static const char *romPath_xxx = "/accounts/1000/shared/misc/roms";
static const char *romPath_nes = "/accounts/1000/shared/misc/roms/nes";
static const char *romPath_gba = "/accounts/1000/shared/misc/roms/gba";
static const char *romPath_pce = "/accounts/1000/shared/misc/roms/pce";
static const char *romPath_smd = "/accounts/1000/shared/misc/roms/smd";
static const char *romPath_gb  = "/accounts/1000/shared/misc/roms/gb";


//*********************************************************
//
//*********************************************************
PlaybookRom::PlaybookRom(rom_type_t rtype)
{
  activeRom_m = "";
  activeRomPath_m = "";
  activeRomList_vsm.clear();
  romType_m = rtype;
  activeRomIndex_m =0;

  switch(romType_m)
  {
    case rom_nes_c:
    	 activeRomPath_m = romPath_nes;
    	 extensions_vsm.push_back("nes");
    	 extensions_vsm.push_back("NES");
         break;

    case rom_gb_c:
    case rom_gbc_c:
    	 activeRomPath_m = romPath_gb;
    	 extensions_vsm.push_back("gb");
    	 extensions_vsm.push_back("gbc");
    	 extensions_vsm.push_back("bin");
    	 extensions_vsm.push_back("zip");
    	break;

    case rom_gba_c:
    	 activeRomPath_m = romPath_gba;
    	 extensions_vsm.push_back("gba");
    	 extensions_vsm.push_back("GBA");
    	 extensions_vsm.push_back("zip");
    	 extensions_vsm.push_back("bin");
         break;

    case rom_pce_c:
    	 activeRomPath_m = romPath_pce;
    	 extensions_vsm.push_back("pce");
    	 break;

    case rom_smd_c:
    	 activeRomPath_m = romPath_smd;
    	 extensions_vsm.push_back("smd");
    	 extensions_vsm.push_back("gen");
    	 extensions_vsm.push_back("bin");
    	 break;

    default:  activeRomPath_m = romPath_xxx;
              break;
  }

  // root path is now set ...

}


bool PlaybookRom::extensionIsValid(string ext)
{
  unsigned int i = 0;
  for(i =0; i < extensions_vsm.size(); i++)
  {
	 if( extensions_vsm[i] == ext)
	 {
		 return true;
	 }
  }

  return false;
}

//*********************************************************
//
//*********************************************************
bool PlaybookRom::pathExists(string dpath)
{
	DIR *dp =0;
	if( (dp=opendir(dpath.c_str())) == NULL) {
		closedir(dp);
		return false;
	}
	 else
	{
		closedir(dp);
		return true;
	}
}

void PlaybookRom::setRomPath(string dpath)
{
 if( pathExists(dpath) == true)
 {
     activeRomPath_m = dpath;
     getRomList();
 }
}

//*********************************************************
//
//*********************************************************
vector<string> PlaybookRom::getRomList( void )
{

	DIR* dirp;
	struct dirent* direntp;

	if(activeRomPath_m == "")
	{
        activeRomList_vsm.clear();
		return activeRomList_vsm;
	}

  dirp = opendir( activeRomPath_m.c_str() );
  if( dirp != NULL )
  {
	 for(;;)
	 {
		direntp = readdir( dirp );
		if( direntp == NULL )
		  break;

   	     string tmp = direntp->d_name;

		  if( strcmp( direntp->d_name, ".") == 0)
		  {
			 continue;
		  }

		  if( strcmp( direntp->d_name,"..") == 0)
		  {
			  continue;
		  }

		  string extension = tmp.substr(tmp.find_last_of(".") +1);
          if( extensionIsValid( extension ) == true)
		  {
	          fprintf(stderr,"ROM: %s\n", direntp->d_name);
			  activeRomList_vsm.push_back(direntp->d_name);
		  }
	 }
  }
  else
  {
	fprintf(stderr,"dirp is NULL ...\n");
  }

 // sort list here .

 fprintf(stderr,"number of files %d\n", activeRomList_vsm.size() );
 return activeRomList_vsm;
}





//*********************************************************
//
//*********************************************************
const char *PlaybookRom::getRomNext(void)
{
    fprintf(stderr,"getRomNext: %d\n", romType_m);
    static char romName[128];

    memset(romName,0,128);

	if( activeRomList_vsm.size() == 0)
	{
	   fprintf(stderr,"getRomNext: error no games in vecList\n");
	   return "";
	}

    if(++activeRomIndex_m >= activeRomList_vsm.size())
    	activeRomIndex_m = 0;

    if( activeRomList_vsm.size() == 1)
    	activeRomIndex_m = 0;

    if( activeRomIndex_m == activeRomList_vsm.size())
    	activeRomIndex_m = 0;


    activeRom_m = activeRomList_vsm[activeRomIndex_m];
    string baseDir = activeRomPath_m + "/" + activeRomList_vsm[activeRomIndex_m];

    sprintf(romName,"%s",baseDir.c_str());

    fprintf(stderr,"loading: %d/%d '%s'\n", activeRomIndex_m, activeRomList_vsm.size(), baseDir.c_str() );
    return romName;
}

//*********************************************************
//
//*********************************************************
const char *PlaybookRom::getRomPrev(void)
{
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

void PlaybookRom::setActiveRomBad()
{
  badRomList_vim.push_back( activeRomIndex_m );
}

bool PlaybookRom::isBadRom()
{
  vector<unsigned int>::iterator i;
  for( i = badRomList_vim.begin(); i != badRomList_vim.end(); ++i)
  {
    if( activeRomIndex_m == *i )
    	return true;
  }
  return false;
}

string PlaybookRom::getInfoStr()
{
  return activeRom_m;
}


//*******************************
//
//*******************************
void PlaybookRom::updateRomList(void)
{
  (void) getRomList();
}


void   PlaybookRom::setRomIndex(unsigned int idx)
{
   if( idx <= activeRomList_vsm.size() )
	  activeRomIndex_m = idx;
   if( activeRomList_vsm.size() )
   activeRom_m = activeRomList_vsm[activeRomIndex_m];
}


