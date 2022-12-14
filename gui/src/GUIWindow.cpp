#include "GUIWindow.h"

using json = nlohmann::json;

GUIWindow::GUIWindow(Ui_MainWindow* _ui, RemoteEngine* _remote, QWidget* _parent) {
    _ui->setupUi(this);
    UI = _ui;
    REMOTE = _remote;

    init_connects();
    init_document_actions();
    init_context_menus();
}

void GUIWindow::init_connects() {
    QObject::connect(UI->tableDocuments, &QTableWidget::cellDoubleClicked, [this](int row, int column){
        openDocumentFromCell(UI->tableDocuments, Auto, 0, row);
    });
    QObject::connect(UI->tableRecentConfigs, &QTableWidget::cellDoubleClicked, this, &GUIWindow::openRecentConfig);
    QObject::connect(UI->tableRequestsStory, &QTableWidget::cellClicked, this, &GUIWindow::displayRecentRequest);

    QObject::connect(UI->tableRequestsStory, &QTableWidget::cellDoubleClicked, [this](int row, int column){
        reprocessRecentRequest(row, column);
    });
    QObject::connect(UI->tableAnswers, &QTableWidget::cellDoubleClicked, [this](int row, int column){
        openDocumentFromCell(UI->tableAnswers, Auto, 0, row);
    });
    QObject::connect(UI->pushButton_DumpIndex, &QPushButton::pressed, [this](){
        UI->statusbar->showMessage("Starting to dump index to configuration directory...", 3000);
        REMOTE->dump_index();
    });
    QObject::connect(UI->pushButton_LoadIndex, &QPushButton::pressed, [this](){
        UI->statusbar->showMessage("Attempting to load index from configuration directory...", 3000);
        REMOTE->load_index();
        updateDocumentsTable();
    });
    QObject::connect(UI->actionDisplay_full_filenames, &QAction::toggled, [this](bool state){
        toggleDisplayFullFilenames(UI->tableDocuments, 0, state);
        toggleDisplayFullFilenames(UI->tableRecentConfigs, 0, state);
        toggleDisplayFullFilenames(UI->tableAnswers, 0, state);
    });
}

void GUIWindow::init_document_actions() {
    QAction* act;
    auto docMenu = new QMenu;
    act = docMenu->addAction("Open selected");
    QObject::connect(act, &QAction::triggered, [this](){
        openDocumentFromCell(UI->tableDocuments, Auto);
    });

    act = docMenu->addAction("Add files");
    QObject::connect(act, &QAction::triggered, this, &GUIWindow::addDocuments);

    act = docMenu->addAction("Add URLs");
    QObject::connect(act, &QAction::triggered, this, &GUIWindow::addURLs);

    docMenu->addSeparator();
    act = docMenu->addAction("Update table");
    QObject::connect(act, &QAction::triggered, this, &GUIWindow::updateDocumentsTable);

    act = docMenu->addAction("Reindex selected");
    QObject::connect(act, &QAction::triggered, this, &GUIWindow::reindexSelected);

    act = docMenu->addAction("Reindex all");
    QObject::connect(act, &QAction::triggered, this, &GUIWindow::reindexAll);

    docMenu->addSeparator();
    act = docMenu->addAction("Remove selected");
    QObject::connect(act, &QAction::triggered, this, &GUIWindow::removeSelected);

    UI->toolButton_documentActions->setMenu(docMenu);
}

