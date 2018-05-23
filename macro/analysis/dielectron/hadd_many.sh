fileSizeLimit=50000
nofEventsInFile=1000

#for plutoParticle in omegaepem; do
for plutoParticle in inmed qgp rho0 omegaepem omegadalitz phi; do
	
	dir=/lustre/nyx/cbm/users/slebedev/cbm/data/lmvm/test6/${plutoParticle}/
	filePattern=analysis.auau.8gev.centr.*.root
	outputFile=${dir}/analysis.auau.8gev.centr.all.root
	xterm -hold -e "root -l -b -q 'hadd.C(\"${dir}/${filePattern}\", \"${outputFile}\", ${fileSizeLimit}, ${nofEventsInFile})'"&
	
done