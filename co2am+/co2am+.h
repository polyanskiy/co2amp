#ifndef MAINDIALOGIMPL_H
#define MAINDIALOGIMPL_H

#include <QClipboard>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QShortcut>
#include <QStandardPaths>
#include <QString>
#include <QSvgWidget>
#include <QSyntaxHighlighter>
#include <QTextStream>

#include <cmath>
#include "ui_mainwindow.h"


class YamlHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

    public:
        YamlHighlighter(QTextDocument *parent = 0);

    protected:
        void highlightBlock(const QString &text) override;

    private:
        struct HighlightingRule
        {
            QRegularExpression pattern;
            QTextCharFormat format;
        };
        QVector<HighlightingRule> highlightingRules;

        QTextCharFormat keyFormat;
        QTextCharFormat specialKeyFormat;
        QTextCharFormat commentFormat;
        QTextCharFormat numberFormat;
        QTextCharFormat wordFormat;
        QTextCharFormat boolFormat;
        QTextCharFormat tabulatedFormat;
        QTextCharFormat wrongTabFormatFormat;
        QTextCharFormat streamFormat;
        QTextCharFormat quotationFormat;
};


class MainWindow : public QMainWindow, public Ui::MainWindowClass
{
    Q_OBJECT
    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();
        float formatVersion;
        QString work_dir;
        QString project_file;
        QString def_dir;
        QString yaml_dir;
        QString path_to_co2amp;
        QString path_to_gnuplot;
        QString path_to_7zip;
        QStringList configFile_id;
        QStringList configFile_type;
        QStringList configFile_content;
        QProcess *process;
        bool flag_project_modified;
        bool flag_calculating;
        bool flag_plot_postponed;
        bool always_ok_to_invalidate;
        QShortcut *F6;
        int tmp_precision_t, tmp_precision_r, tmp_save_interval, tmp_method;

    private:
        YamlHighlighter *highlighter;

    private slots:
        // project.cpp
        void SaveProject();
        void LoadProject();
        void NewProject();
        void ClearWorkDir();
        void CopyHitranFilesToWorkDir();
        bool SaveBeforeClose();
        void InvalidateResults();
        bool CalcResultsExist();
        bool OkToInvalidate();
        // environment.cpp
        void FindExternalPrograms();
        // config.cpp
        void UpdateConfigurationFiles();
        void ReadConfigurationFiles();
        // calc.cpp
        void Calculate();
        void Abort();
        void BeforeProcessStarted();
        void AfterProcessFinished();
        // mainwindow.cpp
        void on_pushButton_new_clicked();
        void on_pushButton_open_clicked();
        void on_pushButton_saveas_clicked();
        void on_pushButton_savePulse_clicked();
        void on_tabWidget_main_currentChanged(int tab);
        void closeEvent(QCloseEvent*);
        // tab0-config.cpp
        void on_toolButton_configFile_add_clicked();
        void on_toolButton_configFile_up_clicked();
        void on_toolButton_configFile_down_clicked();
        void on_toolButton_configFile_rename_clicked();
        void on_toolButton_configFile_remove_clicked();
        void on_listWidget_configFile_list_currentRowChanged(int);
        void on_plainTextEdit_configFile_content_textChanged();
        void on_pushButton_configFile_load_clicked();
        void on_pushButton_configFile_save_clicked();
        void PopulateConfigFileList();
        QString SuggestConfigFileName(QString type);
        bool ConfigFileNameExists(QString ID);
        // tab1-process.cpp
        void WriteToTerminal();
        void on_lineEdit_v0_textEdited(QString);
        void on_lineEdit_t_min_textEdited(QString);
        void on_lineEdit_t_max_textEdited(QString);
        void on_lineEdit_time_tick_textEdited(QString);
        void on_spinBox_save_interval_valueChanged(int);
        void on_comboBox_precision_t_activated(int);
        void on_comboBox_precision_r_activated(int);
        void on_comboBox_method_activated(int);
        // tab2-plot.cpp
        void Plot();
        void ClearPlot();
        void FlagModifiedAndPlot();
        void PostponePlot();
        void FlagModifiedAndPostponePlot();
        void PlotIfPostponed();
        // tab3-comments.cpp
        void on_plainTextEdit_comments_textChanged();
        // tab4-about.cpp
        void SetAboutText();
        // clipboard.cpp
        int  FigureMenu();
        void on_svg_fig1_customContextMenuRequested();
        void on_svg_fig2_customContextMenuRequested();
        void on_svg_fig3_customContextMenuRequested();
        void on_svg_fig4_customContextMenuRequested();
        void on_svg_fig5_customContextMenuRequested();
        void on_svg_fig6_customContextMenuRequested();
        void on_svg_fig7_customContextMenuRequested();
        void on_svg_fig8_customContextMenuRequested();
        void on_svg_fig9_customContextMenuRequested();
        void CopyMultipassData(QString longext);
        void CopyDataFromFile(QString filename);
        void CopyPixmap(QSvgWidget *svg);
        void SaveSVG(QString svg_path);
        // update.cpp
        void Update();
        // yaml.cpp
        void YamlFixFormat();
        // data.cpp
        QString Type(QString);
        int PassNumber(int);
        //int AmNumber(int);
        void SelectEnergies();
};

#endif