void GUIWindow::init_context_menus() {
    UI->tableDocuments->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(UI->tableDocuments, &QTableWidget::customContextMenuRequested, [this](QPoint pos){
        auto table = UI->tableDocuments;
        auto row = table_current_row(table);
        if (row < 0) return;

        auto& menu = context_menus[table];
        if (menu.isEmpty()) {
            menu.addAction("Open file", [this](){openDocumentFromCell(UI->tableDocuments, File);});
            menu.addAction("Open in internet browser", [this](){openDocumentFromCell(UI->tableDocuments, Site);});
            menu.addSeparator();
            menu.addAction("Reindex", [this](){reindexSelected();});
            menu.addSeparator();
            menu.addAction("Remove", [this](){removeSelected();});
        };

        auto cell = table->item(row, 0);
        if (cell == nullptr) return;
        auto filepath = cell->data(GUI_FILEPATH_DATA_ROLE).value<QString>();
        menu.actions()[1]->setEnabled(HTTPFetcher::is_http_link(filepath.toStdString()));

        menu.popup(table->viewport()->mapToGlobal(pos));
    });

    UI->tableAnswers->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(UI->tableAnswers, &QTableWidget::customContextMenuRequested, [this](QPoint pos){
        auto table = UI->tableAnswers;
        auto row = table_current_row(table);
        if (row < 0) return;

        auto& menu = context_menus[table];
        if (menu.isEmpty()) {
            menu.addAction("Open file", [this](){openDocumentFromCell(UI->tableAnswers, File);});
            menu.addAction("Open in internet browser", [this](){openDocumentFromCell(UI->tableAnswers, Site);});
        };

        auto cell = table->item(row, 0);
        if (cell == nullptr) return;
        auto filepath = cell->data(GUI_FILEPATH_DATA_ROLE).value<QString>();
        menu.actions()[1]->setEnabled(HTTPFetcher::is_http_link(filepath.toStdString()));

        menu.popup(table->viewport()->mapToGlobal(pos));
    });

    UI->tableRecentConfigs->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(UI->tableRecentConfigs, &QTableWidget::customContextMenuRequested, [this](QPoint pos){
        auto table = UI->tableRecentConfigs;
        if (table->selectedRanges().empty()) return;

        auto& menu = context_menus[table];
        if (menu.isEmpty()) {
            menu.addAction("Open configuration", [this](){openRecentConfig();});
            menu.addAction("Open file", [this](){openRecentConfigFile();});
            menu.addAction("Open directory", [this](){openRecentConfigCatalog();});
            menu.addSeparator();
            menu.addAction("Remove entry", [this](){removeRecentConfigEntry();});
        };
        menu.popup(table->viewport()->mapToGlobal(pos));
    });

    UI->tableRequestsStory->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(UI->tableRequestsStory, &QTableWidget::customContextMenuRequested, [this](QPoint pos){
        auto table = UI->tableRequestsStory;
        if (table->selectedRanges().empty()) return;

        auto& menu = context_menus[table];
        if (menu.isEmpty()) {
            menu.addAction("Open cached result", [this](){displayRecentRequest();});
            menu.addAction("Reprocess request", [this](){reprocessRecentRequest();});
        };
        menu.popup(table->viewport()->mapToGlobal(pos));
    });
}

void GUIWindow::resize_table_columns(QTableWidget* table, int filepath_column, int control_width) {
    table->resizeColumnsToContents();

    if (filepath_column >= 0 && table->columnWidth(filepath_column) > control_width) {
        table->setColumnWidth(filepath_column, control_width);
    }

    // resizeColumnsToContents() ignores StretchLastSection flag, workaround:
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setStretchLastSection(true);
};

void GUIWindow::save_settings(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) return;

    nlohmann::json settings;

    // flags
    settings["show_recent_configs"] = UI->actionShow_recent_configurations->isChecked();
    settings["show_recent_requests"] = UI->actionShow_recent_requests->isChecked();
    settings["display_full_filenames"] = UI->actionDisplay_full_filenames->isChecked();
    settings["show_tooltips"] = UI->actionShow_tooltip->isChecked();

    // recent configs list
    auto table = UI->tableRecentConfigs;
    settings["recent_configs"] = nlohmann::json::array();
    auto& recent_configs = settings["recent_configs"];
    for (int i = 0; i < table->rowCount(); ++i) {
        auto cell = table->item(i, 0);
        if (cell != nullptr) {
            recent_configs.push_back(cell->data(GUI_FILEPATH_DATA_ROLE).value<QString>().toStdString());
        }
    }

    file << settings.dump(2);
    file.close();
}

