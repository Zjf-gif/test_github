#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdialog.h"
#include "searchdialog.h"
#include "replacedialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFileInfo>
#include <QFontDialog>
#include <QColorDialog>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QTextBlock>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QClipboard>
#include <QMimeData>
#include <QTextCharFormat>

/* ======================= CodeEditor：带行号的编辑框 ======================= */

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , lineNumberArea(new QWidget(this))
{
    lineNumberArea->setObjectName("lineNumberArea");
    lineNumberArea->hide();

    // 关键修复：安装事件过滤器，让行号栏的 paint 事件能触发绘制
    lineNumberArea->installEventFilter(this);

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);

    updateLineNumberAreaWidth();
}

int CodeEditor::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    // 左右各留 3px 边距
    return 6 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditor::updateLineNumberAreaWidth()
{
    setViewportMargins(lineNumberArea->isVisible() ? lineNumberAreaWidth() : 0, 0, 0, 0);
}

void CodeEditor::setLineNumberAreaVisible(bool visible)
{
    lineNumberArea->setVisible(visible);
    // 触发一次边距刷新（setViewportMargins 为 protected，只能在类内部调用）
    updateLineNumberAreaWidth();
    update();
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (!lineNumberArea->isVisible())
        return;

    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth();
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    const QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                      lineNumberAreaWidth(), cr.height()));
}

bool CodeEditor::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == lineNumberArea && event->type() == QEvent::Paint) {
        lineNumberAreaPaintEvent(static_cast<QPaintEvent *>(event));
        return true;
    }
    return QPlainTextEdit::eventFilter(obj, event);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(240, 240, 240));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(120, 120, 120));
            painter.drawText(0, top, lineNumberArea->width() - 3,
                             fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

/* ============================== MainWindow ============================== */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , textChanged(false)
{
    ui->setupUi(this);

    // 状态栏：长度/行数、行列、作者
    statusCursorlabel.setMaximumWidth(200);
    ui->statusbar->addPermanentWidget(&statusCursorlabel);

    statuslabel.setMaximumWidth(200);
    ui->statusbar->addPermanentWidget(&statuslabel);

    QLabel *author = new QLabel(tr("张家富"), ui->statusbar);
    ui->statusbar->addPermanentWidget(author);

    // 光标位置变化 -> 更新行列
    connect(ui->TextEdit, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::on_TextEdit_cursorPositionChanged);

    // 关键修复：选区变化（鼠标拖选/双击选词/Shift+方向键）不一定触发
    // textChanged 或 cursorPositionChanged，单独监听 selectionChanged，
    // 否则复制/剪切动作会一直处于禁用状态，表现为“有时能用有时不能用”。
    connect(ui->TextEdit, &QPlainTextEdit::selectionChanged,
            this, &MainWindow::onEditorSelectionChanged);

    // 剪贴板内容变化 -> 刷新粘贴动作可用状态
    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &MainWindow::updateActionsState);

    // 菜单项可勾选状态
    ui->actionWordWrap->setCheckable(true);
    ui->actionWordWrap->setChecked(true);
    ui->actionShowLineNumbers->setCheckable(true);
    ui->actionShowLineNumbers->setChecked(false);
    ui->actiontoolbar->setCheckable(true);
    ui->actiontoolbar->setChecked(true);
    ui->actionStatusBar->setCheckable(true);
    ui->actionStatusBar->setChecked(true);

    // 修正 .ui 中 actionSaveAs 与 actionSelect All 快捷键冲突（都是 Ctrl+A）
    ui->actionSaveAs->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+S")));

    // 新建空文档（此时 textChanged 已初始化，不会误标为已修改）
    on_actionNew_triggered();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QPlainTextEdit *MainWindow::editor() const
{
    return ui->TextEdit;
}

/* ------------------------------ 文件操作 ------------------------------ */

void MainWindow::on_actionAbout_triggered()
{
    aboutDialog dig(this);
    dig.exec();
}

void MainWindow::on_actionFind_triggered()
{
    SearchDialog dig(this);
    dig.setEditor(ui->TextEdit);
    dig.exec();
}

void MainWindow::on_actionReplace_triggered()
{
    ReplaceDialog dig(this);
    dig.setEditor(ui->TextEdit);
    dig.exec();
}

void MainWindow::on_actionNew_triggered()
{
    if (!userEditComfirmed())
        return;

    // 先屏蔽 textChanged 标记，避免 clear() 触发“已修改”
    textChanged = false;

    ui->TextEdit->clear();
    setCurrentFile(QString());

    textChanged = false;

    updateStatusBar();
    updateWindowTitle();
    updateActionsState();
}

void MainWindow::on_actionOpen_triggered()
{
    if (!userEditComfirmed())
        return;

    const QString filename = QFileDialog::getOpenFileName(
                this, tr("打开文件"), QString(),
                tr("文本文件 (*.txt);;所有文件 (*.*)"));
    if (filename.isEmpty())          // 用户取消
        return;

    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("打开文件"),
                             tr("无法打开文件：\n%1").arg(filename));
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    const QString text = in.readAll();
    file.close();

    // 关键修复：用 setPlainText 覆盖，而不是 insertPlainText 追加
    ui->TextEdit->setPlainText(text);
    setCurrentFile(filename);

    textChanged = false;
    updateStatusBar();
    updateWindowTitle();
    updateActionsState();
}

