#include "mainwindow.h"


void MainWindow::on_toolButton_components_clicked()
{
    QStringList list;
    QString str;
    list << "FORMAT:";
    list << "";
    list << "id\ttype\tfield\tparam1\tparam2";
    list << "";
    list << "id: component identifier; ex: \"am\" \"probe1\". Cannot match type specifier.";
    list << "";
    list << "type: component type (AM, PROBE, MASK, ABSORBER, LENS or WINDOW). Case sensitive.";
    list << "";
    list << "field: max radial field in cm";
    list << "";
    list << "param1 and param2: additional parameters:";
    list << "AM: active medium section length in cm";
    list << "PROBE: no parameters";
    list << "MASK: mask radius in cm";
    list << "ABSORBER: transmittance";
    list << "LENS: focal length in cm";
    list << "WINDOW: param1 - material (CdTe, GaAs, Ge, KCl, NaCl, Si or ZnSe); param2 - thickness in cm";
    list << "STRETCHER: pulse chirping factor in ps/THz (positive for red chirp)";
    str = list.join("\n");
    QMessageBox mb( "Hint - co2amp", str, QMessageBox::Information, QMessageBox::Ok, 0, 0);
    mb.exec();
}

void MainWindow::on_toolButton_layout_clicked()
{
    QStringList list;
    QString str;
    list << "FORMAT:";
    list << "";
    list << "id1 - dist1 - id2 - dist2 - ... - id_n";
    list << "";
    list << "id: component identifier";
    list << "dist: propagation distance in cm";
    str = list.join("\n");
    QMessageBox mb( "Hint - co2amp", str, QMessageBox::Information, QMessageBox::Ok, 0, 0);
    mb.exec();
}

void MainWindow::on_toolButton_discharge_clicked()
{
    QStringList list;
    QString str;
    list << "DISCHARGE PROFILE FORMAT:";
    list << "";
    list << "t0[us]   I0[A]   U0[V]";
    list << "t0[us]   I1[A]   U1[V]";
    list << "t1[us]   I2[A]   U2[V]";
    list << "...";
    str = list.join("\n");
    QMessageBox mb( "Hint - co2amp", str, QMessageBox::Information, QMessageBox::Ok, 0, 0);
    mb.exec();
}
