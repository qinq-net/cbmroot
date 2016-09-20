# Build all libraries which com from external sources like other subversion or git repositories

Add_Subdirectory(ipc)
Add_Subdirectory(ipc_legacy)
Add_Subdirectory(flib_dpb)
Add_Subdirectory(spadic)

Execute_Process(COMMAND ${ROOT_CONFIG_EXECUTABLE} --has-vc
                OUTPUT_VARIABLE ROOT_HAS_VC
               )
String(STRIP ${ROOT_HAS_VC} ROOT_HAS_VC)
Set(ROOT_HAS_VC ${ROOT_HAS_VC} PARENT_SCOPE)

If(NOT ${ROOT_HAS_VC})
  Message(STATUS "Root does not support VC. Compile the internal version.")
  ExternalProject_Add(vc
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/vc
    PREFIX vc
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}
  )
  Add_Library(Vc STATIC IMPORTED GLOBAL)
  Set_Property(TARGET Vc PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/libVc.a)
  Set(VC_INCLUDE_DIRS       
      ${CBMROOT_SOURCE_DIR}/external/vc 
      ${CBMROOT_SOURCE_DIR}/external/vc/include/
      PARENT_SCOPE
     )
  Install(FILES $${CMAKE_BINARY_DIR}/lib/libVc.a DESTINATION lib)
EndIf()
