#include "mainwindow.h"


YamlHighlighter::YamlHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    numberFormat.setFontWeight(QFont::Bold);
    numberFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression("\\b[0-9][0-9.eE\\-\\+]*\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    wordFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\b[A-Za-z][A-Za-z0-9_]*\\b");
    rule.format = wordFormat;
    highlightingRules.append(rule);

    boolFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("\\b(true)|(false)\\b");
    rule.format = boolFormat;
    highlightingRules.append(rule);

    keyFormat.setFontWeight(QFont::Bold);
    keyFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression("^[A-Za-z][A-Za-z0-9_]*:[ ][\\|]*");
    rule.format = keyFormat;
    highlightingRules.append(rule);

    streamFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression(" >> ");
    rule.format = streamFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("^[0-9.eE\\-\\+\\s\\t]*$");
    rule.format = wrongtabFormatFormat;
    highlightingRules.append(rule);

    tabulatedFormat.setFontItalic(true);
    tabulatedFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression("^   ( [0-9][0-9.eE\\-\\+]*)*$");
    rule.format = tabulatedFormat;
    highlightingRules.append(rule);

    quotationFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    commentFormat.setFontItalic(true);
    commentFormat.setForeground(Qt::darkGray);
    rule.pattern = QRegularExpression("#[^\n]*");
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
    QString yaml = plainTextEdit_configFile_content->toPlainText();
    yaml.replace(QRegularExpression("\\t"), " ");
    yaml.replace(QRegularExpression(":[ ]*([A-Za-z0-9_\\|])"), ": \\1");
    yaml.replace(QRegularExpression("[ ]+\\n"), "\n");
    yaml.replace(QRegularExpression("[:]+\\n"), ": |\n");
    yaml.replace(QRegularExpression("[ ]*\\>\\>[ ]*"), " >> ");
    yaml.replace(QRegularExpression("\\n[ ]+([A-Za-z0-9_]+)[:]"), "\n\\1:");
    yaml.replace(QRegularExpression("\\n[ ]*([0-9][0-9.eE\\-\\+]*)[ ]+([0-9][0-9.eE\\-\\+]*)[ ]+([0-9][0-9.eE\\-\\+]*)[ ]+([0-9][0-9.eE\\-\\+]*)"), "\n    \\1 \\2 \\3 \\4");
    yaml.replace(QRegularExpression("\\n[ ]*([0-9][0-9.eE\\-\\+]*)[ ]+([0-9][0-9.eE\\-\\+]*)[ ]+([0-9][0-9.eE\\-\\+]*)"), "\n    \\1 \\2 \\3");
    yaml.replace(QRegularExpression("\\n[ ]*([0-9][0-9.eE\\-\\+]*)[ ]+([0-9][0-9.eE\\-\\+]*)"), "\n    \\1 \\2");
    yaml.replace(QRegularExpression("\\n[ ]*([0-9][0-9.eE\\-\\+]*)"), "\n    \\1");
    plainTextEdit_configFile_content->setPlainText(yaml);
}
