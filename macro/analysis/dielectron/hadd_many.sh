fileSizeLimit=50000
nofEventsInFile=2

#for plutoParticle in omegaepem; do
for plutoParticle in inmed qgp rho0 omegaepem omegadalitz phi; do
	
	dir=/lustre/nyx/cbm/users/slebedev/cbm/data/lmvm/test6/8gev/${plutoParticle}/
	filePattern=analysis.auau.8gev.centr.*.root
	outputFile=${dir}/analysis.auau.8gev.centr.all.root
        rm -rf ${outputFile}
	xterm -hold -e "root -l -b -q 'hadd.C(\"${dir}/${filePattern}\", \"${outputFile}\", ${fileSizeLimit}, ${nofEventsInFile})'"&
	
done