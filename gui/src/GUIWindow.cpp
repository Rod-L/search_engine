#include "GUIWindow.h"

using json = nlohmann::json;

GUIWindow::GUIWindow(Ui_MainWindow* _ui, RemoteEngine* _remote, QWidget* _parent) {
    _ui->setupUi(this);
    UI = _ui;
    REMOTE = _remote;

    QObject::connect(UI->tableDocuments, &QTableWidget::cellDoubleClicked, this, &GUIWindow::openDocument);
    QObject::connect(UI->tableRecentConfigs, &QTableWidget::cellDoubleClicked, this, &GUIWindow::openRecentConfig);
    QObject::connect(UI->tableRequestsStory, &QTableWidget::cellClicked, this, &GUIWindow::display_recent_request);

    QObject::connect(UI->tableRequestsStory, &QTableWidget::cellDoubleClicked, [this](int row, int column){
        reprocess_recent_request(row, column);
    });
    QObject::connect(UI->tableAnswers, &QTableWidget::cellDoubleClicked, [this](int row, int column){
        openDocumentFromCell(UI->tableAnswers, 0, row);
    });
    QObject::connect(UI->pushButton_DumpIndex, &QPushButton::pressed, [this](){
        REMOTE->dump_index();
    });
    QObject::connect(UI->pushButton_LoadIndex, &QPushButton::pressed, [this](){
        REMOTE->load_index();
        update_documents_table();
    });
    QObject::connect(UI->actionDisplay_full_filenames, &QAction::toggled, [this](bool state){
        toggleDisplayFullFilenames(UI->tableDocuments, 0, state);
        toggleDisplayFullFilenames(UI->tableRecentConfigs, 0, state);
        toggleDisplayFullFilenames(UI->tableAnswers, 0, state);
    });

    // doc actions menu
    QAction* act;
    auto docMenu = new QMenu;
    act = docMenu->addAction("Open selected");
    QObject::connect(act, &QAction::triggered, this, &GUIWindow::openDocument);

    act = docMenu->addAction("Add files");
    QObject::connect(act, &QAction::triggered, this, &GUIWindow::addDocuments);

    act = docMenu->addAction("Add URLs");
    QObject::connect(act, &QAction::triggered, this, &GUIWindow::addURLs);

    docMenu->addSeparator();
    act = docMenu->addAction("Update table");
    QObject::connect(act, &QAction::triggered, this, &GUIWindow::update_documents_table);

    act = docMenu->addAction("Reindex selected");
    QObject::connect(act, &QAction::triggered, this, &GUIWindow::reindexSelected);

    act = docMenu->addAction("Reindex all");
    QObject::connect(act, &QAction::triggered, this, &GUIWindow::reindexAll);

    docMenu->addSeparator();
    act = docMenu->addAction("Remove selected");
    QObject::connect(act, &QAction::triggered, this, &GUIWindow::removeSelected);

    UI->toolButton_documentActions->setMenu(docMenu);

    // context menus
    UI->tableDocuments->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(UI->tableDocuments, &QTableWidget::customContextMenuRequested, [this](QPoint pos){
        auto table = UI->tableDocuments;
        if (table->selectedRanges().empty()) return;

        auto& menu = context_menus[table];
        if (menu.isEmpty()) {
            menu.addAction("Open file", [this](){openDocument();});
            menu.addAction("Open in browser", [this](){openUrlFromCell(UI->tableDocuments, 0);});
            menu.addSeparator();
            menu.addAction("Reindex", [this](){reindexSelected();});
            menu.addSeparator();
            menu.addAction("Remove", [this](){removeSelected();});
        };
        menu.popup(table->viewport()->mapToGlobal(pos));
    });

    UI->tableAnswers->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(UI->tableAnswers, &QTableWidget::customContextMenuRequested, [this](QPoint pos){
        auto table = UI->tableAnswers;
        if (table->selectedRanges().empty()) return;

        auto& menu = context_menus[table];
        if (menu.isEmpty()) {
            menu.addAction("Open file", [this](){openDocumentFromCell(UI->tableAnswers, 0);});
            menu.addAction("Open in browser", [this](){openUrlFromCell(UI->tableAnswers, 0);});
        };
        menu.popup(table->viewport()->mapToGlobal(pos));
    });

    UI->tableRecentConfigs->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(UI->tableRecentConfigs, &QTableWidget::customContextMenuRequested, [this](QPoint pos){
        auto table = UI->tableRecentConfigs;
        if (table->selectedRanges().empty()) return;

        auto& menu = context_menus[table];
        if (menu.isEmpty()) {
            menu.addAction("Open configuration", [this](){openRecentConfig();});
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
            menu.addAction("Open cached result", [this](){display_recent_request();});
            menu.addAction("Reprocess request", [this](){reprocess_recent_request();});
        };
        menu.popup(table->viewport()->mapToGlobal(pos));
    });

}

