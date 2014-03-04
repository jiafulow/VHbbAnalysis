#ifndef TriggerReaderNew__H
#define TriggerReaderNew__H

#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"

#include <iostream>
#include <string>
#include <map>
#include <regex.h>


class TriggerReader {
  public:
    TriggerReader(bool passAllEvents=false, bool verb=false)
      : passAll(passAllEvents), verbose(verb), run(1), cacheRun(-1) {}

    void setEvent(fwlite::Event * e, const char *process, const std::vector < std::string > triggers) {
        ev = e;
        if (passAll)  return;
        
        triggerResults.getByLabel(*ev, "TriggerResults", "", process);
        run = ev->eventAuxiliary().id().run();

        if (run != cacheRun){  // enters a new run
            if (verbose)
                std::cout << "new run " << run << std::endl;
        
            const edm::TriggerNames & triggerNames = ev->triggerNames(*triggerResults);
            nameMap.clear();
            regex_t regex;
            for (unsigned int j = 0; j < triggers.size(); ++j) {
                nameMap[triggers[j]] = 999999;
                for (unsigned int i = 0; i < triggerNames.size(); ++i) {
                    int regc = regcomp(&regex, triggers[j].c_str(), 0);
                    if (regc) {
                        std::cerr << "ERROR: Could not compile regex: " << triggers[j] << std::endl;
                    }
                    std::string triggerName(triggerNames.triggerName(i));
                    bool found = (regexec(&regex, triggerName.c_str(), 0, NULL, 0) == 0);
                    if (found){
                        nameMap[triggers[j]] = i;
                        cacheNames[triggers[j]] = triggerName;  // stores the name for sanity check
                        if (verbose)
                            std::cout << "FOUND: " << triggerNames.triggerName(i) << " is bit  " << i << std::endl;
                    }
                }  // end loop over triggerNames
            }  // end loop over triggers
            cacheRun = run;
        }
    }

    bool accept(const std::string & triggerName) {
        if (passAll)  return true;
        
        if (nameMap.empty()) {
            std::cerr << "ERROR: No trigger is stored in nameMap!" << std::endl;
            return false;
        }
        
        std::map < std::string, unsigned int >::iterator nit = nameMap.find(triggerName);
        if (nit == nameMap.end()) {
            std::cerr << "ERROR: Trigger name is not stored in nameMap!" << std::endl;
            return false;
        }
        
        if (nit->second == 999999) {
            //std::cerr << "ERROR: Trigger name is not found in this run!" << std::endl;
            return false;
        }
        
        const edm::TriggerNames & triggerNames = ev->triggerNames(*triggerResults);
        if (nit->second > triggerResults->size() || cacheNames[triggerName] != triggerNames.triggerName(nit->second)){
            std::cerr << "ERROR: The stored trigger in nameMap doesn't match the trigger in this run!" << std::endl;
            std::cerr << triggerName << " " << cacheNames[triggerName] << " " << triggerNames.triggerName(nit->second) << std::endl;
        }
        return triggerResults->accept(nit->second);
    }

  private:
    bool passAll;
    bool verbose;
    fwlite::Event * ev;
    fwlite::Handle < edm::TriggerResults > triggerResults;
    unsigned int run;
    unsigned int cacheRun;
    std::map < std::string, std::string > cacheNames;
    
    std::map < std::string, unsigned int > nameMap;
};
#endif  // TriggerReaderNew__H

