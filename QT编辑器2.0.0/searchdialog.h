#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>

class QPlainTextEdit;

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = nullptr);
    ~SearchDialog();

    // 绑定被搜索的编辑框
    void setEditor(QPlainTextEdit *editor);

private slots:
    void on_pushButton_clicked();      // 查找下一个
    void on_pushButton_2_clicked();    // 取消

private:
    bool findNext();

    Ui::SearchDialog *ui;
    QPlainTextEdit *m_editor;
};

#endif // SEARCHDIALOG_H
