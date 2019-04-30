//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_JOURNAL_H
#define SAMPP_JOURNAL_H

#include <map>
#include <vector>
#include <fstream>

#include "csv/writer.hpp"

#include "SelectionStrategy.h"
#include "Submission.h"
#include "MetaAnalysis.h"

namespace sam {

    class Journal{
        
        double max_pubs;
        
        bool still_accepting = true;

        //! Journal's Selection Model/Strategy
        SelectionStrategy* selection_strategy;
        
        //! List of all acceptec Submissions, i.e., publications
        std::vector<Submission> publications_list;
        
    public:

        Journal(json& config);

        /**
         Point Journal's selection strategy to the given strategy

         @param s The pointer to the given selection strategy
         */
        void setSelectionStrategy(SelectionStrategy *s) {
            selection_strategy = s;
        }

        /**
         * @brief      Review the Submission by calling
         * `SelectionStrategy::review()`.
         *
         * @param[in]  s     A reference to the Submission
         *
         * @return     A boolean indicating whether the Submission should
         * be accpeted or not.
         */
        bool review(Submission &s);

        /**
         * @brief      Accept the Submission by adding it to the
         * `publicationList`.
         *
         * @param[in]  s     A copy of the Submission
         */
        void accept(Submission s);

        /**
         * @brief      Rejecting the Submission!
         *
         * @param[in]  s     A reference to the Submission
         */
        void reject(Submission &s);

        bool isStillAccepting() const {
            return still_accepting;
        }

        /**
         Save enteries of publications_list to a CSV file.

         @param simid The index to be used for the given set.
         @param writer The output file.
         */
        void saveSubmissions(int simid, std::ofstream& writer);
        
        /**
         Clear the publications_list vector.
         */
        void clear() {
            publications_list.clear();
            still_accepting = true;
        }
        
        void testMeta();

    };

}

#endif //SAMPP_JOURNAL_H
