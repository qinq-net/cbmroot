

void eLinkMappingCrobStsFebA()
{
   const UInt_t kuNbElinks = 42;
   const UInt_t kuNbFebs   =  5;
   const UInt_t kuNbSmx    =  8;
   UInt_t uElinkMap[ kuNbFebs ][ kuNbSmx ] = {
         { 35, 34, 33, 38, 40, 37, 39, 41 },
         { 19, 22, 32, 31, 30, 29, 36, 28 },
         { 25, 27, 15, 14, 24, 23, 21, 20 },
         { 13,  7,  6,  5, 18, 26, 17, 16 },
         { 12,  8,  4,  3,  9,  2,  1,  0 }
      };
   UInt_t uAddrMap[ kuNbFebs ][ kuNbSmx ] = {
         {  1,  3,  5,  7,  0,  2,  4,  6 },
         {  1,  3,  5,  7,  0,  2,  4,  6 },
         {  1,  3,  5,  7,  0,  2,  4,  6 },
         {  1,  3,  5,  7,  0,  2,  4,  6 },
         {  1,  3,  5,  7,  0,  2,  4,  6 }
      };

   UInt_t uSmxMap[ kuNbElinks ];
   UInt_t uElinkMapReordered[ kuNbFebs ][ kuNbSmx ];

   for( UInt_t uElinkIdx = 0; uElinkIdx < kuNbElinks; ++uElinkIdx )
   {
      Bool_t bFoundSmx = kFALSE;
      for( UInt_t uFebIdx = 0; uFebIdx < kuNbFebs; ++uFebIdx )
         for( UInt_t uSmxIdx = 0; uSmxIdx < kuNbSmx; ++uSmxIdx )
            if( uElinkMap[ uFebIdx ][ uSmxIdx ] == uElinkIdx )
            {
               UInt_t uGlobSmxIdx = uFebIdx * kuNbSmx + uAddrMap[ uFebIdx ][ uSmxIdx ];
               uSmxMap[ uElinkIdx ] = uGlobSmxIdx;
               std::cout << Form("eLink %02u FEB %u SMX %u => Global SMX Idx %02u",
                                    uElinkIdx, uFebIdx, uSmxIdx, uGlobSmxIdx )
                         << std::endl;
               bFoundSmx = kTRUE;
            } // if( uElinkMap[ uFebIdx ][ uSmxIdx ] == uElinkIdx )
      if( kFALSE == bFoundSmx )
      {
         uSmxMap[ uElinkIdx ] = 0xFFFF;
         std::cout << Form("eLink %02u               => Not Used!!", uElinkIdx )
                         << std::endl;
      }
   } // for( UInt_t uElinkIdx = 0; uElinkIdx < kuNbElinks; ++uElInkIdx )

   for( UInt_t uFebIdx = 0; uFebIdx < kuNbFebs; ++uFebIdx )
      for( UInt_t uSmxIdx = 0; uSmxIdx < kuNbSmx; ++uSmxIdx )
      {
         uElinkMapReordered[ uFebIdx ][ uAddrMap[ uFebIdx ][ uSmxIdx ] ] = uElinkMap[ uFebIdx ][ uSmxIdx ];
      } // Loop on FEB and not ordered smx

   std::cout << " Parameter map from eLink to Asic: "
             << std::endl;
   for( UInt_t uElinkIdx = 0; uElinkIdx < kuNbElinks; ++uElinkIdx )
   {
//      std::cout << Form( " %6u", uSmxMap[ uElinkIdx ] );
      std::cout << Form( " 0x%04X", uSmxMap[ uElinkIdx ] );
      if( kuNbElinks - 1 != uElinkIdx && 5 == uElinkIdx % 6  )
         std::cout << " \\" << std::endl;
   } // for( UInt_t uElinkIdx = 0; uElinkIdx < kuNbElinks; ++uElInkIdx )
   std::cout << std::endl;

   std::cout << " Reordered map from (Feb, Asic) to eLink: "
             << std::endl;
   for( UInt_t uFebIdx = 0; uFebIdx < kuNbFebs; ++uFebIdx )
   {
      for( UInt_t uSmxIdx = 0; uSmxIdx < kuNbSmx; ++uSmxIdx )
         std::cout << Form( " %2u", uElinkMapReordered[ uFebIdx ][ uSmxIdx ] );
      std::cout << std::endl;
   } // for( UInt_t uFebIdx = 0; uFebIdx < kuNbFebs; ++uFebIdx )

   return;
}

