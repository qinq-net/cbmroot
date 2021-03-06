#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class CbmDirectPhotonGenerator+;
#pragma link C++ class CbmEcalGenerator+;
#pragma link C++ class CbmHsdLvmGenerator+;
#pragma link C++ class CbmHsdGenerator+;
#pragma link C++ class CbmPHSDGenerator;
#pragma link C++ class CbmShieldGenerator+;
#pragma link C++ class CbmIonGenerator+;
#pragma link C++ class CbmCosmicGenerator;

#pragma link C++ class CbmUnigenGenerator+;
#pragma link C++ class URun+;
#pragma link C++ class UEvent+;
#pragma link C++ class UParticle+;

#pragma link C++ class CbmPlutoGenerator+;
#pragma link C++ class PDataBase;
#pragma link C++ class PMesh;
#pragma link C++ class PParticle;
#pragma link C++ class PStaticData;
#pragma link C++ class PStdData;
#pragma link C++ class PValues;

#ifdef HAS_PLUTO
#pragma link C++ class CbmPlutoReactionGenerator+;
#pragma link C++ class CbmDYMuonGenerator;
#pragma link C++ class CbmSemiMuonGenerator;
#endif

#endif

