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

public:
    /**
     * @param _ui pointer to Ui_MainWindow, all child widgets will be deallocated upon deletion of GUIWindow object, Ui_MainWindow object - won't
     * @param _remote pointer to RemoteEngine object to use for communication with search engine, won't be deallocated upon GUIWindow deletion
     * @param _parent optional service QT value
     */
    GUIWindow(Ui_MainWindow* _ui, RemoteEngine* _remote, QWidget* _parent = nullptr);

    /**
     * Tries to load GUI settings from given file.
     * @param filepath path to the file to load settings from
     */
    void save_settings(const std::string& filepath);

    /**
     * Tries to save GUI settings to given file
     * @param filepath path to the file to save settings to
     */
    void load_settings(const std::string& filepath);

private:
    enum OpenDocMode {
        Auto,
        File,
        Site
    };

    Ui_MainWindow* UI;
    RemoteEngine* REMOTE;
    std::map<QTableWidget*,QMenu> context_menus;
    std::vector<time_t> requests_dates;
    std::map<QString,std::vector<std::pair<QString,QString>>> recent_requests_data;

    const QTableWidgetItem& add_recent_config(const QString& config_path);
    const QTableWidgetItem& add_recent_request(const QString& request);
    void index_dump_load_accessibility();
    void open_document(const QString& filepath, OpenDocMode mode = Auto) const;
    void init_connects();
    void init_context_menus();
    void init_document_actions();
    void update_documents_entries(const std::vector<size_t>& ids);

    void write_gui_settings(nlohmann::json& config);
    void write_remote_settings(nlohmann::json& config);
    void write_default_settings(nlohmann::json& config);

    static void resize_table_columns(QTableWidget* table, int filepath_column = -1, int control_width = 100);
    static int table_current_row(QTableWidget* table, int* acceptor = nullptr);

    void indexationChecker();
    bool loadConfigFile(const QString& filepath, bool add_recent);
    void openDocumentFromCell(QTableWidget* table, OpenDocMode mode = Auto, int column = 0, int row = -1);
    void toggleDisplayFullFilenames(QTableWidget* table, int column, bool state);

public slots:
    void updateDocumentsTable();
    void removeRecentConfigEntry();
    void enableConfigChangesControls();
    void applyConfigChanges();
    void revertConfigChanges();
    void selectAndOpenConfig();
    void openRecentConfig(int row = -1, int column = 0);
    void openRecentConfigCatalog(int row = -1, int column = 0);
    void openRecentConfigFile(int row = -1, int column = 0);
    void newConfig(bool use_defaults = true);
    void saveConfigAs();
    void addDocuments();
    void addURLs();
    void processRequest();
    void reindexAll();
    void reindexSelected();
    void removeSelected();
    void displayRecentRequest(int row = -1, int column = 0);
    void reprocessRecentRequest(int row = -1, int column = 0);
};

