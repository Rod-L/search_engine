#include <iostream>
#include <ctime>

#include <QApplication>

#include "gui.h"
#include "GUIWindow.h"

GUIWindow* MAIN_WINDOW;
Ui_MainWindow* MAIN_UI;
RemoteEngine* REMOTE;

int main(int argc, char *argv[]) {
    std::string engine_path;
    if (argc < 2) {
        std::cout << "Search engine executable path have not been passed as command line parameter." << std::endl;
        std::cout << "Selecting default: search_engine" << std::endl;
        engine_path = "search_engine";
    } else {
        engine_path = argv[1];
    }

    std::srand(std::time(nullptr));

    QApplication app(argc, argv);
    MAIN_UI = new Ui_MainWindow;
    REMOTE = new RemoteEngine(RemoteEngine::Process, engine_path);
    MAIN_WINDOW = new GUIWindow(MAIN_UI, REMOTE, nullptr);

    MAIN_WINDOW->load_settings("gui.settings.json");
    MAIN_WINDOW->show();

    auto exec_result = app.exec();
    MAIN_WINDOW->save_settings("gui.settings.json");

    if (REMOTE != nullptr) delete REMOTE;
    if (MAIN_UI != nullptr) delete MAIN_UI;
    return exec_result;
}