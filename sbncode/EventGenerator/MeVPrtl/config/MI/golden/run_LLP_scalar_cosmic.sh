#!/bin/bash

OUTDIR=/exp/sbnd/app/users/gfricano/generator/larsoft_v10_10_03/srcs/sbncode/sbncode/EventGenerator/MeVPrtl/config/MI/golden/test
NEVENTS=2
NJOBS=1
MAXJOBS=1

BASEFCL=prodoverlay_corsika_MeVPrtl_LLP_scalar_m250_BRprod1em11_ctau1000_BRee1p0_TPC_sbnd.fcl

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
