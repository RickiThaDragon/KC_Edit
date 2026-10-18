#include "mainwindow.h"
#include "SSHighlighter.h"
#include <QFont>
#include <QWidget>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QKeySequence>
#include <QToolBar>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QStatusBar>
#include <QString>
#include <QFile>
#include <QFileDialog>
#include <QFontDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tabWidget(nullptr)
    , m_highlighter(nullptr)
    , m_mainToolBar(nullptr)
    , customMenu(new QWidget(this))
    , m_fileButton(nullptr)
    , m_editButton(nullptr)
    , m_viewButton(nullptr)
    , m_statusLabel(nullptr)
    , m_cursorPosLabel(nullptr)
{
    currentFont = QFont("Sour Gummy", 19);
    currentFontSize = 19;
    
    auto *central = new QWidget(this);
    auto *centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setDocumentMode(true);
    
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onCloseTab);
    centralLayout->addWidget(m_tabWidget);
    
    setWindowTitle("KC_EDIT");
    resize(1000, 600);
    
    setCentralWidget(m_tabWidget);
    onNewFile();
    
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    createNewTab();
    
}

MainWindow::~MainWindow() = default;

void MainWindow::setupMenuBar()
{
    QMenu *m_preferencesMenu = menuBar()->addMenu("Preferences");
    
    QMenu *m_helpMenu = menuBar()->addMenu("Help");
    
    m_actAbout = m_helpMenu->addAction(tr("About"));
    connect(m_actAbout, &QAction::triggered, this, &MainWindow::onAbout);
}


void MainWindow::setupToolBar()
{
    m_mainToolBar = addToolBar(tr("Main"));
    m_mainToolBar->setMovable(false);
    
    auto *customMenu = new QWidget(this);
    auto *menuLayout = new QVBoxLayout(customMenu);
    
    auto *buttonLayout = new QHBoxLayout();
    
    m_fileButton = new QPushButton(this);
    m_fileButton->setText(tr("File"));
   
    buttonLayout->addWidget(m_fileButton);
    
    m_editButton = new QPushButton(this);
    m_editButton->setText(tr("Edit"));
    
    buttonLayout->addWidget(m_editButton);
    
    m_viewButton = new QPushButton(this);
    m_viewButton->setText(tr("View"));
    
    buttonLayout->addWidget(m_viewButton);
    
    menuLayout->addLayout(buttonLayout);
    m_mainToolBar->addWidget(customMenu);
    
    QMenu *m_fileMenu = new QMenu("File", this);
    m_actNewFile = m_fileMenu->addAction(tr("New"), QKeySequence::New);
    connect(m_actNewFile, &QAction::triggered, this, &MainWindow::onNewFile);
    
    m_actNewWindow = m_fileMenu->addAction(tr("New Window"), QKeySequence("Ctrl+Shift+N"));
    connect(m_actNewWindow, &QAction::triggered, this, &MainWindow::onNewWindow);
    
    m_actOpenFile = m_fileMenu->addAction(tr("Open File"), QKeySequence::Open);
    connect(m_actOpenFile, &QAction::triggered, this, &MainWindow::onOpenFile);
    
    m_actCloseTab = m_fileMenu->addAction(tr("Close Tab"));
    connect(m_actCloseTab, &QAction::triggered, this, &MainWindow::onCloseTab);
    
     m_fileButton->setMenu(m_fileMenu);
    
    QMenu *m_editMenu = new QMenu(tr("Edit"), this);
    
    m_actUndo = m_editMenu->addAction(tr("Undo"), QKeySequence::Undo);
    connect(m_actUndo, &QAction::triggered, this, &MainWindow::onUndo);
    
    m_actRedo = m_editMenu->addAction(tr("Redo"), QKeySequence::Redo);
    connect(m_actRedo, &QAction::triggered, this, &MainWindow::onRedo);
    
    m_editButton->setMenu(m_editMenu);
    
    
    QMenu *m_viewMenu = new QMenu(tr("View"), this);
    
    m_actChangeFont = m_viewMenu->addAction(tr("Change Font"));
    connect(m_actChangeFont, &QAction::triggered, this, &MainWindow::onChangeFont);
    
    m_actZoomIn = m_viewMenu->addAction(tr("Zoom In"), QKeySequence("Ctrl+="));
    connect(m_actZoomIn, &QAction::triggered, this, &MainWindow::onZoomIn);
    
    m_actZoomOut = m_viewMenu->addAction(tr("Zoom Out"), QKeySequence("Ctrl+-"));
    connect(m_actZoomOut, &QAction::triggered, this, &MainWindow::onZoomOut);
    
   m_viewButton->setMenu(m_viewMenu);
    

}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel(tr("Ready"));
    statusBar()->addWidget(m_statusLabel);
    
    m_cursorPosLabel = new QLabel(tr("Ln1, Col1"));
    statusBar()->addPermanentWidget(m_cursorPosLabel);
}

