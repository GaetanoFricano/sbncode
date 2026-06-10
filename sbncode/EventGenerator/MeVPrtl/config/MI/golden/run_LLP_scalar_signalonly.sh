#!/bin/bash

OUTDIR=/exp/sbnd/data/users/gfricano/gen/LLP_scalar_signalonly/m250_BRprod1em11_ctau5000_BRee1p0
NEVENTS=25
NJOBS=400
MAXJOBS=1

BASEFCL=prodMeVPrtl_LLP_scalar_m250_BRprod1em11_ctau5000_BRee1p0_TPC_sbnd.fcl

mkdir -p "$OUTDIR"

for i in $(seq 1 $NJOBS); do

  SEED=$((1000 + i))
  TMPFCL=tmp_${i}.fcl

  sed "s/BASESEED/${SEED}/g" "$BASEFCL" > "$TMPFCL"

  lar -c "$TMPFCL" \
      -n "$NEVENTS" \
      -o "${OUTDIR}/mevprtl_${i}.root"

done

rm tmp_*.fcl
