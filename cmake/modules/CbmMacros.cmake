# Remove trailing slash from a path passed as argument
Macro(Remove_Trailing_Slash _variable)
  String(FIND ${_variable} "/" _pos_last_slash REVERSE)
  STRING(LENGTH ${_variable} _length)
  Math(EXPR _last_pos ${_pos_last_slash}+1)
  If(${_last_pos} EQUAL ${_length})  
    String(SUBSTRING ${_variable} 0 ${_pos_last_slash} _ret_val)
  Else()
    Set(_ret_val ${_variable})
  EndIf()
EndMacro()

Macro(FairRootVersion)

  Execute_Process(COMMAND $ENV{FAIRROOTPATH}/bin/fairroot-config --version
                  OUTPUT_VARIABLE _fairroot_version
                  OUTPUT_STRIP_TRAILING_WHITESPACE
                 )
  String(REGEX MATCH "v-([0-9]+)\\.([0-9]+)([a-z]*)" _version_matches "${_fairroot_version}")
  Set(FairRoot_VERSION_MAJOR ${CMAKE_MATCH_1})
  Set(FairRoot_VERSION_MINOR ${CMAKE_MATCH_2})
  Set(FairRoot_VERSION_PATCH ${CMAKE_MATCH_3})

  If(FairRoot_VERSION_PATCH MATCHES "a")
    Set(FairRoot_VERSION_PATCH 1)
  ElseIf(FairRoot_VERSION_PATCH MATCHES "b")
    Set(FairRoot_VERSION_PATCH 2)
  ElseIf(FairRoot_VERSION_PATCH MATCHES "c")
    Set(FairRoot_VERSION_PATCH 3)
  ElseIf(FairRoot_VERSION_PATCH MATCHES "d")
    Set(FairRoot_VERSION_PATCH 4)
  ElseIf(FairRoot_VERSION_PATCH MATCHES "e")
    Set(FairRoot_VERSION_PATCH 4)
  ElseIf(FairRoot_VERSION_PATCH MATCHES "f")
    Set(FairRoot_VERSION_PATCH 5)
  ElseIf(FairRoot_VERSION_PATCH MATCHES "g")
    Set(FairRoot_VERSION_PATCH 6)
  Else()
    Set(FairRoot_VERSION_PATCH 0)
  EndIf()

  Set(FairRoot_VERSION
      ${FairRoot_VERSION_MAJOR}.${FairRoot_VERSION_MINOR}.${FairRoot_VERSION_PATCH}
     ) 
EndMacro()

Macro(Gen_Exe_Script _ExeName) 

  set(shell_script_name "${_ExeName}.sh")

  string(REPLACE ${PROJECT_SOURCE_DIR}
         ${PROJECT_BINARY_DIR} new_path ${CMAKE_CURRENT_SOURCE_DIR}
        )

  set(my_exe_name ${EXECUTABLE_OUTPUT_PATH}/${_ExeName})

  configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/run_binary.sh.in
                   ${new_path}/${shell_script_name}
                  )

  Execute_Process(COMMAND /bin/chmod u+x  ${new_path}/${shell_script_name}
                  OUTPUT_QUIET
                 )

EndMacro(Generate_Exe_Script)
