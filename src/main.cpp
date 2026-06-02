#include <iostream>
#include <string>

#include "../include/maincontroller.hpp"
#include "../include/ui.hpp"

static void printHelp() {
    UIManager::showHelp();
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string arg1(argv[1]);
        if (arg1 == "--help" || arg1 == "-h") {
            printHelp();
            return 0;
        }
    }

    std::string url = "http://speedtest.tele2.net/100MB.zip";
    int threadCount = 8;
    std::string outputName = "largefile.zip";

    if (argc >= 2) {
        url = argv[1];
    }

    if (argc >= 3) {
        threadCount = std::stoi(argv[2]);
    }

    if (argc >= 4) {
        outputName = argv[3];
    }

    UIManager::showWelcome();

    MainController controller(url, threadCount, outputName);
    if (!controller.initializeSystem()) {
        return 1;
    }

    if (!controller.startDownload()) {
        return 1;
    }

    return 0;
}
