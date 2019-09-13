#include "co2am+.h"


void MainWindow::on_plainTextEdit_comments_textChanged()
{
    flag_project_modified = true;
    Update();
}
