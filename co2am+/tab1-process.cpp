#include "co2am+.h"


void MainWindow::WriteToTerminal()
{
    QStringList strlist;
    QString str;

    str = process->readAllStandardOutput();

    str.replace(QRegularExpression("\\r\\n"), "\n"); // Windows "\r\n" endline -> "\n"

    strlist = str.split("\r");

    int number_of_returns = strlist.size() - 1;

    for (int i=0; i<=number_of_returns; i++)
    {
        textBrowser_terminal->insertPlainText(strlist[i]);
        textBrowser_terminal->moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    }
    textBrowser_terminal->moveCursor(QTextCursor::End);
}


void MainWindow::on_lineEdit_v0_textEdited(QString){
    if(CalcResultsExist())
    {
        if(OkToInvalidate())
            InvalidateResults();
        else
        {
            lineEdit_v0->blockSignals(true);
            lineEdit_v0->undo();
            lineEdit_v0->blockSignals(false);
            return;
        }
    }
    flag_project_modified = true;
    Update();
}


void MainWindow::on_lineEdit_t_min_textEdited(QString)
{
    if(CalcResultsExist())
    {
        if(OkToInvalidate())
            InvalidateResults();
        else
        {
            lineEdit_t_min->blockSignals(true);
            lineEdit_t_min->undo();
            lineEdit_t_min->blockSignals(false);
            return;
        }
    }
    flag_project_modified = true;
    Update();
}


void MainWindow::on_lineEdit_t_max_textEdited(QString)
{
    if(CalcResultsExist())
    {
        if(OkToInvalidate())
            InvalidateResults();
        else
        {
            lineEdit_t_max->blockSignals(true);
            lineEdit_t_max->undo();
            lineEdit_t_max->blockSignals(false);
            return;
        }
    }
    flag_project_modified = true;
    Update();
}


void MainWindow::on_lineEdit_time_tick_textEdited(QString)
{
    if(CalcResultsExist())
    {
        if(OkToInvalidate())
            InvalidateResults();
        else
        {
            lineEdit_time_tick->blockSignals(true);
            lineEdit_time_tick->undo();
            lineEdit_time_tick->blockSignals(false);
            return;
        }
    }
    flag_project_modified = true;
    Update();
}


void MainWindow::on_comboBox_precision_t_activated(int i)
{
    if(i == tmp_precision_t)
        return;

    if(CalcResultsExist())
    {
        if(OkToInvalidate())
            InvalidateResults();
        else
        {
            comboBox_precision_t->blockSignals(true);
            comboBox_precision_t->setCurrentIndex(tmp_precision_t);
            comboBox_precision_t->blockSignals(false);
            return;
        }
    }

    flag_project_modified = true;
    Update();
}


void MainWindow::on_comboBox_precision_r_activated(int i)
{
    if(i == tmp_precision_r)
        return;

    if(CalcResultsExist())
    {
        if(OkToInvalidate())
            InvalidateResults();
        else
        {
            comboBox_precision_r->blockSignals(true);
            comboBox_precision_r->setCurrentIndex(tmp_precision_r);
            comboBox_precision_r->blockSignals(false);
            return;
        }
    }

    flag_project_modified = true;
    Update();
}


void MainWindow::on_comboBox_method_activated(int i)
{
    if(i == tmp_method)
        return;

    if(CalcResultsExist())
    {
        if(OkToInvalidate())
            InvalidateResults();
        else
        {
            comboBox_method->blockSignals(true);
            comboBox_method->setCurrentIndex(tmp_method);
            comboBox_method->blockSignals(false);
            return;
        }
    }

    flag_project_modified = true;
    Update();
}
