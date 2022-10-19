#pragma once

#include <QDesktopServices>
#include <QMainWindow>
#include <QFileDialog>
#include <QInputDialog>
#include <QTimer>
#include <QDateTime>
#include <map>

#include "nlohmann/json.hpp"
#include "gui.h"
#include "RemoteConfig.h"
#include "HTTPTextFetcher.h"
#include "PathHelper.h"

const int GUI_FILEPATH_DATA_ROLE = 2001;

class GUIWindow: public QMainWindow {
    Q_OBJECT

private:
    Ui_MainWindow* UI;
    RemoteEngine* REMOTE;
    std::map<QTableWidget*,QMenu> context_menus;
    std::vector<time_t> requests_dates;
    std::map<QString,std::vector<std::pair<QString,QString>>> recent_requests_data;

    const QTableWidgetItem& add_recent_config(const QString& config_path);
    const QTableWidgetItem& add_recent_request(const QString& request);
    bool loadConfigFile(const QString& filepath, bool add_recent);
    void index_dump_load_accessibility();
    void indexationChecker();
    void open_document(const QString& filepath) const;
    void openDocumentFromCell(QTableWidget* table, int column, int row = -1);
    void openUrlFromCell(QTableWidget* table, int column, int row = -1);
    void toggleDisplayFullFilenames(QTableWidget* table, int column, bool state);

public:
    explicit GUIWindow(Ui_MainWindow* _ui, RemoteEngine* _remote, QWidget* _parent = nullptr);

    void update_documents_entries(const std::vector<size_t>& ids);
    void save_settings(const std::string& filepath);
    void load_settings(const std::string& filepath);

    static int table_current_row(QTableWidget* table, int* acceptor = nullptr);

public slots:
    void update_documents_table();
    void removeRecentConfigEntry();
    void enableConfigChangesControls();
    void applyConfigChanges();
    void revertConfigChanges();
    void selectAndOpenConfig();
    void openRecentConfig(int row = -1, int column = 0);
    void newConfig();
    void saveConfigAs();
    void openDocument();
    void addDocuments();
    void addURLs();
    void processRequest();
    void reindexAll();
    void reindexSelected();
    void removeSelected();
    void display_recent_request(int row = -1, int column = 0);
    void reprocess_recent_request(int row = -1, int column = 0);
};

