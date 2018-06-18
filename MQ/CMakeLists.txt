# Extract the FairSoft version from fairsoft-config
# The version info is of the form Major.Minor e.g. 18.5 and
# is stored in the variable FairSoft_VERSION
FairSoftVersion()

# One can't compile the MQ subdirectory with a combination of FairSoft may18 and
# FairRoot v-17.10 or older. The reason is that FairMQ and the FairLogger
# were moved as external projects from FairRoot to FairSoft with FairSoft 
# release may18.
# Combinations of FairSoft may18 and FairRoot v18.0.0 or FairSoft oct17 and 
# FairRoot v-17.10 or older work without problem

If( (FairRoot_VERSION VERSION_LESS 17.10.99 AND FairSoft_VERSION VERSION_LESS 17.11.0) OR
    (FairRoot_VERSION VERSION_GREATER 17.10.99 AND FairSoft_VERSION VERSION_GREATER 17.11.0) )
  Message("Build MQ subdirectory.")
  add_subdirectory(source)
  add_subdirectory(sink)
  add_subdirectory(unpacker)
  add_subdirectory(parmq)
EndIf()
