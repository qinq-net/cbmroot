#ifndef CBMTUTORIAL2DETCONTFACT_H
#define CBMTUTORIAL2DETCONTFACT_H

#include "FairContFact.h"

class FairContainer;

class CbmTutorial2DetContFact : public FairContFact {
 private:
  void setAllContainers();
 public:
  CbmTutorial2DetContFact();
  ~CbmTutorial2DetContFact() {}
  FairParSet* createContainer(FairContainer*);
  ClassDef( CbmTutorial2DetContFact,0) // Factory for all MyDet parameter containers
};

#endif  /* !CBMTUTORIALDETCONTFACT_H */
