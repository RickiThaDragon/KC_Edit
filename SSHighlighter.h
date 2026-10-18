#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QHash>
#include <QString>
#include <QColor>

class SSHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	explicit SSHighlighter(QTextDocument *parent = nullptr);

	void setTheme(const QHash<QString, QColor> &themeColors);
	void setLanguage(const QString &lang);
	QString getCurrentLanguage() const { return currentLanguage; }

protected:
	void highlightBlock(const QString&text) override;

private:
	struct HighlightingRule {
		QRegularExpression pattern;
		QTextCharFormat format;
		QString tokenType;
		int priority;
	};

	QVector<HighlightingRule> highlightingRules;
	QString currentLanguage;
	QHash<QString, QColor> themeColors;
	QHash<QString, QTextCharFormat> formatCache;

	void setupCppHighlighting();
	void setupCHighlighting();
	void setupAcpiHighlighting();

	QTextCharFormat getFormat(const QString &tokenType);
	void addRule(const QString &pattern, const QString &tokenType, int priority = 50);
	void clearRules();
};
