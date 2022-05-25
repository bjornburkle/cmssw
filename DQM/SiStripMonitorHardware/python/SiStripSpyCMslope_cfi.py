import FWCore.ParameterSet.Config as cms

SiStripSpyCMslope = cms.EDAnalyzer(
    'SiStripSpyCMslope',
    # Vector detIDs to examine
    detIDs = cms.vuint32( ),
    ################################
    #  Det ID    #   Part  # Layer #
    ################################
    # 369120297  #   TIB   #   1   #
    # 369121406  #   TIB   #   1   #
    # 369125577  #   TIB   #   1   #
    # 369142137  #   TIB   #   2   #
    # 369154280  #   TIB   #   3   #
    # 369170856  #   TIB   #   4   #
    # 402672405  #   TID   #   1   #
    # 402672818  #   TID   #   2   #
    # 402673464  #   TID   #   3   #
    # 402674345  #   TID   #   1   #
    # 402674961  #   TID   #   2   #
    # 402675512  #   TID   #   3   #
    ################################
    #  These showed large noise    #
    ################################
    # 369153236  #   TIB   #   3   #
    # 436294296  #   TOB   #   5   #
    ################################
    checkIDs = cms.vuint32( 369120297, 369121406, 369125577, 369142137, 369154280, 369170856, 402672405, 402672818, 402673464, 402674345, 402674961, 402675512, 369153236, 436294296 ),
    #checkIDs = cms.vuint32( 369120277, 369120278, 369120282 ),
    #
    # Spy Channel (raw) digi sources (change in your config)
    #========================================================
    InputPostPedestalRawDigiLabel = cms.InputTag("", ""),
    InputZeroSuppressedDigiLabel = cms.InputTag("",""),
    DEBUG = cms.bool(False),
    AllParts = cms.bool(True),
    OutputFolderName = cms.string("DEFAULT_OUTPUTNAME"),
    slopeMax = cms.double(0.1),
    slopeBins = cms.int32(200),
    stddevMax = cms.double(50.),
    #stddevLimit = cms.double(1.5*11.3),
    stddevLimit = cms.double(25),
    avgMax = cms.double(250.),
    avgBins = cms.int32(250),
    filterClusters = cms.bool(False),
    adcCutoff = cms.int32(175),
    minStrips = cms.int32(5),
    )
