import FWCore.ParameterSet.Config as cms

process = cms.Process("FWLitePlots")

process.fwliteInput = cms.PSet(
    fileNames   = cms.vstring("file:ZmumuHbbEdmNtuples.root"),  ## mandatory   
    maxEvents   = cms.int32(-1),                                ## optional
    outputEvery = cms.uint32(1000),                             ## optional
    )

process.fwliteOutput = cms.PSet(
    fileName  = cms.string('histos_analysis.root'),## mandatory
    )

zmumuHbbM115 = cms.vstring(
"dcache:/pnfs/cms/WAX/11/store/user/jiafu/ZmumuHbb/zmumuHbbM115/ZmumuHbbEdmNtuples_10_1_a5S.root",
"dcache:/pnfs/cms/WAX/11/store/user/jiafu/ZmumuHbb/zmumuHbbM115/ZmumuHbbEdmNtuples_11_1_0If.root",
"dcache:/pnfs/cms/WAX/11/store/user/jiafu/ZmumuHbb/zmumuHbbM115/ZmumuHbbEdmNtuples_1_1_GmW.root",
"dcache:/pnfs/cms/WAX/11/store/user/jiafu/ZmumuHbb/zmumuHbbM115/ZmumuHbbEdmNtuples_2_1_TLn.root",
"dcache:/pnfs/cms/WAX/11/store/user/jiafu/ZmumuHbb/zmumuHbbM115/ZmumuHbbEdmNtuples_3_1_zEx.root",
"dcache:/pnfs/cms/WAX/11/store/user/jiafu/ZmumuHbb/zmumuHbbM115/ZmumuHbbEdmNtuples_4_1_lxx.root",
"dcache:/pnfs/cms/WAX/11/store/user/jiafu/ZmumuHbb/zmumuHbbM115/ZmumuHbbEdmNtuples_5_1_3dY.root",
"dcache:/pnfs/cms/WAX/11/store/user/jiafu/ZmumuHbb/zmumuHbbM115/ZmumuHbbEdmNtuples_6_1_wJf.root",
"dcache:/pnfs/cms/WAX/11/store/user/jiafu/ZmumuHbb/zmumuHbbM115/ZmumuHbbEdmNtuples_7_1_oKl.root",
"dcache:/pnfs/cms/WAX/11/store/user/jiafu/ZmumuHbb/zmumuHbbM115/ZmumuHbbEdmNtuples_8_1_7K2.root",
"dcache:/pnfs/cms/WAX/11/store/user/jiafu/ZmumuHbb/zmumuHbbM115/ZmumuHbbEdmNtuples_9_1_7Wk.root",
)


process.Analyzer = cms.PSet(
    usePartonP4 = cms.untracked.bool(False),
    )

process.fwliteInput.fileNames = zmumuHbbM115
