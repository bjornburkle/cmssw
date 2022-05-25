/* \file SiStripSpyCMslope.cc
 * \brief File containing code for calculating the slope of common mode and plotting.
 */
// -*- C++ -*-
//
// Package:    SiStripMonitorHardware
// Class:      SiStripSpyCMslope
// 
// Standard


#include "DQM/SiStripMonitorHardware/interface/SiStripSpyCMslope.h"


/*! \brief EDAnalyzer for the online monitoring of the FED using STT spy channel data.
 *
 * \author Jangbae Lee and Bjorn Burkle
 * \date July 2018
 *
 */

                          
using namespace sistrip;
using namespace std;

//
// constructors and destructor
//
SiStripSpyCMslope::SiStripSpyCMslope(const edm::ParameterSet& iConfig) :
    detIDs_(                             iConfig.getParameter< std::vector<uint32_t> >( "detIDs") ),
    checkIDs_(                           iConfig.getParameter< std::vector<uint32_t> >( "checkIDs") ),
    inputPostPedestalRawDigiLabel_(      iConfig.getParameter<edm::InputTag>( "InputPostPedestalRawDigiLabel" ) ),
    inputZeroSuppressedDigiLabel_(       iConfig.getParameter<edm::InputTag>( "InputZeroSuppressedDigiLabel" ) ),
    outputFolderName_(                   iConfig.getParameter<std::string>(   "OutputFolderName"    ) ),
    allParts_(                           iConfig.getParameter<bool>( "AllParts" ) ),
    slopeMax_(                           iConfig.getParameter<double>( "slopeMax" ) ),
    slopeBins_(                          iConfig.getParameter<int>( "slopeBins" ) ),
    stddevMax_(                          iConfig.getParameter<double>( "stddevMax" ) ),
    stddevLimit_(                        iConfig.getParameter<double>( "stddevLimit") ),
    avgMax_(                             iConfig.getParameter<double>( "avgMax" ) ),
    avgBins_(                            iConfig.getParameter<int>( "avgBins" ) ),
    filterClusters_(                     iConfig.getParameter<bool>( "filterClusters" ) ),
    adcCutoff_(                          iConfig.getParameter<int>( "adcCutoff" ) ),
    minStrips_(                          iConfig.getParameter<int>( "minStrips" ) ),
    DEBUG(                               iConfig.getParameter<bool>( "DEBUG" ) )

{

    //now do what ever initialization is needed
    inputPostPedestalRawDigiToken_     = consumes<edm::DetSetVector<SiStripRawDigi> >(inputPostPedestalRawDigiLabel_     );
    inputZeroSuppressedDigiToken_      = consumes<edm::DetSetVector<SiStripDigi> >(inputZeroSuppressedDigiLabel_    );

    edm::Service<TFileService> fs;
    // Check that the TFileService has been configured
    if ( !fs.isAvailable() ) {
        throw cms::Exception("Configuration") << "TFileService not available: did you configure it ?";
    }
    an_dir_ = fs->mkdir( outputFolderName_ );

}


SiStripSpyCMslope::~SiStripSpyCMslope()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}

void
SiStripSpyCMslope::beginRun(const edm::Run & iRun, const edm::EventSetup & iSetup)
{
    // Retrieve FED cabling object
    //iSetup.get<SiStripDetCablingRcd>().get( cabling_ );
    //std::stringstream ss;
    //cabling_->print(ss);
    //std::cout << ss.str() << std::endl;

} // end of beginRun method.

