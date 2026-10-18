#include "SSHighlighter.h"
#include <QDebug>

SSHighlighter::SSHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    qDebug() << "===== HIGHLIGHTER CONSTRUCTOR CALLED =====";
    currentLanguage = "cpp";
    setupCppHighlighting();
    qDebug() << "Highlighter created with" << highlightingRules.size() << "rules for language:" << currentLanguage;
    qDebug() << "==========================================";
}

void SSHighlighter::setTheme(const QHash<QString, QColor> &colors)
{
    qDebug() << "===== SET THEME CALLED =====";
    qDebug() << "Theme has" << colors.size() << "color entries";
    themeColors = colors;
    formatCache.clear();
    qDebug() << "Calling rehighlight()...";
    rehighlight();
    qDebug() << "============================";
}

void SSHighlighter::setLanguage(const QString &lang)
{
    qDebug() << "===== SET LANGUAGE CALLED =====";
    qDebug() << "Current language:" << currentLanguage << "→ New language:" << lang;

    if (currentLanguage == lang) {
        qDebug() << "Language unchanged, skipping";
        qDebug() << "===============================";
        return;
    }

    currentLanguage = lang.toLower();
    clearRules();

    if (lang == "cpp" || lang == "c++") {
        qDebug() << "Setting up C++ highlighting...";
        setupCppHighlighting();
    } else if (lang == "c") {
        qDebug() << "Setting up C highlighting...";
        setupCHighlighting();
    } else if (lang == "asl" || lang == "dsl") {
        qDebug() << "Setting up ACPI highlighting...";
        setupAcpiHighlighting();
    }

    qDebug() << "Now have" << highlightingRules.size() << "rules";
    qDebug() << "Calling rehighlight()...";
    rehighlight();
    qDebug() << "===============================";
}

QTextCharFormat SSHighlighter::getFormat(const QString &tokenType)
{
    if (formatCache.contains(tokenType)) {
        return formatCache[tokenType];
    }

    QTextCharFormat format;

    // Try exact match first
    if (themeColors.contains(tokenType)) {
        format.setForeground(themeColors[tokenType]);
    } else {
        // Try progressively more generic tokens
        QStringList parts = tokenType.split('.');

        // Try removing language suffix (e.g., "comment.line.cpp" → "comment.line")
        if (parts.size() > 2) {
            QString withoutLang = parts.mid(0, parts.size() - 1).join('.');
            if (themeColors.contains(withoutLang)) {
                format.setForeground(themeColors[withoutLang]);
                formatCache[tokenType] = format;
                return format;
            }
        }

        // Try first two levels (e.g., "keyword.return.cpp" → "keyword.return")
        if (parts.size() >= 2) {
            QString twoLevel = parts[0] + "." + parts[1];
            if (themeColors.contains(twoLevel)) {
                format.setForeground(themeColors[twoLevel]);
                formatCache[tokenType] = format;
                return format;
            }
        }

        // Try first level (e.g., "keyword.return.cpp" → "keyword")
        if (parts.size() >= 1) {
            if (themeColors.contains(parts[0])) {
                format.setForeground(themeColors[parts[0]]);
                formatCache[tokenType] = format;
                return format;
            }
        }

        // Fallback: Use a default color based on category
        if (tokenType.startsWith("comment")) {
            format.setForeground(QColor(0x5f, 0x5b, 0x61));
        } else if (tokenType.startsWith("string")) {
            format.setForeground(QColor(0xfe, 0x8f, 0x8f));
        } else if (tokenType.startsWith("keyword")) {
            format.setForeground(QColor(0xd0, 0xc6, 0x95));
        } else if (tokenType.startsWith("constant")) {
            format.setForeground(QColor(0xd0, 0xc6, 0x85));
        } else if (tokenType.startsWith("entity")) {
            format.setForeground(QColor(0xdd, 0x32, 0xbb));
        } else if (tokenType.startsWith("variable")) {
            format.setForeground(QColor(0xfe, 0x6f, 0x6f));
        } else if (tokenType.startsWith("support")) {
            format.setForeground(QColor(0xfe, 0x8f, 0x8f));
        } else {
            format.setForeground(QColor(0xad, 0xad, 0xad));
        }
    }

    formatCache[tokenType] = format;
    return format;
}