void eLinkMappingCrobStsFebB()
{
   const UInt_t kuNbElinks = 42;
   const UInt_t kuNbFebs   =  5;
   const UInt_t kuNbSmx    =  8;
   UInt_t uElinkMap[ kuNbFebs ][ kuNbSmx ] = {
         { 35, 40, 34, 37, 33, 39, 38, 41 },
         { 19, 30, 22, 29, 32, 36, 31, 28 },
         { 25, 24, 27, 23, 15, 21, 14, 20 },
         { 13, 18,  7, 26,  6, 17,  5, 16 },
         { 12,  9,  8,  2,  4,  1,  3,  0 }
      };
   UInt_t uAddrMap[ kuNbFebs ][ kuNbSmx ] = {
         {  0,  1,  2,  3,  4,  5,  6,  7 },
         {  0,  1,  2,  3,  4,  5,  6,  7 },
         {  0,  1,  2,  3,  4,  5,  6,  7 },
         {  0,  1,  2,  3,  4,  5,  6,  7 },
         {  0,  1,  2,  3,  4,  5,  6,  7 },
      };


   UInt_t uSmxMap[ kuNbElinks ];
   UInt_t uElinkMapReordered[ kuNbFebs ][ kuNbSmx ];

   for( UInt_t uElinkIdx = 0; uElinkIdx < kuNbElinks; ++uElinkIdx )
   {
      Bool_t bFoundSmx = kFALSE;
      for( UInt_t uFebIdx = 0; uFebIdx < kuNbFebs; ++uFebIdx )
         for( UInt_t uSmxIdx = 0; uSmxIdx < kuNbSmx; ++uSmxIdx )
            if( uElinkMap[ uFebIdx ][ uSmxIdx ] == uElinkIdx )
            {
               UInt_t uGlobSmxIdx = uFebIdx * kuNbSmx + uAddrMap[ uFebIdx ][ uSmxIdx ];
               uSmxMap[ uElinkIdx ] = uGlobSmxIdx;
               std::cout << Form("eLink %02u FEB %u SMX %u => Global SMX Idx %02u",
                                    uElinkIdx, uFebIdx, uSmxIdx, uGlobSmxIdx )
                         << std::endl;
               bFoundSmx = kTRUE;
            } // if( uElinkMap[ uFebIdx ][ uSmxIdx ] == uElinkIdx )
      if( kFALSE == bFoundSmx )
      {
         uSmxMap[ uElinkIdx ] = 0xFFFF;
         std::cout << Form("eLink %02u               => Not Used!!", uElinkIdx )
                         << std::endl;
      }
   } // for( UInt_t uElinkIdx = 0; uElinkIdx < kuNbElinks; ++uElInkIdx )

   for( UInt_t uFebIdx = 0; uFebIdx < kuNbFebs; ++uFebIdx )
      for( UInt_t uSmxIdx = 0; uSmxIdx < kuNbSmx; ++uSmxIdx )
      {
         uElinkMapReordered[ uFebIdx ][ uAddrMap[ uFebIdx ][ uSmxIdx ] ] = uElinkMap[ uFebIdx ][ uSmxIdx ];
      } // Loop on FEB and not ordered smx

   std::cout << " Parameter map from eLink to Asic: "
             << std::endl;
   for( UInt_t uElinkIdx = 0; uElinkIdx < kuNbElinks; ++uElinkIdx )
   {
//      std::cout << Form( " %6u", uSmxMap[ uElinkIdx ] );
      std::cout << Form( " 0x%04X", uSmxMap[ uElinkIdx ] );
      if( kuNbElinks - 1 != uElinkIdx && 5 == uElinkIdx % 6  )
         std::cout << " \\" << std::endl;
   } // for( UInt_t uElinkIdx = 0; uElinkIdx < kuNbElinks; ++uElInkIdx )
   std::cout << std::endl;

   std::cout << " Reordered map from (Feb, Asic) to eLink: "
             << std::endl;
   for( UInt_t uFebIdx = 0; uFebIdx < kuNbFebs; ++uFebIdx )
   {
      for( UInt_t uSmxIdx = 0; uSmxIdx < kuNbSmx; ++uSmxIdx )
         std::cout << Form( " %2u", uElinkMapReordered[ uFebIdx ][ uSmxIdx ] );
      std::cout << std::endl;
   } // for( UInt_t uFebIdx = 0; uFebIdx < kuNbFebs; ++uFebIdx )

   return;
}

void eLinkMappingCrobSts()
{
   std::cout << "================= FEB A ===================" << std::endl;
   eLinkMappingCrobStsFebA();

   std::cout << "================= FEB B ===================" << std::endl;
   eLinkMappingCrobStsFebB();

   return;
}