// ------------ method called once each job just before starting event loop  ------------
void 
SiStripSpyCMslope::beginJob()
{

    LogTrace(mlDqmSource_)
        << "[SiStripSpyCMslope::" << __func__ << "]"
        << "Creating output structure..." << std::endl;

    // Make histograms that will be filled
    an_dir_.make<TH1D>("CommonModeSlope", ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);
    an_dir_.make<TH1D>("CommonModeAverage", ";Avg ADC counts / strip;N APVs", avgBins_, 0, avgMax_);
    an_dir_.make<TH1D>("CommonModeStddev", ";ADC counts / strip std dev;N APVs", slopeBins_, 0, stddevMax_);
    an_dir_.make<TH1D>("CommonModeSlopeUncert", ";CM Slope Uncert;N APVs", slopeBins_, 0, 0.06);
    an_dir_.make<TH1D>("CommonModeSlopeRelUncert", "CM Slope Rel Uncert;N APVs", slopeBins_, 0, 10);

    // Making directories for each partition
    TFileDirectory TIB_dir = an_dir_.mkdir("TIB");
    TFileDirectory TID_dir = an_dir_.mkdir("TID");
    TFileDirectory TOB_dir = an_dir_.mkdir("TOB");
    TFileDirectory TEC_dir = an_dir_.mkdir("TEC");

    // Making primary hists for each partition
    TIB_dir.make<TH1D>("CommonModeSlope_TIB", ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);
    TIB_dir.make<TH1D>("CommonModeAverage_TIB", ";Avg ADC counts / strip;N APVs", avgBins_, 0, avgMax_);
    TIB_dir.make<TH1D>("CommonModeStddev_TIB", ";ADC counts / strip std dev;N APVs", slopeBins_, 0, stddevMax_);
    TIB_dir.make<TH1D>("CommonModeSlopeUncert_TIB", ";CM Slope;N APVs", slopeBins_, 0, 0.06);
    TIB_dir.make<TH1D>("CommonModeSlopeRelUncert_TIB", ";CM Slope;N APVs", slopeBins_, 0, 10);

    TOB_dir.make<TH1D>("CommonModeSlope_TOB", ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);
    TOB_dir.make<TH1D>("CommonModeAverage_TOB", ";Avg ADC counts / strip;N APVs", avgBins_, 0, avgMax_);
    TOB_dir.make<TH1D>("CommonModeStddev_TOB", ";ADC counts / strip std dev;N APVs", slopeBins_, 0, stddevMax_);
    TOB_dir.make<TH1D>("CommonModeSlopeUncert_TOB", ";CM Slope;N APVs", slopeBins_, 0, 0.06);
    TOB_dir.make<TH1D>("CommonModeSlopeRelUncert_TOB", ";CM Slope;N APVs", slopeBins_, 0, 10);

    TID_dir.make<TH1D>("CommonModeSlope_TID", ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);
    TID_dir.make<TH1D>("CommonModeAverage_TID", ";Avg ADC counts / strip;N APVs", avgBins_, 0, avgMax_);
    TID_dir.make<TH1D>("CommonModeStddev_TID", ";ADC counts / strip std dev;N APVs", slopeBins_, 0, stddevMax_);
    TID_dir.make<TH1D>("CommonModeSlopeUncert_TID", ";CM Slope;N APVs", slopeBins_, 0, 0.06);
    TID_dir.make<TH1D>("CommonModeSlopeRelUncert_TID", ";CM Slope;N APVs", slopeBins_, 0, 10);

    TEC_dir.make<TH1D>("CommonModeSlope_TEC", ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);
    TEC_dir.make<TH1D>("CommonModeAverage_TEC", ";Avg ADC counts / strip;N APVs", avgBins_, 0, avgMax_);
    TEC_dir.make<TH1D>("CommonModeStddev_TEC", ";ADC counts / strip std dev;N APVs", slopeBins_, 0, stddevMax_);
    TEC_dir.make<TH1D>("CommonModeSlopeUncert_TEC", ";CM Slope;N APVs", slopeBins_, 0, 0.06);
    TEC_dir.make<TH1D>("CommonModeSlopeRelUncert_TEC", ";CM Slope;N APVs", slopeBins_, 0, 10);

    // Make folders showing number of sensors with large stddev
    // This is done on partition level because of detId numbering
    // Min and max bin values are the range of detIds for sensors in
    // those partitions
    TIB_dir.make<TH1I>("One_LargeStddevDetId_TIB", ";detId;N Events", 55143, 369120277, 369175420);
    TOB_dir.make<TH1I>("One_LargeStddevDetId_TOB", ";detId;N Events", 88371, 436228133, 436316504);
    TID_dir.make<TH1I>("One_LargeStddevDetId_TID", ";detId;N Events", 13515, 402664069, 402677584);
    TEC_dir.make<TH1I>("One_LargeStddevDetId_TEC", ";detId;N Events", 399311, 470044965, 470444276);

    TIB_dir.make<TH1I>("Two_LargeStddevDetId_TIB", ";detId;N Events", 55143, 369120277, 369175420);
    TOB_dir.make<TH1I>("Two_LargeStddevDetId_TOB", ";detId;N Events", 88371, 436228133, 436316504);
    TID_dir.make<TH1I>("Two_LargeStddevDetId_TID", ";detId;N Events", 13515, 402664069, 402677584);
    TEC_dir.make<TH1I>("Two_LargeStddevDetId_TEC", ";detId;N Events", 399311, 470044965, 470444276);

    TIB_dir.make<TH1I>("Three_LargeStddevDetId_TIB", ";detId;N Events", 55143, 369120277, 369175420);
    TOB_dir.make<TH1I>("Three_LargeStddevDetId_TOB", ";detId;N Events", 88371, 436228133, 436316504);
    TID_dir.make<TH1I>("Three_LargeStddevDetId_TID", ";detId;N Events", 13515, 402664069, 402677584);
    TEC_dir.make<TH1I>("Three_LargeStddevDetId_TEC", ";detId;N Events", 399311, 470044965, 470444276);

    TIB_dir.make<TH1I>("Four_LargeStddevDetId_TIB", ";detId;N Events", 55143, 369120277, 369175420);
    TOB_dir.make<TH1I>("Four_LargeStddevDetId_TOB", ";detId;N Events", 88371, 436228133, 436316504);
    TID_dir.make<TH1I>("Four_LargeStddevDetId_TID", ";detId;N Events", 13515, 402664069, 402677584);
    TEC_dir.make<TH1I>("Four_LargeStddevDetId_TEC", ";detId;N Events", 399311, 470044965, 470444276);

    TIB_dir.make<TH1I>("Five_LargeStddevDetId_TIB", ";detId;N Events", 55143, 369120277, 369175420);
    TOB_dir.make<TH1I>("Five_LargeStddevDetId_TOB", ";detId;N Events", 88371, 436228133, 436316504);
    TID_dir.make<TH1I>("Five_LargeStddevDetId_TID", ";detId;N Events", 13515, 402664069, 402677584);
    TEC_dir.make<TH1I>("Five_LargeStddevDetId_TEC", ";detId;N Events", 399311, 470044965, 470444276);

    TIB_dir.make<TH1I>("Six_LargeStddevDetId_TIB", ";detId;N Events", 55143, 369120277, 369175420);
    TOB_dir.make<TH1I>("Six_LargeStddevDetId_TOB", ";detId;N Events", 88371, 436228133, 436316504);
    TID_dir.make<TH1I>("Six_LargeStddevDetId_TID", ";detId;N Events", 13515, 402664069, 402677584);
    TEC_dir.make<TH1I>("Six_LargeStddevDetId_TEC", ";detId;N Events", 399311, 470044965, 470444276);

    // Make folder to hole detIDs
    TFileDirectory id_dir = an_dir_.mkdir("Det_IDs");

    // If you are checking IDs, make histograms for each ID
    std::stringstream id_str;
    if (checkIDs_.size())
    {
    std::stringstream ss;
    ss << "[SiStripSpyCMslope::" << __func__ << "]" << "Making det ID histograms" << std::endl;
    for (std::vector<uint32_t>::iterator Id=checkIDs_.begin(); Id!=checkIDs_.end(); Id++) {
        id_str.str(std::string());
        id_str << *Id;
        ss << "Making hist detId_" << id_str.str() << std::endl;
        //id_dir.mkdir(id_str.str());
        id_dir.make<TH1D>(( std::string("detId_")+id_str.str() ).c_str(), ";CM Slope;N Events", slopeBins_, -1*slopeMax_, slopeMax_);
        id_dir.make<TH1D>(( std::string("detId_")+id_str.str()+std::string("_Uncert") ).c_str(), ";CM Slope Uncert;N Events", slopeBins_, 0, 0.06);
        id_dir.make<TH1D>(( std::string("detId_")+id_str.str()+std::string("_RelUncert") ).c_str(), ";CM Slope Rel Uncert;N Events", slopeBins_, 0, 10);
        id_dir.make<TH1D>(( std::string("detId_")+id_str.str()+std::string("_APV_1") ).c_str(), ";CM Slope;N Events", slopeBins_, -1*slopeMax_, slopeMax_);
        id_dir.make<TH1D>(( std::string("detId_")+id_str.str()+std::string("_APV_2") ).c_str(), ";CM Slope;N Events", slopeBins_, -1*slopeMax_, slopeMax_);
        id_dir.make<TH1D>(( std::string("detId_")+id_str.str()+std::string("_APV_3") ).c_str(), ";CM Slope;N Events", slopeBins_, -1*slopeMax_, slopeMax_);
        id_dir.make<TH1D>(( std::string("detId_")+id_str.str()+std::string("_APV_4") ).c_str(), ";CM Slope;N Events", slopeBins_, -1*slopeMax_, slopeMax_);
        id_dir.make<TH1D>(( std::string("detId_")+id_str.str()+std::string("_APV_5") ).c_str(), ";CM Slope;N Events", slopeBins_, -1*slopeMax_, slopeMax_);
        id_dir.make<TH1D>(( std::string("detId_")+id_str.str()+std::string("_APV_6") ).c_str(), ";CM Slope;N Events", slopeBins_, -1*slopeMax_, slopeMax_);
        id_dir.make<TH1D>(( std::string("detId_")+id_str.str()+std::string("_MaxADC") ).c_str(), ";Max ADC Count;N Events", 256, 0, 256);
        } // end loop over detIDs to check
    LogDebug(mlDqmSource_) << ss.str();
    } // end if size > 1


    // If we want to fill plots for all of the partitions
    // loop through folder structure and make plots for each side and layer
    // these parameters defined in class constructor. Unfortunately, this uses
    // a lot of embedded for loops
    if (allParts_)
    { 
    std::stringstream ss;
    ss << "[SiStripSpyCMslope::" << __func__ << "]" << "Making histograms for all detector partitions" << std::endl;
    std::stringstream hist_str;
    for (unsigned int ip=0; ip<nParts_; ip++)
    {
    for (unsigned int il=0; il<layers_[ip]; il++)
    {
    // if only one side, then make hists for each layer
    if (partitions_[ip] == "TIB") {
        hist_str.str(std::string());
        hist_str << partitions_[ip] << "_L" << il+1;
        ss << "making plot: " << partitions_[ip] << "/CommonModeSlope_" << hist_str.str() << std::endl;
        TIB_dir.make<TH1D>((std::string("CommonModeSlope_") + hist_str.str()).c_str(), ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);
    } // end if TIB
    else if (partitions_[ip] == "TOB") {
        hist_str.str(std::string());
        hist_str << partitions_[ip] << "_L" << il+1;
        ss << "making plot: " << partitions_[ip] << "/CommonModeSlope_" << hist_str.str() << std::endl;
        TOB_dir.make<TH1D>((std::string("CommonModeSlope_") + hist_str.str()).c_str(), ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);
    } // end if TOB
    else if (partitions_[ip] == "TID") {
        hist_str.str(std::string());
        hist_str << partitions_[ip] << "_mL" << il+1;
        ss << "making plot: " << partitions_[ip] << "/CommonModeSlope_" << hist_str.str() << std::endl;
        TID_dir.make<TH1D>((std::string("CommonModeSlope_") + hist_str.str()).c_str(), ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);

        hist_str.str(std::string());
        hist_str << partitions_[ip] << "_pL" << il+1;
        ss << "making plot: " << partitions_[ip] << "/CommonModeSlope_" << hist_str.str() << std::endl;
        TID_dir.make<TH1D>((std::string("CommonModeSlope_") + hist_str.str()).c_str(), ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);
        } // end if TID
    else if (partitions_[ip] == "TEC") {
        hist_str.str(std::string());
        hist_str << partitions_[ip] << "_mL" << il+1;
        ss << "making plot: " << partitions_[ip] << "/CommonModeSlope_" << hist_str.str() << std::endl;
        TEC_dir.make<TH1D>((std::string("CommonModeSlope_") + hist_str.str()).c_str(), ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);

        hist_str.str(std::string());
        hist_str << partitions_[ip] << "_pL" << il+1;
        ss << "making plot: " << partitions_[ip] << "/CommonModeSlope_" << hist_str.str() << std::endl;
        TEC_dir.make<TH1D>((std::string("CommonModeSlope_") + hist_str.str()).c_str(), ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);
        } // end if TEC
    } // end layers_ loop
    } // end nParts loop
    LogDebug(mlDqmSource_) << ss.str();
    } // end if allParts_
    
    std::cout << "Finishing begin job" << std::endl;

} // end of beginJob method.