void GUIWindow::save_settings(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) return;

    nlohmann::json settings;

    // flags
    settings["show_recent_configs"] = UI->actionShow_recent_configurations->isChecked();
    settings["show_recent_requests"] = UI->actionShow_recent_requests->isChecked();
    settings["display_full_filenames"] = UI->actionDisplay_full_filenames->isChecked();

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
    std::vector<DocInfo> status;
    if (!REMOTE->status(ids, status)) return;

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

    update_documents_table();
    UI->pushButton_processRequests->setEnabled(true);
    UI->toolButton_documentActions->setEnabled(true);
    index_dump_load_accessibility();

    UI->tableRequestsStory->setRowCount(0);
    UI->tableAnswers->setRowCount(0);
    UI->lineEdit_request->clear();
    UI->statusbar->showMessage(filepath + " loaded successfully", 3000);
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
        UI->statusbar->showMessage("All indexation tasks have been finished.", 3000);
    }
}

void GUIWindow::toggleDisplayFullFilenames(QTableWidget* table, int column, bool state) {
    for (int i = 0; i < table->rowCount(); ++i) {
        auto cell = table->item(i, column);
        auto filename = cell->data(GUI_FILEPATH_DATA_ROLE).value<QString>();
        std::string full = filename.toStdString();
        if (state || HTTPFetcher::is_url(full)) {
            cell->setText(filename);
        } else {
            std::string repr = PathHelper::file_name(full);
            repr.append(PathHelper::file_extension(full));
            cell->setText(QString::fromStdString(repr));
        }
    }
}

void GUIWindow::open_document(const QString& filepath) const {
    UI->statusbar->showMessage("Opening " + filepath + " ...", 5000);
    auto path = filepath.toStdString();

    if (HTTPFetcher::is_url(path)) {
        std::string html;
        HTTPFetcher::get_html(path, html);
        path = "gui_tmp.html";
        std::ofstream output(path);
        if (!output.is_open()) {
            UI->statusbar->showMessage("Could not create temporary html file for " + filepath, 5000);
            return;
        }
        output << html;
        output.close();
    }

    auto cmd = "\"" + path + "\"";
    system(cmd.c_str());
}

void GUIWindow::openDocumentFromCell(QTableWidget* table, int column, int row) {
    if (row < 0 && table_current_row(table, &row) < 0) return;
    auto item = table->item(row, column);
    if (item == nullptr) return;

    auto filepath = item->data(GUI_FILEPATH_DATA_ROLE).value<QString>();
    open_document(filepath);
}

void GUIWindow::openUrlFromCell(QTableWidget* table, int column, int row) {
    if (row < 0 && table_current_row(table, &row) < 0) return;
    auto item = table->item(row, column);
    if (item == nullptr) return;

    auto str_url = item->data(GUI_FILEPATH_DATA_ROLE).value<QString>();
    UI->statusbar->showMessage("Opening " + str_url + " ...", 5000);

    if (!HTTPFetcher::is_url(str_url.toStdString())) {
        open_document(str_url);
        return;
    }

    bool success = QDesktopServices::openUrl(QUrl(str_url, QUrl::TolerantMode));
    if (!success) {
        UI->statusbar->showMessage("Could not open " + str_url, 5000);
    }
}

//// slots

void GUIWindow::update_documents_table() {
    update_documents_entries({});
    UI->tableDocuments->resizeColumnsToContents();
    UI->tableDocuments->horizontalHeader()->setStretchLastSection(false);
    UI->tableDocuments->horizontalHeader()->setStretchLastSection(true);
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
    UI->pushButton_revertConfigChanges->setEnabled(true);
}

