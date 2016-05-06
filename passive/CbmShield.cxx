/* Generated by Together */

#include "CbmShield.h"

#include "CbmShieldGeo.h"
#include "CbmGeoPassivePar.h"

#include "FairGeoLoader.h"
#include "FairGeoInterface.h"
#include "FairRuntimeDb.h"
#include "FairRun.h"
#include "FairGeoVolume.h"
#include "FairGeoNode.h"

#include "TList.h"
#include "TObjArray.h"

CbmShield::~CbmShield()
{
}
CbmShield::CbmShield()
  : FairModule()
{
}

CbmShield::CbmShield(const char * name,  const char * title)
  : FairModule(name ,title)
{
}
void CbmShield::ConstructGeometry()
{

  TString fileName = GetGeometryFileName();
  if ( fileName.EndsWith(".geo") ) {
    LOG(INFO) << "Constructing SHIELD geometry from ASCII file "
              << fileName.Data() << FairLogger::endl;
  }
  else
    LOG(FATAL) << "Geometry format of SHIELD file " << fileName.Data()
               << " not supported." << FairLogger::endl;

	FairGeoLoader *loader=FairGeoLoader::Instance();
	FairGeoInterface *GeoInterface =loader->getGeoInterface();
	CbmShieldGeo *MGeo=new CbmShieldGeo();
	MGeo->setGeomFile(GetGeometryFileName());
	GeoInterface->addGeoModule(MGeo);
	Bool_t rc = GeoInterface->readSet(MGeo);
	if ( rc ) MGeo->create(loader->getGeoBuilder());

        TList* volList = MGeo->getListOfVolumes();
        // store geo parameter
        FairRun *fRun = FairRun::Instance();
        FairRuntimeDb *rtdb= FairRun::Instance()->GetRuntimeDb();
        CbmGeoPassivePar* par=(CbmGeoPassivePar*)(rtdb->getContainer("CbmGeoPassivePar"));
        TObjArray *fSensNodes = par->GetGeoSensitiveNodes();
        TObjArray *fPassNodes = par->GetGeoPassiveNodes();

        TListIter iter(volList);
        FairGeoNode* node   = NULL;
        FairGeoVolume *aVol=NULL;

        while( (node = (FairGeoNode*)iter.Next()) ) {
            aVol = dynamic_cast<FairGeoVolume*> ( node );
            if ( node->isSensitive()  ) {
                fSensNodes->AddLast( aVol );
            }else{
                fPassNodes->AddLast( aVol );
            }
        }
	ProcessNodes( volList );
        par->setChanged();
        par->setInputVersion(fRun->GetRunId(),1);

}
ClassImp(CbmShield)



