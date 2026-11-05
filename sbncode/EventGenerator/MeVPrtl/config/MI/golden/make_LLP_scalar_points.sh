#!/bin/bash

POINTS=(
  "100 1000  m100_ctau1000"
  "100 5000  m100_ctau5000"
  "100 10000 m100_ctau10000"
  "250 1000  m250_ctau1000"
  "250 10000 m250_ctau10000"
)

for P in "${POINTS[@]}"; do
  set -- $P

  MASS_MEV=$1
  CTAU_CM=$2

  MASS_GEV=$(awk "BEGIN {printf \"%.3f\", ${MASS_MEV}/1000}")
  NAME="m${MASS_MEV}_BRprod1em11_ctau${CTAU_CM}_BRee1p0_TPC_sbnd"

  SIGNAL_FILE="prodMeVPrtl_LLP_scalar_${NAME}.fcl"
  COSMIC_FILE="prodoverlay_corsika_MeVPrtl_LLP_scalar_${NAME}.fcl"

  cat > "$SIGNAL_FILE" <<EOL
#include "prodMeVPrtl_LLP_scalar_sbnd.fcl"

###----------Set scalar mass [GeV]----------###

scalarM: ${MASS_GEV}

physics.producers.generator.Flux.M: @local::scalarM
physics.producers.generator.RayTrace.ReferencePrtlMass: @local::scalarM
physics.producers.generator.Decay.ReferenceMass: @local::scalarM


###----------Set production BR K -> pi S ----------###

BRprod: 1e-11

physics.producers.generator.Flux.BRprod: @local::BRprod


###----------Set scalar lifetime c*tau [cm]----------###

CTau_cm: ${CTAU_CM}

physics.producers.generator.Decay.CTau_cm: @local::CTau_cm
physics.producers.generator.Decay.ReferenceCTau_cm: @local::CTau_cm


###----------Set scalar decay BR S -> e+e- ----------###

BR_ee: 1.0

physics.producers.generator.Decay.BR_ee: @local::BR_ee


###-----------Verbose--------------###

#include "set_MeVPrtl_Verbose.fcl"


###----------Set geometry ----------###

#include "set_MeVPrtl_GeoTPC.fcl"
EOL

  cat > "$COSMIC_FILE" <<EOL
#include "prodoverlay_corsika_MeVPrtl_LLP_scalar_sbnd.fcl"

###----------Set scalar mass [GeV]----------###

scalarM: ${MASS_GEV}

physics.producers.generator.Flux.M: @local::scalarM
physics.producers.generator.RayTrace.ReferencePrtlMass: @local::scalarM
physics.producers.generator.Decay.ReferenceMass: @local::scalarM


###----------Set production BR K -> pi S ----------###

BRprod: 1e-11

physics.producers.generator.Flux.BRprod: @local::BRprod


###----------Set scalar lifetime c*tau [cm]----------###

CTau_cm: ${CTAU_CM}

physics.producers.generator.Decay.CTau_cm: @local::CTau_cm
physics.producers.generator.Decay.ReferenceCTau_cm: @local::CTau_cm


###----------Set scalar decay BR S -> e+e- ----------###

BR_ee: 1.0

physics.producers.generator.Decay.BR_ee: @local::BR_ee


###-----------Verbose--------------###

#include "set_MeVPrtl_Verbose.fcl"


###----------Set geometry ----------###

#include "set_MeVPrtl_GeoTPC.fcl"
EOL

  echo "Creati:"
  echo "  $SIGNAL_FILE"
  echo "  $COSMIC_FILE"
done