void SSHighlighter::addRule(const QString &pattern, const QString &tokenType, int priority)
{
    HighlightingRule rule;
    rule.pattern = QRegularExpression(pattern);
    rule.format = getFormat(tokenType);
    rule.tokenType = tokenType;
    rule.priority = priority;
    highlightingRules.append(rule);
}

void SSHighlighter::clearRules()
{
    highlightingRules.clear();
}

void SSHighlighter::setupCppHighlighting()
{
    clearRules();

    // PRIORITY SYSTEM: Higher numbers = matched first
    // 100 = Highest (strings, comments)
    // 90 = Very high (preprocessor directives)
    // 80 = High (keywords)
    // 70 = Medium-high (types, functions)
    // 60 = Medium (variables, operators)
    // 50 = Default

    // COMMENTS (Priority: 100 - must match before anything else)
    addRule("//[^\n]*", "comment.line.cpp", 100);
    addRule("/\\*.*?\\*/", "comment.block.cpp", 100);

    // STRINGS (Priority: 100)
    addRule("\"(?:\\\\.|[^\\\\\"])*\"", "string.quoted.double.cpp", 100);
    addRule("'(?:\\\\.|[^\\\\'])*'", "string.quoted.single.cpp", 100);

    // PREPROCESSOR DIRECTIVES (Priority: 90)
    addRule("#\\s*include\\s*<[^>]+>", "string.other.lt-gt.include.cpp", 90);
    addRule("#\\s*include\\s*\"[^\"]+\"", "keyword.directive.include.cpp", 90);
    addRule("#\\s*define\\b", "keyword.control.directive.define.cpp", 90);
    addRule("#\\s*ifdef\\b", "keyword.control.directive.ifdef.cpp", 90);
    addRule("#\\s*ifndef\\b", "keyword.control.directive.ifndef.cpp", 90);
    addRule("#\\s*endif\\b", "keyword.control.directive.endif.cpp", 90);
    addRule("#\\s*pragma\\b", "keyword.control.directive.pragma.cpp", 90);
    addRule("#\\s*undef\\b", "keyword.control.directive.undef.cpp", 90);
    addRule("#\\s*if\\b", "keyword.control.directive.if.cpp", 90);
    addRule("#\\s*else\\b", "keyword.control.directive.else.cpp", 90);
    addRule("#\\s*elif\\b", "keyword.control.directive.elif.cpp", 90);

    // CONSTANTS (Priority: 85)
    addRule("\\bNULL\\b", "constant.language.null.cpp", 85);
    addRule("\\bnullptr\\b", "constant.language.null.cpp", 85);
    addRule("\\btrue\\b", "constant.language.boolean.true.cpp", 85);
    addRule("\\bfalse\\b", "constant.language.boolean.false.cpp", 85);
    addRule("\\b0[xX][0-9A-Fa-f]+\\b", "constant.numeric.hex.cpp", 85);
    addRule("\\b\\d+\\.\\d+[fF]?\\b", "constant.numeric.float.cpp", 85);
    addRule("\\b\\d+\\b", "constant.numeric.cpp", 85);

    // CONTROL KEYWORDS (Priority: 80)
    addRule("\\breturn\\b", "keyword.return.cpp", 80);
    addRule("\\bif\\b", "keyword.conditional.cpp", 80);
    addRule("\\belse\\b", "keyword.conditional.cpp", 80);
    addRule("\\bswitch\\b", "keyword.conditional.cpp", 80);
    addRule("\\bcase\\b", "keyword.conditional.cpp", 80);
    addRule("\\bdefault\\b", "keyword.conditional.cpp", 80);
    addRule("\\bfor\\b", "keyword.control.loop.cpp", 80);
    addRule("\\bwhile\\b", "keyword.control.loop.cpp", 80);
    addRule("\\bdo\\b", "keyword.control.loop.cpp", 80);
    addRule("\\bbreak\\b", "keyword.control.flow.cpp", 80);
    addRule("\\bcontinue\\b", "keyword.control.flow.cpp", 80);
    addRule("\\bgoto\\b", "keyword.control.flow.cpp", 80);
    addRule("\\btry\\b", "keyword.control.exception.cpp", 80);
    addRule("\\bcatch\\b", "keyword.control.exception.cpp", 80);
    addRule("\\bthrow\\b", "keyword.control.exception.cpp", 80);

    // MEMORY KEYWORDS (Priority: 80)
    addRule("\\bnew\\b", "keyword.new.cpp", 80);
    addRule("\\bdelete\\b", "keyword.delete.cpp", 80);

    // STORAGE MODIFIERS (Priority: 78)
    addRule("\\bpublic\\b", "storage.modifier.public.cpp", 78);
    addRule("\\bprivate\\b", "storage.modifier.private.cpp", 78);
    addRule("\\bprotected\\b", "storage.modifier.protected.cpp", 78);
    addRule("\\bstatic\\b", "storage.modifier.static.cpp", 78);
    addRule("\\bconst\\b", "storage.modifier.const.cpp", 78);
    addRule("\\bvolatile\\b", "storage.modifier.volatile.cpp", 78);
    addRule("\\bmutable\\b", "storage.modifier.mutable.cpp", 78);
    addRule("\\bextern\\b", "storage.modifier.extern.cpp", 78);
    addRule("\\binline\\b", "storage.modifier.inline.cpp", 78);
    addRule("\\bvirtual\\b", "storage.modifier.virtual.cpp", 78);
    addRule("\\bexplicit\\b", "storage.modifier.explicit.cpp", 78);
    addRule("\\bconstexpr\\b", "storage.modifier.constexpr.cpp", 78);

    // TYPE KEYWORDS (Priority: 75)
    addRule("\\bclass\\b", "keyword.class.cpp", 75);
    addRule("\\bstruct\\b", "keyword.struct.cpp", 75);
    addRule("\\benum\\b", "keyword.enum.cpp", 75);
    addRule("\\bunion\\b", "keyword.union.cpp", 75);
    addRule("\\btypedef\\b", "keyword.typedef.cpp", 75);
    addRule("\\busing\\b", "keyword.using.cpp", 75);
    addRule("\\bnamespace\\b", "keyword.namespace.cpp", 75);
    addRule("\\btemplate\\b", "keyword.template.cpp", 75);
    addRule("\\btypename\\b", "keyword.typename.cpp", 75);

    // BUILTIN TYPES (Priority: 75)
    addRule("\\bvoid\\b", "support.storage.type.builtin.cpp", 75);
    addRule("\\bbool\\b", "support.storage.type.builtin.cpp", 75);
    addRule("\\bchar\\b", "support.storage.type.builtin.cpp", 75);
    addRule("\\bwchar_t\\b", "support.storage.type.builtin.cpp", 75);
    addRule("\\bchar16_t\\b", "support.storage.type.builtin.cpp", 75);
    addRule("\\bchar32_t\\b", "support.storage.type.builtin.cpp", 75);
    addRule("\\bshort\\b", "support.storage.type.builtin.cpp", 75);
    addRule("\\bint\\b", "support.storage.type.builtin.cpp", 75);
    addRule("\\blong\\b", "support.storage.type.builtin.cpp", 75);
    addRule("\\bfloat\\b", "support.storage.type.builtin.cpp", 75);
    addRule("\\bdouble\\b", "support.storage.type.builtin.cpp", 75);
    addRule("\\bsigned\\b", "support.storage.type.builtin.cpp", 75);
    addRule("\\bunsigned\\b", "support.storage.type.builtin.cpp", 75);
    addRule("\\bsize_t\\b", "support.storage.type.builtin.cpp", 75);
    addRule("\\bauto\\b", "support.storage.type.builtin.cpp", 75);

    // THIS KEYWORD (Priority: 75)
    addRule("\\bthis\\b", "variable.language.this.cpp", 75);

    // NAMESPACES (Priority: 70) - Must come before function names
    addRule("\\b([A-Za-z_][A-Za-z0-9_]*)\\s*::", "support.other.namespace.cpp", 70);

    // CLASS NAMES (Priority: 70)
    addRule("\\b([A-Z][A-Za-z0-9_]*)\\b(?=\\s*[({])", "entity.name.class.cpp", 70);

    // FUNCTION DEFINITIONS (Priority: 68)
    addRule("\\b([a-z_][A-Za-z0-9_]*)\\s*\\(", "entity.name.function.cpp", 68);

    // METHOD CALLS (Priority: 68)
    addRule("\\.\\s*([a-z_][A-Za-z0-9_]*)\\s*\\(", "entity.name.function.method.cpp", 68);
    addRule("->\\s*([a-z_][A-Za-z0-9_]*)\\s*\\(", "entity.name.function.method.cpp", 68);

    // MEMBER VARIABLES (Priority: 65)
    addRule("\\.\\s*([a-z_][A-Za-z0-9_]*)\\b(?!\\s*\\()", "variable.other.member.cpp", 65);
    addRule("->\\s*([a-z_][A-Za-z0-9_]*)\\b(?!\\s*\\()", "variable.other.member.cpp", 65);

    // FUNCTION PARAMETERS (Priority: 65) - In function declarations
    addRule("\\(([^)]+)\\)", "variable.parameter.cpp", 65);

    // POINTER DECLARATIONS (Priority: 63)
    addRule("\\*\\s*([a-z_][A-Za-z0-9_]*)\\b", "variable.other.declaration.pointer.cpp", 63);
    addRule("&\\s*([a-z_][A-Za-z0-9_]*)\\b", "variable.other.declaration.reference.cpp", 63);

    // VARIABLE ASSIGNMENTS (Priority: 60)
    addRule("\\b([a-z_][A-Za-z0-9_]*)\\s*=", "variable.other.assignment.cpp", 60);

    // OPERATORS (Priority: 55)
    addRule("[+\\-*/%=<>!&|^~]", "keyword.operator.cpp", 55);
    addRule("\\+\\+|--|<<|>>|<=|>=|==|!=|&&|\\|\\||\\+=|-=|\\*=|/=|%=|&=|\\|=|\\^=", "keyword.operator.cpp", 55);

    // PUNCTUATION (Priority: 50)
    addRule("[;,()\\[\\]{}]", "punctuation.cpp", 50);
}

