#ifndef MAINDIALOGIMPL_H
#define MAINDIALOGIMPL_H

#include <QMainWindow>
#include <QProcess>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QTextStream>
#include <QString>
#include <QFileInfo>
#include <QCloseEvent>
#include <QMenu>
#include <QClipboard>
#include <QElapsedTimer>
#include <QSvgWidget>
#include <QShortcut>
#include <QSyntaxHighlighter>
#include <QStandardPaths>
#include <cmath>
#include "ui_mainwindow.h"


class CoreVariables
{
    public:
        QStringList configFile_id, configFile_type, configFile_content;
        bool noprop;
        QString vc, t_min, t_max, time_tick;
        int precision_t, precision_r;
        int optic, pulse;
};


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
        QTextCharFormat commentFormat;
        QTextCharFormat numberFormat;
        QTextCharFormat wordFormat;
        QTextCharFormat boolFormat;
        QTextCharFormat tabulatedFormat;
        QTextCharFormat wrongtabFormatFormat;
        QTextCharFormat streamFormat;
        QTextCharFormat quotationFormat;
};


class MainWindow : public QMainWindow, public Ui::MainWindowClass
{
    Q_OBJECT
    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();
        int version;
        QString work_dir;
        QString project_file;
        QString def_dir;
        QString yaml_dir;
        QString path_to_core, path_to_gnuplot, path_to_7zip;
        QProcess *process;
        bool flag_calculating;
        bool flag_calculation_success;
        bool flag_field_ready_to_save; // can save .co2x file
        bool flag_projectloaded;
        bool flag_results_modified;
        bool flag_plot_modified;
        bool flag_plot_postponed;
        bool flag_input_file_error;
        CoreVariables Saved, Memorized;
        QShortcut *F6;
        void LoadProject();
        int FigureMenu();

    private:
        YamlHighlighter *highlighter;

    private slots:
        void on_pushButton_new_clicked();
        void on_pushButton_open_clicked();
        void on_pushButton_saveas_clicked();
        void on_toolButton_configFile_add_clicked();
        void on_toolButton_configFile_up_clicked();
        void on_toolButton_configFile_down_clicked();
        void on_toolButton_configFile_rename_clicked();
        void on_toolButton_configFile_remove_clicked();
        void on_listWidget_configFile_list_currentRowChanged(int);
        void on_plainTextEdit_configFile_content_textChanged();
        void on_pushButton_configFile_load_clicked();
        void on_pushButton_configFile_save_clicked();
        void closeEvent(QCloseEvent*);
        void on_svg_fig1_customContextMenuRequested();
        void on_svg_fig2_customContextMenuRequested();
        void on_svg_fig3_customContextMenuRequested();
        void on_svg_fig4_customContextMenuRequested();
        void on_svg_fig5_customContextMenuRequested();
        void on_svg_fig6_customContextMenuRequested();
        void on_svg_fig7_customContextMenuRequested();
        void on_svg_fig8_customContextMenuRequested();
        void on_svg_fig9_customContextMenuRequested();
        void on_tabWidget_main_currentChanged(int tab);
        void SaveSettings(QString what_to_save); //what_to_save: "all" - input and plot settings; "plot" - plot settings only
        void MemorizeSettings();
        void SaveProject();
        void LoadSettings(QString);
        void PopulateConfigFileList();
        void NewProject();
        void ClearWorkDir();
        void Calculate();
        void Abort();
        void Plot();
        void FlagModifiedAndPlot();
        void PostponePlot();
        void FlagModifiedAndPostponePlot();
        void PlotIfPostponed();
        void ClearPlot();
        void SelectEnergies();
        void CopyMultipassData(QString filename);
        void CopyPixmap(QSvgWidget *svg);
        void SaveSVG(QString svg_path);
        void UpdateTerminal();
        void UpdateControls();
        void YamlFixFormat();
        QString SuggestConfigFileName(QString type);
        bool ConfigFileNameExists(QString ID);
        void BeforeProcessStarted();
        void AfterProcessFinished();
        void OnModified();
        void LoadInputPulse();
        int PassNumber(int);
        int DatasetNumber(int pulse_n, int optic_n, int pass_n, QString filename);
        int AmNumber(int);
        bool SaveBeforeClose();
        void BlockSignals(bool block);
};




#endif
