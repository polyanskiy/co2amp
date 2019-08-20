#include "mainwindow.h"


YamlHighlighter::YamlHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    numberFormat.setFontWeight(QFont::Bold);
    numberFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression(QStringLiteral("\\b[0-9][0-9.eE\\-\\+]*\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);

    wordFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z][A-Za-z0-9_]+\\b"));
    rule.format = wordFormat;
    highlightingRules.append(rule);

    keyFormat.setFontWeight(QFont::Bold);
    keyFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+: |\\b"));
    rule.format = keyFormat;
    highlightingRules.append(rule);

    streamFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression(QStringLiteral(" >> "));
    rule.format = streamFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("^[0-9.eE\\-\\+ ]*$"));
    rule.format = wrongtabFormatFormat;
    highlightingRules.append(rule);

    tabulatedFormat.setFontItalic(true);
    tabulatedFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression(QStringLiteral("^   ( [0-9][0-9.eE\\-\\+]*)*$"));
    rule.format = tabulatedFormat;
    highlightingRules.append(rule);

    quotationFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    commentFormat.setFontItalic(true);
    commentFormat.setForeground(Qt::darkGray);
    rule.pattern = QRegularExpression(QStringLiteral("#[^\n]*"));
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
    //setCurrentBlockState(0);
}
