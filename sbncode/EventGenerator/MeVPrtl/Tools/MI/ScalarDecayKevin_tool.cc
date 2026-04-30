/**
 * ScalarDecay_tool_kevin.cc 
 *
 * Inputs (FHiCL):
 *   - CTau_cm : proper decay length c*tau in cm
 *   - BR_ee   : BR(S->e+e-)  (dimensionless)
 *
 * Mass mS comes from flux.mass.
 *
 * Mean lab decay distance:
 *   mean_dist_cm = CTau_cm * gamma * beta
 *
 * Forced-decay weight in [in,out]:
 *   w = exp(-a/mean) - exp(-b/mean)
 *
 * Channel selection:
 *   - if mS < 2me -> error
 *   - if 2me < mS < 2mμ -> force ee
 *   - if mS > 2mμ -> pick ee with BR_ee, μμ with 1-BR_ee (if enabled)
 */

#include "art/Utilities/ToolMacros.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "sbncode/EventGenerator/MeVPrtl/Tools/IMeVPrtlDecay.h"
#include "sbncode/EventGenerator/MeVPrtl/Tools/Constants.h"
#include "sbnobj/Common/EventGen/MeVPrtl/MeVPrtlFlux.h"

#include "CLHEP/Random/RandFlat.h"

#include <cmath>
#include <algorithm>
#include <string>

namespace evgen {
namespace ldm {

namespace {

double flat_to_exp_rand(double x, double mean, double a, double b) {
  const double A = (1. - std::exp(-(b-a)/mean));
  return - mean * std::log(1 - x * A) + a;
}

double forcedecay_weight(double mean, double a, double b) {
  return std::exp(-a/mean) - std::exp(-b/mean);
}

int pick_by_br(double br_e, double br_mu, double u01) {
  const double sum = br_e + br_mu;
  if (sum <= 0.0) return -1;
  return (u01 * sum < br_e) ? 0 : 1;
}

} // anon

class ScalarDecayKevin : public IMeVPrtlDecay {
public:
  ScalarDecayKevin(fhicl::ParameterSet const& pset);
  ~ScalarDecayKevin() override = default;

  void configure(fhicl::ParameterSet const& pset) override;

  bool Decay(const MeVPrtlFlux& flux,
             const TVector3& in,
             const TVector3& out,
             MeVPrtlDecay& decay,
             double& weight) override;

  double MaxWeight() override { return fMaxWeight; }

private:
  bool fAllowElectronDecay = true;
  bool fAllowMuonDecay     = true;
  bool fAddTimeOfFlight    = true;
  bool fVerbose            = false;

  // FHiCL inputs
  double fCTau_cm = 1.0;
  double fBR_ee   = 1.0;


  double fReferenceRayLength   = -1;
  double fReferenceRayDistance = 0.;
  double fReferenceMass        = -1;
  double fReferenceCTau_cm     = -1;
  double fReferenceEnergy      = -1;

