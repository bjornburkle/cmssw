import FWCore.ParameterSet.Config as cms

from Geometry.HGCalCommonData.hgcalEEParametersInitialize_cfi import *

from Configuration.ProcessModifiers.dd4hep_cff import dd4hep

dd4hep.toModify(hgcalEEParametersInitialize,
                fromDD4hep = cms.bool(True)
)

hgcalHESiParametersInitialize = hgcalEEParametersInitialize.clone(
    name  = cms.string("HGCalHESiliconSensitive"),
    nameW = cms.string("HGCalHEWafer"),
    nameC = cms.string("HGCalHECell"),
    nameX  = cms.string("HGCalHESiliconSensitive"),
)

hgcalHEScParametersInitialize = hgcalEEParametersInitialize.clone(
    name  = cms.string("HGCalHEScintillatorSensitive"),
    nameW = cms.string("HGCalWafer"),
    nameC = cms.string("HGCalCell"),
    nameX = cms.string("HGCalHEScintillatorSensitive"),
)