void GUIWindow::load_settings(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    nlohmann::json settings;
    try {
        settings = nlohmann::json::parse(file);
        file.close();
    } catch (...) {
        file.close();
        return;
    }

    // flags
    auto select_bool = [&settings](const std::string& json_field){
        return !settings.contains(json_field) || settings[json_field].get<bool>();
    };
    auto toggle_setting = [&select_bool](QAction* action, const std::string& json_field){
        bool val = select_bool(json_field);
        action->toggled(val);
        action->setChecked(val);
    };
    toggle_setting(UI->actionShow_recent_configurations, "show_recent_configs");
    toggle_setting(UI->actionShow_recent_requests, "show_recent_requests");
    toggle_setting(UI->actionDisplay_full_filenames, "display_full_filenames");
    toggle_setting(UI->actionShow_tooltip, "show_tooltips");

    // recent configs list
    if (settings.contains("recent_configs") && settings["recent_configs"].is_array()) {
        auto& recent_configs = settings["recent_configs"];
        auto table = UI->tableRecentConfigs;
        table->setRowCount(recent_configs.size());

        for (int i = 0; i < recent_configs.size(); ++i) {
            auto path = QString::fromStdString(recent_configs[i]);
            auto cell = new QTableWidgetItem(path);
            cell->setData(GUI_FILEPATH_DATA_ROLE, QVariant(path));
            table->setItem(i, 0, cell);
        }
    }

    toggleDisplayFullFilenames(UI->tableRecentConfigs, 0, UI->actionDisplay_full_filenames->isChecked());

}

int GUIWindow::table_current_row(QTableWidget *table, int* acceptor) {
    auto selection = table->selectedRanges();
    if (selection.empty()) return -1;
    int row = selection[0].topRow();
    if (acceptor != nullptr) *acceptor = row;
    return row;
}

void GUIWindow::update_documents_entries(const std::vector<size_t>& ids) {
    std::vector<DocInfo> acceptor;
    std::vector<DocInfo>& status = acceptor;

    if (ids.empty()) {
        if (!REMOTE->full_status()) return;
        status = REMOTE->current_status;
    } else {
        REMOTE->files_status(ids, &status);
    }

    int docs_count = REMOTE->current_status.size();
    auto table = UI->tableDocuments;
    table->setRowCount(docs_count);

    auto set_text_cell = [](QTableWidget* table, const QString& value, int row, int column){
        auto cell = new QTableWidgetItem(value);
        table->setItem(row, column, cell);
    };

    QDateTime index_date;
    for (auto& doc_info : status) {
        auto row = doc_info.doc_id;

        QString filepath = QString::fromStdString(doc_info.filepath);
        auto cell = new QTableWidgetItem();
        cell->setData(GUI_FILEPATH_DATA_ROLE, QVariant(filepath));
        table->setItem(row, 0, cell);

        if (doc_info.index_date == 0) {
            set_text_cell(table, "", row, 3);
        } else {
            index_date.setTime_t(doc_info.index_date);
            set_text_cell(table, index_date.toString("dd.MM.yyyy hh:mm:ss"), row, 3);
        }

        set_text_cell(table, (doc_info.indexed ? "+" : "-"), row, 1);
        set_text_cell(table, (doc_info.indexing_in_progress ? "+" : "-"), row, 2);
        set_text_cell(table, QString::fromStdString(doc_info.error_text), row, 4);
    }

    toggleDisplayFullFilenames(table, 0, UI->actionDisplay_full_filenames->isChecked());
}

void GUIWindow::write_gui_settings(nlohmann::json& config) {
    auto& settings = config["config"];
    settings["name"] = UI->lineEdit_ConfigName->text().toStdString();
    settings["max_responses"] = UI->spinBox_max_responses->value();
    settings["max_indexation_threads"] = UI->spinBox_max_indexation_threads->value();
    settings["auto_reindex"] = UI->checkBox_auto_reindex->isChecked();
    settings["auto_dump_index"] = UI->checkBox_auto_dump_index->isChecked();
    settings["auto_load_index_dump"] = UI->checkBox_auto_load_index_dump->isChecked();
    settings["relative_to_config_folder"] = UI->checkBox_relative_to_config_folder->isChecked();
    settings["use_independent_dicts_method"] = UI->checkBox_use_independent_dicts_method->isChecked();
}

