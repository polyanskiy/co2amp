#include "co2amp.h"

void MainWindow::SetAboutText()
{
    QString path_to_manual = QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/doc/co2amp.pdf");

    QString about =
            "<b>Version:</b> " + co2ampVersion + "<br>"
            "<br>"
            "<b>Documentation:</b> <a href=\"" + path_to_manual + "\">Manual</a><br>"
            "<br>"
            "<b>License:</b> <a href=\"https://gnu.org/licenses/gpl.html\">GPL</a><br>"
            "<br>"
            "<b>Authors:</b><br>"
            "<a href=\"mailto:Mikhail Polyanskiy &lt;polyanskiy@bnl.gov&gt;\">"
            "Mikhail N. &quot;Misha&quot; Polyanskiy</a>, Brookhaven National Laboratory, USA""<br>"
            "Early versions of <code>co2amp</code> were based on a Mathcad code written by<br>"
            "Viktor T. Platonenko, Moscow State University, Russia<br>"
            "<br>"
            "<b>Citation:</b><br>"
            "M. N. Polyanskiy. <code>co2amp</code>: A software program for modeling the dynamics of ultrashort<br>"
            "pulses in optical systems with CO<sub>2</sub> amplifiers, "
            "<a href=\"https://doi.org/10.1364/AO.54.005136\"><i>Appl. Opt.</i> <b>54</b>, 5136</a> (2015)";

    label_about->setText(about);
}
