#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

class QPlainTextEdit;

namespace Ui {
class ReplaceDialog;
}

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent = nullptr);
    ~ReplaceDialog();

    void setEditor(QPlainTextEdit *editor);

private slots:
    void on_pushButton_clicked();      // 查找下一个
    void on_pushButton_2_clicked();    // 替换
    void on_pushButton_4_clicked();    // 全部替换
    void on_pushButton_3_clicked();    // 取消

private:
    bool findNext();
    bool replaceCurrent();

    Ui::ReplaceDialog *ui;
    QPlainTextEdit *m_editor;
};

#endif // REPLACEDIALOG_H
