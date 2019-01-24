#include <iostream>
#include <vector>
#include "Utilities.h"
#include "docopt.h"
#include <nlohmann/json.hpp>

static const char USAGE[] =
R"(SAMpp

    Usage:
        SAMpp [--n-groups G] [--n-dvs D] [--master-seed S]

    Options:
        -h --help          Show this screen.
        --version          Show version.
        --master-seed=S    Set the master seed.
        --n-groups=G       Number of groups.
        --n-dvs=D          Number of dependent variables.
)";

// for convenience
using json = nlohmann::json;




int main(int argc, const char** argv){

    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
                                                               { argv + 1, argv + argc },
                                                               true,                // show help if requested
                                                               "SAMpp 0.1 (Beta)");        // version string

    for(auto const& arg : args) {
        std::cout << arg.first << ": " << arg.second << std::endl;
    }

    std::cout << "Hello" << std::endl;
    std::cout << "Here are some random numbers..." << std::endl;
//    std::vector<std::vector<double>> rng = norm_rng();
//    std::vector<std::vector<double>> rng = gen_norm_rng();

    std::vector<std::vector<double>> rng = mvnorm_rng();

    for (auto &row : rng){
//        for (auto &v : row){
//            std::cout << v << ',';
//        }
        std::cout << "mean: " << mean(row);
        std::cout << " - sd: " << sd(row);
        std::cout << std::endl;
    }

    std::cout << "cor: " << cor(rng, 0, 1);

    return 0;
}
