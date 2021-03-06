// -------------------------------------------------------------------------
// -----                  CbmMvdSensorPlugin source file              -----
// -----                  Created 02.02.2012 by M. Deveaux            -----
// -------------------------------------------------------------------------
#include "CbmMvdSensorPlugin.h"

#include "TClonesArray.h"
#include "TObjArray.h"



// -----   Default constructor   -------------------------------------------
CbmMvdSensorPlugin::CbmMvdSensorPlugin() 
 : TObject(),
   fNextPlugin(NULL), 
   fPreviousPlugin(NULL),
   bFlag(false),
   initialized(kFALSE),
    fShowDebugHistos(kFALSE),
    fName("CbmMvdSensorPlugin")
{
}
// -------------------------------------------------------------------------
// -----   constructor   -------------------------------------------
CbmMvdSensorPlugin::CbmMvdSensorPlugin(const char* name)
 : TObject(),
   fNextPlugin(NULL), 
   fPreviousPlugin(NULL),
   bFlag(false),
   initialized(kFALSE),
fShowDebugHistos(kFALSE),
fName(name)
{
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmMvdSensorPlugin::~CbmMvdSensorPlugin() {
}
// -------------------------------------------------------------------------



ClassImp(CbmMvdSensorPlugin)
