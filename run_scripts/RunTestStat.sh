#!/bin/bash


mkdir -p output/CUPID_sens/frequentist_$1/
mkdir -p output/CUPID_sens/frequentist_$1/test_stat/

#./bin/CUPID_Sens -m B -n 10000 -s 1 -b 1e-4 -f 0 -e bayesian_baseline_simple_shape > log/bayesian_baseline_simple_shape.log &
#./bin/CUPID_Sens -m B -n 10000 -s 1 -b 1e-4 -f 1 -e bayesian_baseline_full_shape -F 0 -L 100 -G 0
#./bin/CUPID_Sens -m B -n 10000 -s 1 -b 6e-5 -f 0 -e bayesian_optimist_simple_shape > log/bayesian_optimist_simple_shape.log &
#./bin/CUPID_Sens -m B -n 10000 -s 1 -b 6e-5 -f 1 -e bayesian_optimist_full_shape > log/bayesian_optimist_full_shape.log &
#./bin/CUPID_Sens -m B -n 10000 -s 1 -b 2e-5 -f 0 -e bayesian_reach_simple_shape > log/bayesian_reach_simple_shape.log &
#./bin/CUPID_Sens -m B -n 10000 -s 1 -b 2e-5 -f 1 -e bayesian_reach_full_shape > log/bayesian_reach_full_shape.log &

export Nsig=100
export first=0
export Ntoys=100000
echo $Nsig

python /data/users/tdixon/QueueDaemon.py -c /data/users/tdixon/BATGraphHistoFit/Sensitivity_QD.pbs -o "CONFIG=-m F -n $Ntoys -s $Nsig -b ${2} -f ${3} -e frequentist_${1} -r 5e-27"  -n $Nsig -t 10 -s $first 
