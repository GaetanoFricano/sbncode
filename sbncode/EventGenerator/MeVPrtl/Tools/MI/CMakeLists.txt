cet_build_plugin( Kaon2LLPFluxMI art::tool
  LIBRARIES
    art::Framework_Core
    art::Framework_Services_Registry
    art_root_io::TFileService_service
    art::Framework_Principal
    art::Persistency_Common
    art::Utilities canvas::canvas
    cetlib::cetlib cetlib_except::cetlib_except
    ROOT::EGPythia6
    CLHEP::CLHEP
    sbnobj::Common_EventGen_MeVPrtl
    nugen::EventGeneratorBase_GENIE
    nusimdata::SimulationBase
    nurandom::RandomUtils_NuRandomService_service
    GSL::gsl
    sbncode_EventGenerator_MeVPrtl_Constants
)

cet_build_plugin( Meson2LLPFluxMI art::tool
  LIBRARIES
    art::Framework_Core
    art::Framework_Services_Registry
    art_root_io::TFileService_service
    art::Framework_Principal
    art::Persistency_Common
    art::Utilities canvas::canvas
    cetlib::cetlib cetlib_except::cetlib_except
    ROOT::EGPythia6
    CLHEP::CLHEP
    sbnobj::Common_EventGen_MeVPrtl
    nugen::EventGeneratorBase_GENIE
    nusimdata::SimulationBase
    nurandom::RandomUtils_NuRandomService_service
    GSL::gsl
    sbncode_EventGenerator_MeVPrtl_Constants
)
 

cet_build_plugin( Meson2Scalar art::tool
  LIBRARIES
    art::Framework_Core
    art::Framework_Services_Registry
    art_root_io::TFileService_service
    art::Framework_Principal
    art::Persistency_Common
    art::Utilities canvas::canvas
    cetlib::cetlib
    cetlib_except::cetlib_except
    ROOT::EGPythia6
    CLHEP::CLHEP
    sbnobj::Common_EventGen_MeVPrtl
    nugen::EventGeneratorBase_GENIE
    nusimdata::SimulationBase
    nurandom::RandomUtils_NuRandomService_service
    GSL::gsl
    sbncode_EventGenerator_MeVPrtl_Constants
)

cet_build_plugin( ScalarDecay art::tool
  LIBRARIES
    art::Framework_Core
    art::Framework_Services_Registry
    art_root_io::TFileService_service
    art::Framework_Principal
    art::Persistency_Common
    art::Utilities canvas::canvas
    cetlib::cetlib
    cetlib_except::cetlib_except
    ROOT::EGPythia6
    CLHEP::CLHEP
    sbnobj::Common_EventGen_MeVPrtl
    nugen::EventGeneratorBase_GENIE
    nusimdata::SimulationBase
    nurandom::RandomUtils_NuRandomService_service
    GSL::gsl
    sbncode_EventGenerator_MeVPrtl_Constants
)

cet_build_plugin( Meson2ScalarKevin art::tool
  LIBRARIES
    art::Framework_Core
    art::Framework_Services_Registry
    art_root_io::TFileService_service
    art::Framework_Principal
    art::Persistency_Common
    art::Utilities canvas::canvas
    cetlib::cetlib
    cetlib_except::cetlib_except
    ROOT::EGPythia6
    CLHEP::CLHEP
    sbnobj::Common_EventGen_MeVPrtl
    nugen::EventGeneratorBase_GENIE
    nusimdata::SimulationBase
    nurandom::RandomUtils_NuRandomService_service
    GSL::gsl
    sbncode_EventGenerator_MeVPrtl_Constants
)

cet_build_plugin( ScalarDecayKevin art::tool
  LIBRARIES
    art::Framework_Core
    art::Framework_Services_Registry
    art_root_io::TFileService_service
    art::Framework_Principal
    art::Persistency_Common
    art::Utilities canvas::canvas
    cetlib::cetlib
    cetlib_except::cetlib_except
    ROOT::EGPythia6
    CLHEP::CLHEP
    sbnobj::Common_EventGen_MeVPrtl
    nugen::EventGeneratorBase_GENIE
    nusimdata::SimulationBase
    nurandom::RandomUtils_NuRandomService_service
    GSL::gsl
    sbncode_EventGenerator_MeVPrtl_Constants
)

install_headers()
install_fhicl()
install_source()