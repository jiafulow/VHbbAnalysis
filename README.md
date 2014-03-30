VHbbAnalysis
============

This is a CMSSW package for VHbbAnalysis


Setup
-----
### CMSSW_5_3_11
Use [additionalFiles/checkout.txt](https://github.com/jiafulow/VHbbAnalysis/blob/EDMV42_Step2_V8_clean/additionalFiles/checkout.txt)

### CMSSW_5_3_14_patch2
**Experimental**

Switch to `CMSSW_5_3_14_patch2_dev` branch, and use [additionalFiles/addpkg_5_3_14_patch2.txt](https://github.com/jiafulow/VHbbAnalysis/blob/CMSSW_5_3_14_patch2_dev/additionalFiles/addpkg_5_3_14_patch2.txt)


### CMSSW_7_0_0
**Experimental**

Switch to `CMSSW_7_0_0_dev` branch, and use [additionalFiles/addpkg_7_0_0.txt](https://github.com/jiafulow/VHbbAnalysis/blob/CMSSW_7_0_0_dev/additionalFiles/addpkg_7_0_0.txt)


Make Step1
----------
Step1 files are PAT-tuples without any skimming. To make them,

```
cd $CMSSW_BASE/src/VHbbAnalysis/HbbAnalyzer/test/
cmsRun patMC.py
# For data, use 'patData.py'
#cmsRun patData.py
```

Double check `VHbbAnalysis/HbbAnalyzer/additionalFiles/addpkg_X_Y_Z.txt` to get the correct config file.


Make Step2
----------
Step2 files are flat ROOT ntuples after pre-selection. To make them,

```
cd $CMSSW_BASE/src/VHbbAnalysis/VHbbDataFormats/bin/
Ntupler ntuple_ZnnHbb.py
```

Remember to configure the python config before use.


TODO
----
Step1 & Step2 productions are not yet validated. In particular, Step2 production will probably need a major rewrite. In addition,

- [ ] Reduce Step1 file size
- [ ] Remove unnecesary files in VHbbAnalysis/VHbbDataFormats/bin/
- [ ] Correct FastJet2 to FastJet3 migration