void GUIWindow::write_remote_settings(nlohmann::json& config) {
    auto& settings = config["config"];
    settings["name"] = REMOTE->name;
    settings["max_responses"] = REMOTE->max_responses;
    settings["max_indexation_threads"] = REMOTE->max_indexation_threads;
    settings["auto_reindex"] = REMOTE->auto_reindex;
    settings["auto_dump_index"] = REMOTE->auto_dump_index;
    settings["auto_load_index_dump"] = REMOTE->auto_load_index_dump;
    settings["relative_to_config_folder"] = REMOTE->relative_to_config_folder;
    settings["use_independent_dicts_method"] = REMOTE->use_independent_dicts_method;
}

void GUIWindow::write_default_settings(nlohmann::json& config) {
    auto& settings = config["config"];
    settings["name"] = "configuration template";
    settings["max_responses"] = 5;
    settings["max_indexation_threads"] = 4;
    settings["auto_reindex"] = false;
    settings["auto_dump_index"] = true;
    settings["auto_load_index_dump"] = true;
    settings["relative_to_config_folder"] = true;
    settings["use_independent_dicts_method"] = false;
}

const QTableWidgetItem& GUIWindow::add_recent_config(const QString& config_path) {
    auto table = UI->tableRecentConfigs;

    for (int i = 0; i < table->rowCount(); ++i) {
        auto cell = table->item(i, 0);
        if (cell != nullptr && cell->data(GUI_FILEPATH_DATA_ROLE).value<QString>() == config_path) {
            table->setCurrentItem(cell);
            return *cell;
        }
    }

    table->setRowCount(table->rowCount() + 1);
    auto cell = new QTableWidgetItem(config_path);
    cell->setData(GUI_FILEPATH_DATA_ROLE, QVariant(config_path));
    table->setItem(table->rowCount() - 1, 0, cell);
    table->setCurrentItem(cell);
    toggleDisplayFullFilenames(UI->tableRecentConfigs, 0, UI->actionDisplay_full_filenames->isChecked());
    return *cell;
}

const QTableWidgetItem& GUIWindow::add_recent_request(const QString& request) {
    auto table = UI->tableRequestsStory;

    for (int i = 0; i < table->rowCount(); ++i) {
        auto cell = table->item(i, 0);
        if (cell != nullptr && cell->text() == request) {
            table->setCurrentItem(cell);
            requests_dates[i] = std::time(nullptr);
            return *cell;
        }
    }

    auto rows_count = table->rowCount();
    table->setRowCount(rows_count + 1);
    auto cell = new QTableWidgetItem(request);
    table->setItem(rows_count, 0, cell);
    table->setCurrentItem(cell);

    requests_dates.resize(rows_count + 1);
    requests_dates[rows_count] = std::time(nullptr);

    return *cell;
}

void GUIWindow::index_dump_load_accessibility() {
    if (REMOTE->auto_dump_index && REMOTE->auto_load_index_dump) {
        UI->pushButton_DumpIndex->setEnabled(false);
        UI->pushButton_LoadIndex->setEnabled(false);
        UI->label_IndexationTooltip->setText("Auto dump/load enabled");
    } else if (REMOTE->auto_dump_index) {
        UI->pushButton_DumpIndex->setEnabled(false);
        UI->pushButton_LoadIndex->setEnabled(true);
        UI->label_IndexationTooltip->setText("Auto dump enabled");
    } else if (REMOTE->auto_load_index_dump) {
        UI->pushButton_DumpIndex->setEnabled(true);
        UI->pushButton_LoadIndex->setEnabled(false);
        UI->label_IndexationTooltip->setText("Auto load enabled");
    } else {
        UI->pushButton_DumpIndex->setEnabled(true);
        UI->pushButton_LoadIndex->setEnabled(true);
        UI->label_IndexationTooltip->setText("|");
    }
}

