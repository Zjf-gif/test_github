#include "replacedialog.h"
#include "ui_replacedialog.h"

#include <QPlainTextEdit>
#include <QTextCursor>
#include <QTextDocument>
#include <QMessageBox>

ReplaceDialog::ReplaceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReplaceDialog),
    m_editor(nullptr)
{
    ui->setupUi(this);

    setWindowTitle(tr("替换"));
    ui->radioButton_2->setChecked(true);   // 默认向下

    connect(ui->lineEdit_2, &QLineEdit::returnPressed,
            this, &ReplaceDialog::on_pushButton_clicked);
    connect(ui->lineEdit, &QLineEdit::returnPressed,
            this, &ReplaceDialog::on_pushButton_2_clicked);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

void ReplaceDialog::setEditor(QPlainTextEdit *editor)
{
    m_editor = editor;
    if (!m_editor)
        return;

    const QString selected = m_editor->textCursor().selectedText();
    if (!selected.isEmpty() && !selected.contains(QChar::ParagraphSeparator))
        ui->lineEdit_2->setText(selected);
}

bool ReplaceDialog::findNext()
{
    if (!m_editor)
        return false;

    const QString target = ui->lineEdit_2->text();
    if (target.isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请输入查找内容。"));
        ui->lineEdit_2->setFocus();
        return false;
    }

    QTextDocument::FindFlags flags;
    if (ui->checkBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (ui->radioButton->isChecked())          // 向上
        flags |= QTextDocument::FindBackward;

    if (m_editor->find(target, flags))
        return true;

    // 回绕查找
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(ui->radioButton->isChecked() ? QTextCursor::End
                                                     : QTextCursor::Start);
    m_editor->setTextCursor(cursor);

    if (m_editor->find(target, flags))
        return true;

    QMessageBox::information(this, tr("替换"),
                             tr("找不到 \"%1\"。").arg(target));
    return false;
}

bool ReplaceDialog::replaceCurrent()
{
    if (!m_editor)
        return false;

    QTextCursor cursor = m_editor->textCursor();
    if (!cursor.hasSelection())
        return false;

    const QString target = ui->lineEdit_2->text();
    const QString replacement = ui->lineEdit->text();

    Qt::CaseSensitivity cs = ui->checkBox->isChecked() ? Qt::CaseSensitive
                                                       : Qt::CaseInsensitive;
    if (cursor.selectedText().compare(target, cs) != 0)
        return false;

    cursor.insertText(replacement);
    m_editor->setTextCursor(cursor);
    return true;
}

void ReplaceDialog::on_pushButton_clicked()
{
    findNext();
}

void ReplaceDialog::on_pushButton_2_clicked()
{
    // 当前选区就是查找内容则替换，否则先查找
    if (!replaceCurrent())
        findNext();
}

void ReplaceDialog::on_pushButton_4_clicked()
{
    if (!m_editor)
        return;

    const QString target = ui->lineEdit_2->text();
    if (target.isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请输入查找内容。"));
        ui->lineEdit_2->setFocus();
        return;
    }

    const QString replacement = ui->lineEdit->text();
    Qt::CaseSensitivity cs = ui->checkBox->isChecked() ? Qt::CaseSensitive
                                                       : Qt::CaseInsensitive;

    // 从文档开头逐个替换，避免死循环
    QTextCursor cursor(m_editor->document());
    cursor.beginEditBlock();

    int count = 0;
    while (true) {
        QTextCursor found = m_editor->document()->find(target, cursor, QTextDocument::FindFlags());
        if (found.isNull())
            break;

        // 大小写敏感时再校验一次
        if (found.selectedText().compare(target, cs) != 0) {
            cursor = found;
            cursor.movePosition(QTextCursor::NextCharacter);
            continue;
        }

        found.insertText(replacement);
        // 关键修复：替换后必须把游标移到替换文本之后，
        // 否则当 replacement 包含 target（如 a -> aa）时会原地反复匹配，造成死循环。
        cursor = found;
        ++count;
    }

    cursor.endEditBlock();

    QMessageBox::information(this, tr("替换"),
                             tr("共替换 %1 处。").arg(count));
}

void ReplaceDialog::on_pushButton_3_clicked()
{
    reject();
}
