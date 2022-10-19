#include <iostream>
#include <ctime>

#include <QApplication>

#include "gui.h"
#include "GUIWindow.h"
#include "RemoteConfig.h"

GUIWindow* MAIN_WINDOW;
Ui_MainWindow* MAIN_UI;
RemoteEngine* REMOTE = nullptr;

int main(int argc, char *argv[]) {
    std::srand(std::time(nullptr));

    QApplication app(argc, argv);
    MAIN_UI = new Ui_MainWindow;
    REMOTE = new RemoteEngine(RemoteMode::Process);
    MAIN_WINDOW = new GUIWindow(MAIN_UI, REMOTE, nullptr);

    MAIN_WINDOW->load_settings("gui.settings.json");
    MAIN_WINDOW->show();

    auto exec_result = app.exec();
    MAIN_WINDOW->save_settings("gui.settings.json");

    if (REMOTE != nullptr) delete REMOTE;
    if (MAIN_UI != nullptr) delete MAIN_UI;
    return exec_result;
}