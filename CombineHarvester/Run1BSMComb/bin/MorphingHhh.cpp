#include <string>
#include <map>
#include <set>
#include <iostream>
#include <vector>
#include <utility>
#include <cstdlib>
#include "boost/filesystem.hpp"
#include "CombineHarvester/CombineTools/interface/CombineHarvester.h"
#include "CombineHarvester/CombineTools/interface/Utilities.h"
#include "CombineHarvester/CombineTools/interface/HttSystematics.h"
#include "CombineHarvester/CombinePdfs/interface/MorphFunctions.h"
#include "CombineHarvester/CombineTools/interface/TFileIO.h"

#include "RooWorkspace.h"
#include "RooRealVar.h"
#include "TH2.h"
#include "RooDataHist.h"
#include "RooHistFunc.h"
#include "RooFormulaVar.h"
#include "RooProduct.h"

using namespace std;

int main() {
	ch::CombineHarvester cb;

	// cb.SetVerbosity(1);

	typedef vector<pair<int, string>> Categories;
	typedef vector<string> VString;

	string auxiliaries  = string(getenv("CMSSW_BASE")) + "/src/auxiliaries/";
	string aux_shapes   = auxiliaries +"shapes/";
	string aux_pruning  = auxiliaries +"pruning/";

    // RooFit will be quite noisy if we don't set this
    RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);
    
    RooRealVar mA("mA", "mA", 260., 350.);
    RooRealVar mH("mH", "mH", 260., 350.);
    RooRealVar mh("mh", "mh", 260., 350.);
  
    VString chns =
	{"et", "mt","tt"};

	map<string, string> input_folders = {
		{"et", "Imperial"},
		{"mt", "Imperial"},
		{"tt", "Italians"}
	};

	map<string, VString> bkg_procs;
	bkg_procs["et"] = {"ZTT", "W", "QCD", "ZL", "ZJ", "TT", "VV"};
	bkg_procs["mt"] = {"ZTT", "W", "QCD", "ZL", "ZJ", "TT", "VV"};
	bkg_procs["tt"] = {"ZTT", "W", "QCD", "ZLL", "TT", "VV"};

	/*map<string,VString> sm_procs;
	  sm_procs["et"] = {"ggH_SM125","VH_SM125","qqH_SM125"};
	  sm_procs["mt"] = {"ggH_SM125","VH_SM125","qqH_SM125"};
	  sm_procs["tt"] = {"ggH_SM125","VH_SM125","qqH_SM125"};
	 */

	//VString sig_procs={"ggHTohhTo2Tau2B"};
    map<string, VString> signal_types = {
      {"ggHTohhTo2Tau2B", {"ggH_Hhhbbtautau"}}
    };

	map<string, Categories> cats;
	cats["et_8TeV"] = {
		{0, "eleTau_2jet0tag"}, {1, "eleTau_2jet1tag"},
		{2, "eleTau_2jet2tag"}};

	cats["mt_8TeV"] = {
		{0, "muTau_2jet0tag"}, {1, "muTau_2jet1tag"},
		{2, "muTau_2jet2tag"}};

	  cats["tt_8TeV"] = {
	    {0, "tauTau_2jet0tag"}, {1, "tauTau_2jet1tag"},
	    {2, "tauTau_2jet2tag"}};
	 


	vector<string> masses = ch::MassesFromRange("260-350:10");

	cout << ">> Creating processes and observations...\n";
	for (string era : {"8TeV"}) {
		for (auto chn : chns) {
			cb.AddObservations(
					{"*"}, {"htt"}, {era}, {chn}, cats[chn+"_"+era]);
			cb.AddProcesses(
					{"*"}, {"htt"}, {era}, {chn}, bkg_procs[chn], cats[chn+"_"+era], false);
			//    cb.AddProcesses(
			//    {"*"},{"htt"}, {era}, {chn}, sm_procs[chn], cats[chn+"_"+era],false);
			cb.AddProcesses(
					masses, {"htt"}, {era}, {chn}, signal_types["ggHTohhTo2Tau2B"], cats[chn+"_"+era], true);
		}
	}

    //Remove W background from 2jet1tag and 2jet2tag categories for tt channel
    cb.FilterProcs([](ch::Process const* p) {
      return (p->bin() == "tauTau_2jet1tag") && p->process() == "W";
    });
    cb.FilterProcs([](ch::Process const* p) {
      return (p->bin() == "tauTau_2jet2tag") && p->process() == "W";
    });
	
    cout << ">> Adding systematic uncertainties...\n";
	ch::AddSystematics_hhh_et_mt(cb);
    ch::AddSystematics_hhh_tt(cb);

	cout << ">> Extracting histograms from input root files...\n";
	for (string era : {"8TeV"}) {
		for (string chn : chns) {
			string file = aux_shapes + input_folders[chn] + "/htt_" + chn +
				".inputs-Hhh-" + era + ".root";
			cb.cp().channel({chn}).era({era}).backgrounds().ExtractShapes(
					file, "$BIN/$PROCESS", "$BIN/$PROCESS_$SYSTEMATIC");
			cb.cp().channel({chn}).era({era}).process(signal_types["ggHTohhTo2Tau2B"]).ExtractShapes(
					file, "$BIN/ggHTohhTo2Tau2B$MASS", "$BIN/ggHTohhTo2Tau2B$MASS_$SYSTEMATIC");
		}
	}


	cout << ">> Merging bin errors...\n";
	ch::CombineHarvester cb_et = move(cb.cp().channel({"et"}));
	for (string era : {"8TeV"}) {
		cb_et.cp().era({era}).bin_id({0, 1, 2}).process({"QCD","W","ZL","ZJ","VV","ZTT","TT"})
			.MergeBinErrors(0.1, 0.5);
	}
	ch::CombineHarvester cb_mt = move(cb.cp().channel({"mt"}));
	for (string era : {"8TeV"}) {
		cb_mt.cp().era({era}).bin_id({0, 1, 2}).process({"QCD","W","ZL","ZJ","VV","ZTT","TT"})
			.MergeBinErrors(0.1, 0.5);
	}
	ch::CombineHarvester cb_tt = move(cb.cp().channel({"tt"}));
	for (string era : {"8TeV"}) {
		cb_tt.cp().era({era}).bin_id({0, 1, 2}).process({"QCD","W","ZLL","VV","ZTT","TT"})
			.MergeBinErrors(0.1, 0.5);
	}

	cout << ">> Generating bbb uncertainties...\n";
	cb_mt.cp().bin_id({0, 1, 2}).process({"ZL","ZJ","W", "QCD","TT", "ZTT","VV"})
		.AddBinByBin(0.1, true, &cb);

	cb_et.cp().bin_id({0, 1, 2}).process({"ZL", "ZJ", "QCD", "W", "TT","ZTT","VV"})
		.AddBinByBin(0.1, true, &cb);

	cb_tt.cp().bin_id({0, 1, 2}).era({"8TeV"}).process({"ZL", "ZJ","W", "TT", "QCD", "ZTT","VV"})
	   .AddBinByBin(0.1, true, &cb);

	cout << ">> Setting standardised bin names...\n";
	ch::SetStandardBinNames(cb);


    RooWorkspace ws("htt", "htt");
 
    TFile demo("Hhh_demo.root", "RECREATE");

    bool do_morphing = true;
    map<string, RooAbsReal *> mass_var = {
      {"ggH_Hhhbbtautau", &mH}
    };
    if (do_morphing) {
      auto bins = cb.bin_set();
      for (auto b : bins) {
        auto procs = cb.cp().bin({b}).signals().process_set();
        for (auto p : procs) {
          ch::BuildRooMorphing(ws, cb, b, p, *(mass_var[p]),
                               "norm", true, true, true, &demo);
        }
      }
    }
    demo.Close();
    cb.AddWorkspace(ws);
    cb.cp().signals().ExtractPdfs(cb, "htt", "$BIN_$PROCESS_morph");
    cb.PrintAll();
    

	string folder = "output/hhh_cards_nomodel";
	boost::filesystem::create_directories(folder);
  
    TFile output((folder + "/htt_input.Hhh.root").c_str(), "RECREATE");
    cb.cp().mass({"*"}).WriteDatacard(folder + "/htt_cmb_Hhh.txt", output);
    auto bins = cb.bin_set();
    for (auto b : bins) {
      cb.cp().bin({b}).mass({"*"}).WriteDatacard(
      folder + "/" + b + ".txt", output);
    }
    output.Close();

	cout << "\n>> Done!\n";
}
