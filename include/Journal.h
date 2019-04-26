//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_JOURNAL_H
#define SAMPP_JOURNAL_H

#include <vector>
#include <fstream>

#include "SelectionStrategies.h"
#include "SubmissionRecord.h"

class Journal{
public:

    std::vector<Submission> publicationList;

    SelectionStrategy* selectionStrategy;
    
    Journal(json& config);

    // Journal(double max_pubs) :
    //         max_pubs(max_pubs) {
    //     still_accepting = true;
    // };

    void setSelectionStrategy(SelectionStrategy* s);

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

    void clear();

    bool isStillAccepting(){
        return still_accepting;
    }

    void saveSubmissions(int simid, std::ofstream& writer);
    
    void testMeta();

private:
    double max_pubs;

    bool still_accepting = true;

};

#endif //SAMPP_JOURNAL_H