bool MainWindow::saveToFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("保存文件"),
                             tr("无法写入文件：\n%1").arg(fileName));
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << ui->TextEdit->toPlainText();
    file.flush();
    file.close();

    setCurrentFile(fileName);
    textChanged = false;
    updateWindowTitle();
    return true;
}

void MainWindow::on_actionSave_triggered()
{
    if (filePath.isEmpty()) {
        // 没有文件名 -> 走另存为流程
        on_actionSaveAs_triggered();
        return;
    }

    saveToFile(filePath);
    updateActionsState();
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString filename = QFileDialog::getSaveFileName(
                this, tr("另存为"), filePath.isEmpty() ? QString() : filePath,
                tr("文本文件 (*.txt);;所有文件 (*.*)"));
    if (filename.isEmpty())          // 用户取消，不创建任何文件
        return;

    if (saveToFile(filename))
        updateActionsState();
}

/* ------------------------------ 编辑操作 ------------------------------ */

void MainWindow::on_actionUndo_2_triggered()
{
    ui->TextEdit->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    ui->TextEdit->redo();
}

void MainWindow::on_actionCut_triggered()
{
    // 没有选区时 cut() 什么也不做，这里直接忽略，避免误以为“失效”
    if (!ui->TextEdit->textCursor().hasSelection())
        return;

    ui->TextEdit->cut();
    updateActionsState();
}

void MainWindow::on_actionCopy_triggered()
{
    if (!ui->TextEdit->textCursor().hasSelection())
        return;

    ui->TextEdit->copy();
    updateActionsState();
}

void MainWindow::on_actionPaste_triggered()
{
    ui->TextEdit->paste();
    updateActionsState();
}

void MainWindow::on_actionSelectAll_triggered()
{
    ui->TextEdit->selectAll();
}

/* ------------------------------ 格式操作 ------------------------------ */

void MainWindow::on_actionWordWrap_triggered(bool checked)
{
    ui->TextEdit->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth
                                          : QPlainTextEdit::NoWrap);
}

void MainWindow::on_actionShowLineNumbers_triggered(bool checked)
{
    CodeEditor *editor = qobject_cast<CodeEditor *>(ui->TextEdit);
    if (!editor)
        return;

    editor->setLineNumberAreaVisible(checked);
}

void MainWindow::on_actionFont_triggered()
{
    // 关键修复：QPlainTextEdit::setFont() 只改控件字体，不会影响已有文本，
    // 必须通过 QTextCharFormat 把字体合并到选区（无选区时作用于后续输入）。
    bool ok = false;
    const QFont font = QFontDialog::getFont(&ok, ui->TextEdit->font(), this,
                                            tr("选择字体"));
    if (!ok)
        return;

    QTextCharFormat fmt;
    fmt.setFontFamily(font.family());
    fmt.setFontPointSize(font.pointSizeF() > 0 ? font.pointSizeF()
                                               : QFontInfo(font).pointSizeF());
    fmt.setFontWeight(font.weight());
    fmt.setFontItalic(font.italic());
    fmt.setFontUnderline(font.underline());
    fmt.setFontStrikeOut(font.strikeOut());

    QTextCursor cursor = ui->TextEdit->textCursor();
    if (cursor.hasSelection()) {
        cursor.mergeCharFormat(fmt);
        ui->TextEdit->mergeCurrentCharFormat(fmt);
    } else {
        // 无选区：只影响接下来输入的文字
        ui->TextEdit->mergeCurrentCharFormat(fmt);
    }

    ui->TextEdit->setFocus();
}

void MainWindow::on_actionFontColor_triggered()
{
    // QPlainTextEdit 没有 textColor()/setTextColor()，用字符格式处理前景色
    const QColor current = ui->TextEdit->currentCharFormat().foreground().color();
    const QColor color = QColorDialog::getColor(current.isValid() ? current : Qt::black,
                                                this, tr("选择字体颜色"));
    if (!color.isValid())
        return;

    QTextCharFormat fmt;
    fmt.setForeground(color);

    QTextCursor cursor = ui->TextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.mergeCharFormat(fmt);   // 只给选中的文字上色

    // 无论有无选区，都让后续输入使用该颜色
    ui->TextEdit->mergeCurrentCharFormat(fmt);
    ui->TextEdit->setFocus();
}

