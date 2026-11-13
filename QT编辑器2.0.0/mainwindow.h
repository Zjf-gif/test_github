#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPlainTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 带行号显示的文本编辑框（纯代码实现，避免改动 .ui 结构）
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;

    // 显示/隐藏行号栏（setViewportMargins 是 protected，只能由类内部调用）
    void setLineNumberAreaVisible(bool visible);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *lineNumberArea;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 供查找/替换对话框使用
    QPlainTextEdit *editor() const;

private slots:
    void on_actionAbout_triggered();
    void on_actionFind_triggered();
    void on_actionReplace_triggered();

    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();

    void on_actionUndo_2_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionSelectAll_triggered();

    void on_actionWordWrap_triggered(bool checked);
    void on_actionShowLineNumbers_triggered(bool checked);
    void on_actionFont_triggered();
    void on_actionFontColor_triggered();
    void on_actionFontBackgroundColor_triggered();
    void on_actionEditorBackgroundColor_triggered();

    void on_actiontoolbar_triggered(bool checked);
    void on_actionStatusBar_triggered(bool checked);

    void on_TextEdit_textChanged();
    void on_TextEdit_cursorPositionChanged();

private:
    Ui::MainWindow *ui;

    QLabel statusCursorlabel;
    QLabel statuslabel;

    QString filePath;
    bool textChanged;

    bool userEditComfirmed();
    bool saveToFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    void updateStatusBar();
    void updateWindowTitle();
    void updateActionsState();
};
#endif // MAINWINDOW_H
