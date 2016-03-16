# #!
# run KF Particle Finder

MAINDIR=`pwd`

if [ -z $1 ]
then
  NEVENTS=1000
else
  NEVENTS=$1
fi

if [ -z $2 ]
then
  WORKDIR=$MAINDIR"/data"
else
  WORKDIR=$2
fi
cd $WORKDIR

NNN=40
TASKS=0
ONE=1
NOL=0


PID=""
for i in {0..78}
do
  CURDIR="Signal"$i
  cd $CURDIR
  echo `pwd`

  cp -rf $MAINDIR/Signal/* .
  
  bash runphys.sh $NEVENTS >& runphys.log &
  PID=$PID" "$!

#   if [ $TASKS -eq $NNN ]
#   then
#     wait $PID
#     PID=""
#     TASKS=0
#   fi
  
  TASKS=`expr $TASKS + $ONE`
  
  cd ../
done
wait $PID

cd $MAINDIR

root -l -b -q 'CalculateEfficincy.C("'$WORKDIR'")' > Efficiencies.txt

echo -e "\007"