// ------------ method called once each job just after ending the event loop  ------------
void SiStripSpyCMslope::endJob() {}

// ------------ method called to for each event  ------------
void
SiStripSpyCMslope::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    using namespace edm;
    using namespace std;


    //retrieve cabling
    //const SiStripDetCabling* lCabling = utility_.getDetCabling( iSetup );
    LogDebug(mlDqmSource_) << "[SiStripSpyCMslope::" << __func__ << "]" << "Obtaining Tracker Topology" << std::endl;
    //Get Tracker Topology
    edm::ESHandle<TrackerTopology> tTopo;
    //iSetup.get<IdealGeometryRecord>().get(tTopo);
    iSetup.get<TrackerTopologyRcd>().get(tTopo);

    //iSetup.get<TrackerDigiGeometryRecord>().get(tTopo);
    //iSetup.get<TrackerRecoGeometryRecord>().get(tTopo);

    // Initialize hists for the total run information
    TH1D * RUN_SLOPE_HIST;
    TH1D * RUN_AVG_HIST;
    TH1D * RUN_STDDEV_HIST;
    TH1D * RUN_UNCERT_HIST;
    TH1D * RUN_REL_UNCERT_HIST;

    // Set hists that were initialized up above
    RUN_SLOPE_HIST = an_dir_.getObject<TH1D>("CommonModeSlope","");
    RUN_AVG_HIST = an_dir_.getObject<TH1D>("CommonModeAverage","");
    RUN_STDDEV_HIST = an_dir_.getObject<TH1D>("CommonModeStddev","");
    RUN_UNCERT_HIST = an_dir_.getObject<TH1D>("CommonModeSlopeUncert","");
    RUN_REL_UNCERT_HIST = an_dir_.getObject<TH1D>("CommonModeSlopeRelUncert","");

    // Make the event directory filename
    stringstream ev_dir_name; 
    event_ = iEvent.id().event();
    ev_dir_name << "run" << iEvent.id().run() << "_event" << iEvent.id().event();
    TFileDirectory evdir = an_dir_.mkdir( ev_dir_name.str() );

    std::cout << "Analyzing Run: " << iEvent.id().run() << ", Event: " << iEvent.id().event() << std::endl;
    

    //make the histo for common mode for each event
    TH1D * EV_SLOPE_HIST = evdir.make<TH1D>("CommonModeSlope", ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);
    TH1D * EV_AVG_HIST = evdir.make<TH1D>("CommonModeAverage", ";Avg ADC counts / strip;N APVs", avgBins_, 0, avgMax_);
    TH1D * EV_STDDEV_HIST = evdir.make<TH1D>("CommonModeStddev", ";ADC counts / strip stddev;N APVs", slopeBins_, 0, stddevMax_);
    TH1D * EV_UNCERT_HIST = evdir.make<TH1D>("CommonModeSlopeUncert", ";CM Slope Uncert;N APVs", slopeBins_, 0, 0.06);
    TH1D * EV_REL_UNCERT_HIST = evdir.make<TH1D>("CommonModeSlopeRelUncert", ";CM Slope Rel Uncert;N APVs", slopeBins_, 0, 10);

    
    evdir.make<TH1D>("CommonModeSlope_Event_TOB", ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);
    evdir.make<TH1D>("CommonModeSlope_Event_TIB", ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);
    evdir.make<TH1D>("CommonModeSlope_Event_TID", ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);
    evdir.make<TH1D>("CommonModeSlope_Event_TEC", ";CM Slope;N APVs", slopeBins_, -1*slopeMax_, slopeMax_);

    evdir.make<TH1D>("CommonModeAverage_Event_TOB", ";Avg ADC Counts/ strip;N APVs", avgBins_, 0, avgMax_);
    evdir.make<TH1D>("CommonModeAverage_Event_TIB", ";Avg ADC Counts/ strip;N APVs", avgBins_, 0, avgMax_);
    evdir.make<TH1D>("CommonModeAverage_Event_TID", ";Avg ADC Counts/ strip;N APVs", avgBins_, 0, avgMax_);
    evdir.make<TH1D>("CommonModeAverage_Event_TEC", ";Avg ADC Counts/ strip;N APVs", avgBins_, 0, avgMax_);

    evdir.make<TH1D>("CommonModeStddev_Event_TOB", ";ADC counts / strip stddev;N APVs", slopeBins_, 0, stddevMax_);
    evdir.make<TH1D>("CommonModeStddev_Event_TIB", ";ADC counts / strip stddev;N APVs", slopeBins_, 0, stddevMax_);
    evdir.make<TH1D>("CommonModeStddev_Event_TID", ";ADC counts / strip stddev;N APVs", slopeBins_, 0, stddevMax_);
    evdir.make<TH1D>("CommonModeStddev_Event_TEC", ";ADC counts / strip stddev;N APVs", slopeBins_, 0, stddevMax_);
    
    if (allParts_)
    {   
    std::stringstream hist_str;
    std::stringstream ss;
    ss << LogDebug(mlDqmSource_) << "[SiStripSpyCMslope::" << __func__ << "]";
    for (unsigned int ip=0; ip<nParts_; ip++)
    {   
    for (unsigned int il=0; il<layers_[ip]; il++)
    {   
    // if only one side, then make hists for each layer
    if (partitions_[ip] == "TIB") {
        hist_str.str(std::string());
        hist_str << partitions_[ip] << "_L" << il+1;
        ss << "making plot: " << partitions_[ip] << "/CommonModeSlope_Event_" << hist_str.str() << std::endl;
        evdir.make<TH1D>((std::string("CommonModeSlope_Event_") + hist_str.str()).c_str(), ";CM Slope;N APVs", slopeBins_, -1*slopeMax_,   slopeMax_);
    } // end if TIB
    else if (partitions_[ip] == "TOB") {
        hist_str.str(std::string());
        hist_str << partitions_[ip] << "_L" << il+1;
        ss << "making plot: " << partitions_[ip] << "/CommonModeSlope_Event_" << hist_str.str() << std::endl;
        evdir.make<TH1D>((std::string("CommonModeSlope_Event_") + hist_str.str()).c_str(), ";CM Slope;N APVs", slopeBins_, -1*slopeMax_,   slopeMax_);
    } // end if TOB
    else if (partitions_[ip] == "TID") {
        hist_str.str(std::string());
        hist_str << partitions_[ip] << "_mL" << il+1;
        ss << "making plot: " << partitions_[ip] << "/CommonModeSlope_Event_" << hist_str.str() << std::endl;
        evdir.make<TH1D>((std::string("CommonModeSlope_Event_") + hist_str.str()).c_str(), ";CM Slope;N APVs", slopeBins_, -1*slopeMax_,   slopeMax_);

        hist_str.str(std::string());
        hist_str << partitions_[ip] << "_pL" << il+1;
        ss << "making plot: " << partitions_[ip] << "/CommonModeSlope_Event_" << hist_str.str() << std::endl;
        evdir.make<TH1D>((std::string("CommonModeSlope_Event_") + hist_str.str()).c_str(), ";CM Slope;N APVs", slopeBins_, -1*slopeMax_,   slopeMax_);
        } // end if TID
    else if (partitions_[ip] == "TEC") {
        hist_str.str(std::string());
        hist_str << partitions_[ip] << "_mL" << il+1;
        ss << "making plot: " << partitions_[ip] << "/CommonModeSlope_Event_" << hist_str.str() << std::endl;
        evdir.make<TH1D>((std::string("CommonModeSlope_Event_") + hist_str.str()).c_str(), ";CM Slope;N APVs", slopeBins_, -1*slopeMax_,   slopeMax_);

        hist_str.str(std::string());
        hist_str << partitions_[ip] << "_pL" << il+1;
        ss << "making plot: " << partitions_[ip] << "/CommonModeSlope_Event_" << hist_str.str() << std::endl;
        evdir.make<TH1D>((std::string("CommonModeSlope_Event_") + hist_str.str()).c_str(), ";CM Slope;N APVs", slopeBins_, -1*slopeMax_,   slopeMax_);
        } // end if TEC
    } // end layers_ loop
    } // end nParts loop
    //edm::LogDebug(mlDqmSource_);
    } // end if allParts_

    //if there are no detIds

	edm::Handle<edm::DetSetVector<SiStripRawDigi> > digisHandle;
    iEvent.getByToken( inputPostPedestalRawDigiToken_, digisHandle );	

    if (detIDs_.empty()) {
        for(edm::DetSetVector<SiStripRawDigi>::const_iterator DSViter=digisHandle->begin(); DSViter!=digisHandle->end();DSViter++ ){
            //std::cout<<"detID : "<<DSViter->detId()<<" "<<std::endl;
            detIDs_.push_back(DSViter->detId());            
        }
    }

    // Loop over detIDs as obtained from the SpyChannelMonitor config file.
    for (std::vector<uint32_t>::iterator d = detIDs_.begin(); d!=detIDs_.end(); ++d) {
        // TODO: Need some error checking here, probably...
        //const std::vector<const FedChannelConnection *> & conns = lCabling->getConnections( *d );
        //cout << "________________________________________________" << endl;
        //cout << "FED channels found in detId " << *d << " is " << conns.size() << endl;
        //if (!(conns.size())) {
        //    // TODO: Properly DEBUG/warning this...
        //    //cout << "Skipping detID " << uint32_t(*d) << endl;
        //    continue;
        //}

        // Create a histogram directory for every detID
        // If this detID is not in the list of ones we care about, no hists will be filled
        // In that case, cmssw automatically deletes the empty folders
        stringstream sss;  //!< detID folder filename
        sss << "detID_" << *d;
        TFileDirectory detID_dir = evdir.mkdir( sss.str() );

        

        // Post-Pedestal Raw (PP)
        //========================
        if (!((inputPostPedestalRawDigiLabel_.label()=="") && (inputPostPedestalRawDigiLabel_.instance()==""))) {
            edm::Handle< edm::DetSetVector< SiStripRawDigi > > pp_rawdigis;
            edm::Handle< edm::DetSetVector< SiStripDigi > > zs_digis;
	        //iEvent.getByLabel( inputPostPedestalRawDigiLabel_, pp_rawdigis );
            iEvent.getByToken( inputPostPedestalRawDigiToken_, pp_rawdigis );
            iEvent.getByToken( inputZeroSuppressedDigiToken_, zs_digis );
            if (DEBUG) cout << endl << endl << endl << "Making post-pedestal module histogram for detID " << *d << endl;
            //if (!(MakeRawDigiHist_(pp_rawdigis, *d, detID_dir, POST_PEDESTAL))) { ; }
            if (!(MakeRawDigiHist_(pp_rawdigis, zs_digis, *d, evdir, POST_PEDESTAL, tTopo))) { ; }
        }
        
    } // end of loop over detIDs specified in the config.
    
    //adding event hist to run hist
    RUN_SLOPE_HIST->Add(EV_SLOPE_HIST);
    RUN_AVG_HIST->Add(EV_AVG_HIST);
    RUN_STDDEV_HIST->Add(EV_STDDEV_HIST);
    RUN_UNCERT_HIST->Add(EV_UNCERT_HIST);
    RUN_REL_UNCERT_HIST->Add(EV_REL_UNCERT_HIST);

    //delete tTopo;

} // end of Analyze method.

