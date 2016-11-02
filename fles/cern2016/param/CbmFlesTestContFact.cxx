/////////////////////////////////////////////////////////////
//
//  CbmFlesTestContFact
//
//  Factory for the parameter containers in libFlesTest
//
/////////////////////////////////////////////////////////////

#include "CbmFlesTestContFact.h"

#include "CbmFHodoUnpackPar.h"

#include "FairRuntimeDb.h"

ClassImp(CbmFlesTestContFact)

static CbmFlesTestContFact gCbmFlesTestContFact;

CbmFlesTestContFact::CbmFlesTestContFact() {
  // Constructor (called when the library is loaded)
  fName="CbmFlesTestContFact";
  fTitle="Factory for parameter containers for fles test library";
  setAllContainers();
  FairRuntimeDb::instance()->addContFactory(this);
}

void CbmFlesTestContFact::setAllContainers() {
  /** Creates the Container objects with all accepted contexts and adds them to
   *  the list of containers for the fles test library.*/

    FairContainer* p= new FairContainer("CbmFHodoUnpackPar",
                                          "Fibre Hodoscope Unpack Parameters",
                                          "TestDefaultContext");
    p->addContext("TestNonDefaultContext");

    containers->Add(p); 
}

FairParSet* CbmFlesTestContFact::createContainer(FairContainer* c) {
  /** Calls the constructor of the corresponding parameter container.
   * For an actual context, which is not an empty string and not the default context
   * of this container, the name is concatinated with the context. */
  const char* name=c->GetName();
  FairParSet* p=0;
  if (strcmp(name,"CbmFHodoUnpackPar")==0) {
      p=new CbmFHodoUnpackPar(c->getConcatName().Data(),c->GetTitle(),c->getContext());
  }
  return p;
}

