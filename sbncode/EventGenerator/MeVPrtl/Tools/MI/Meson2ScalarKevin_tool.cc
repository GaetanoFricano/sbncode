/**
 * Meson2Scalar_tool_kevin.cc (NEW)
 *
 * K± -> π± + S (two-body)
 *
 * FHiCL inputs:
 *   - M      : scalar mass [GeV]
 *   - BRprod : BR(K->pi S) (dimensionless)
 *
 * Weight (DK2NU semantics unchanged):
 *   weight = parent.weight * BRprod / denom
 */

#include "art/Utilities/ToolMacros.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "sbncode/EventGenerator/MeVPrtl/Tools/IMeVPrtlFlux.h"
#include "sbnobj/Common/EventGen/MeVPrtl/MeVPrtlFlux.h"
#include "sbnobj/Common/EventGen/MeVPrtl/MesonParent.h"

#include "TDatabasePDG.h"
#include "TParticlePDG.h"

#include <cmath>
#include <algorithm>
#include <string>
#include <vector>

namespace evgen {
namespace ldm {

class Meson2ScalarKevin : public IMeVPrtlFlux {
public:
  Meson2ScalarKevin(fhicl::ParameterSet const& pset);
  ~Meson2ScalarKevin() override = default;

  void configure(fhicl::ParameterSet const& pset) override;
  bool MakeFlux(const simb::MCFlux& flux, MeVPrtlFlux& llp, double& weight) override;

  double MaxWeight() override { return 1.0; }

private:
  double fM      = 0.0;  // [GeV]
  double fBRprod = 0.0;  // BR(K->pi S)

  bool fVerbose = false;

  std::vector<int> fParents;
  bool fCorrectDK2NU = true;

  std::string fSecondarySignConvention = "legacy"; // legacy/physical

  static double pdg_mass_GeV(int pdg) {
    auto* db = TDatabasePDG::Instance();
    TParticlePDG* p = db->GetParticle(pdg);
    return p ? p->Mass() : -1.0;
  }

  static double twobody_momentum(double M, double m1, double m2) {
    if (M < m1 + m2) return -1.0;
    const double a = M*M - (m1+m2)*(m1+m2);
    const double b = M*M - (m1-m2)*(m1-m2);
    if (a < 0 || b < 0) return -1.0;
    return std::sqrt(a*b) / (2.0*M);
  }

  static double SMKaonToNuBR(int kaon_pdg) {
    switch (kaon_pdg) {
      case 321:
      case -321:
        return 0.6339 + 0.0559 + 0.0330;
      default:
        return 1.0;
    }
  }

  static double SMPionToNuBR(int pion_pdg) {
    if (std::abs(pion_pdg) == 211) return 1.0;
    return 1.0;
  }

  int make_secondary_pdg_pion(int parent_pdg) const {
    if (fSecondarySignConvention == "legacy") return 211; // always pi+
    if (parent_pdg == 321)  return  211;
    if (parent_pdg == -321) return -211;
    return 211;
  }

  bool parent_allowed(int abs_pdg) const {
    for (int p : fParents) if (abs_pdg == std::abs(p)) return true;
    return false;
  }
};

Meson2ScalarKevin::Meson2ScalarKevin(fhicl::ParameterSet const& pset)
  : IMeVPrtlStage("Meson2ScalarKevin")
  , IMeVPrtlFlux(pset)
{
  configure(pset);
}

void Meson2ScalarKevin::configure(fhicl::ParameterSet const& pset)
{
  fM      = pset.get<double>("M");
  fBRprod = pset.get<double>("BRprod");
  fVerbose = pset.get<bool>("Verbose", false);

  fParents = pset.get<std::vector<int>>("Parents", std::vector<int>{321}); // default K±
  fCorrectDK2NU = pset.get<bool>("CorrectDK2NU", true);

  fSecondarySignConvention =
    pset.get<std::string>("SecondarySignConvention", "legacy");
  std::transform(fSecondarySignConvention.begin(), fSecondarySignConvention.end(),
                 fSecondarySignConvention.begin(), ::tolower);
}

bool Meson2ScalarKevin::MakeFlux(const simb::MCFlux& flux,
                            evgen::ldm::MeVPrtlFlux& llp,
                            double& weight)
{
  evgen::ldm::MesonParent parent(flux);

  const int mpdg = parent.meson_pdg;
  const int abs_mpdg = std::abs(mpdg);

  if (!(abs_mpdg == 321 || abs_mpdg == 211)) return false;
  if (!parent_allowed(abs_mpdg)) return false;

  // implement K± -> π± S only
  if (abs_mpdg != 321) return false;

  const double mS = fM;
  if (!(mS > 0.0)) return false;

  double br = fBRprod;
  if (!(br > 0.0)) return false;
  if (br > 1.0) {
    if (fVerbose) mf::LogWarning("Meson2ScalarKevin") << "BRprod>1 (" << br << "), capping to 1.";
    br = 1.0;
  }

  
  TLorentzVector Beam4 = BeamOrigin();
  llp.pos_beamcoord = parent.pos;
  llp.pos = parent.pos;
  llp.pos.Transform(fBeam2Det);
  llp.pos += Beam4;

  llp.mmom_beamcoord = parent.mom;
  llp.mmom = parent.mom;
  llp.mmom.Transform(fBeam2Det);

  const int pi_pdg = make_secondary_pdg_pion(mpdg);

  const double mK  = pdg_mass_GeV(321);
  const double mpi = pdg_mass_GeV(pi_pdg);
  if (mK <= 0 || mpi <= 0) return false;

  const double pstar = twobody_momentum(mK, mpi, mS);
  if (pstar < 0) return false;

  const double eS = std::sqrt(pstar*pstar + mS*mS);

  // isotropic S in K rest frame
  llp.mom = TLorentzVector(pstar * RandomUnitVector(), eS);

  // boost to lab
  TLorentzVector mom_lab = llp.mom;
  mom_lab.Boost(parent.mom.BoostVector());

  llp.mom_beamcoord = mom_lab;
  llp.mom = mom_lab;
  llp.mom.Transform(fBeam2Det);

  llp.sec_beamcoord = llp.mmom_beamcoord - llp.mom_beamcoord;
  llp.sec = llp.mmom - llp.mom;

  double denom = 1.0;
  if (fCorrectDK2NU) {
    if (abs_mpdg == 321) denom = SMKaonToNuBR(mpdg);
    if (abs_mpdg == 211) denom = SMPionToNuBR(mpdg);
    if (denom <= 0) denom = 1.0;
  }

  weight = parent.weight * br / denom;
  if (weight == 0.0) return false;

  llp.mass = mS;
  llp.meson_pdg = mpdg;
  llp.secondary_pdg = pi_pdg;

  llp.generator = 1;
  llp.equiv_enu = EnuLab(flux.fnecm, llp.mmom, llp.pos);

 
  llp.C1 = 0.0; llp.C2 = 0.0; llp.C3 = 0.0; llp.C4 = 0.0; llp.C5 = 0.0;

  if (fVerbose) {
    mf::LogInfo("Meson2ScalarKevin")
      << "parent=" << mpdg
      << " mS=" << mS
      << " BRprod=" << br
      << " denom=" << denom
      << " weight=" << weight;
  }

  return true;
}

DEFINE_ART_CLASS_TOOL(Meson2ScalarKevin)

} // namespace ldm
} // namespace evgen