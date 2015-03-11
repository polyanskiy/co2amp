#include "mainwindow.h"


void MainWindow::on_toolButton_components_clicked()
{
    QStringList list;
    QString str;
    list << "<p>FORMAT: \"<code><b>id  type  field  param1  param2</b></code>\"</p>";
    list << "<p><code><b>id</b></code>: component identifier<br>";
    list << "&nbsp; &nbsp; &nbsp; example: \"<code>am</code>\" \"<code>probe1</code>\"</p>";
    list << "<p><code><b>type</b></code>: component type<br>";
    list << "&nbsp; &nbsp; &nbsp; <code>AM</code>, <code>PROBE</code>, <code>MASK</code>, <code>ABSORBER</code>, <code>LENS</code>, <code>WINDOW</code>, <code>STRETCER</code>, <code>BANDPASS</code></p>";
    list << "<p><code><b>field</b></code>: max radial field in cm</p>";
    list << "<p><code><b>param1</b></code> and <code><b>param2</b></code>: type-specific parameters<br>";
    list << "&nbsp; &nbsp; &nbsp; <code>AM</code>: length in cm<br>";
    list << "&nbsp; &nbsp; &nbsp; <code>PROBE</code>: no parameters<br>";
    list << "&nbsp; &nbsp; &nbsp; <code>MASK</code>: radius in cm<br>";
    list << "&nbsp; &nbsp; &nbsp; <code>ABSORBER</code>: transmittance<br>";
    list << "&nbsp; &nbsp; &nbsp; <code>LENS</code>: focal length in cm<br>";
    list << "&nbsp; &nbsp; &nbsp; <code>WINDOW</code>: material (<code>CdTe</code>, <code>GaAs</code>, <code>Ge</code>, <code>KCl</code>, <code>NaCl</code>, <code>Si</code>, <code>ZnSe</code>) and thickness in cm<br>";
    list << "&nbsp; &nbsp; &nbsp; <code>STRETCHER</code>: pulse chirping factor in ps/THz, positive for red chirp<br>";
    list << "&nbsp; &nbsp; &nbsp; <code>BANDPASS</code>: band center and bandwidth in THz</p>";
    str = "<qt>"+list.join("")+"</qt>";
    QMessageBox mb( "Hint - co2amp", str, QMessageBox::Information, QMessageBox::Ok, 0, 0);
    mb.exec();
}

void MainWindow::on_toolButton_layout_clicked()
{
    QStringList list;
    QString str;
    list << "<p>FORMAT: \"<code><b>id<sub>1</sub>-dist<sub>1</sub>-id<sub>2</sub>-dist<sub>2</sub>-...-dist<sub>n-1</sub>-id<sub>n</sub></b></code>\"</p>";
    list << "<p><code><b>id</b></code>: component identifier</p>";
    list << "<p><code><b>dist</b></code>: propagation distance in cm</p>";
    str = "<qt>"+list.join("")+"</qt>";
    QMessageBox mb( "Hint - co2amp", str, QMessageBox::Information, QMessageBox::Ok, 0, 0);
    mb.exec();
}

void MainWindow::on_toolButton_discharge_clicked()
{
    QStringList list;
    QString str;
    list << "<p>DISCHARGE PROFILE FORMAT:</p>";
    list << "<p><code>";
    list << "t<sub>0</sub>[&mu;s]   I<sub>0</sub>[A]   U<sub>0</sub>[V]<br>";
    list << "t<sub>1</sub>[&mu;s]   I<sub>1</sub>[A]   U<sub>1</sub>[V]<br>";
    list << "t<sub>2</sub>[&mu;s]   I<sub>2</sub>[A]   U<sub>2</sub>[V]<br>";
    list << "...</code></p>";
    str = "<qt>"+list.join("")+"</qt>";
    QMessageBox mb( "Hint - co2amp", str, QMessageBox::Information, QMessageBox::Ok, 0, 0);
    mb.exec();
}
