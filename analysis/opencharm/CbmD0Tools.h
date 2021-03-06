// ------------------------------------------------------
// -----             CbmD0Tools header file         -----
// -----          Created 27.05.2015 by P.Sitzmann  -----
// ------------------------------------------------------


#ifndef CBMD0TOOLS
#define CBMD0TOOLS 1


#include "TObject.h"

class CbmVertex;
class CbmTrack;

class KFPVertex;
class KFPTrack;


class CbmD0Tools : public TObject
{
public:
/** Default constructor **/
CbmD0Tools();

/** Destructor **/
~CbmD0Tools();

/** Accessors **/
KFPVertex* CbmVertexToKFPVertex(const CbmVertex& vertex);
//KFPTrack*  CbmTrackToKFPTrack(const CbmTrack& track);

/** Modifiers **/


private:

ClassDef(CbmD0Tools,1);

};


#endif
