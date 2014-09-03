/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created: Wed Aug 10 09:07:47 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      45,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,
      46,   11,   11,   11, 0x08,
      76,   11,   11,   11, 0x08,
     107,   11,   11,   11, 0x08,
     142,   11,   11,   11, 0x08,
     173,   11,   11,   11, 0x08,
     208,   11,   11,   11, 0x08,
     246,   11,   11,   11, 0x08,
     274,   11,   11,   11, 0x08,
     303,   11,   11,   11, 0x08,
     334,   11,   11,   11, 0x08,
     368,   11,   11,   11, 0x08,
     393,   11,   11,   11, 0x08,
     436,   11,   11,   11, 0x08,
     479,   11,   11,   11, 0x08,
     522,   11,   11,   11, 0x08,
     565,   11,   11,   11, 0x08,
     608,   11,   11,   11, 0x08,
     651,   11,   11,   11, 0x08,
     694,   11,   11,   11, 0x08,
     737,   11,   11,   11, 0x08,
     790,  780,   11,   11, 0x08,
     809,   11,   11,   11, 0x08,
     823,   11,   11,   11, 0x08,
     845,   11,   11,   11, 0x08,
     858,   11,   11,   11, 0x08,
     873,   11,   11,   11, 0x08,
     885,   11,   11,   11, 0x08,
     892,   11,   11,   11, 0x08,
     909,   11,   11,   11, 0x08,
     932,  923,   11,   11, 0x08,
     959,   11,   11,   11, 0x08,
     976,   11,   11,   11, 0x08,
     993,   11,   11,   11, 0x08,
    1014,   11,   11,   11, 0x08,
    1037,   11,   11,   11, 0x08,
    1060,   11,   11,   11, 0x08,
    1073,   11,   11,   11, 0x08,
    1096,   11,   11,   11, 0x08,
    1116,   11,   11,   11, 0x08,
    1136,   11,   11,   11, 0x08,
    1157,   11, 1153,   11, 0x08,
    1198, 1176, 1153,   11, 0x08,
    1225,   11, 1153,   11, 0x08,
    1244,   11, 1239,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0on_pushButton_calculate_clicked()\0"
    "on_pushButton_abort_clicked()\0"
    "on_pushButton_update_clicked()\0"
    "on_toolButton_input_file_clicked()\0"
    "on_toolButton_layout_clicked()\0"
    "on_toolButton_components_clicked()\0"
    "on_tabWidget_main_currentChanged(int)\0"
    "on_pushButton_new_clicked()\0"
    "on_pushButton_open_clicked()\0"
    "on_pushButton_saveas_clicked()\0"
    "on_pushButton_discharge_clicked()\0"
    "closeEvent(QCloseEvent*)\0"
    "on_label_fig1_customContextMenuRequested()\0"
    "on_label_fig2_customContextMenuRequested()\0"
    "on_label_fig3_customContextMenuRequested()\0"
    "on_label_fig4_customContextMenuRequested()\0"
    "on_label_fig5_customContextMenuRequested()\0"
    "on_label_fig6_customContextMenuRequested()\0"
    "on_label_fig7_customContextMenuRequested()\0"
    "on_label_fig8_customContextMenuRequested()\0"
    "on_label_fig9_customContextMenuRequested()\0"
    "plot_only\0SaveSettings(bool)\0SaveProject()\0"
    "LoadSettings(QString)\0NewProject()\0"
    "ClearWorkDir()\0Calculate()\0Plot()\0"
    "SelectEnergies()\0ShowFigures()\0filename\0"
    "CopyMultipassData(QString)\0UpdateTerminal()\0"
    "UpdateControls()\0PopulateComboboxes()\0"
    "BeforeProcessStarted()\0AfterProcessFinished()\0"
    "OnModified()\0OnComponentsModified()\0"
    "DisplayedToMemory()\0MemoryToDisplayed()\0"
    "LoadInputPulse()\0int\0GetPassNumber(int)\0"
    "pulse_n,comp_n,pass_n\0DatasetNumber(int,int,int)\0"
    "AmNumber(int)\0bool\0SaveBeforeClose()\0"
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    if (!strcmp(_clname, "Ui::MainWindowClass"))
        return static_cast< Ui::MainWindowClass*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_pushButton_calculate_clicked(); break;
        case 1: on_pushButton_abort_clicked(); break;
        case 2: on_pushButton_update_clicked(); break;
        case 3: on_toolButton_input_file_clicked(); break;
        case 4: on_toolButton_layout_clicked(); break;
        case 5: on_toolButton_components_clicked(); break;
        case 6: on_tabWidget_main_currentChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: on_pushButton_new_clicked(); break;
        case 8: on_pushButton_open_clicked(); break;
        case 9: on_pushButton_saveas_clicked(); break;
        case 10: on_pushButton_discharge_clicked(); break;
        case 11: closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        case 12: on_label_fig1_customContextMenuRequested(); break;
        case 13: on_label_fig2_customContextMenuRequested(); break;
        case 14: on_label_fig3_customContextMenuRequested(); break;
        case 15: on_label_fig4_customContextMenuRequested(); break;
        case 16: on_label_fig5_customContextMenuRequested(); break;
        case 17: on_label_fig6_customContextMenuRequested(); break;
        case 18: on_label_fig7_customContextMenuRequested(); break;
        case 19: on_label_fig8_customContextMenuRequested(); break;
        case 20: on_label_fig9_customContextMenuRequested(); break;
        case 21: SaveSettings((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 22: SaveProject(); break;
        case 23: LoadSettings((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 24: NewProject(); break;
        case 25: ClearWorkDir(); break;
        case 26: Calculate(); break;
        case 27: Plot(); break;
        case 28: SelectEnergies(); break;
        case 29: ShowFigures(); break;
        case 30: CopyMultipassData((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 31: UpdateTerminal(); break;
        case 32: UpdateControls(); break;
        case 33: PopulateComboboxes(); break;
        case 34: BeforeProcessStarted(); break;
        case 35: AfterProcessFinished(); break;
        case 36: OnModified(); break;
        case 37: OnComponentsModified(); break;
        case 38: DisplayedToMemory(); break;
        case 39: MemoryToDisplayed(); break;
        case 40: LoadInputPulse(); break;
        case 41: { int _r = GetPassNumber((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 42: { int _r = DatasetNumber((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 43: { int _r = AmNumber((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 44: { bool _r = SaveBeforeClose();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
        _id -= 45;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
