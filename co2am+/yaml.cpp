#include "co2am+.h"


YamlHighlighter::YamlHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Number
    numberFormat.setFontWeight(QFont::Bold);
    numberFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression("[-|+]?\\b[0-9][0-9.eE\\-\\+]*\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // "Word" (alphanumeric, starting with a letter, can include underscore)
    wordFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\b[A-Za-z][A-Za-z0-9_]*\\b");
    rule.format = wordFormat;
    highlightingRules.append(rule);

    // "true", "false" or "null"
    boolFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("\\b(true)|(false)|(null)\\b");
    rule.format = boolFormat;
    highlightingRules.append(rule);

    // Key (alphanumeric, starting with a letter, can include underscore)
    // starts at beginning of a string or starts with '- '
    // followed by a column and space
    // can include a '|' sign in the end
    keyFormat.setFontWeight(QFont::Bold);
    keyFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression("^(- |)[A-Za-z][A-Za-z0-9_]*:[ ]+[\\|]*");
    rule.format = keyFormat;
    highlightingRules.append(rule);

    // 'times' key intended by 2 spaces (used in layout only)
    specialKeyFormat.setFontWeight(QFont::Bold);
    specialKeyFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression("^  times: ");
    rule.format = specialKeyFormat;
    highlightingRules.append(rule);

    // plane separator (used in layout only)
    streamFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression(" >> ");
    rule.format = streamFormat;
    highlightingRules.append(rule);

    // misformated tabulated data
    rule.pattern = QRegularExpression("^[0-9.eE\\-\\+\\s\\t]*$");
    rule.format = wrongTabFormatFormat;
    highlightingRules.append(rule);

    // tabulated data
    tabulatedFormat.setFontItalic(true);
    tabulatedFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression("^   ( [-|+]?[0-9][0-9.eE\\-\\+]*)*$");
    rule.format = tabulatedFormat;
    highlightingRules.append(rule);

    // quoted text
    quotationFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // comments
    commentFormat.setFontItalic(true);
    commentFormat.setForeground(Qt::darkGray);
    rule.pattern = QRegularExpression("^#[^\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);
}


void YamlHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}


void MainWindow::YamlFixFormat()
{
    QString old_yaml = plainTextEdit_configFile_content->toPlainText();
    QString new_yaml = old_yaml;

    // tab -> space
    new_yaml.replace(QRegularExpression("\\t"), " ");

    // put a space after column in key/value lines
    new_yaml.replace(QRegularExpression(":([A-Za-z0-9_\\|])"), ": \\1");

    // remove spaces at the end of line
    new_yaml.replace(QRegularExpression("[ ]+$", QRegularExpression::MultilineOption), "");

    // add pipe at the end of a line if key is not followed by a value in the same line
    // use a single space between column and pipe
    new_yaml.replace(QRegularExpression(":[ ]*[\\|]*$", QRegularExpression::MultilineOption), ": |");

    // standartize plane separators in the layout
    new_yaml.replace(QRegularExpression("[ ]*[\\>]+([ ]*[\\>]*[ ]*)*"), " >> ");

    // remove indentation before keys
    new_yaml.replace(QRegularExpression("^[ ]*([A-Za-z0-9_]+)[ ]*[:]", QRegularExpression::MultilineOption), "\\1:");

    // special keys: 'times' and 'go' in the layout YAML
    new_yaml.replace(QRegularExpression("^[ ]*times: ", QRegularExpression::MultilineOption), "  times: ");
    new_yaml.replace(QRegularExpression("^[ ]*[-]*[ ]*go: ", QRegularExpression::MultilineOption), "- go: ");

    // remove indentation of comment lines
    new_yaml.replace(QRegularExpression("^[ ]*([#][.]*)", QRegularExpression::MultilineOption), "\\1");

    //tabulated data - 4 columns
    new_yaml.replace(QRegularExpression("^[ ]*([-|+]?[0-9][0-9.eE\\-\\+]*)[ ]+([-|+]?[0-9][0-9.eE\\-\\+]*)[ ]+([-|+]?[0-9][0-9.eE\\-\\+]*)[ ]+([-|+]?[0-9][0-9.eE\\-\\+]*)",
                                        QRegularExpression::MultilineOption), "    \\1 \\2 \\3 \\4");

    //tabulated data - 3 columns
    new_yaml.replace(QRegularExpression("^[ ]*([-|+]?[0-9][0-9.eE\\-\\+]*)[ ]+([-|+]?[0-9][0-9.eE\\-\\+]*)[ ]+([-|+]?[0-9][0-9.eE\\-\\+]*)",
                                        QRegularExpression::MultilineOption), "    \\1 \\2 \\3");

    //tabulated data - 2 columns
    new_yaml.replace(QRegularExpression("^[ ]*([-|+]?[0-9][0-9.eE\\-\\+]*)[ ]+([-|+]?[0-9][0-9.eE\\-\\+]*)",
                                        QRegularExpression::MultilineOption), "    \\1 \\2");

    //tabulated data - 1 column
    new_yaml.replace(QRegularExpression("^[ ]*([-|+]?[0-9][0-9.eE\\-\\+]*)",
                                        QRegularExpression::MultilineOption), "    \\1");

    if(new_yaml != old_yaml){
        int current_config_file = listWidget_configFile_list->currentRow();
        configFile_content[current_config_file] = new_yaml;

        flag_project_modified = true;
        Update();
    }
}
