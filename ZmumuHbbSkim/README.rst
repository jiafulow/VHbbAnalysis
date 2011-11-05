=========================================
ZmumuHbbSkim for MET = 0 constraint study
=========================================
:Author: Jia Fu Low <jlow@phys.ufl.edu>
:Date: $Date: 2011-11-04 $
:Revision: $Revision: 1 $
:Description: The ZmumuHbbSkim package is based on the ``CMS AN-2011/240`` VHbb Analysis. In this channel, MET should be zero if the energies of the final state particles (2 muons & 2 b-tagged jets) are well-measured. So the constraint MET = 0 can be used to fit the b-jet energies. The following are instructions of how to use this preliminary package to produced ntuples, and then make histograms from those ntuples.


Analysis Event Selection
========================
:Applied:

- |pT| (|b1|) > 20 GeV
- |pT| (|b2|) > 20 GeV
- |eta| (|b1|) < 2.5
- |eta| (|b2|) < 2.5
- |pT| (|mu1|) > 20 GeV
- |pT| (|mu2|) > 20 GeV
- |eta| (|mu1|) < 2.4
- |eta| (|mu2|) < 2.4
- max(CSV(|b1|), CSV(|b2|)) > 0.898
- min(CSV(|b1|), CSV(|b2|)) > 0.5
- muon pair has opposite charges, electron veto

  (along with standard muon id and jet energy correction)

:Not applied:

- |pT| (dijet) > 100 GeV
- |pT| (dimuon) > 100 GeV
- |dphi| (dijet, dimuon) > 2.9
- |mass| (dijet) < 15 GeV
- primary vertex selection

:Also applied but not used in the AN:

- exactly 2 b-tagged jets
- exactly 2 muons
- skip events where a b-tagged jet has no GenJet match


Recipe
======

Current Ntuples
---------------
:CMSSW version: ``CMSSW_4_2_8_patch3``
:MC Dataset: ``/ZH_ZToLL_HToBB_M-115_7TeV-powheg-herwigpp/Summer11-PU_S4_START42_V11-v1/AODSIM``
:Location: 
  ``(on CMSLPC) /pnfs/cms/WAX/11/store/user/jiafu/ZmumuHbb/zmumuHbbM115/``

  ``(on LXPLUS) /afs/cern.ch/user/j/jiafulow/public/ZmumuHbbEdmNtuples/``


Code checkout
-------------
::

  scram p CMSSW CMSSW_4_2_8_patch3
  cd CMSSW_4_2_8_patch3    /src
  cmsenv
  cvs co -d VHbbAnalysis UserCode/jiafulow/VHbbAnalysis
  cvs co -d Utilities/Parang UserCode/SAKoay/Utilities/Parang
  addpkg RecoJets/Configuration          V02-04-17
  addpkg RecoBTag/SecondaryVertex        V01-07-00
  addpkg RecoVertex/AdaptiveVertexFinder V02-01-00
  addpkg JetMETCorrections/Type1MET      V04-04-04
  scram b -c
  scram b -j 4

The current *official* VHbbAnalysis framework <https://twiki.cern.ch/twiki/bin/viewauth/CMS/VHbbAnalysisNewCode> is too complicated for a simple analysis. However, their framework is used as the reference, so this package should be able to be integrated into their framework in the future.

The Parang package <https://twiki.cern.ch/twiki/bin/view/Main/Parang> is a plot-making suite developed by Sue Ann Koay at UCSB. Its most useful feature is to book histograms with a single command.


How to create ntuples
---------------------

In directory ``VHbbAnalysis/ZmumuHbbSkim/test``, the cmsRun config file ``zmumuHbbSkim_cfg.py`` is used to create EDM ntuples from a MC dataset. The CRAB config file ``crab_zmumuHbbSkim_cfg.py`` is used to submit CRAB jobs. The created ntuples on both CMSLPC and LXPLUS are listed above.


How to make histograms
----------------------

In the same directory, the FWLite config file ``fwliteAnalysis_cfg.py`` is used to make histograms from the created ntuples. Running "``FWLiteBasicPlotter fwliteAnalyzer_cfg.py``" should output a ROOT file with histograms plotted using RECO quantities. Alternatively, ``fwliteAnalysisParton_cfg.py`` makes histograms using GEN quantities. ``FWLiteBasicPlotter`` is the FWLite program compiled from the ``bin`` subdirectory. (Run ``rehash`` first if the shell complains about "command not found".) The first set of histograms made by the current codes can be found in the ``histos_analysis.root`` file in::

  (on CMSLPC) /uscms/home/jiafu/shared/
  (on LXPLUS) /afs/cern.ch/user/j/jiafulow/public/

Inside each directory, the histograms printed as PNG files can be found in the subdirectory ``images``. ``histograms.html`` shows all the histograms on a single webpage.



TO DO
=====
- pileup treatment on jet energy and muon isolation
- primary vertex selection

.. Substitutions
   -------------
.. |pT|   replace:: p\ :sub:`T`\
.. |eta|  replace:: \|η|
.. |dphi| replace:: \|Δϕ|
.. |mass| replace:: \|M - M\ :sub:`H`\|
.. |b1|   replace:: b\ :sub:`1`\
.. |b2|   replace:: b\ :sub:`2`\
.. |mu1|  replace:: mu\ :sub:`1`\
.. |mu2|  replace:: mu\ :sub:`2`\

