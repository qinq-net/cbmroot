# Needed to run macro via script
export SCRIPT=yes

run_hadd() {
   export LMVM_MAIN_DIR=${1}
   export LMVM_FILE_ARRAY=${2}
   export LMVM_ADD_STRING=${3}
   export LMVM_NOF_FILES=${4}
   root -l -b -q hadd.C
}

run_hadd_ana_litqa() {
	run_hadd ${1} ${2} litqa ${3}
	run_hadd ${1} ${2} analysis ${3}
}

run_hadd_ana_litqa /hera/cbm/users/slebedev/mc/dielectron/dec14/8gev/stsv13d/richv14a/trd4/tofv13/1.0field/nomvd/ .auau.8gev.centr. 250
run_hadd_ana_litqa /hera/cbm/users/slebedev/mc/dielectron/dec14/25gev/stsv13d/richv14a/trd10/tofv13/1.0field/nomvd/ .auau.25gev.centr. 250  
run_hadd_ana_litqa /hera/cbm/users/slebedev/mc/dielectron/dec14/8gev/stsv13d/richv14a_bepipe/trd4/tofv13/1.0field/nomvd/ .auau.8gev.centr. 250
run_hadd_ana_litqa /hera/cbm/users/slebedev/mc/dielectron/dec14/25gev/stsv13d/richv14a_bepipe/trd10/tofv13/1.0field/nomvd/ .auau.25gev.centr. 250
run_hadd_ana_litqa /hera/cbm/users/slebedev/mc/dielectron/dec14/8gev/stsv13d/richv14a/trd4/tofv13/0.7field/nomvd/ .auau.8gev.centr. 250
run_hadd_ana_litqa /hera/cbm/users/slebedev/mc/dielectron/dec14/8gev/stsv14_2cm/richv14a/trd4/tofv13/1.0field/nomvd/ .auau.8gev.centr. 250
run_hadd_ana_litqa /hera/cbm/users/slebedev/mc/dielectron/dec14/25gev/stsv14_2cm/richv14a/trd10/tofv13/1.0field/nomvd/ .auau.25gev.centr. 250
run_hadd_ana_litqa /hera/cbm/users/slebedev/mc/dielectron/dec14/8gev/stsv14_4cm/richv14a/trd4/tofv13/1.0field/nomvd/ .auau.8gev.centr. 250
run_hadd_ana_litqa /hera/cbm/users/slebedev/mc/dielectron/dec14/25gev/stsv14_4cm/richv14a/trd10/tofv13/1.0field/nomvd/ .auau.25gev.centr. 250

export SCRIPT=no