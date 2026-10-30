#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStack>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString operand;
    QString opertor;
    QStack<QString> operands;
    QStack<QString> opertors;
    QString calculation(bool *ok=NULL);

private slots:
    void btnNumclicked();

    void on_btnDecimalPoint_clicked();

    void on_btnBackspace_clicked();

    void btnbinaryOperatorClicked();

    void on_btnEquals_clicked();

    void btnUnaryOperatorClicked();

    void on_btnClear_clicked();

    void on_btnClearEntry_clicked();

    void on_btnNegatePlusMinus_clicked();

protected:
    // 键盘输入：把按键映射成与按钮文字相同的字符串，复用同一套核心逻辑
    void    keyPressEvent(QKeyEvent *event) override;

private:

    void    inputDigit(const QString &digit);
    void    inputOperator(const QString &op);
    void    applyUnary(const QString &op);
    void    calculate();
    void    showNumber(double value);
    void    showError(const QString &msg);
    void    resetAll();
    double  currentValue() const;
    QString opFromButtonText(const QString &t) const;

    // 与鼠标槽共用的动作函数（不依赖 sender()，键盘与鼠标都调用它们）
    void    doDecimalPoint();
    void    doBackspace();
    void    doEquals();
    void    doClear();
    void    doClearEntry();
    void    doNegate();


    double  accumulator = 0.0;
    QString pendingOp;
    QString lastOp;
    double  lastOperand = 0.0;
    bool    typing = false;
    bool    errorState = false;
    bool    hasLastOp = false;

    Ui::MainWindow *ui;
};
#endif