void MainWindow::createNewTab()
{
    currentFont = QFont("Sour Gummy", 25);
    currentFontSize = 25;
    
    QPlainTextEdit *m_editor = new QPlainTextEdit(this);
    m_tabWidget->addTab(m_editor, "Untitled");
    new SSHighlighter(m_editor->document());
    
    QPalette pal = palette();
    pal.setColor(QPalette::Base, QColor("#121417"));
    pal.setColor(QPalette::Text, QColor("#adadad"));
    m_editor->setPalette(pal);
    m_editor->setAutoFillBackground(true);
    m_editor->setTabStopDistance(40.0);
    m_editor->setPlaceholderText("Start Typing Here");
    m_editor->setFont(currentFont);
}

QPlainTextEdit*MainWindow::currentEditor() const
{
    if (!m_tabWidget)
        return nullptr;
    return qobject_cast<QPlainTextEdit *>(m_tabWidget->currentWidget());
}

void MainWindow::onNewFile()
{
    createNewTab();
}

void MainWindow::onNewWindow()
{
    MainWindow *newWindow = new MainWindow();
    newWindow->show();
}

void MainWindow::onOpenFile()
{
    QPlainTextEdit *m_editor = currentEditor();
    if (!m_editor)
        return;
    
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "All Files (*);; CPP Files (*.cpp *.hpp *.h);;C Files (*.c *.h);;TEXT Files (*.txt)");
    if(!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            m_editor->setPlainText(file.readAll());
            file.close();
        }
    }
}

void MainWindow::onCloseTab(int index)
{
    QPlainTextEdit *m_editor = currentEditor();
    if (!m_editor)
        return;
    
    if (m_editor->document()->isModified()) {
        m_tabWidget->setCurrentIndex(index);
    }
    
    m_tabWidget->currentIndex();
    m_tabWidget->removeTab(index);
    m_editor->deleteLater();
    createNewTab();
}

void MainWindow::onUndo()
{
    QPlainTextEdit *m_editor = currentEditor();
    if (!m_editor)
        return;
    if (m_editor){
        m_editor->undo();
    }
}

void MainWindow::onRedo()
{
    QPlainTextEdit *m_editor = currentEditor();
    if (!m_editor)
        return;
    
    if (m_editor){
        m_editor->redo();
    }
}

void MainWindow::onChangeFont()
{
    QPlainTextEdit *m_editor = currentEditor();
    if (!m_editor)
        return;
    
    bool ok;
    QFont font = QFontDialog::getFont(&ok, currentFont, this, "Select Font");
    if (ok) {
        m_editor->setFont(font);
    }
}

void MainWindow::onZoomIn()
{
    QPlainTextEdit *m_editor = currentEditor();
    if (!m_editor)
        return;
    
    zoomLevel++;
    QFont font = m_editor->font();
    font.setPointSize(font.pointSize() +1);
    m_editor->setFont(font);
}

void MainWindow::onZoomOut()
{
    QPlainTextEdit *m_editor = currentEditor();
    if (!m_editor)
        return;
    
    zoomLevel--;
    QFont font = m_editor->font();
    font.setPointSize(font.pointSize() -1);
    m_editor->setFont(font);
}

void MainWindow::onAbout(){}
