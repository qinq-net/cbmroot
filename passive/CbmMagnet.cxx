/* Generated by Together */

#include "CbmMagnet.h"

#include "CbmGeoMagnet.h"
#include "CbmGeoPassivePar.h"

#include "FairGeoLoader.h"
#include "FairGeoInterface.h"
#include "FairRuntimeDb.h"
#include "FairRun.h"
#include "FairGeoVolume.h"
#include "FairGeoNode.h"

#include "TList.h"
#include "TObjArray.h"
#include "TGeoMatrix.h"

CbmMagnet::CbmMagnet()
  : CbmModule("CbmMagnet", "CbmMagnet"),
    frot(NULL),
    fposrot(NULL)
{
}

CbmMagnet::CbmMagnet(const char *name, const char *title, Double_t px, Double_t py, Double_t pz,
							Double_t rx, Double_t ry, Double_t rz)
  : CbmModule(name, title),
    frot(new TGeoRotation("", rx, ry, rz)),
    fposrot(new TGeoCombiTrans(px, py, pz, frot))
{
}

CbmMagnet::~CbmMagnet()
{
}

void CbmMagnet::ConstructGeometry()
{
	TString fileName=GetGeometryFileName();
	if (fileName.EndsWith(".root"))	{
	  LOG(INFO) << "Constructing MAGNET        from ROOT  file " << fileName.Data() << FairLogger::endl;
	  ConstructRootGeometry();
        } else if (fileName.EndsWith(".geo")) {
          LOG(INFO) << "Constructing MAGNET        from ASCII file " << fileName.Data() << FairLogger::endl;
          ConstructASCIIGeometry();
        } else if (fileName.EndsWith(".gdml")) {
	  LOG(INFO) << "Constructing MAGNET        from GDML  file " << fileName.Data() << FairLogger::endl;
	  ConstructGDMLGeometry(fposrot);
        } else {
	  LOG(FATAL) << "Geometry format of MAGNET file " << fileName.Data()
		     << " not supported." << FairLogger::endl;
	}
}

void CbmMagnet::ConstructASCIIGeometry()
{
	FairGeoLoader *loader=FairGeoLoader::Instance();
	FairGeoInterface *GeoInterface =loader->getGeoInterface();
	CbmGeoMagnet *MGeo=new CbmGeoMagnet();
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

Bool_t CbmMagnet::CheckIfSensitive(std::string) 
{
  // There are no sensitive volumes in the magnet
  return kFALSE;
}


ClassImp(CbmMagnet)
