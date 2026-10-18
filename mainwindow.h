#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QToolBar>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QFont>

class SSHighlighter;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    
private slots:
    void onNewFile();
    void onNewWindow();
    void onOpenFile();
    void onCloseTab(int index);
    
    void onUndo();
    void onRedo();
    
    void onChangeFont();
    void onZoomIn();
    void onZoomOut();
    
    void onAbout();
    
private:
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void createNewTab();
    
    QFont currentFont;
    int currentFontSize;
    int zoomLevel;
    
    QWidget *central;
    QTabWidget *m_tabWidget;
    QPlainTextEdit *m_editor;
    SSHighlighter *m_highlighter;
    QToolBar *m_mainToolBar;
    QPushButton *m_fileButton;
    QPushButton *m_editButton;
    QPushButton *m_viewButton;
    QWidget *customMenu;
    QLabel *m_statusLabel;
    QLabel *m_cursorPosLabel;
    
    QMenu *m_preferencesMenu;
    
    QMenu *m_fileMenu;
    QAction *m_actNewFile;
    QAction *m_actNewWindow;
    QAction *m_actOpenFile;
    QAction *m_actCloseTab;
    
    QMenu *m_editMenu;
    QAction *m_actUndo;
    QAction *m_actRedo;
    
    QMenu *m_viewMenu;
    QAction *m_actChangeFont;
    QAction *m_actZoomIn;
    QAction *m_actZoomOut;
    
    QMenu *m_helpMenu;
    QAction *m_actAbout;
    
    QPlainTextEdit *currentEditor() const;
    
    QString fileName;
    QFile file;
};