bool GUIWindow::loadConfigFile(const QString& filepath, bool add_recent) {
    UI->statusbar->showMessage("Loading " + filepath);
    if (!REMOTE->load_config(filepath.toStdString())) {
        UI->statusbar->showMessage("Could not load " + filepath, 3000);
        return false;
    }

    if (add_recent) add_recent_config(filepath);

    UI->actionSave_as->setEnabled(true);

    UI->labelConfigFilepath->setText(filepath);
    UI->lineEdit_ConfigName->setText(QString::fromStdString(REMOTE->name));
    UI->spinBox_max_responses->setValue(REMOTE->max_responses);
    UI->spinBox_max_indexation_threads->setValue(REMOTE->max_indexation_threads);
    UI->checkBox_auto_reindex->setChecked(REMOTE->auto_reindex);
    UI->checkBox_auto_dump_index->setChecked(REMOTE->auto_dump_index);
    UI->checkBox_auto_load_index_dump->setChecked(REMOTE->auto_load_index_dump);
    UI->checkBox_relative_to_config_folder->setChecked(REMOTE->relative_to_config_folder);
    UI->checkBox_use_independent_dicts_method->setChecked(REMOTE->use_independent_dicts_method);

    updateDocumentsTable();
    UI->pushButton_processRequests->setEnabled(true);
    UI->toolButton_documentActions->setEnabled(true);
    index_dump_load_accessibility();

    UI->tableRequestsStory->setRowCount(0);
    UI->tableAnswers->setRowCount(0);
    UI->lineEdit_request->clear();
    UI->statusbar->showMessage(filepath + " loaded successfully", 3000);

    if (REMOTE->auto_reindex) QTimer::singleShot(2000, this, &GUIWindow::indexationChecker);

    return true;
}

void GUIWindow::indexationChecker() {
    std::vector<size_t> ids;
    for (auto& doc_info : REMOTE->current_status) {
        if (doc_info.indexing_in_progress) ids.push_back(doc_info.doc_id);
    }

    if (ids.size() > 0) {
        QString message = "Indexation of ";
        message.append(QString::fromStdString(std::to_string(ids.size())));
        message.append(" files remains in progress...");
        UI->statusbar->showMessage(message);
        update_documents_entries(ids);
        QTimer::singleShot(2000, this, &GUIWindow::indexationChecker);
    } else {
        UI->statusbar->showMessage("All indexation tasks have been finished.");
    }
}

void GUIWindow::toggleDisplayFullFilenames(QTableWidget* table, int column, bool state) {
    for (int i = 0; i < table->rowCount(); ++i) {
        auto cell = table->item(i, column);
        if (cell == nullptr) continue;
        auto filename = cell->data(GUI_FILEPATH_DATA_ROLE).value<QString>();
        std::string full = filename.toStdString();
        if (state || HTTPFetcher::is_http_link(full)) {
            cell->setText(filename);
        } else {
            std::string repr = PathHelper::file_name(full);
            repr.append(PathHelper::file_extension(full));
            cell->setText(QString::fromStdString(repr));
        }
    }
}

void GUIWindow::open_document(const QString& filepath, OpenDocMode mode) const {
    UI->statusbar->showMessage("Opening " + filepath + " ...", 5000);
    auto path = filepath.toStdString();

    bool is_url = HTTPFetcher::is_http_link(path);
    bool success = false;

    if (is_url && mode == File) {
        std::stringstream html;
        HTTPFetcher::get_html(path, html);
        path = "gui_tmp.html";
        std::ofstream output(path);
        if (!output.is_open()) {
            UI->statusbar->showMessage("Could not create temporary html file for " + filepath, 5000);
            return;
        }
        output << html.str();
        output.close();

        success = QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(path)));
    } else if (is_url && (mode == Site || mode == Auto)) {
        success = QDesktopServices::openUrl(QUrl(filepath, QUrl::TolerantMode));
    } else {
        success = QDesktopServices::openUrl(QUrl::fromLocalFile(filepath));
    }

    if (!success) UI->statusbar->showMessage("Could not open " + filepath, 5000);
}

void GUIWindow::openDocumentFromCell(QTableWidget* table, OpenDocMode mode, int column, int row) {
    if (row < 0 && table_current_row(table, &row) < 0) return;
    auto item = table->item(row, column);
    if (item == nullptr) return;

    auto filepath = item->data(GUI_FILEPATH_DATA_ROLE).value<QString>();
    open_document(filepath, mode);
}