// TODO make sure the histograms are deleted before function goes out of scopre
// make sure that the clusters array is deleted after used in the function his makes the hists

/*
int * SiStripSpyCMslope::FindClusters_(
    const edm::Handle< edm::DetSetVector< SiStripRawDigi > > & pd_handle,
    const edm::Handle< edm::DetSetVector< SiStripProcessedRawDigi > > & pc_handle,
    uint32_t specifier)
{
    static int clusters[768];
    TH1S pcm_h = new TH1S("PostCommonMode", ";Strip Number;ADC counts /  strip", 768, 0, 768);
    TH1S noise_h = new TH1S("Noise", ";Strip Number;ADC counts / strip" 768, 0, 768);
    
    ///////////////////////////
    // fill noise histogram  //
    ///////////////////////////

    // first make sure that the noise vector is not empty
    std::vector< edm::DetSet<SiStripProcessedRawDigi> >::const_iterator pd_it = pd_handle->find( specifier );
    if ( pd_it == pd_handle->end() ) {
        delete pcm_h;
        delete noise_h;
        return clusters;
    }
    // fill the noise values histogram
    edm::DetSet<SiStripProcessedRawDigi>::const_iterator idigis_noise = pd_it->data.being();
    uint32_t count = 0;
    for ( ; idigis_noise != pd_it->data.end() ; idigis_noise++ ) {
        count++;
        noise_h->SetBinContent(count, static_cast<float>((*idigi_noise).adc()));
    }

    //////////////////////////////////////
    // fill CM subtracted ADC histogram //
    //////////////////////////////////////

    // first make sure that the post-cm subtracted vector is not empty
    std::vector< edm::DetSet<SiStripProcessedRawDigi> >::const_iterator pc_it = pc_handle->find(specifier);
    if (pc_it == pc_handle->end()) {
        delete pcm_h;
        delete noise_h;
        return clusters;
    }
    // fill the post-cm subtracted adc counts into the histogram
    count = 0;
    for ( ; idigis_adc != pc_it->data.end() idigis_adc++ )
    {
        count++;
        pcm_h->SetBinContent(count, static_cast<float>((*idigis_adc).adc()));
    }

    /////////////////////////////////////////////////////////////////
    // use post-cm subtracted adc count and noise to find clusters //
    /////////////////////////////////////////////////////////////////


    delete pcm_h;
    delete noise_h;
    return clusters;

}
*/

