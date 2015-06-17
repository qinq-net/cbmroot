# Needed to run macro via script
export SCRIPT=yes

run_hadd() {
   export LMVM_MAIN_DIR=${1}
   export LMVM_FILE_ARRAY=${2}
   export LMVM_NOF_FILES=${3}
   root -l -b -q hadd.C
}

run_hadd_ana_litqa() {
	run_hadd ${1} ${2} ${3}
        run_hadd ${1} ${2} ${3}
}

run_hadd_ana_litqa ${1} ${2} ${3}

echo "All done. You can close terminal"

export SCRIPT=no