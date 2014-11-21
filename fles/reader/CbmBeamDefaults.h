#ifndef CBMBEAMDEFAULTS_H
#define CBMBEAMDEFAULTS_H 1

/**  DetectorID enumerator  **/
enum Exp_Group {kRich=0xE000,
                kMuenster=0xE001,
                kFrankfurt=0xE002,
                kBucarest=0xE003};

static const Int_t NrOfSyscores = 3;
static const Int_t NrOfSpadics = 3;
static const Int_t NrOfHalfSpadics = 6;

static const Int_t SpadicBaseAddress = 0;

#endif
