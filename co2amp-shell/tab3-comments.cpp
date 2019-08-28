#include "co2amp.h"


void MainWindow::on_plainTextEdit_comments_textChanged()
{
    flag_project_modified = true;
    Update();
}
