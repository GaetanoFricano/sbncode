foreach(tool Meson2ScalarKevin ScalarDecayKevin)
  cet_build_plugin( ${tool} art::tool
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
endforeach()

install_headers()
install_fhicl()
install_source()
