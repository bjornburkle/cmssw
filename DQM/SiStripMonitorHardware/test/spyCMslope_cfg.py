## Configuration for testing the FED processing monitor module
##=============================================================

import FWCore.ParameterSet.Config as cms
import sys, os
from Configuration.AlCa.GlobalTag import GlobalTag
import FWCore.ParameterSet.VarParsing as VarParsing

options = VarParsing.VarParsing('analysis')
options.register('start',
    default=-1,
    mult=VarParsing.VarParsing.multiplicity.singleton,
    mytype=VarParsing.VarParsing.varType.int,
    info='First event to process')
options.register('end',
    default=-1,
    mult=VarParsing.VarParsing.multiplicity.singleton,
    mytype=VarParsing.VarParsing.varType.int,
    info='Last event to process')
options.register('run',
    default=0,
    mult=VarParsing.VarParsing.multiplicity.singleton,
    mytype=VarParsing.VarParsing.varType.int,
    info='run number of event being processed')
options.register('clusterFilter',
    default=1,
    mult=VarParsing.VarParsing.multiplicity.singleton,
    mytype=VarParsing.VarParsing.varType.int,
    info='Use ZS signal to subtract strips with clusters from common mode')
options.register('TEST',
    default=0,
    mult=VarParsing.VarParsing.multiplicity.singleton,
    mytype=VarParsing.VarParsing.varType.int,
    info='Run in test mode. Makes nEvts=5 and output=test.root')
options.parseArguments()

#process = cms.Process('SPYCMSLOPE')
process = cms.Process('SiStripSpyCMslope')

run_number = options.run
start = options.start
end = options.end
#run_number = 321054
#start = 0
#end = 5

filter_str = ''
if options.clusterFilter:
    filter_str='_cluster_filtered'

str_dict = str_dict = {'0':run_number,'1':start,'2':end, '3':filter_str}
range_string = '%(0)d:%(1)d-%(0)d:%(2)d' % str_dict
input_file = 'file:/home/bjornb/SpyData/FED/%(0)d/FED%(0)d_%(1)d-%(2)d.root' % str_dict

maxEvts = -1
if options.TEST:
    maxEvts = 5


process.source = cms.Source(
    "PoolSource",
    fileNames = cms.untracked.vstring(
    input_file,
	),
	#eventsToProcess = cms.untracked.VEventRange(range_string),
    )

process.options = cms.untracked.PSet(
    SkipEvent = cms.untracked.vstring('SiStripSpyCMslope'),
    )
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(maxEvts))

## ---- Services ----
#process.load("DQM.SiStripCommon.MessageLogger_cfi")

## ---- Conditions ----

## Global tag see SWGuideFrontierConditions
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
#process.GlobalTag = GlobalTag(process.GlobalTag, '101X_dataRun2_HLT_v7', '')
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_data', '')
#process.load("Configuration.Geometry.GeometryRecoDB_cff")

## ---- Retrieve the ZS digis from the mainline ----
## ---- if running on matched events     ----
process.load('EventFilter.SiStripRawToDigi.SiStripDigis_cfi')
#process.siStripDigis.ProductLabel = cms.InputTag('source')
process.siStripDigis.UnpackCommonModeValues = cms.bool(True)


## ---- SpyChannel Monitoring ----
## For my plugin for the spy channel monitoring
process.load('DQM.SiStripMonitorHardware.SiStripSpyCMslope_cfi')

if options.clusterFilter:
    process.SiStripSpyCMslope.filterClusters = cms.bool(True)
else:
    process.SiStripSpyCMslope.filterClusters = cms.bool(False)

# ---- ESProducers for Tracker Geometry ----
process.load("Geometry.CMSCommonData.cmsIdealGeometryXML_cfi")
process.load("Geometry.TrackerNumberingBuilder.trackerTopology_cfi")
process.load("Geometry.TrackerGeometryBuilder.trackerParameters_cfi")
process.load("Geometry.TrackerGeometryBuilder.trackerGeometry_cfi")
process.load("Geometry.TrackerNumberingBuilder.trackerNumberingGeometry_cfi")
process.load("Geometry.CMSCommonData.cmsExtendedGeometry2017XML_cfi")
process.trackerTopology = cms.ESProducer("TrackerTopologyEP")

## Select the detIDs of choice here
process.SiStripSpyCMslope.detIDs = cms.vuint32(
    )
process.SiStripSpyCMslope.InputPostPedestalRawDigiLabel     = cms.InputTag("SiStripFEDEmulator","PedSubtrModuleDigis")
process.SiStripSpyCMslope.InputZeroSuppressedDigiLabel      = cms.InputTag("SiStripFEDEmulator","ZSModuleDigis")

##mainline data - if running on matched events
process.SiStripSpyCMslope.OutputFolderName = cms.string("CMslope")

## ---- Sequence ----
process.p = cms.Path(
    process.SiStripSpyCMslope
    )

if not os.path.isdir('/home/bjornb/SpyData/SpyOut/%d%s' % (run_number, filter_str)):
    os.makedirs('/home/bjornb/SpyData/SpyOut/%d%s' % (run_number, filter_str))

outString = 'file:/home/bjornb/SpyData/SpyOut/%(0)d%(3)s/CMSlope_%(0)d_%(1)d-%(2)d.root' % str_dict 
#outString = 'file:/home/bjornb/SpyData/SpyOut/%(0)d_APV_Pair/CMSlope_%(0)d_%(1)d-%(2)d.root' % str_dict 
if options.TEST:
    outString = 'file:test.root'
    process.SiStripSpyCMslope.DEBUG = cms.bool(True)

## ------ TFileService
process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string( outString ),
    #fileName = cms.string('file:test.root'),
    )

