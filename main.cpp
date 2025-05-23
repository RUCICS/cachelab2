#include "cachelab.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        throw std::runtime_error("Usage: ./main case0/case1/case2/case2");
    }
    if (std::string(argv[1]) == "case0") {
        case0();
    } else if (std::string(argv[1]) == "case1") {
        case1();
    } else if (std::string(argv[1]) == "case2") {
        case2();
    } else if (std::string(argv[1]) == "case3") {
        case3();
    } else if (std::string(argv[1]) == "case0_baseline") {
        case0_baseline();
    } else if (std::string(argv[1]) == "case1_baseline") {
        case1_baseline();
    } else if (std::string(argv[1]) == "case2_baseline") {
        case2_baseline();
    } else if (std::string(argv[1]) == "case3_baseline") {
        case3_baseline();
    } else {
        throw std::runtime_error("Usage: ./main case0/case1/case2/case2");
    }
}