void SSHighlighter::setupCHighlighting()
{
    clearRules();

    // COMMENTS (Priority: 100)
    addRule("//[^\n]*", "comment.line.c", 100);
    addRule("/\\*.*?\\*/", "comment.block.c", 100);

    // STRINGS (Priority: 100)
    addRule("\"(?:\\\\.|[^\\\\\"])*\"", "string.quoted.double.c", 100);
    addRule("'(?:\\\\.|[^\\\\'])*'", "string.quoted.single.c", 100);

    // PREPROCESSOR DIRECTIVES (Priority: 90)
    addRule("#\\s*include\\s*<[^>]+>", "string.other.lt-gt.include.c", 90);
    addRule("#\\s*include\\s*\"[^\"]+\"", "keyword.include.c", 90);
    addRule("#\\s*define\\b", "keyword.define.c", 90);
    addRule("#\\s*ifdef\\b", "keyword.control.directive.c", 90);
    addRule("#\\s*ifndef\\b", "keyword.control.directive.c", 90);
    addRule("#\\s*endif\\b", "keyword.control.directive.c", 90);
    addRule("#\\s*pragma\\b", "keyword.control.directive.c", 90);

    // CONSTANTS (Priority: 85)
    addRule("\\bNULL\\b", "constant.language.null.c", 85);
    addRule("\\btrue\\b", "constant.language.boolean.true.c", 85);
    addRule("\\bfalse\\b", "constant.language.boolean.false.c", 85);
    addRule("\\b0[xX][0-9A-Fa-f]+\\b", "constant.numeric.hex.c", 85);
    addRule("\\b\\d+\\.\\d+[fF]?\\b", "constant.numeric.float.c", 85);
    addRule("\\b\\d+\\b", "constant.numeric.c", 85);

    // CONTROL KEYWORDS (Priority: 80)
    addRule("\\breturn\\b", "keyword.return.c", 80);
    addRule("\\bif\\b", "keyword.conditional.c", 80);
    addRule("\\belse\\b", "keyword.conditional.c", 80);
    addRule("\\bswitch\\b", "keyword.conditional.c", 80);
    addRule("\\bcase\\b", "keyword.conditional.c", 80);
    addRule("\\bdefault\\b", "keyword.conditional.c", 80);
    addRule("\\bfor\\b", "keyword.control.loop.c", 80);
    addRule("\\bwhile\\b", "keyword.control.loop.c", 80);
    addRule("\\bdo\\b", "keyword.control.loop.c", 80);
    addRule("\\bbreak\\b", "keyword.control.flow.c", 80);
    addRule("\\bcontinue\\b", "keyword.control.flow.c", 80);
    addRule("\\bgoto\\b", "keyword.control.flow.c", 80);

    // STORAGE MODIFIERS (Priority: 78)
    addRule("\\bstatic\\b", "storage.modifier.static.c", 78);
    addRule("\\bconst\\b", "storage.modifier.const.c", 78);
    addRule("\\bvolatile\\b", "storage.modifier.volatile.c", 78);
    addRule("\\bextern\\b", "storage.modifier.extern.c", 78);
    addRule("\\binline\\b", "storage.modifier.inline.c", 78);
    addRule("\\bregister\\b", "storage.modifier.register.c", 78);

    // TYPE KEYWORDS (Priority: 75)
    addRule("\\bstruct\\b", "keyword.struct.c", 75);
    addRule("\\benum\\b", "keyword.enum.c", 75);
    addRule("\\bunion\\b", "keyword.union.c", 75);
    addRule("\\btypedef\\b", "keyword.typedef.c", 75);

    // BUILTIN TYPES (Priority: 75)
    addRule("\\bvoid\\b", "support.storage.type.builtin.c", 75);
    addRule("\\bchar\\b", "support.storage.type.builtin.c", 75);
    addRule("\\bshort\\b", "support.storage.type.builtin.c", 75);
    addRule("\\bint\\b", "support.storage.type.builtin.c", 75);
    addRule("\\blong\\b", "support.storage.type.builtin.c", 75);
    addRule("\\bfloat\\b", "support.storage.type.builtin.c", 75);
    addRule("\\bdouble\\b", "support.storage.type.builtin.c", 75);
    addRule("\\bsigned\\b", "support.storage.type.builtin.c", 75);
    addRule("\\bunsigned\\b", "support.storage.type.builtin.c", 75);
    addRule("\\bsize_t\\b", "support.storage.type.builtin.c", 75);

    // FUNCTION DEFINITIONS (Priority: 68)
    addRule("\\b([a-z_][A-Za-z0-9_]*)\\s*\\(", "entity.name.function.c", 68);

    // MEMBER ACCESS (Priority: 65)
    addRule("\\.\\s*([a-z_][A-Za-z0-9_]*)\\b", "variable.other.member.c", 65);
    addRule("->\\s*([a-z_][A-Za-z0-9_]*)\\b", "variable.other.member.c", 65);

    // POINTER DECLARATIONS (Priority: 63)
    addRule("\\*\\s*([a-z_][A-Za-z0-9_]*)\\b", "variable.other.declaration.pointer.c", 63);

    // VARIABLE ASSIGNMENTS (Priority: 60)
    addRule("\\b([a-z_][A-Za-z0-9_]*)\\s*=", "variable.other.assignment.c", 60);

    // OPERATORS (Priority: 55)
    addRule("[+\\-*/%=<>!&|^~]", "keyword.operator.c", 55);

    // PUNCTUATION (Priority: 50)
    addRule("[;,()\\[\\]{}]", "punctuation.c", 50);
}

