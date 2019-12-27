#include "co2am+.h"

void MainWindow::SetAboutText()
{  
    QString co2amplus_version = "2019-11-19";

    // get co2amp version string
    process = new QProcess(this);
    process->start("\"" + path_to_co2amp + "\" -version");
    process->waitForFinished();
    QString co2amp_version = process->readAllStandardOutput();
    delete process;

    QString path_to_manual =
            QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/doc/co2amp.pdf");

    QString about =
            "<b>Versions:</b><br>"
            "<code>"
            "co2amp: v." + co2amp_version + "<br>"
            "co2am+: v." + co2amplus_version + "<br>"
            "</code>"
            "<br>"
            "<b>Documentation:</b> <a href=\"file:///" + path_to_manual + "\">Manual (PDF)</a><br>"
            "<br>"
            "<b>License:</b> <a href=\"https://gnu.org/licenses/gpl.html\">GPL</a><br>"
            "<br>"
            "<b>Authors:</b>"
            "<ul>"
            "<li><a href=\"mailto:Mikhail Polyanskiy &lt;polyanskiy@bnl.gov&gt;\">"
            "Mikhail N. &quot;Misha&quot; Polyanskiy</a>, Brookhaven National Laboratory, USA</li>"
            "<li>Early versions of <code>co2amp</code> were based on a Mathcad code written by<br>"
            "Viktor T. Platonenko, Moscow State University, Russia</li>"
            "</ul>"
            "<b>Citation:</b><br>"
            "M. N. Polyanskiy. <code>co2amp</code>: A software program for modeling the dynamics of ultrashort<br>"
            "pulses in optical systems with CO<sub>2</sub> amplifiers, "
            "<a href=\"https://doi.org/10.1364/AO.54.005136\"><i>Appl. Opt.</i> <b>54</b>, 5136</a> (2015)";

    label_about->setText(about);
}
