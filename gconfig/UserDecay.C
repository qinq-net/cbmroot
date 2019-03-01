void UserDecayConfig() {

   // Obviously, this macro is not in use any longer.
   // Was last touched 18 July 2008.
   // Should be removed. Do we need the functionality still? I mean, there must
   // have been some intention to it.

   cout << "Loading User Decay Config from macro"<< endl;  
   LOG(FATAL) << "Do not use UserDecay.C" << FairLogger::endl;
   TDatabasePDG *db= TDatabasePDG::Instance();
   TParticlePDG *p=0;

   Int_t mode[6][3];                  
   Float_t bratio[6];
   Int_t AlphaPDG, He5PDG;
   p= db->GetParticle("Alpha");
   if(p) AlphaPDG=p->PdgCode();
   p=db->GetParticle("He5");

   if(p) He5PDG=p->PdgCode();
   for (Int_t kz = 0; kz < 6; kz++) {
      bratio[kz] = 0.;
      mode[kz][0] = 0;
      mode[kz][1] = 0;
      mode[kz][2] = 0;
    //  cout << mode[kz][0] << " " << 	mode[kz][1] << " " << mode[kz][2] << endl;
   }
   bratio[0] = 100.;
   mode[0][0] =2112  ;
   mode[0][1] =AlphaPDG  ;
   
 /*  bratio[1] = 50.;
   mode[1][0] =2212  ;
   mode[1][1] =AlphaPDG  ;
    
  */
   gMC->SetDecayMode(He5PDG,bratio,mode);
    

    
}