//// slots

void GUIWindow::updateDocumentsTable() {
    update_documents_entries({});
    resize_table_columns(UI->tableDocuments, 0, 400);
}

void GUIWindow::removeRecentConfigEntry() {
    auto table = UI->tableRecentConfigs;
    auto selected = table->selectedRanges();
    if (selected.empty()) return;
    auto current_row = selected[0].topRow();
    table->removeRow(current_row);

    if (current_row >= table->rowCount()) {
        table->setCurrentCell(table->rowCount() - 1, 0);
    } else {
        table->setCurrentCell(current_row, 0);
    }
}

void GUIWindow::enableConfigChangesControls() {
    UI->pushButton_applyConfigChanges->setEnabled(true);
    if (!REMOTE->get_filepath().empty()) UI->pushButton_revertConfigChanges->setEnabled(true);
}

void GUIWindow::applyConfigChanges() {
    UI->pushButton_applyConfigChanges->setEnabled(false);
    UI->pushButton_revertConfigChanges->setEnabled(false);

    std::string config_filepath = REMOTE->get_filepath();
    if (config_filepath.empty()) {
        newConfig(false);
        index_dump_load_accessibility();
        return;
    }

    std::ifstream file(config_filepath);
    json config;
    try {
        config = json::parse(file);
        file.close();
        if (!config.contains("config")) {
            config["config"] = {};
        }
    } catch(...) {
        file.close();
        config["config"] = {};
        config["files"] = json::array();
        for (auto& doc_info : REMOTE->current_status) {
            config["files"].push_back(doc_info.filepath);
        }
    }

    write_gui_settings(config);

    std::ofstream output(config_filepath);
    if (!output.is_open()) {
        std::cerr << "Could not open file '" << config_filepath << "'." << std::endl;
        return;
    }
    output << config.dump(2);
    output.close();

    REMOTE->reload_config();
    index_dump_load_accessibility();
}

void GUIWindow::revertConfigChanges() {
    UI->pushButton_applyConfigChanges->setEnabled(false);
    UI->pushButton_revertConfigChanges->setEnabled(false);

    UI->lineEdit_ConfigName->setText(QString::fromStdString(REMOTE->name));
    UI->spinBox_max_responses->setValue(REMOTE->max_responses);
    UI->spinBox_max_indexation_threads->setValue(REMOTE->max_indexation_threads);
    UI->checkBox_auto_reindex->setChecked(REMOTE->auto_reindex);
    UI->checkBox_auto_dump_index->setChecked(REMOTE->auto_dump_index);
    UI->checkBox_auto_load_index_dump->setChecked(REMOTE->auto_load_index_dump);
    UI->checkBox_relative_to_config_folder->setChecked(REMOTE->relative_to_config_folder);
    UI->checkBox_use_independent_dicts_method->setChecked(REMOTE->use_independent_dicts_method);
}

void GUIWindow::openRecentConfig(int row, int column) {
    auto table = UI->tableRecentConfigs;
    if (row < 0 && table_current_row(table, &row) < 0) {
        selectAndOpenConfig();
        return;
    }

    auto cell = table->item(row, column);
    if (cell == nullptr) return;
    auto filepath = cell->data(GUI_FILEPATH_DATA_ROLE).value<QString>();
    loadConfigFile(filepath, false);
}

void GUIWindow::openRecentConfigCatalog(int row, int column) {
    auto table = UI->tableRecentConfigs;
    if (row < 0 && table_current_row(table, &row) < 0) return;

    auto cell = table->item(row, column);
    if (cell == nullptr) return;
    auto filepath = cell->data(GUI_FILEPATH_DATA_ROLE).value<QString>();

    QString dir = QString::fromStdString(PathHelper::catalog_from_filepath(filepath.toStdString()));
    bool success = QDesktopServices::openUrl(QUrl::fromLocalFile(dir));

    if (!success) UI->statusbar->showMessage("Unable to open directory: " + dir, 8000);
}

