/********************************************************************************
** Form generated from reading UI file 'guivNgzka.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef GUIVNGZKA_H
#define GUIVNGZKA_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionShow_recent_configurations;
    QAction *actionShow_recent_requests;
    QAction *actionOpen_configuration;
    QAction *actionSave_as;
    QAction *actionShow_console_messages;
    QAction *actionShow_tooltips;
    QAction *actionNew_configuration;
    QAction *actionExit;
    QAction *actionDisplay_full_filenames;
    QAction *actionShow_tooltip;
    QWidget *centralwidget;
    QGridLayout *gridLayout_4;
    QSplitter *splitter;
    QFrame *frame_recentConfigs;
    QGridLayout *gridLayout_3;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton_openConfiguration;
    QPushButton *pushButton_removeRecentConfigEntry;
    QSpacerItem *horizontalSpacer_5;
    QTableWidget *tableRecentConfigs;
    QTabWidget *tabWidget;
    QWidget *tabSettings;
    QGridLayout *gridLayout_2;
    QVBoxLayout *verticalLayout;
    QLabel *labelConfigFilepath;
    QLineEdit *lineEdit_ConfigName;
    QFrame *line_tooltip1;
    QHBoxLayout *horizontalLayout_2;
    QSpinBox *spinBox_max_responses;
    QLabel *label;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout_3;
    QSpinBox *spinBox_max_indexation_threads;
    QLabel *label_2;
    QSpacerItem *horizontalSpacer_3;
    QCheckBox *checkBox_auto_dump_index;
    QCheckBox *checkBox_auto_load_index_dump;
    QCheckBox *checkBox_auto_reindex;
    QCheckBox *checkBox_relative_to_config_folder;
    QCheckBox *checkBox_use_independent_dicts_method;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_applyConfigChanges;
    QPushButton *pushButton_revertConfigChanges;
    QWidget *tabDocuments;
    QHBoxLayout *horizontalLayout_8;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_5;
    QToolButton *toolButton_documentActions;
    QSpacerItem *horizontalSpacer_4;
    QLabel *label_IndexationTooltip;
    QPushButton *pushButton_DumpIndex;
    QPushButton *pushButton_LoadIndex;
    QTableWidget *tableDocuments;
    QWidget *tabRequests;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout_7;
    QLineEdit *lineEdit_request;
    QPushButton *pushButton_processRequests;
    QSplitter *splitter_3;
    QTableWidget *tableRequestsStory;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_requestDate;
    QTableWidget *tableAnswers;
    QMenuBar *menubar;
    QMenu *menuSettins;
    QMenu *menuFile;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1013, 615);
        actionShow_recent_configurations = new QAction(MainWindow);
        actionShow_recent_configurations->setObjectName(QString::fromUtf8("actionShow_recent_configurations"));
        actionShow_recent_configurations->setCheckable(true);
        actionShow_recent_configurations->setChecked(true);
        actionShow_recent_requests = new QAction(MainWindow);
        actionShow_recent_requests->setObjectName(QString::fromUtf8("actionShow_recent_requests"));
        actionShow_recent_requests->setCheckable(true);
        actionShow_recent_requests->setChecked(true);
        actionOpen_configuration = new QAction(MainWindow);
        actionOpen_configuration->setObjectName(QString::fromUtf8("actionOpen_configuration"));
        actionSave_as = new QAction(MainWindow);
        actionSave_as->setObjectName(QString::fromUtf8("actionSave_as"));
        actionSave_as->setEnabled(false);
        actionShow_console_messages = new QAction(MainWindow);
        actionShow_console_messages->setObjectName(QString::fromUtf8("actionShow_console_messages"));
        actionShow_console_messages->setCheckable(true);
        actionShow_console_messages->setChecked(true);
        actionShow_tooltips = new QAction(MainWindow);
        actionShow_tooltips->setObjectName(QString::fromUtf8("actionShow_tooltips"));
        actionShow_tooltips->setCheckable(true);
        actionShow_tooltips->setChecked(true);
        actionNew_configuration = new QAction(MainWindow);
        actionNew_configuration->setObjectName(QString::fromUtf8("actionNew_configuration"));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionDisplay_full_filenames = new QAction(MainWindow);
        actionDisplay_full_filenames->setObjectName(QString::fromUtf8("actionDisplay_full_filenames"));
        actionDisplay_full_filenames->setCheckable(true);
        actionDisplay_full_filenames->setChecked(true);
        actionShow_tooltip = new QAction(MainWindow);
        actionShow_tooltip->setObjectName(QString::fromUtf8("actionShow_tooltip"));
        actionShow_tooltip->setCheckable(true);
        actionShow_tooltip->setChecked(true);
        actionShow_tooltip->setEnabled(false);
        actionShow_tooltip->setVisible(false);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout_4 = new QGridLayout(centralwidget);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        splitter = new QSplitter(centralwidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setLineWidth(1);
        splitter->setOrientation(Qt::Horizontal);
        splitter->setHandleWidth(5);
        splitter->setChildrenCollapsible(false);
        frame_recentConfigs = new QFrame(splitter);
        frame_recentConfigs->setObjectName(QString::fromUtf8("frame_recentConfigs"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(frame_recentConfigs->sizePolicy().hasHeightForWidth());
        frame_recentConfigs->setSizePolicy(sizePolicy);
        frame_recentConfigs->setFrameShape(QFrame::NoFrame);
        frame_recentConfigs->setFrameShadow(QFrame::Plain);
        gridLayout_3 = new QGridLayout(frame_recentConfigs);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_3->setContentsMargins(0, 0, 0, 0);
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButton_openConfiguration = new QPushButton(frame_recentConfigs);
        pushButton_openConfiguration->setObjectName(QString::fromUtf8("pushButton_openConfiguration"));

        horizontalLayout->addWidget(pushButton_openConfiguration);

        pushButton_removeRecentConfigEntry = new QPushButton(frame_recentConfigs);
        pushButton_removeRecentConfigEntry->setObjectName(QString::fromUtf8("pushButton_removeRecentConfigEntry"));

        horizontalLayout->addWidget(pushButton_removeRecentConfigEntry);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_5);


        verticalLayout_3->addLayout(horizontalLayout);

        tableRecentConfigs = new QTableWidget(frame_recentConfigs);
        if (tableRecentConfigs->columnCount() < 1)
            tableRecentConfigs->setColumnCount(1);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        __qtablewidgetitem->setTextAlignment(Qt::AlignLeading|Qt::AlignBottom);
        tableRecentConfigs->setHorizontalHeaderItem(0, __qtablewidgetitem);
        tableRecentConfigs->setObjectName(QString::fromUtf8("tableRecentConfigs"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tableRecentConfigs->sizePolicy().hasHeightForWidth());
        tableRecentConfigs->setSizePolicy(sizePolicy1);
        tableRecentConfigs->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableRecentConfigs->setRowCount(0);
        tableRecentConfigs->setColumnCount(1);
        tableRecentConfigs->horizontalHeader()->setStretchLastSection(true);

        verticalLayout_3->addWidget(tableRecentConfigs);


        gridLayout_3->addLayout(verticalLayout_3, 0, 0, 1, 1);

        splitter->addWidget(frame_recentConfigs);
        tabWidget = new QTabWidget(splitter);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setEnabled(true);
        tabWidget->setElideMode(Qt::ElideNone);
        tabWidget->setDocumentMode(true);
        tabSettings = new QWidget();
        tabSettings->setObjectName(QString::fromUtf8("tabSettings"));
        tabSettings->setEnabled(true);
        tabSettings->setLayoutDirection(Qt::LeftToRight);
        gridLayout_2 = new QGridLayout(tabSettings);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, -1, 0, 0);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        labelConfigFilepath = new QLabel(tabSettings);
        labelConfigFilepath->setObjectName(QString::fromUtf8("labelConfigFilepath"));

        verticalLayout->addWidget(labelConfigFilepath);

        lineEdit_ConfigName = new QLineEdit(tabSettings);
        lineEdit_ConfigName->setObjectName(QString::fromUtf8("lineEdit_ConfigName"));

        verticalLayout->addWidget(lineEdit_ConfigName);

        line_tooltip1 = new QFrame(tabSettings);
        line_tooltip1->setObjectName(QString::fromUtf8("line_tooltip1"));
        line_tooltip1->setFrameShape(QFrame::HLine);
        line_tooltip1->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_tooltip1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setSizeConstraint(QLayout::SetDefaultConstraint);
        spinBox_max_responses = new QSpinBox(tabSettings);
        spinBox_max_responses->setObjectName(QString::fromUtf8("spinBox_max_responses"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(spinBox_max_responses->sizePolicy().hasHeightForWidth());
        spinBox_max_responses->setSizePolicy(sizePolicy2);
        spinBox_max_responses->setMinimumSize(QSize(80, 0));
        spinBox_max_responses->setMinimum(1);
        spinBox_max_responses->setMaximum(50);

        horizontalLayout_2->addWidget(spinBox_max_responses);

        label = new QLabel(tabSettings);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_2->addWidget(label);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        spinBox_max_indexation_threads = new QSpinBox(tabSettings);
        spinBox_max_indexation_threads->setObjectName(QString::fromUtf8("spinBox_max_indexation_threads"));
        spinBox_max_indexation_threads->setMinimumSize(QSize(80, 0));
        spinBox_max_indexation_threads->setMinimum(1);
        spinBox_max_indexation_threads->setMaximum(40);

        horizontalLayout_3->addWidget(spinBox_max_indexation_threads);

        label_2 = new QLabel(tabSettings);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_3->addWidget(label_2);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout_3);

        checkBox_auto_dump_index = new QCheckBox(tabSettings);
        checkBox_auto_dump_index->setObjectName(QString::fromUtf8("checkBox_auto_dump_index"));

        verticalLayout->addWidget(checkBox_auto_dump_index);

        checkBox_auto_load_index_dump = new QCheckBox(tabSettings);
        checkBox_auto_load_index_dump->setObjectName(QString::fromUtf8("checkBox_auto_load_index_dump"));

        verticalLayout->addWidget(checkBox_auto_load_index_dump);

        checkBox_auto_reindex = new QCheckBox(tabSettings);
        checkBox_auto_reindex->setObjectName(QString::fromUtf8("checkBox_auto_reindex"));

        verticalLayout->addWidget(checkBox_auto_reindex);

        checkBox_relative_to_config_folder = new QCheckBox(tabSettings);
        checkBox_relative_to_config_folder->setObjectName(QString::fromUtf8("checkBox_relative_to_config_folder"));

        verticalLayout->addWidget(checkBox_relative_to_config_folder);

        checkBox_use_independent_dicts_method = new QCheckBox(tabSettings);
        checkBox_use_independent_dicts_method->setObjectName(QString::fromUtf8("checkBox_use_independent_dicts_method"));

        verticalLayout->addWidget(checkBox_use_independent_dicts_method);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer);

        pushButton_applyConfigChanges = new QPushButton(tabSettings);
        pushButton_applyConfigChanges->setObjectName(QString::fromUtf8("pushButton_applyConfigChanges"));
        pushButton_applyConfigChanges->setEnabled(false);
        pushButton_applyConfigChanges->setCheckable(false);

        horizontalLayout_4->addWidget(pushButton_applyConfigChanges);

        pushButton_revertConfigChanges = new QPushButton(tabSettings);
        pushButton_revertConfigChanges->setObjectName(QString::fromUtf8("pushButton_revertConfigChanges"));
        pushButton_revertConfigChanges->setEnabled(false);

        horizontalLayout_4->addWidget(pushButton_revertConfigChanges);


        verticalLayout->addLayout(horizontalLayout_4);


        gridLayout_2->addLayout(verticalLayout, 0, 0, 1, 1);

        tabWidget->addTab(tabSettings, QString());
        tabDocuments = new QWidget();
        tabDocuments->setObjectName(QString::fromUtf8("tabDocuments"));
        horizontalLayout_8 = new QHBoxLayout(tabDocuments);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        horizontalLayout_8->setContentsMargins(0, -1, 0, 0);
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        toolButton_documentActions = new QToolButton(tabDocuments);
        toolButton_documentActions->setObjectName(QString::fromUtf8("toolButton_documentActions"));
        toolButton_documentActions->setEnabled(false);
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(toolButton_documentActions->sizePolicy().hasHeightForWidth());
        toolButton_documentActions->setSizePolicy(sizePolicy3);
        toolButton_documentActions->setMinimumSize(QSize(150, 0));
        toolButton_documentActions->setCheckable(false);
        toolButton_documentActions->setPopupMode(QToolButton::InstantPopup);
        toolButton_documentActions->setAutoRaise(false);

        horizontalLayout_5->addWidget(toolButton_documentActions);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_4);

        label_IndexationTooltip = new QLabel(tabDocuments);
        label_IndexationTooltip->setObjectName(QString::fromUtf8("label_IndexationTooltip"));

        horizontalLayout_5->addWidget(label_IndexationTooltip);

        pushButton_DumpIndex = new QPushButton(tabDocuments);
        pushButton_DumpIndex->setObjectName(QString::fromUtf8("pushButton_DumpIndex"));
        pushButton_DumpIndex->setEnabled(false);

        horizontalLayout_5->addWidget(pushButton_DumpIndex);

        pushButton_LoadIndex = new QPushButton(tabDocuments);
        pushButton_LoadIndex->setObjectName(QString::fromUtf8("pushButton_LoadIndex"));
        pushButton_LoadIndex->setEnabled(false);

        horizontalLayout_5->addWidget(pushButton_LoadIndex);


        verticalLayout_2->addLayout(horizontalLayout_5);

        tableDocuments = new QTableWidget(tabDocuments);
        if (tableDocuments->columnCount() < 5)
            tableDocuments->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableDocuments->setHorizontalHeaderItem(0, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableDocuments->setHorizontalHeaderItem(1, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableDocuments->setHorizontalHeaderItem(2, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tableDocuments->setHorizontalHeaderItem(3, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tableDocuments->setHorizontalHeaderItem(4, __qtablewidgetitem5);
        tableDocuments->setObjectName(QString::fromUtf8("tableDocuments"));
        tableDocuments->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableDocuments->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableDocuments->horizontalHeader()->setDefaultSectionSize(120);
        tableDocuments->horizontalHeader()->setStretchLastSection(true);

        verticalLayout_2->addWidget(tableDocuments);


        horizontalLayout_8->addLayout(verticalLayout_2);

        tabWidget->addTab(tabDocuments, QString());
        tabRequests = new QWidget();
        tabRequests->setObjectName(QString::fromUtf8("tabRequests"));
        gridLayout = new QGridLayout(tabRequests);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, -1, 0, 0);
        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        horizontalLayout_7->setSizeConstraint(QLayout::SetDefaultConstraint);
        lineEdit_request = new QLineEdit(tabRequests);
        lineEdit_request->setObjectName(QString::fromUtf8("lineEdit_request"));

        horizontalLayout_7->addWidget(lineEdit_request);

        pushButton_processRequests = new QPushButton(tabRequests);
        pushButton_processRequests->setObjectName(QString::fromUtf8("pushButton_processRequests"));
        pushButton_processRequests->setEnabled(false);

        horizontalLayout_7->addWidget(pushButton_processRequests);


        verticalLayout_5->addLayout(horizontalLayout_7);

        splitter_3 = new QSplitter(tabRequests);
        splitter_3->setObjectName(QString::fromUtf8("splitter_3"));
        sizePolicy.setHeightForWidth(splitter_3->sizePolicy().hasHeightForWidth());
        splitter_3->setSizePolicy(sizePolicy);
        splitter_3->setOrientation(Qt::Horizontal);
        splitter_3->setChildrenCollapsible(false);
        tableRequestsStory = new QTableWidget(splitter_3);
        if (tableRequestsStory->columnCount() < 1)
            tableRequestsStory->setColumnCount(1);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        __qtablewidgetitem6->setTextAlignment(Qt::AlignLeading|Qt::AlignBottom);
        tableRequestsStory->setHorizontalHeaderItem(0, __qtablewidgetitem6);
        tableRequestsStory->setObjectName(QString::fromUtf8("tableRequestsStory"));
        tableRequestsStory->setEnabled(true);
        sizePolicy.setHeightForWidth(tableRequestsStory->sizePolicy().hasHeightForWidth());
        tableRequestsStory->setSizePolicy(sizePolicy);
        tableRequestsStory->setMinimumSize(QSize(100, 0));
        tableRequestsStory->setBaseSize(QSize(100, 0));
        tableRequestsStory->setEditTriggers(QAbstractItemView::NoEditTriggers);
        splitter_3->addWidget(tableRequestsStory);
        tableRequestsStory->horizontalHeader()->setStretchLastSection(true);
        verticalLayoutWidget_2 = new QWidget(splitter_3);
        verticalLayoutWidget_2->setObjectName(QString::fromUtf8("verticalLayoutWidget_2"));
        verticalLayout_4 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_requestDate = new QLabel(verticalLayoutWidget_2);
        label_requestDate->setObjectName(QString::fromUtf8("label_requestDate"));

        horizontalLayout_6->addWidget(label_requestDate);


        verticalLayout_4->addLayout(horizontalLayout_6);

        tableAnswers = new QTableWidget(verticalLayoutWidget_2);
        if (tableAnswers->columnCount() < 2)
            tableAnswers->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tableAnswers->setHorizontalHeaderItem(0, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        tableAnswers->setHorizontalHeaderItem(1, __qtablewidgetitem8);
        tableAnswers->setObjectName(QString::fromUtf8("tableAnswers"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy4.setHorizontalStretch(1);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(tableAnswers->sizePolicy().hasHeightForWidth());
        tableAnswers->setSizePolicy(sizePolicy4);
        tableAnswers->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableAnswers->horizontalHeader()->setDefaultSectionSize(100);
        tableAnswers->horizontalHeader()->setStretchLastSection(true);

        verticalLayout_4->addWidget(tableAnswers);

        splitter_3->addWidget(verticalLayoutWidget_2);

        verticalLayout_5->addWidget(splitter_3);


        gridLayout->addLayout(verticalLayout_5, 0, 0, 1, 1);

        tabWidget->addTab(tabRequests, QString());
        splitter->addWidget(tabWidget);

        gridLayout_4->addWidget(splitter, 0, 0, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1013, 21));
        menuSettins = new QMenu(menubar);
        menuSettins->setObjectName(QString::fromUtf8("menuSettins"));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuSettins->menuAction());
        menuSettins->addAction(actionShow_recent_configurations);
        menuSettins->addAction(actionShow_recent_requests);
        menuSettins->addAction(actionShow_tooltip);
        menuSettins->addAction(actionDisplay_full_filenames);
        menuFile->addAction(actionNew_configuration);
        menuFile->addAction(actionOpen_configuration);
        menuFile->addAction(actionSave_as);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);

        retranslateUi(MainWindow);
        QObject::connect(actionShow_recent_configurations, SIGNAL(toggled(bool)), frame_recentConfigs, SLOT(setVisible(bool)));
        QObject::connect(actionShow_recent_requests, SIGNAL(toggled(bool)), tableRequestsStory, SLOT(setVisible(bool)));
        QObject::connect(lineEdit_ConfigName, SIGNAL(editingFinished()), MainWindow, SLOT(enableConfigChangesControls()));
        QObject::connect(spinBox_max_responses, SIGNAL(editingFinished()), MainWindow, SLOT(enableConfigChangesControls()));
        QObject::connect(spinBox_max_indexation_threads, SIGNAL(editingFinished()), MainWindow, SLOT(enableConfigChangesControls()));
        QObject::connect(checkBox_auto_dump_index, SIGNAL(clicked()), MainWindow, SLOT(enableConfigChangesControls()));
        QObject::connect(checkBox_auto_load_index_dump, SIGNAL(clicked()), MainWindow, SLOT(enableConfigChangesControls()));
        QObject::connect(checkBox_auto_reindex, SIGNAL(clicked()), MainWindow, SLOT(enableConfigChangesControls()));
        QObject::connect(checkBox_relative_to_config_folder, SIGNAL(clicked()), MainWindow, SLOT(enableConfigChangesControls()));
        QObject::connect(pushButton_applyConfigChanges, SIGNAL(clicked()), MainWindow, SLOT(applyConfigChanges()));
        QObject::connect(pushButton_revertConfigChanges, SIGNAL(clicked()), MainWindow, SLOT(revertConfigChanges()));
        QObject::connect(pushButton_removeRecentConfigEntry, SIGNAL(clicked()), MainWindow, SLOT(removeRecentConfigEntry()));
        QObject::connect(actionNew_configuration, SIGNAL(triggered()), MainWindow, SLOT(newConfig()));
        QObject::connect(actionExit, SIGNAL(triggered()), MainWindow, SLOT(close()));
        QObject::connect(actionOpen_configuration, SIGNAL(triggered()), MainWindow, SLOT(selectAndOpenConfig()));
        QObject::connect(actionSave_as, SIGNAL(triggered()), MainWindow, SLOT(saveConfigAs()));
        QObject::connect(pushButton_processRequests, SIGNAL(clicked()), MainWindow, SLOT(processRequest()));
        QObject::connect(lineEdit_request, SIGNAL(returnPressed()), MainWindow, SLOT(processRequest()));
        QObject::connect(checkBox_use_independent_dicts_method, SIGNAL(clicked()), MainWindow, SLOT(enableConfigChangesControls()));
        QObject::connect(pushButton_openConfiguration, SIGNAL(clicked()), MainWindow, SLOT(openRecentConfig()));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Skillbox search engine", nullptr));
        actionShow_recent_configurations->setText(QCoreApplication::translate("MainWindow", "Show recent configurations", nullptr));
        actionShow_recent_requests->setText(QCoreApplication::translate("MainWindow", "Show recent requests", nullptr));
        actionOpen_configuration->setText(QCoreApplication::translate("MainWindow", "Open configuration", nullptr));
        actionSave_as->setText(QCoreApplication::translate("MainWindow", "Save configuration as...", nullptr));
        actionShow_console_messages->setText(QCoreApplication::translate("MainWindow", "Show console messages", nullptr));
        actionShow_tooltips->setText(QCoreApplication::translate("MainWindow", "Show tooltips", nullptr));
        actionNew_configuration->setText(QCoreApplication::translate("MainWindow", "New configuration", nullptr));
        actionExit->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
        actionDisplay_full_filenames->setText(QCoreApplication::translate("MainWindow", "Display full filenames", nullptr));
        actionShow_tooltip->setText(QCoreApplication::translate("MainWindow", "Show tooltips", nullptr));
        pushButton_openConfiguration->setText(QCoreApplication::translate("MainWindow", "Open configuration", nullptr));
        pushButton_removeRecentConfigEntry->setText(QCoreApplication::translate("MainWindow", "Remove entry", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableRecentConfigs->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "Recent configurations", nullptr));
        labelConfigFilepath->setText(QCoreApplication::translate("MainWindow", "Create new or open existing configuration to start working", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Amount of search results to be written to \"answers.json\" file after request processed", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Maximum amount of threads to use for files indexation", nullptr));
        checkBox_auto_dump_index->setText(QCoreApplication::translate("MainWindow", "Save index automatically at end of work with configuration", nullptr));
        checkBox_auto_load_index_dump->setText(QCoreApplication::translate("MainWindow", "Load previously saved index at configuration startup, if one exists", nullptr));
        checkBox_auto_reindex->setText(QCoreApplication::translate("MainWindow", "Start full indexation automatically on configuration startup, if none files in base are indexed", nullptr));
        checkBox_relative_to_config_folder->setText(QCoreApplication::translate("MainWindow", "Relative file paths, listed in \"files\" section of  configuration file, are relative to configuration file directory", nullptr));
        checkBox_use_independent_dicts_method->setText(QCoreApplication::translate("MainWindow", "Use an alternative indexing method: higher memory consumption, but the more indexing threads, the higher the performance.", nullptr));
        pushButton_applyConfigChanges->setText(QCoreApplication::translate("MainWindow", "Apply", nullptr));
        pushButton_revertConfigChanges->setText(QCoreApplication::translate("MainWindow", "Revert", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabSettings), QCoreApplication::translate("MainWindow", "Settings", nullptr));
        toolButton_documentActions->setText(QCoreApplication::translate("MainWindow", "Document actions...", nullptr));
        label_IndexationTooltip->setText(QCoreApplication::translate("MainWindow", "|", nullptr));
        pushButton_DumpIndex->setText(QCoreApplication::translate("MainWindow", "Dump index", nullptr));
        pushButton_LoadIndex->setText(QCoreApplication::translate("MainWindow", "Load index", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableDocuments->horizontalHeaderItem(0);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "Filename", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableDocuments->horizontalHeaderItem(1);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "Ind.", "Indexed"));
#if QT_CONFIG(tooltip)
        ___qtablewidgetitem2->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Document have been indexed</p></body></html>", "Indexed"));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        ___qtablewidgetitem2->setWhatsThis(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Document have been indexed</p></body></html>", "Indexed"));
#endif // QT_CONFIG(whatsthis)
        QTableWidgetItem *___qtablewidgetitem3 = tableDocuments->horizontalHeaderItem(2);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "Ind. on.", "Indexation ongoing"));
#if QT_CONFIG(tooltip)
        ___qtablewidgetitem3->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Indexation of this document is in progress</p></body></html>", "Indexation ongoing"));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        ___qtablewidgetitem3->setWhatsThis(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Indexation of this document is in progress</p></body></html>", "Indexation ongoing"));
#endif // QT_CONFIG(whatsthis)
        QTableWidgetItem *___qtablewidgetitem4 = tableDocuments->horizontalHeaderItem(3);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow", "Index date", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tableDocuments->horizontalHeaderItem(4);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("MainWindow", "Error description", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabDocuments), QCoreApplication::translate("MainWindow", "Documents", nullptr));
        lineEdit_request->setPlaceholderText(QCoreApplication::translate("MainWindow", "Enter request", nullptr));
        pushButton_processRequests->setText(QCoreApplication::translate("MainWindow", "Process", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = tableRequestsStory->horizontalHeaderItem(0);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("MainWindow", "Recent requests", nullptr));
        label_requestDate->setText(QCoreApplication::translate("MainWindow", "Request processed at ", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = tableAnswers->horizontalHeaderItem(0);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("MainWindow", "Document", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = tableAnswers->horizontalHeaderItem(1);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("MainWindow", "Relative index", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabRequests), QCoreApplication::translate("MainWindow", "Search requests", nullptr));
        menuSettins->setTitle(QCoreApplication::translate("MainWindow", "Settings", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // GUIVNGZKA_H
