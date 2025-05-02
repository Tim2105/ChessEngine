#include "core/utils/magics/Magics.h"

#include "uci/UCI.h"

int main(int argc, char* argv[]) {
    Magics::initializeMagics();

    std::vector<std::string> args;
    args.reserve(argc - 1);
    for(int i = 1; i < argc; ++i)
        args.push_back(argv[i]);

    UCI::listen(args);

    return 0;
}