void GUIWindow::openRecentConfigFile(int row, int column) {
    auto table = UI->tableRecentConfigs;
    if (row < 0 && table_current_row(table, &row) < 0) return;

    auto cell = table->item(row, column);
    if (cell == nullptr) return;
    auto filepath = cell->data(GUI_FILEPATH_DATA_ROLE).value<QString>();
    open_document(filepath);
}

void GUIWindow::selectAndOpenConfig() {
    QString caption = "Select configuration file for search engine";
    QString filter = "JSON Files (*.json)";
    auto filepath = QFileDialog::getOpenFileName(this, caption, "", filter);

    if (filepath.isEmpty()) return;
    std::cout << filepath.toStdString() << std::endl;
    loadConfigFile(filepath, UI->actionShow_recent_configurations->isEnabled());
}

void GUIWindow::newConfig(bool use_defaults) {
    QString caption = "Select file to save configuration to";
    QString filter = "JSON Files (*.json)";
    auto filepath = QFileDialog::getSaveFileName(this, caption, "", filter);
    if (filepath.isEmpty()) return;

    std::string config_filepath = filepath.toStdString();

    json config;
    config["config"] = {};
    config["files"] = json::array();

    if (use_defaults) {
        write_default_settings(config);
    } else {
        write_gui_settings(config);
    }

    std::remove(config_filepath.c_str());
    std::ofstream file(config_filepath);
    if (!file.is_open()) {
        std::cerr << "Could not open file '" << config_filepath << "'." << std::endl;
        UI->statusbar->showMessage(QString::fromStdString("Could not open file :" + config_filepath));
        return;
    }
    file << config.dump(2);
    file.close();
    loadConfigFile(filepath, UI->actionShow_recent_configurations->isEnabled());
}

void GUIWindow::saveConfigAs() {
    QString caption = "Select file to save configuration to";
    QString filter = "JSON Files (*.json)";
    auto filepath = QFileDialog::getSaveFileName(this, caption, "", filter);

    if (filepath.isEmpty()) return;
    REMOTE->save_config(filepath.toStdString(), true);
    UI->statusbar->showMessage("Waiting for file to be saved...");
    loadConfigFile(filepath, UI->actionShow_recent_configurations->isEnabled());
}

void GUIWindow::addDocuments() {
    auto documents = QFileDialog::getOpenFileNames(this, "Select files to add", "", "Supported files (*.txt *.html);; All files (*.*)");
    if (documents.isEmpty()) return;

    std::vector<std::string> filenames;
    filenames.reserve(documents.size());
    for (auto& doc : documents) filenames.push_back(doc.toStdString());
    REMOTE->add_files(filenames);
    REMOTE->save_config(REMOTE->get_filepath());
    updateDocumentsTable();
}

void GUIWindow::addURLs() {
    bool accept_input;
    auto text = QInputDialog::getMultiLineText(this, "Enter URLs to add", "", "", &accept_input);

    if (!accept_input) return;

    std::string url;
    std::vector<std::string> URLs;
    std::stringstream text_stream(text.toStdString());

    while (std::getline(text_stream, url)) {
        if (HTTPFetcher::is_http_link(url)) URLs.push_back(url);
    }

    if (URLs.empty()) return;

    REMOTE->add_files(URLs);
    REMOTE->save_config(REMOTE->get_filepath());
    updateDocumentsTable();
}

