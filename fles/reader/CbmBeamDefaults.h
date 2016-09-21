#ifndef CBMBEAMDEFAULTS_H
#define CBMBEAMDEFAULTS_H 1

#include <map>

/**  DetectorID enumerator  **/
enum Exp_Group {kFlesRich=0xE000,
                kFlesMuenster=0xE001,
                kFlesFrankfurt=0xE002,
                kFlesBucarest=0xE003,
		kFlesHodo1=0xE004,
		kFlesHodo2=0xE005};

/** Maximum number of Syscores which could be in the system **/
static const Int_t NrOfSyscores = 3;

/* Number of Active Syscores in the System */
static const Int_t NrOfActiveSyscores=1;

/** Maximum number of Spadics which could attached to one Syscore **/
static const Int_t NrOfSpadics = 3;
static const Int_t NrOfHalfSpadics = NrOfSpadics*2;

/* Number of Active Spadics on each Syscore the System */
static const Int_t NrOfActiveSpadics=2;
static const Int_t NrOfActiveHalfSpadics = NrOfActiveSpadics*2;


/** Base address which will be added to each half spadic number 
    the first halfspadic which is connected to the Syscore 
    get SpadicBaseAddress+0, the second SpadicBaseAddress +1 ... */
static const Int_t SpadicBaseAddress = 0;
static const Int_t BaseEquipmentID = kFlesMuenster;


/*
static const std::map<Int_t, Int_t> groupToExpMap 
  = { std::make_pair(kMuenster,0),
      std::make_pair(kFrankfurt,1),
      std::make_pair(Bucarest,2)
};
*/
#ifndef __CINT__
static const std::map<Int_t, Int_t> groupToExpMap = {
  {kFlesMuenster,0},
  {kFlesFrankfurt,1},
  {kFlesBucarest,2}
};
#endif

#endif
