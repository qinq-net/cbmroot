fileSizeLimit=50000
nofEventsInFile=1000

#for plutoParticle in omegaepem; do
for plutoParticle in inmed qgp omegaepem omegadalitz phi; do
	
	dir=/lustre/nyx/cbm/users/slebedev/cbm/data/lmvm/may18_100k/8gev/${plutoParticle}/
	filePatternAna=analysis.auau.8gev.centr.*.root
	outputFileAna=${dir}/analysis.auau.8gev.centr.all.root
        rm -rf ${outputFileAna}
	xterm -hold -e "root -l -b -q 'hadd.C(\"${dir}/${filePatternAna}\", \"${outputFileAna}\", ${fileSizeLimit}, ${nofEventsInFile})'"&
	
        filePatternLitqa=litqa.auau.8gev.centr.*.root
	outputFileLitqa=${dir}/litqa.auau.8gev.centr.all.root
        rm -rf ${outputFileLitqa}
	xterm -hold -e "root -l -b -q 'hadd.C(\"${dir}/${filePatternLitqa}\", \"${outputFileLitqa}\", ${fileSizeLimit}, ${nofEventsInFile})'"&
done