void GUIWindow::processRequest() {
    auto request = UI->lineEdit_request->text();;
    if (request.isEmpty()) return;

    std::vector<RequestAnswer> answers;
    if (!REMOTE->process_request(request.toStdString(), answers)) return;

    auto table = UI->tableAnswers;
    if (answers.empty()) {
        table->setRowCount(1);
        table->setItem(0, 0, new QTableWidgetItem("No results"));
        table->setItem(0, 1, new QTableWidgetItem(""));
        return;
    }

    table->setRowCount(answers.size());
    std::vector<std::pair<QString,QString>>* recent_request_records;
    bool track_recent_requests = UI->actionShow_recent_requests->isEnabled();
    if (track_recent_requests) {
        recent_request_records = &recent_requests_data[request];
        recent_request_records->resize(answers.size());
    }

    int row = 0;
    for (auto& answer : answers) {
        auto filename = QString::fromStdString(REMOTE->current_status[answer.doc_id].filepath);
        auto cell = new QTableWidgetItem("");
        cell->setData(GUI_FILEPATH_DATA_ROLE, QVariant(filename));
        table->setItem(row, 0, cell);

        auto rank = QString::fromStdString(std::to_string(answer.rank));
        cell = new QTableWidgetItem(rank);
        table->setItem(row, 1, cell);
        if (track_recent_requests) {
            auto& record = (*recent_request_records)[row];
            record.first = filename;
            record.second = rank;
        }
        ++row;
    }
    toggleDisplayFullFilenames(table, 0, UI->actionDisplay_full_filenames->isChecked());
    resize_table_columns(table, 0, track_recent_requests ? 400 : 500);

    QDateTime request_date;
    request_date.setTime_t(std::time(nullptr));
    UI->label_requestDate->setText("Request processed at " + request_date.toString("dd.MM.yyyy hh:mm:ss"));

    if (track_recent_requests) add_recent_request(request);
}

void GUIWindow::reindexAll() {
    REMOTE->reindex({});
    updateDocumentsTable();
    QTimer::singleShot(2000, this, &GUIWindow::indexationChecker);
}

void GUIWindow::reindexSelected() {
    auto selections = UI->tableDocuments->selectedRanges();
    auto total_rows = UI->tableDocuments->rowCount();

    if (total_rows == 0) return;

    std::vector<size_t> docs;
    for (auto& selection : selections) {
        if (selection.rowCount() == total_rows) {
            reindexAll();
            return;
        } else {
            for (auto i = selection.topRow(); i <= selection.bottomRow(); ++i) {
                docs.push_back(i);
            }
        }
    }

    if (docs.empty()) return;
    REMOTE->reindex(docs);
    update_documents_entries(docs);
    QTimer::singleShot(2000, this, &GUIWindow::indexationChecker);
}

void GUIWindow::removeSelected() {
    if (REMOTE->indexation_ongoing()) {
        UI->statusbar->showMessage("Documents cannot be removed, while indexation tasks are in progress.");
        return;
    }

    auto selections = UI->tableDocuments->selectedRanges();

    std::vector<size_t> docs;
    for (auto& selection : selections) {
        for (auto i = selection.topRow(); i <= selection.bottomRow(); ++i) {
            docs.push_back(i);
        }
    }

    if (docs.empty()) return;
    REMOTE->remove_files(docs);
    REMOTE->save_config(REMOTE->get_filepath());
    updateDocumentsTable();
    UI->statusbar->showMessage("Removal task have been completed.", 3000);
}

void GUIWindow::displayRecentRequest(int row, int column) {
    if (row < 0 && table_current_row(UI->tableRequestsStory, &row) < 0) return;

    auto cell = UI->tableRequestsStory->item(row, column);
    if (cell == nullptr) return;

    auto table = UI->tableAnswers;
    auto request = cell->text();
    UI->lineEdit_request->setText(request);
    auto& records = recent_requests_data[request];
    table->setRowCount(records.size());
    for (int i = 0; i < records.size(); ++i) {
        auto filename = records[i].first;
        auto cell = new QTableWidgetItem();
        cell->setData(GUI_FILEPATH_DATA_ROLE, QVariant(filename));
        table->setItem(i, 0, cell);
        table->setItem(i, 1, new QTableWidgetItem(records[i].second));
    }

    QDateTime request_date;
    request_date.setTime_t(requests_dates[row]);
    UI->label_requestDate->setText("Request processed at " + request_date.toString("dd.MM.yyyy hh:mm:ss"));

    toggleDisplayFullFilenames(table, 0, UI->actionDisplay_full_filenames->isChecked());
}

void GUIWindow::reprocessRecentRequest(int row, int column) {
    if (row < 0 && table_current_row(UI->tableRequestsStory, &row) < 0) return;

    auto cell = UI->tableRequestsStory->item(row, column);
    if (cell == nullptr) return;
    auto request = cell->text();
    UI->lineEdit_request->setText(request);
    processRequest();
}

