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
        bool flag_plot_postponed_modified;
        bool flag_comments_modified;
        bool flag_input_file_error;
        //bool noam; // no amplifier optics
        QElapsedTimer timer;
        CoreVariables Saved, Memorized;
        QShortcut *keyF8;
        void LoadProject();
        int FigureMenu();

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
        void on_comboBox_optic_activated(QString);
        void on_comboBox_pulse_activated(QString);
        void on_lineEdit_passes_textEdited(QString str);
        void on_lineEdit_passes_returnPressed();
        void on_comboBox_energyPlot_activated(QString);
        void on_checkBox_log_clicked();
        void on_comboBox_timeScale_activated(QString);
        void on_comboBox_freqScale_activated(QString);
        void on_spinBox_width_valueChanged(int);
        void on_spinBox_width_editingFinished();
        void on_spinBox_height_valueChanged(int);
        void on_spinBox_height_editingFinished();
        void on_doubleSpinBox_zoom_valueChanged(double);
        void on_doubleSpinBox_zoom_editingFinished();
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
        void ClearPlot();
        void Comments();
        void SelectEnergies();
        void CopyMultipassData(QString filename);
        void CopyPixmap(QSvgWidget *svg);
        void SaveSVG(QString svg_path);
        void UpdateTerminal();
        void UpdateControls();
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


/*class StringListModel : public QAbstractListModel
{
    Q_OBJECT

    public:
        StringListModel(const QStringList &strings, QObject *parent = nullptr): QAbstractListModel(parent), stringList(strings) {}
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        bool setData(const QModelIndex &index, const QVariant &value,int role = Qt::EditRole) override;
        bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
        bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

    private:
        QStringList stringList;
};*/

#endif
