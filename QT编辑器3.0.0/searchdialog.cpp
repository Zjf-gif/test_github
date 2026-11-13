#include "searchdialog.h"
#include "ui_searchdialog.h"

#include <QPlainTextEdit>
#include <QTextCursor>
#include <QTextDocument>
#include <QMessageBox>

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog),
    m_editor(nullptr)
{
    ui->setupUi(this);

    // 默认向下查找
    ui->radioButton_2->setChecked(true);

    // 回车直接查找下一个
    connect(ui->lineEdit, &QLineEdit::returnPressed,
            this, &SearchDialog::on_pushButton_clicked);
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::setEditor(QPlainTextEdit *editor)
{
    m_editor = editor;
    if (!m_editor)
        return;

    // 若编辑框中有选中文本，自动填入查找框
    const QString selected = m_editor->textCursor().selectedText();
    if (!selected.isEmpty() && !selected.contains(QChar::ParagraphSeparator))
        ui->lineEdit->setText(selected);
}

bool SearchDialog::findNext()
{
    if (!m_editor)
        return false;

    const QString target = ui->lineEdit->text();
    if (target.isEmpty()) {
        QMessageBox::information(this, tr("查找"), tr("请输入查找内容。"));
        ui->lineEdit->setFocus();
        return false;
    }

    QTextDocument::FindFlags flags;
    if (ui->checkBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (ui->radioButton->isChecked())          // 向上
        flags |= QTextDocument::FindBackward;

    // 先按当前光标查找
    if (m_editor->find(target, flags))
        return true;

    // 未找到 -> 从文档头/尾回绕再找一次
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(ui->radioButton->isChecked() ? QTextCursor::End
                                                     : QTextCursor::Start);
    m_editor->setTextCursor(cursor);

    if (m_editor->find(target, flags))
        return true;

    QMessageBox::information(this, tr("查找"),
                             tr("找不到 \"%1\"。").arg(target));
    return false;
}

void SearchDialog::on_pushButton_clicked()
{
    findNext();
}

void SearchDialog::on_pushButton_2_clicked()
{
    reject();
}
