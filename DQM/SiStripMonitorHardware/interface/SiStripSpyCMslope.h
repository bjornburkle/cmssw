#ifndef DQM_SiStripMonitorHardware_SiStripCMslope_H
#define DQM_SiStripMonitorHardware_SiStripCMslope_H

// Standard
#include <memory>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>

// Framework include files
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

// Needed for the SST cabling
#include "CalibTracker/Records/interface/SiStripDetCablingRcd.h"
#include "CalibFormats/SiStripObjects/interface/SiStripDetCabling.h"
#include "CondFormats/SiStripObjects/interface/FedChannelConnection.h"

// Needed for the pedestal values
#include "CondFormats/SiStripObjects/interface/SiStripPedestals.h"
#include "CondFormats/DataRecord/interface/SiStripPedestalsRcd.h"

// Needed for the noise values
#include "CondFormats/SiStripObjects/interface/SiStripNoises.h"
#include "CondFormats/DataRecord/interface/SiStripNoisesRcd.h"

// For translating between FED key and det ID
#include "DataFormats/SiStripCommon/interface/SiStripFedKey.h"

#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/SiStripDigi/interface/SiStripRawDigi.h"
#include "DataFormats/SiStripDigi/interface/SiStripProcessedRawDigi.h"
#include "DataFormats/SiStripDigi/interface/SiStripDigi.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/SiStripCommon/interface/SiStripConstants.h"

#include "DataFormats/TrackerCommon/interface/TrackerTopology.h" //to get det partitions
#include "DataFormats/SiStripDetId/interface/StripSubdetector.h"
#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/TrackerGeometryBuilder/interface/StripGeomDetUnit.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerParametersFromDD.h" //might    not need
#include "Geometry/TrackerNumberingBuilder/interface/GeometricDet.h" //might not need
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/CommonTopologies/interface/StripTopology.h"
#include "Geometry/Records/interface/IdealGeometryRecord.h"
#include "Geometry/Records/interface/TrackerTopologyRcd.h"
#include "RecoTracker/Record/interface/TrackerRecoGeometryRecord.h"
#include "MagneticField/Engine/interface/MagneticField.h"
//#include "RecoTracker/Record/interface/TrackerRecoGeometryRecord.h"

// Needed for the FED raw data processing
#include "EventFilter/SiStripRawToDigi/interface/SiStripFEDBuffer.h"
#include "EventFilter/SiStripRawToDigi/interface/SiStripFEDBufferGenerator.h"

// #include "EventFilter/SiStripRawToDigi/interface/SiStripDigiToRaw.h"

//for cabling
#include "DQM/SiStripMonitorHardware/interface/SiStripSpyUtilities.h"

// For plotting
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1S.h"
#include "TH1D.h"
#include "TF1.h" //for fitting //Bjorn
#include "TGraphErrors.h"
#include "TMath.h"

//
// constants, enums and typedefs
//
enum FEDSpyHistogramType {SCOPE_MODE,
                      PAYLOAD_RAW,
                      REORDERED_PAYLOAD_RAW,
                      REORDERED_MODULE_RAW,
                      PEDESTAL_VALUES,
                      NOISE_VALUES,
                      POST_PEDESTAL,
                      POST_COMMON_MODE,
                      ZERO_SUPPRESSED_PADDED,
                      ZERO_SUPPRESSED,
                      VR_COMP,
                      ZERO_SUPPRESSED_COMP};

static const unsigned int nParts = 4;
static const std::string partitions_[nParts] = {"TIB", "TOB", "TID", "TEC"};
static const unsigned int layers_[nParts] = {4, 6, 3, 9};
static const unsigned int sides_[nParts] = {1, 1, 2, 2};



class SiStripSpyCMslope : public edm::EDAnalyzer{

public:
    explicit SiStripSpyCMslope(const edm::ParameterSet&);
    ~SiStripSpyCMslope() override;

private:
    void beginRun(const edm::Run&, const edm::EventSetup&) override;
    void beginJob() override;
    void analyze(const edm::Event&, const edm::EventSetup&) override;
    void endJob() override;

    Bool_t MakeRawDigiHist_(
            const edm::Handle<edm::DetSetVector<SiStripRawDigi>> &digi_handle,
            const edm::Handle<edm::DetSetVector<SiStripDigi>> &zs_handle,
            uint32_t specifier,
            const TFileDirectory & dir,
            FEDSpyHistogramType type,
            const edm::ESHandle<TrackerTopology> tTopo
            );

    // Member Data
    // ============
    std::vector<uint32_t> detIDs_;
    std::vector<uint32_t> checkIDs_;
    //sistrip::SpyUtilities utility_;

    // Data input labels
    // ==================
    edm::InputTag inputPostPedestalRawDigiLabel_;
    edm::InputTag inputZeroSuppressedDigiLabel_;
    edm::EDGetTokenT<edm::DetSetVector<SiStripRawDigi>> inputPostPedestalRawDigiToken_;
    edm::EDGetTokenT<edm::DetSetVector<SiStripDigi>> inputZeroSuppressedDigiToken_;

    // Output information
    // ===================
    std::string outputFolderName_;

    // Make hists for all partitions
    // ==============================
    bool allParts_;

    // Histogram Parameters
    // =====================
    double slopeMax_;
    int slopeBins_;
    double stddevMax_;
    double stddevLimit_;
    double avgMax_;
    int avgBins_;

    // Strip and sensor filtering parameters
    bool filterClusters_;
    int adcCutoff_;
    int minStrips_;

    // DEBUG
    // ======
    bool DEBUG;

    // File Directories
    // =================
    TFileDirectory an_dir;

    // Run Hists
    // ==========
    //TH1D * RUN_SLOPE_HIST;
    //TH1D * RUN_AVG_HIST;
    //TH1D * RUN_STDDEV_HIST;

}; // end of SiStripSpyCMslope class

#endif //DQM_SiStripMonitorHardware_SiStripCMslope_H
