#!/bin/bash

OUTDIR=/exp/sbnd/data/users/gfricano/gen/scalar
NEVENTS=25
NJOBS=400
MAXJOBS=1   # <-- UNA SOLA istanza lar

BASEFCL=run_mevprtl_scalar2leptons_kevin.fcl

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