void GUIWindow::applyConfigChanges() {
    UI->pushButton_applyConfigChanges->setEnabled(false);
    UI->pushButton_revertConfigChanges->setEnabled(false);

    bool new_config = false;
    std::string config_filepath = REMOTE->get_filepath();
    if (config_filepath.empty()) {
        new_config = true;
        config_filepath = "gui_new_config.json";
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

    auto& settings = config["config"];
    settings["name"] = UI->lineEdit_ConfigName->text().toStdString();
    settings["max_responses"] = UI->spinBox_max_responses->value();
    settings["max_indexation_threads"] = UI->spinBox_max_indexation_threads->value();
    settings["auto_reindex"] = UI->checkBox_auto_reindex->isChecked();
    settings["auto_dump_index"] = UI->checkBox_auto_dump_index->isChecked();
    settings["auto_load_index_dump"] = UI->checkBox_auto_load_index_dump->isChecked();
    settings["relative_to_config_folder"] = UI->checkBox_relative_to_config_folder->isChecked();

    std::ofstream output(config_filepath);
    if (!output.is_open()) {
        std::cerr << "Could not open file '" << config_filepath << "'." << std::endl;
        return;
    }
    output << config.dump(2);
    output.close();

    if (new_config) {
        loadConfigFile(QString::fromStdString(config_filepath), UI->actionShow_recent_configurations->isEnabled());
    } else {
        REMOTE->reload_config();
    }
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
}

void GUIWindow::openRecentConfig(int row, int column) {
    auto table = UI->tableRecentConfigs;
    if (row < 0 && table_current_row(table, &row) < 0) return;

    auto cell = table->item(row, column);
    if (cell == nullptr) return;
    auto filepath = cell->data(GUI_FILEPATH_DATA_ROLE).value<QString>();
    loadConfigFile(filepath, false);
}

void GUIWindow::selectAndOpenConfig() {
    QString caption = "Select configuration file for search engine";
    QString filter = "JSON Files (*.json)";
    auto filepath = QFileDialog::getOpenFileName(this, caption, "", filter);

    if (filepath.isEmpty()) return;
    std::cout << filepath.toStdString() << std::endl;
    loadConfigFile(filepath, UI->actionShow_recent_configurations->isEnabled());
}

void GUIWindow::newConfig() {
    QString caption = "Select file to save configuration to";
    QString filter = "JSON Files (*.json)";
    auto filepath = QFileDialog::getSaveFileName(this, caption, "", filter);
    if (filepath.isEmpty()) return;

    std::string config_filepath = filepath.toStdString();

    json config;
    config["config"] = {};
    config["files"] = json::array();

    auto& settings = config["config"];
    settings["name"] = "configuration template";
    settings["max_responses"] = 5;
    settings["max_indexation_threads"] = 4;
    settings["auto_reindex"] = false;
    settings["auto_dump_index"] = true;
    settings["auto_load_index_dump"] = true;
    settings["relative_to_config_folder"] = true;

    std::ofstream file(config_filepath);
    if (!file.is_open()) {
        std::cerr << "Could not open file '" << config_filepath << "'." << std::endl;
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
    REMOTE->save_config(filepath.toStdString());
    UI->statusbar->showMessage("Waiting for file to be saved...");
    QTimer::singleShot(1000, [this, filepath](){
        loadConfigFile(filepath, UI->actionShow_recent_configurations->isEnabled());
    });
}

void GUIWindow::openDocument() {
    auto selection = UI->tableDocuments->selectedRanges();
    if (selection.empty()) return;

    auto item = UI->tableDocuments->item(selection[0].topRow(), 0);
    if (item == nullptr) return;
    open_document(item->text());
}

void GUIWindow::addDocuments() {
    auto documents = QFileDialog::getOpenFileNames(this, "Select files to add", "", "Supported files (*.txt *.html);; All files (*.*)");
    if (documents.isEmpty()) return;

    std::vector<std::string> filenames;
    filenames.reserve(documents.size());
    for (auto& doc : documents) filenames.push_back(doc.toStdString());
    REMOTE->add_files(filenames);
    REMOTE->save_config(REMOTE->get_filepath());
    update_documents_table();
}

void GUIWindow::addURLs() {
    bool accept_input;
    auto text = QInputDialog::getMultiLineText(this, "Enter URLs to add", "", "", &accept_input);

    if (!accept_input) return;

    std::string url;
    std::vector<std::string> URLs;
    std::stringstream text_stream(text.toStdString());

    while (!text_stream.eof()) {
        std::getline(text_stream, url);
        if (HTTPFetcher::is_url(url)) URLs.push_back(url);
    }

    if (URLs.empty()) return;

    REMOTE->add_files(URLs);
    REMOTE->save_config(REMOTE->get_filepath());
    update_documents_table();
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
    table->resizeColumnsToContents();
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setStretchLastSection(true);

    QDateTime request_date;
    request_date.setTime_t(std::time(nullptr));
    UI->label_requestDate->setText("Request processed at " + request_date.toString("dd.MM.yyyy hh:mm:ss"));

    if (track_recent_requests) add_recent_request(request);
}

void GUIWindow::reindexAll() {
    REMOTE->reindex({});
    update_documents_table();
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
    update_documents_table();
    UI->statusbar->showMessage("Removal task have been completed.", 3000);
}

void GUIWindow::display_recent_request(int row, int column) {
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

void GUIWindow::reprocess_recent_request(int row, int column) {
    if (row < 0 && table_current_row(UI->tableRequestsStory, &row) < 0) return;

    auto cell = UI->tableRequestsStory->item(row, column);
    if (cell == nullptr) return;
    auto request = cell->text();
    UI->lineEdit_request->setText(request);
    processRequest();
}