Bool_t SiStripSpyCMslope::MakeRawDigiHist_(
    const edm::Handle< edm::DetSetVector< SiStripRawDigi > > & digi_handle,
    const edm::Handle< edm::DetSetVector< SiStripDigi > > & zs_handle,
    uint32_t specifier,
    const TFileDirectory & dir,
    FEDSpyHistogramType type,
    const edm::ESHandle<TrackerTopology> tTopo)
    //TH1D *SLOPE_HIST)
    //const std::string & name)
{
    // TODO: Remove the hard-coded numbers(!).
    TH1S * hist;
    TH1S * zs_hist;
    if (type==POST_PEDESTAL) {
        hist = new TH1S("PostPedestal", ";Strip number;ADC counts / strip",  nStrips_, 0, nStrips_);
        zs_hist = new TH1S("ZeroSuppressed", ";Strip number;ADC counts / strip",  nStrips_, 0, nStrips_);
    }
    else {
        hist = nullptr;
        zs_hist = nullptr;
        return false;
    }

    // TODO: May need to make this error checking independent when refactoring...
    //std::cout << "| * digis for " << type << " and detID " << specifier;
    std::vector< edm::DetSet<SiStripRawDigi> >::const_iterator digis_it = digi_handle->find( specifier );
    if (digis_it == digi_handle->end()) { 
        //std::cout << " not found :( ";
        delete hist;
        delete zs_hist;
        return false;
    }
    std::vector< edm::DetSet<SiStripDigi> >::const_iterator zs_it = zs_handle->find( specifier );
    //first check to see if there are any hits
    bool no_strips = false;
    if (zs_it == zs_handle->end()) {
        no_strips = true;
    }

    // checks whether a given strip contains a cluster
    std::array<int, nStrips_> clusters = {{0}};
    edm::DetSet<SiStripDigi>::const_iterator izs = zs_it->data.begin();
    unsigned int strip = 0;
    // if you don't find any hits, then don't fill the vector
    //if (!(no_strips) && (filterClusters_))
    if (!(no_strips))
    {
    std::stringstream ss;
    ss << "[SiStripSpyCMslope::" << __func__ << "]" << std::endl;
    for ( ; izs != zs_it->data.end(); ++izs )
    {
        strip = static_cast<unsigned int>((*izs).strip());
        if (strip >= nStrips_)
        {
            edm::LogWarning(mlDqmSource_) << "[SiStripSpyCMslope::" << __func__ << "]" << "strip number " << strip << " is greater than "<< nStrips_ <<" but this should not happen" << std::endl;
            break;
        }
        clusters[strip] = static_cast<int>((*izs).adc());
        zs_hist->SetBinContent(static_cast<int>((*izs).strip()),static_cast<int>((*izs).adc()));
        ss << "FOUND CLUSTER: strip " << strip << ", ZS adc count is: " << (*izs).adc() << std::endl;
    }
    //ss << std::endl;
    LogDebug(mlDqmSource_);
    } // end if no_strips
    
    //std::cout << std::endl;

    // Loop over the digis for the detID and APV pair.
    edm::DetSet<SiStripRawDigi>::const_iterator idigi = digis_it->data.begin();
    uint32_t count = 0;
    int digi_adc = 0;
    // Fill array with number of strips with non-empty ADC counts
    //   Needed in order to make sure script is not fitting an APV chip
    //   which failed to be readout - that would lead to seg fault during runtime
    std::array<int, nAPV_> filled = {{0, 0, 0, 0, 0, 0}};
    int iAPV = 0;
    for ( ; idigi != digis_it->data.end(); ++idigi ) {
        count++;
        if ( count % nStripsAPV_ == 0 ) iAPV++;
        digi_adc = static_cast<int>((*idigi).adc());
        //if ((digi_adc > 0) && (digi_adc < adcCutoff_)) {
        // performs cluster filtering (if turned on)
        if ((!(filterClusters_) && (digi_adc > 0)) || ((filterClusters_) && !(clusters[count-1] > 0) && (digi_adc > 0)))
        {
            hist->SetBinContent(count, digi_adc);
            filled[iAPV]++;
            //if (DEBUG) std::cout << "(" << count << ", " << digi_adc << "), ";
        }
        else 
        {
            //if (DEBUG) std::cout << "(" << count << ", " << 0 << "), ";
            //if (DEBUG) std::cout << count << ", " << clusters[count-1] << std::endl;
            if (DEBUG)std::cout << "Found cluster (" << count << ", " << digi_adc << ") SKIPPING STRIP" << std::endl;
        }
        // TODO Make it so you do not include strips wiht ADC count above or below a threshold
        //if (DEBUG) {cout << static_cast<int>((*idigi).adc()) << ", ";}
    } // end of loop over the digis
    //if (DEBUG) {cout << endl;}

    uint32_t npairs = count / nStripsAPV_;

    if (type==POST_PEDESTAL) {

    // Define hists that will be filled

    TH1D * SLOPE_HIST = nullptr;
    TH1D * AVG_HIST = nullptr;
    TH1D * STDDEV_HIST = nullptr;
    TH1D * UNCERT_HIST = nullptr;
    TH1D * REL_UNCERT_HIST = nullptr;

    TH1D * EV_PARTITION_SLOPE = nullptr;
    TH1D * EV_PARTITION_AVG = nullptr;
    TH1D * EV_PARTITION_STDDEV = nullptr;
    TH1D * EV_LAYER_HIST = nullptr;

	TH1D * PARTITION_HIST = nullptr;
    TH1D * PARTITION_UNCERT = nullptr;
    TH1D * PARTITION_REL_UNCERT = nullptr;
	TH1D * PARTITION_AVG = nullptr;
	TH1D * PARTITION_STDDEV = nullptr;
    TH1D * LAYER_HIST = nullptr;

    TH1I * PARTITION_ONE_LARGE_STDDEV = nullptr;
    TH1I * PARTITION_TWO_LARGE_STDDEV = nullptr;
    TH1I * PARTITION_THREE_LARGE_STDDEV = nullptr;
    TH1I * PARTITION_FOUR_LARGE_STDDEV = nullptr;
    TH1I * PARTITION_FIVE_LARGE_STDDEV = nullptr;
    TH1I * PARTITION_SIX_LARGE_STDDEV = nullptr;

    TH1D * ID_HIST = nullptr;
    TH1D * ID_APV_HIST = nullptr;
    TH1D * ID_UNCERT_HIST = nullptr;
    TH1D * ID_REL_UNCERT_HIST = nullptr;
    TH1D * ID_MAX_ADC_HIST = nullptr;
    //TH1S * ID_PP_HIST = nullptr;
    //TH1S * ID_ZS_HIST = nullptr;


	DetId detId = DetId(specifier);
	uint32_t subId = detId.subdetId();
    const uint32_t layer = tTopo->layer(detId);
	const uint32_t side = tTopo->side(detId);

	LogDebug(mlDqmSource_) << "subdetID is: " << subId << "\t layer is " << layer << "\tside is " << side<< endl;

    // Retrieve histograms defined in Event loop or begin job

    //dir.getBareDirectory("..")->GetObject("CommonModeSlope",SLOPE_HIST);
    //dir.getBareDirectory("..")->GetObject("CommonModeAverage",AVG_HIST);
    //dir.getBareDirectory("..")->GetObject("CommonModeStddev",STDDEV_HIST);
    dir.getBareDirectory()->GetObject("CommonModeSlope",SLOPE_HIST);
    dir.getBareDirectory()->GetObject("CommonModeAverage",AVG_HIST);
    dir.getBareDirectory()->GetObject("CommonModeStddev",STDDEV_HIST);
    dir.getBareDirectory()->GetObject("CommonModeSlopeUncert",UNCERT_HIST);
    dir.getBareDirectory()->GetObject("CommonModeSlopeRelUncert",REL_UNCERT_HIST);

	//Check to see which partition of the tracker the detID belongs to
    std::string partition = "";
    if (subId == StripSubdetector::TIB) partition = "TIB";
    else if (subId == StripSubdetector::TOB) partition = "TOB";
    else if (subId == StripSubdetector::TID) partition = "TID";
    else if (subId == StripSubdetector::TEC) partition = "TEC";
    std::string side_str = "";
    if (side == 1) side_str = "m";
    else if (side == 2) side_str = "p";

    // Retrieve histogram for correct detector partition and sub partitions

    dir.getBareDirectory()->GetObject((std::string("CommonModeSlope_Event_") + partition).c_str(), EV_PARTITION_SLOPE);
    dir.getBareDirectory()->GetObject((std::string("CommonModeAverage_Event_") + partition).c_str(), EV_PARTITION_AVG);
    dir.getBareDirectory()->GetObject((std::string("CommonModeStddev_Event_") + partition).c_str(), EV_PARTITION_STDDEV);

    dir.getBareDirectory((std::string("../")+partition).c_str())->GetObject((std::string("CommonModeSlope_")+partition).c_str(),PARTITION_HIST);
    dir.getBareDirectory((std::string("../")+partition).c_str())->GetObject((std::string("CommonModeSlopeUncert_")+partition).c_str(),PARTITION_UNCERT);
    dir.getBareDirectory((std::string("../")+partition).c_str())->GetObject((std::string("CommonModeSlopeRelUncert_")+partition).c_str(),PARTITION_REL_UNCERT);
    dir.getBareDirectory((std::string("../")+partition).c_str())->GetObject((std::string("CommonModeAverage_")+partition).c_str(),PARTITION_AVG);
    dir.getBareDirectory((std::string("../")+partition).c_str())->GetObject((std::string("CommonModeStddev_")+partition).c_str(),PARTITION_STDDEV);

    dir.getBareDirectory((std::string("../")+partition).c_str())->GetObject((std::string("One_LargeStddevDetId_")+partition).c_str(),PARTITION_ONE_LARGE_STDDEV);
    dir.getBareDirectory((std::string("../")+partition).c_str())->GetObject((std::string("Two_LargeStddevDetId_")+partition).c_str(),PARTITION_TWO_LARGE_STDDEV);
    dir.getBareDirectory((std::string("../")+partition).c_str())->GetObject((std::string("Three_LargeStddevDetId_")+partition).c_str(),PARTITION_THREE_LARGE_STDDEV);
    dir.getBareDirectory((std::string("../")+partition).c_str())->GetObject((std::string("Four_LargeStddevDetId_")+partition).c_str(),PARTITION_FOUR_LARGE_STDDEV);
    dir.getBareDirectory((std::string("../")+partition).c_str())->GetObject((std::string("Five_LargeStddevDetId_")+partition).c_str(),PARTITION_FIVE_LARGE_STDDEV);
    dir.getBareDirectory((std::string("../")+partition).c_str())->GetObject((std::string("Six_LargeStddevDetId_")+partition).c_str(),PARTITION_SIX_LARGE_STDDEV);

    // See if current detId matches one of the ones that we want to check
    bool is_match = false;
    for (std::vector<uint32_t>::iterator Id=checkIDs_.begin(); Id!=checkIDs_.end(); Id++) {
        if (specifier == *Id) {
            is_match = true;
            LogTrace(mlDqmSource_) << "Det Id Match!" << std::endl;
            break;
        }
    }
    // If we find a match, retrive special histograms for monitoring that detId
    // If not, delete the uninitialized histograms defined up above
    std::stringstream id_str;
    if (is_match) {
        id_str << specifier;
        //if (DEBUG) std::cout << "Getting hist detId_" << id_str.str() << std::endl;
        // Produce the post pedestal hist so we can use it for extra monitoring
        //TH1S * det_pp = dir.getBareDirectory("../Det_IDs"+id_str.str())->make<TH1S>(id_str.str()+"_PostPedestal_event"+std::string(event),";Strip number;ADC counts / strip",  768, 0, 768);
        std::stringstream pp_ss; 
        std::stringstream zs_ss; 
        pp_ss << "DetId" << id_str.str() << "_PostPedestal_event" << event_;
        zs_ss << "DetId" << id_str.str() << "_ZeroSuppressed_event" << event_;
        TH1S * ID_PP_HIST = dir.make<TH1S>(pp_ss.str().c_str(),";Strip number;ADC counts / strip",  nStrips_, 0, nStrips_);
        TH1S * ID_ZS_HIST = dir.make<TH1S>(zs_ss.str().c_str(),";Strip number;ADC counts / strip",  nStrips_, 0, nStrips_);
        for (unsigned int i=0; i<769; i++) {
            ID_PP_HIST->SetBinContent(i, hist->GetBinContent(i));
            ID_ZS_HIST->SetBinContent(i, zs_hist->GetBinContent(i));
        }
        //dir.make<TH1S>(pp_ss.str().c_str(),";Strip number;ADC counts / strip",  768, 0, 768);
        //dir.make<TH1S>(zs_ss.str().c_str(),";Strip number;ADC counts / strip",  768, 0, 768);
        //hist->Clone(pp_ss.str().c_str());
        //zs_hist->Clone(zs_ss.str().c_str());
        //hist->Copy(dir.getBareDirectory()->GetObject(pp_ss.str().c_str(), ID_PP_HIST));
        //zs_hist->Copy(dir.getBareDirectory()->GetObject(zs_ss.str().c_str(), ID_ZS_HIST));

        //dir.getBareDirectory()->GetObject(pp_ss.str().c_str(), ID_PP_HIST);
        //dir.getBareDirectory()->GetObject(zs_ss.str().c_str(), ID_ZS_HIST);
        //hist->Copy(ID_PP_HIST);
        //zs_hist->Copy(ID_ZS_HIST);

        dir.getBareDirectory("../Det_IDs")->GetObject(( "detId_"+id_str.str() ).c_str(), ID_HIST);
        dir.getBareDirectory("../Det_IDs")->GetObject(( "detId_"+id_str.str()+std::string("_Uncert") ).c_str(), ID_UNCERT_HIST);
        dir.getBareDirectory("../Det_IDs")->GetObject(( "detId_"+id_str.str()+std::string("_RelUncert") ).c_str(), ID_REL_UNCERT_HIST);
        dir.getBareDirectory("../Det_IDs")->GetObject(("detId_"+id_str.str()+std::string("_MaxADC")).c_str(), ID_MAX_ADC_HIST);
    }
    /*
    else {
        ID_HIST = new TH1D("id_hist", "", 1, 0, 1);
        ID_UNCERT_HIST = new TH1D("id_hist", "", 1, 0, 1);
        ID_REL_UNCERT_HIST = new TH1D("id_hist", "", 1, 0, 1);
        ID_APV_HIST = new TH1D("id_apv_hist", "", 1, 0, 1);
        ID_MAX_ADC_HIST = new TH1D("id_max_apv_hist", "", 1, 0, 1);
        ID_HIST->Delete();
        ID_UNCERT_HIST->Delete();
        ID_REL_UNCERT_HIST->Delete();
        ID_APV_HIST->Delete();
        ID_MAX_ADC_HIST->Delete();
    }
    */

    // Loop over APVs and fill appropriate histograms
    //uint32_t nstrips = 128;
    //uint32_t npairs = (hist->GetEntries())/nstrips+1;
    LogDebug(mlDqmSource_) << "Number of hist entries is " << hist->GetEntries() << std::endl;
	double stddev = 0;
	double avg = 0;
    double slope = 0;
    double uncert = 0;
    int high_stddev = 0;
    double max_adc = 0;
    TF1 *f1 = new TF1("f1", "pol1", 0, nStrips_);
	if (npairs != 0) {
	    for (uint32_t ii=0; ii < npairs; ii++) {
            LogDebug(mlDqmSource_) << filled[ii] << std::endl;
		    //checking to make sure APV isn't off
	        //if ( ( (hist->GetBinContent(nstrips*ii+1) == 0) || (hist->GetBinContent(nstrips*ii+2) == 0) || (hist->GetBinContent(nstrips*ii+63) == 0) || (hist->GetBinContent(nstrips*ii+64) == 0) ) ) {
            if ( filled[ii] <= minStrips_ ) {
                edm::LogWarning(mlDqmSource_) << "Something wrong, skipping APV" << std::endl;
                continue;
            }
            f1->Update();
            f1->SetRange(ii*nStripsAPV_+1, (ii+1)*nStripsAPV_);
		    //hist->Fit("f1", "Q", "R", ii*nStripsAPV_+1, (ii+1)*nStripsAPV_);
            //hist->Fit("pol1", "", "", ii*nStripsAPV_+1, (ii+1)*nStripsAPV_);
            // This seems to be the only consistent way to get the function to
            // fit only a range of strips
            hist->GetXaxis()->SetRangeUser(ii*nStripsAPV_+1, (ii+1)*nStripsAPV_);
            if (!(DEBUG)) hist->Fit("f1", "QN", "");
            else hist->Fit("f1", "N", "");
            //if (DEBUG) cout << "Fit status is " << f1->IsValid() << endl;
            //if (!( f1->IsValid() )) continue;
            slope = f1->GetParameter(1);
            uncert = f1->GetParError(1);
            max_adc = hist->GetBinContent(hist->GetMaximumBin());
		    LogDebug(mlDqmSource_) << "slope for strips " << nStripsAPV_*ii+1 << " - " << nStripsAPV_*(ii+1) << " is:  " << slope << " +/- " << uncert << std::endl;
	        SLOPE_HIST->Fill(slope);
            UNCERT_HIST->Fill(uncert);
            REL_UNCERT_HIST->Fill(uncert/abs(slope));
            if (is_match) {
                // Fill histograms to monitor specific detIds
                ID_HIST->Fill(slope);
                ID_UNCERT_HIST->Fill(uncert);
                ID_REL_UNCERT_HIST->Fill(uncert/abs(slope));
                ID_MAX_ADC_HIST->Fill(max_adc);
                std::stringstream apv_str;
                apv_str << ii+1;
                dir.getBareDirectory("../Det_IDs")->GetObject(( "detId_"+id_str.str()+std::string("_APV_")+apv_str.str() ).c_str(), ID_APV_HIST);
                ID_APV_HIST->Fill(slope);
            }
            avg = hist->Integral(ii*nStripsAPV_+1,(ii+1)*nStripsAPV_)/filled[ii];
            LogDebug(mlDqmSource_) << "Average ADC count is:\t" << avg << endl;
            AVG_HIST->Fill(avg);
            // stddev is calculated as:
            // sqrt( Sum( Avg**2 - Strip**2) ) / 128
		    for (uint32_t j=(ii*nStripsAPV_+1); j<=(ii+1)*nStripsAPV_;j++) {
    		    if (hist->GetBinContent(j) > 0) stddev += pow(avg - hist->GetBinContent(j),2);
    	    }
    	    //stddev = TMath::Sqrt(stddev)/nstrips;
    	    stddev = TMath::Sqrt(stddev/(filled[ii]-1));
    	    LogDebug(mlDqmSource_) << "Standard dev is " << stddev << endl;

            STDDEV_HIST->Fill(stddev);

    		PARTITION_HIST->Fill(slope);
            PARTITION_UNCERT->Fill(uncert);
            PARTITION_REL_UNCERT->Fill(uncert/abs(slope));
    		//PARTITION_AVG->Fill(hist->Integral(ii*nstrips,(ii+1)*nstrips)/nstrips);
    		PARTITION_AVG->Fill(avg);
    		PARTITION_STDDEV->Fill(stddev);

            EV_PARTITION_SLOPE->Fill(slope);
    		//EV_PARTITION_AVG->Fill(hist->Integral(ii*nstrips,(ii+1)*nstrips)/nstrips);
    		EV_PARTITION_AVG->Fill(avg);
    		EV_PARTITION_STDDEV->Fill(stddev);

            if (stddev >= stddevLimit_) {
                edm::LogWarning(mlDqmSource_) << "Stddev is greater than limit, saving detID" << std::endl;
                high_stddev++;
            }

            // Fill partition hist if asked to do so
            // Since I only care to initilize hist if it is also being filled, need to
            // initialize in same if statement to avoid compilation errors
            if (allParts_) {
                std::stringstream layer_str;
                layer_str << partition << "_" << side_str << "L" << layer;
                dir.getBareDirectory((std::string("../")+partition).c_str())->GetObject((std::string("CommonModeSlope_")+layer_str.str()).c_str(),LAYER_HIST);
                dir.getBareDirectory()->GetObject((std::string("CommonModeSlope_Event_")+layer_str.str()).c_str(),EV_LAYER_HIST);

                LAYER_HIST->Fill(slope);
                EV_LAYER_HIST->Fill(slope);
	        }
            else {
                LAYER_HIST = new TH1D("layer", "", 10, 0, 10);
                EV_LAYER_HIST = new TH1D("layer", "", 10, 0, 10);
                LAYER_HIST->Delete();
                EV_LAYER_HIST->Delete();
            }
            LogDebug(mlDqmSource_) << "Filled APV" << std::endl;
        } // end of APV pair loop
    } // end npairs != 0
    delete f1;
    if (high_stddev >= 1) PARTITION_ONE_LARGE_STDDEV->Fill((int)specifier);
    if (high_stddev >= 2) PARTITION_TWO_LARGE_STDDEV->Fill((int)specifier);
    if (high_stddev >= 3) PARTITION_THREE_LARGE_STDDEV->Fill((int)specifier);
    if (high_stddev >= 4) PARTITION_FOUR_LARGE_STDDEV->Fill((int)specifier);
    if (high_stddev >= 5) PARTITION_FIVE_LARGE_STDDEV->Fill((int)specifier);
    if (high_stddev >= 6) PARTITION_SIX_LARGE_STDDEV->Fill((int)specifier);
    } // end if POST_PEDASTAL
    delete hist;
    delete zs_hist;

    //}
    
    //end Bjorn stuff

    return true; // Success! (Probably.)
}

// Define this as a plug-in
DEFINE_FWK_MODULE(SiStripSpyCMslope);
