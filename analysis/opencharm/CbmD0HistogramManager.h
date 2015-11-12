// ------------------------------------------------------
// -----         CbmD0HistogramManager header file  -----
// -----          Created 09.11.2015 by P.Sitzmann  -----
// ------------------------------------------------------


#ifndef CBMD0HISTOGRAMMANAGER
#define CBMD0HISTOGRAMMANAGER 1


#include "TObject.h"
#include "CbmHistManager.h"

enum HistoGroup {SINGELTRACK, PAIR, ALL}

class CbmD0HistogramManager : public CbmHistManager
{
public:
/** Default constructor **/
    CbmD0HistogramManager();
    CbmD0HistogramManager(HistoGroup group,Float_t PTCut,Float_t SvZCut, Float_t PZcut)

/** Destructor **/
~CbmD0HistogramManager();

/** Accessors **/


/** Modifiers **/
void SetCuts();
void SetHistogramChois(HistoGroup group);
void SetFile();
void SetFileList(TList* list);
void SetFileListRange(TList* );

void Init();
void Finish();

private:

    void Register();


    Float_t fcutPT;
    Float_t fcutSvZ;
    Float_t fcutPZ;

ClassDef(CbmD0HistogramManager,1);

};


#endif