void SSHighlighter::setupAcpiHighlighting()
{
    clearRules();

    // ACPI ASL/DSL SYNTAX

    // COMMENTS (Priority: 100)
    addRule("//[^\n]*", "comment.line.asl", 100);
    addRule("/\\*.*?\\*/", "comment.block.asl", 100);

    // STRINGS (Priority: 100)
    addRule("\"(?:\\\\.|[^\\\\\"])*\"", "string.quoted.double.asl", 100);

    // ACPI KEYWORDS (Priority: 80)
    addRule("\\bDefinitionBlock\\b", "keyword.control.asl", 80);
    addRule("\\bDevice\\b", "keyword.device.asl", 80);
    addRule("\\bMethod\\b", "keyword.method.asl", 80);
    addRule("\\bName\\b", "keyword.name.asl", 80);
    addRule("\\bScope\\b", "keyword.scope.asl", 80);
    addRule("\\bField\\b", "keyword.field.asl", 80);
    addRule("\\bOperationRegion\\b", "keyword.operationregion.asl", 80);
    addRule("\\bIf\\b", "keyword.conditional.asl", 80);
    addRule("\\bElse\\b", "keyword.conditional.asl", 80);
    addRule("\\bWhile\\b", "keyword.control.loop.asl", 80);
    addRule("\\bReturn\\b", "keyword.return.asl", 80);
    addRule("\\bBreak\\b", "keyword.control.flow.asl", 80);
    addRule("\\bContinue\\b", "keyword.control.flow.asl", 80);
    addRule("\\bStore\\b", "keyword.store.asl", 80);
    addRule("\\bPackage\\b", "keyword.package.asl", 80);
    addRule("\\bBuffer\\b", "keyword.buffer.asl", 80);

    // ACPI ACCESS TYPES (Priority: 75)
    addRule("\\bAnyAcc\\b", "constant.language.asl", 75);
    addRule("\\bByteAcc\\b", "constant.language.asl", 75);
    addRule("\\bWordAcc\\b", "constant.language.asl", 75);
    addRule("\\bDWordAcc\\b", "constant.language.asl", 75);
    addRule("\\bQWordAcc\\b", "constant.language.asl", 75);
    addRule("\\bBufferAcc\\b", "constant.language.asl", 75);

    // ACPI REGION TYPES (Priority: 75)
    addRule("\\bSystemMemory\\b", "constant.regiontype.asl", 75);
    addRule("\\bSystemIO\\b", "constant.regiontype.asl", 75);
    addRule("\\bPCI_Config\\b", "constant.regiontype.asl", 75);
    addRule("\\bEmbeddedControl\\b", "constant.regiontype.asl", 75);
    addRule("\\bSMBus\\b", "constant.regiontype.asl", 75);

    // NUMERIC CONSTANTS (Priority: 85)
    addRule("\\b0[xX][0-9A-Fa-f]+\\b", "constant.numeric.hex.asl", 85);
    addRule("\\b\\d+\\b", "constant.numeric.asl", 85);

    // ACPI PATHS (Priority: 70)
    addRule("\\\\[_A-Za-z0-9\\.]+", "entity.name.path.asl", 70);
    addRule("\\^+", "entity.name.path.asl", 70);

    // OPERATORS (Priority: 55)
    addRule("[+\\-*/%=<>!&|]", "keyword.operator.asl", 55);

    // PUNCTUATION (Priority: 50)
    addRule("[;,()\\[\\]{}]", "punctuation.asl", 50);
}

void SSHighlighter::highlightBlock(const QString &text)
{
    static int callCount = 0;
    if (callCount < 5) { // Only log first 5 calls to avoid spam
        qDebug() << "highlightBlock called #" << ++callCount << "- text length:" << text.length() << "rules:" << highlightingRules.size();
    }

    if (text.isEmpty()) return;

    // Sort rules by priority (higher first)
    std::sort(highlightingRules.begin(), highlightingRules.end(),
              [](const HighlightingRule &a, const HighlightingRule &b) {
                  return a.priority > b.priority;
              });

    // Track which characters have been highlighted
    QVector<bool> highlighted(text.length(), false);

    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            int start = match.capturedStart();
            int length = match.capturedLength();

            // Check if this region is already highlighted
            bool alreadyHighlighted = false;
            for (int i = start; i < start + length && i < highlighted.size(); ++i) {
                if (highlighted[i]) {
                    alreadyHighlighted = true;
                    break;
                }
            }

            // Only apply format if not already highlighted
            if (!alreadyHighlighted) {
                setFormat(start, length, rule.format);
                // Mark these characters as highlighted
                for (int i = start; i < start + length && i < highlighted.size(); ++i) {
                    highlighted[i] = true;
                }
            }
        }
    }
}