void MainWindow::on_actionFontBackgroundColor_triggered()
{
    const QColor color = QColorDialog::getColor(Qt::yellow, this,
                                                tr("选择字体背景色"));
    if (!color.isValid())
        return;

    QTextCharFormat fmt;
    fmt.setBackground(color);

    QTextCursor cursor = ui->TextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.mergeCharFormat(fmt);

    ui->TextEdit->mergeCurrentCharFormat(fmt);
    ui->TextEdit->setFocus();
}

void MainWindow::on_actionEditorBackgroundColor_triggered()
{
    const QColor color = QColorDialog::getColor(ui->TextEdit->palette().color(QPalette::Base),
                                                this, tr("选择编辑器背景色"));
    if (!color.isValid())
        return;

    QPalette pal = ui->TextEdit->palette();
    pal.setColor(QPalette::Base, color);
    ui->TextEdit->setPalette(pal);
}

/* ------------------------------ 查看操作 ------------------------------ */

void MainWindow::on_actiontoolbar_triggered(bool checked)
{
    if (QToolBar *bar = ui->toolBar)
        bar->setVisible(checked);
}

void MainWindow::on_actionStatusBar_triggered(bool checked)
{
    if (QStatusBar *bar = ui->statusbar)
        bar->setVisible(checked);
}

/* ------------------------------ 状态维护 ------------------------------ */

void MainWindow::on_TextEdit_textChanged()
{
    // 关键修复：只在“未修改 -> 已修改”时打一次标记，避免标题出现多个 '*'
    if (!textChanged) {
        textChanged = true;
        updateWindowTitle();
    }
    updateStatusBar();
    updateActionsState();
}

void MainWindow::on_TextEdit_cursorPositionChanged()
{
    updateStatusBar();
    updateActionsState();
}

void MainWindow::onEditorSelectionChanged()
{
    // 选区一变就刷新复制/剪切/全选等动作的可用状态
    updateActionsState();
}

void MainWindow::updateStatusBar()
{
    const QString text = ui->TextEdit->toPlainText();
    const int lines = ui->TextEdit->blockCount();
    statusCursorlabel.setText(tr("length:%1  lines:%2")
                              .arg(text.length())
                              .arg(lines));

    const QTextCursor cursor = ui->TextEdit->textCursor();
    const int ln = cursor.blockNumber() + 1;
    const int col = cursor.positionInBlock() + 1;
    statuslabel.setText(tr("Ln:%1  Col:%2").arg(ln).arg(col));
}

void MainWindow::updateWindowTitle()
{
    QString name = filePath.isEmpty() ? tr("无标题.txt") : QFileInfo(filePath).fileName();
    if (textChanged)
        name.prepend(QLatin1Char('*'));
    setWindowTitle(name + tr(" - 小牛文本编辑器"));
}

void MainWindow::updateActionsState()
{
    const bool hasText = !ui->TextEdit->toPlainText().isEmpty();
    const bool hasSelection = ui->TextEdit->textCursor().hasSelection();

    ui->actionUndo_2->setEnabled(ui->TextEdit->document()->isUndoAvailable());
    ui->actionRedo->setEnabled(ui->TextEdit->document()->isRedoAvailable());
    ui->actionCopy->setEnabled(hasSelection);
    ui->actionCut->setEnabled(hasSelection);
    ui->actionSelectAll->setEnabled(hasText);

    // 粘贴：剪贴板里有文本才可用
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    ui->actionPaste->setEnabled(mime && mime->hasText());
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    filePath = fileName;
    updateWindowTitle();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 有未保存修改时先询问，用户取消则阻止关闭
    if (userEditComfirmed())
        event->accept();
    else
        event->ignore();
}

bool MainWindow::userEditComfirmed()
{
    if (!textChanged)
        return true;

    const QString path = filePath.isEmpty() ? tr("无标题.txt") : filePath;

    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Question);
    msg.setWindowTitle(tr("小牛文本编辑器"));
    msg.setText(tr("是否将更改保存到\n\"%1\"?").arg(path));
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msg.setDefaultButton(QMessageBox::Yes);

    switch (msg.exec()) {
    case QMessageBox::Yes:
        // 关键修复：保存失败/用户取消另存为时，必须返回 false 阻止后续操作
        if (filePath.isEmpty()) {
            const QString filename = QFileDialog::getSaveFileName(
                        this, tr("保存文件"), QString(),
                        tr("文本文件 (*.txt);;所有文件 (*.*)"));
            if (filename.isEmpty())
                return false;
            return saveToFile(filename);
        }
        return saveToFile(filePath);
    case QMessageBox::No:
        textChanged = false;
        return true;
    case QMessageBox::Cancel:
    default:
        return false;
    }
}