  double fMaxWeight = -1.;
};

ScalarDecayKevin::ScalarDecayKevin(fhicl::ParameterSet const& pset)
  : IMeVPrtlStage("ScalarDecayKevin")
{
  configure(pset);
}

void ScalarDecayKevin::configure(fhicl::ParameterSet const& pset)
{
  fAllowElectronDecay = pset.get<bool>("AllowElectronDecay", true);
  fAllowMuonDecay     = pset.get<bool>("AllowMuonDecay", true);
  fAddTimeOfFlight    = pset.get<bool>("AddTimeOfFlight", true);
  fVerbose            = pset.get<bool>("Verbose", false);

  fCTau_cm = pset.get<double>("CTau_cm");
  fBR_ee   = pset.get<double>("BR_ee");


  if (fCTau_cm <= 0.0) {
    throw cet::exception("ScalarDecayKevin") << "CTau_cm must be > 0.";
  }
  fBR_ee = std::clamp(fBR_ee, 0.0, 1.0);

  fReferenceRayLength   = pset.get<double>("ReferenceRayLength", -1.0);
  fReferenceRayDistance = pset.get<double>("ReferenceRayDistance", 0.0);
  fReferenceMass        = pset.get<double>("ReferenceMass", -1.0);
  fReferenceCTau_cm     = pset.get<double>("ReferenceCTau_cm", -1.0);
  fReferenceEnergy      = pset.get<double>("ReferenceEnergy", -1.0);

  if (fReferenceRayLength > 0 && fReferenceMass > 0 && fReferenceCTau_cm > 0 && fReferenceEnergy > 0) {
    const double mS = fReferenceMass;

    const double p = std::sqrt(std::max(0.0, fReferenceEnergy*fReferenceEnergy - mS*mS));
    const double gamma_beta = (mS > 0) ? (p / mS) : 0.0;

    const double mean_dist = fReferenceCTau_cm * gamma_beta;
    if (mean_dist <= 0) { fMaxWeight = 0.0; return; }

    fMaxWeight = forcedecay_weight(mean_dist, fReferenceRayDistance,
                                   fReferenceRayDistance + fReferenceRayLength);
  } else {
    fMaxWeight = -1.;
  }
}

bool ScalarDecayKevin::Decay(const MeVPrtlFlux& flux,
                        const TVector3& in,
                        const TVector3& out,
                        MeVPrtlDecay& decay,
                        double& weight)
{
  const auto& C = Constants::Instance();

  const double mS = flux.mass;
  if (!(mS > 0.0)) return false;

  if (mS <= 2.0 * C.elec_mass) {
    throw cet::exception("ScalarDecayKevin")
      << "BAD MASS: mS=" << mS << " below 2*me threshold.";
  }


  const bool ee_open  = fAllowElectronDecay && (mS > 2.0*C.elec_mass);
  const bool mumu_open = fAllowMuonDecay    && (mS > 2.0*C.muon_mass);

  if (!ee_open && !mumu_open) return false;

  double br_e = 0.0;
  double br_mu = 0.0;

  if (ee_open && mumu_open) {
    br_e  = fBR_ee;
    br_mu = 1.0 - fBR_ee;
  } else if (ee_open) {
    // mu closed -> force ee
    br_e = 1.0;
    br_mu = 0.0;
  } else {
    // ee closed (shouldn't happen if mS>2me) -> force mu
    br_e = 0.0;
    br_mu = 1.0;
  }

  // mean lab decay distance [cm]
  const double mean_dist = fCTau_cm * flux.mom.Gamma() * flux.mom.Beta();
  if (mean_dist <= 0.0) return false;

  const double in_dist  = (flux.pos.Vect() - in ).Mag();
  const double out_dist = (flux.pos.Vect() - out).Mag();

  weight = forcedecay_weight(mean_dist, in_dist, out_dist);
  if (weight == 0.0) return false;

  // pick channel by BR
  const double u = CLHEP::RandFlat::shoot(fEngine, 0.0, 1.0);
  const int ch = pick_by_br(br_e, br_mu, u);
  if (ch < 0) return false;

  const int abs_pdg = (ch == 0) ? 11 : 13;
  const double ml   = (ch == 0) ? C.elec_mass : C.muon_mass;

  // sample decay point along ray within [in,out]
  const double u2 = CLHEP::RandFlat::shoot(fEngine, 0.0, 1.0);
  const double decay_rand = flat_to_exp_rand(u2, mean_dist, in_dist, out_dist);
  TVector3 decay_pos3 = flux.pos.Vect() + decay_rand * (in - flux.pos.Vect()).Unit();

  const double decay_time = fAddTimeOfFlight ? TimeOfFlight(flux, decay_pos3) : flux.pos.T();
  TLorentzVector decay_pos(decay_pos3, decay_time);

  // 2-body kinematics in S rest frame
  const double E_rf = mS / 2.0;
  const double p_rf = std::sqrt(std::max(0.0, E_rf*E_rf - ml*ml));

  TVector3 pA_rf = RandomUnitVector() * p_rf;
  TVector3 pB_rf = -pA_rf;

  TLorentzVector p4A(pA_rf, E_rf);
  TLorentzVector p4B(pB_rf, E_rf);

  p4A.Boost(flux.mom.BoostVector());
  p4B.Boost(flux.mom.BoostVector());


  const double tau_ns = fCTau_cm / C.c_cm_per_ns;
  const double gamma_tot = (tau_ns > 0) ? (C.hbar / tau_ns) : 0.0;

  decay.total_decay_width   = gamma_tot;
  decay.total_mean_lifetime = tau_ns;
  decay.total_mean_distance = mean_dist;
  decay.allowed_decay_fraction = 1.0;

  decay.pos = decay_pos;

  decay.daughter_mom.clear();
  decay.daughter_e.clear();
  decay.daughter_pdg.clear();

  decay.daughter_mom.push_back(p4A.Vect());
  decay.daughter_e.push_back(p4A.E());
  decay.daughter_pdg.push_back(+abs_pdg);

  decay.daughter_mom.push_back(p4B.Vect());
  decay.daughter_e.push_back(p4B.E());
  decay.daughter_pdg.push_back(-abs_pdg);

  if (fVerbose) {
    mf::LogInfo("ScalarDecayKevin")
      << "mS=" << mS
      << " CTau_cm=" << fCTau_cm
      << " BR_ee(eff)=" << br_e << " BR_mumu(eff)=" << br_mu
      << " mean_dist_cm=" << mean_dist
      << " weight=" << weight;
  }

  return true;
}

DEFINE_ART_CLASS_TOOL(ScalarDecayKevin)

} // namespace ldm
} // namespace evgen