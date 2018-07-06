//*-- AUTHOR : Ilse Koenig
//*-- Created : 25/10/2004

/////////////////////////////////////////////////////////////
//
//  CbmTrdContFact
//
//  Factory for the parameter containers in libTrd
//
/////////////////////////////////////////////////////////////

#include "CbmTrdContFact.h"

#include "CbmGeoTrdPar.h"
//#include "CbmTrdDigiPar.h"

#include "CbmTrdParSetAsic.h"
#include "CbmTrdParSetDigi.h"
#include "CbmTrdParSetGas.h"
#include "CbmTrdParSetGain.h"
#include "CbmTrdParSetGeo.h"

#include <FairRuntimeDb.h>
#include <FairLogger.h>

ClassImp(CbmTrdContFact)

static CbmTrdContFact gCbmTrdContFact;

CbmTrdContFact::CbmTrdContFact() {
  // Constructor (called when the library is loaded)
  fName="CbmTrdContFact";
  fTitle="Factory for parameter containers in libTrd";
  setAllContainers();
  FairRuntimeDb::instance()->addContFactory(this);
}

void CbmTrdContFact::setAllContainers() {
  /** Creates the Container objects with all accepted contexts and adds them to
   *  the list of containers for the STS library.*/

    FairContainer* p= new FairContainer("CbmGeoTrdPar",
                                          "Trd Geometry Parameters",
                                          "TestDefaultContext");
    p->addContext("TestNonDefaultContext");

    containers->Add(p);
 
//     FairContainer* p1= new FairContainer("CbmTrdDigiPar",
//                                           "Trd Digi Parameters",
//                                           "TestDefaultContext");
//     p1->addContext("TestNonDefaultContext");
// 
//     containers->Add(p1);

  
  // AB
  FairContainer* par(NULL);
  // ASIC parametsr
  par = new FairContainer("CbmTrdParSetAsic", "Trd ASIC Parameters", "TestDefaultContext");
  par->addContext("TestNonDefaultContext");
  containers->Add(par);    
  // read-out parameters
  par = new FairContainer("CbmTrdParSetDigi", "Trd Read-Out Parameters", "TestDefaultContext");
  par->addContext("TestNonDefaultContext");
  containers->Add(par);    
  // gas parameters
  par = new FairContainer("CbmTrdParSetGas", "Trd Gas Parameters", "TestDefaultContext");
  par->addContext("TestNonDefaultContext");
  containers->Add(par);    
  // gain parameters
  par = new FairContainer("CbmTrdParSetGain", "Trd Gain Parameters", "TestDefaultContext");
  par->addContext("TestNonDefaultContext");
  containers->Add(par);    
  // geometry parameters
  par = new FairContainer("CbmTrdParSetGeo", "Trd Geometry Parameters", "TestDefaultContext");
  par->addContext("TestNonDefaultContext");
  containers->Add(par);    
}

FairParSet* CbmTrdContFact::createContainer(FairContainer* c) {
  /** Calls the constructor of the corresponding parameter container.
   * For an actual context, which is not an empty string and not the default context
   * of this container, the name is concatinated with the context. */
  const char* name=c->GetName();
  LOG(INFO) << GetName() <<"::createContainer :"<< name << FairLogger::endl;

  FairParSet* p(NULL);
  if(strcmp(name,"CbmGeoTrdPar")==0)
    p=new CbmGeoTrdPar(c->getConcatName().Data(),c->GetTitle(),c->getContext());
//   else if (strcmp(name,"CbmTrdDigiPar")==0) 
//     p=new CbmTrdDigiPar(c->getConcatName().Data(),c->GetTitle(),c->getContext());
  else if (strcmp(name,"CbmTrdParSetAsic")==0) 
    p=new CbmTrdParSetAsic(c->getConcatName().Data(),c->GetTitle(),c->getContext());
  else if (strcmp(name,"CbmTrdParSetDigi")==0) 
    p=new CbmTrdParSetDigi(c->getConcatName().Data(),c->GetTitle(),c->getContext());
  else if (strcmp(name,"CbmTrdParSetGas")==0) 
    p=new CbmTrdParSetGas(c->getConcatName().Data(),c->GetTitle(),c->getContext());
  else if (strcmp(name,"CbmTrdParSetGain")==0) 
    p=new CbmTrdParSetGain(c->getConcatName().Data(),c->GetTitle(),c->getContext());
  else if (strcmp(name,"CbmTrdParSetGeo")==0) 
    p=new CbmTrdParSetGeo(c->getConcatName().Data(),c->GetTitle(),c->getContext());
  return p;
}

