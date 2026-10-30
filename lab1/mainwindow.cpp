#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <QPushButton>
#include <QKeyEvent>
#include <QtMath>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnNum0,SIGNAL(clicked()),this,SLOT(btnNumclicked()));
    connect(ui->btnNum1,SIGNAL(clicked()),this,SLOT(btnNumclicked()));
    connect(ui->btnNum2,SIGNAL(clicked()),this,SLOT(btnNumclicked()));
    connect(ui->btnNum3,SIGNAL(clicked()),this,SLOT(btnNumclicked()));
    connect(ui->btnNum4,SIGNAL(clicked()),this,SLOT(btnNumclicked()));
    connect(ui->btnNum5,SIGNAL(clicked()),this,SLOT(btnNumclicked()));
    connect(ui->btnNum6,SIGNAL(clicked()),this,SLOT(btnNumclicked()));
    connect(ui->btnNum7,SIGNAL(clicked()),this,SLOT(btnNumclicked()));
    connect(ui->btnNum8,SIGNAL(clicked()),this,SLOT(btnNumclicked()));
    connect(ui->btnNum9,SIGNAL(clicked()),this,SLOT(btnNumclicked()));
    connect(ui->btnPlus,SIGNAL(clicked()),this,SLOT(btnbinaryOperatorClicked()));
    connect(ui->btnMinus,SIGNAL(clicked()),this,SLOT(btnbinaryOperatorClicked()));
    connect(ui->btnMultiply,SIGNAL(clicked()),this,SLOT(btnbinaryOperatorClicked()));
    connect(ui->btnDivide,SIGNAL(clicked()),this,SLOT(btnbinaryOperatorClicked()));


    connect(ui->btnPercent,SIGNAL(clicked()),this,SLOT(btnUnaryOperatorClicked()));
    connect(ui->btnReciprocal,SIGNAL(clicked()),this,SLOT(btnUnaryOperatorClicked()));
    connect(ui->btnSquare,SIGNAL(clicked()),this,SLOT(btnUnaryOperatorClicked()));
    connect(ui->btnSquareRoot,SIGNAL(clicked()),this,SLOT(btnUnaryOperatorClicked()));

    ui->display->setReadOnly(true);
    ui->display->setText("0");
    operand = "0";
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::calculation(bool *ok)
{
    double result=0;
   if(operands.size()==2&&opertors.size()>0){

       double operand1=operands.front().toDouble();
       operands.pop_front();
       double operand2=operands.front().toDouble();
       operands.pop_front();

       QString op=opertors.front();
       opertors.pop_front();

       if(op=="+"){
           result=operand1+operand2;
       }else if(op=="-"){
           result=operand1-operand2;
       }else if(op=="*"){
           result=operand1*operand2;
       }else if(op=="/"){
           result=operand1/operand2;
       }

       ui->statusbar->showMessage(QString("caluation is progress:operands is %1,opertors is %2").arg(operands.size()).arg(opertors.size()));
    }else
        ui->statusbar->showMessage(QString("operands is %1,opertors is %2").arg(operands.size()).arg(opertors.size()));
    return"";
}

void MainWindow::btnNumclicked()
{
    // 补充：错误状态下按键无效，其余交给 inputDigit 统一处理
    if(errorState)
        return;
    inputDigit(qobject_cast<QPushButton*>(sender())->text());
}


void MainWindow::on_btnDecimalPoint_clicked()
{
    doDecimalPoint();
}

// 小数点核心逻辑（鼠标与键盘共用）
void MainWindow::doDecimalPoint()
{
    if(errorState)
        return;
    if(!typing){
        operand="0.";
        typing=true;
    }else if(!operand.contains(".")){
        operand+=".";
    }
    ui->display->setText(operand);
}

// ============================================================
//  键盘输入：把按键映射成与按钮文字相同的字符串，
//  再调用与鼠标完全相同的核心函数，实现鼠标/键盘复用同一套逻辑
//  （实现见文件末尾）
// ============================================================

void MainWindow::on_btnBackspace_clicked()
{
    doBackspace();
}

// 退格核心逻辑（鼠标与键盘共用）
void MainWindow::doBackspace()
{

    if(errorState||!typing)
        return;
    operand=operand.left(operand.length()-1);

    if(operand.isEmpty()||operand=="-")
        operand="0";
    ui->display->setText(operand);
}

void MainWindow::btnbinaryOperatorClicked()
{
    // 补充：交给 inputOperator 统一处理（支持连续运算与运算符替换）
    if(errorState)
        return;
    inputOperator(opFromButtonText(qobject_cast<QPushButton*>(sender())->text()));
}


void MainWindow::on_btnEquals_clicked()
{
    doEquals();
}

// 等号核心逻辑（鼠标与键盘共用）
void MainWindow::doEquals()
{
    // 补充：执行挂起运算；连按 "=" 时重复上一次运算（Windows 行为）
    if(errorState)
        return;

    if(!pendingOp.isEmpty()){
        lastOp=pendingOp;
        lastOperand=currentValue();
        hasLastOp=true;

        calculate();
        pendingOp.clear();
    }else if(hasLastOp){
        pendingOp=lastOp;
        operand=QString::number(lastOperand,'g',15);
        calculate();
        pendingOp.clear();
    }

    typing=false;
}

void MainWindow::btnUnaryOperatorClicked()
{
    // 补充：交给 applyUnary 统一处理（% 1/x x² √x）
    if(errorState)
        return;
    applyUnary(qobject_cast<QPushButton*>(sender())->text());
}


// ============================================================
//  以下为补充的代码：让计算器行为与 Windows 标准计算器一致
//  采用「累加器 + 待定运算符」模型：
//    accumulator : 已确定的左操作数（上次运算结果）
//    operand     : 当前正在输入的数值字符串
//    pendingOp   : 待执行的二元运算符
// ============================================================

// 工具函数：把按钮上的显示文字转换成内部运算符
// ÷ -> "/"   × -> "*"   其余原样
QString MainWindow::opFromButtonText(const QString &t) const
{
    if (t == QString::fromUtf8("÷")) return "/";
    if (t == QString::fromUtf8("×")) return "*";
    return t;
}

// 工具函数：取当前显示值
double MainWindow::currentValue() const
{
    return operand.toDouble();
}

// 工具函数：把 double 格式化后显示
// 去掉多余的 0，最多保留 15 位有效数字，与 Windows 计算器显示习惯一致
void MainWindow::showNumber(double value)
{
    if (qFuzzyIsNull(value))
        value = 0.0;                 // 避免显示 "-0"

    if (std::isinf(value) || std::isnan(value)) {
        showError("除数不能为零");
        return;
    }

    QString text = QString::number(value, 'g', 15);
    if (text.contains('e') || text.contains('E'))
        text = QString::number(value, 'f', 0);   // 科学计数法转普通写法

    operand = text;
    ui->display->setText(text);
    typing = false;                  // 显示的是结果，下次输入数字重新开始
}


void MainWindow::showError(const QString &msg)
{
    ui->display->setText(msg);
    errorState = true;
    typing     = false;
    pendingOp.clear();
    hasLastOp  = false;
}


void MainWindow::resetAll()
{
    accumulator = 0.0;
    operand     = "0";
    pendingOp.clear();
    lastOp.clear();
    lastOperand = 0.0;
    typing      = false;
    errorState  = false;
    hasLastOp   = false;
    ui->display->setText("0");
}

// 数字输入核心逻辑
void MainWindow::inputDigit(const QString &digit)
{
    if (!typing) {
        // 刚按过运算符或刚出结果：开始输入新的数字
        operand = digit;
        typing  = true;
    } else {
        // 限制输入长度（Windows 计算器最多 16 位）
        QString plain = operand;
        plain.remove('-');
        plain.remove('.');
        if (plain.length() >= 16)
            return;

        // 前导 0 处理
        if (operand == "0")
            operand = digit;
        else if (operand == "-0")
            operand = "-" + digit;
        else
            operand += digit;
    }

    ui->display->setText(operand);
}

// 二元运算符核心逻辑
//  1) 连续按运算符时只替换运算符，不重复计算
//  2) 已有待定运算符且已输入新数字时，先算出中间结果
void MainWindow::inputOperator(const QString &op)
{
    if (typing) {
        if (!pendingOp.isEmpty()) {
            calculate();             // 先执行上一次挂起的运算
            if (errorState)
                return;
        } else {
            accumulator = currentValue();   // 第一次按运算符
        }
        typing = false;
    }
    // 若 !typing 说明刚按过运算符或刚出结果，直接替换运算符即可

    pendingOp = op;
    hasLastOp = false;               // 新运算开始，清掉可重复的 "=" 记录
}

// 执行 pendingOp 运算：accumulator (op) operand
void MainWindow::calculate()
{
    double right = currentValue();

    if (pendingOp == "+") {
        accumulator += right;
    } else if (pendingOp == "-") {
        accumulator -= right;
    } else if (pendingOp == "*") {
        accumulator *= right;
    } else if (pendingOp == "/") {
        if (qFuzzyIsNull(right)) {   // 除数为 0：给出错误提示
            showError("除数不能为零");
            return;
        }
        accumulator /= right;
    }

    showNumber(accumulator);
}

// 一元运算核心逻辑：直接作用于当前显示值，不改变 pendingOp
void MainWindow::applyUnary(const QString &op)
{
    double value = currentValue();

    if (op == "%") {
        // 百分比：有挂起运算时按「左操作数的百分比」计算，如 200 + 10% = 220
        if (!pendingOp.isEmpty())
            value = accumulator * value / 100.0;
        else
            value = value / 100.0;
    } else if (op == QString::fromUtf8("¹⁄ₓ")) {
        if (qFuzzyIsNull(value)) {   // 倒数：0 没有倒数
            showError("除数不能为零");
            return;
        }
        value = 1.0 / value;
    } else if (op == QString::fromUtf8("x²")) {
        value = value * value;       // 平方
    } else if (op == QString::fromUtf8("√x")) {
        if (value < 0) {             // 负数不能开平方
            showError("无效输入");
            return;
        }
        value = std::sqrt(value);
    }

    showNumber(value);
}


void MainWindow::on_btnClear_clicked()
{
    doClear();
}

void MainWindow::doClear()
{
    resetAll();
}


void MainWindow::on_btnClearEntry_clicked()
{
    doClearEntry();
}

void MainWindow::doClearEntry()
{
    if (errorState) {
        resetAll();
        return;
    }

    operand = "0";
    typing  = false;
    ui->display->setText("0");
}

// +/-：正负号切换
void MainWindow::on_btnNegatePlusMinus_clicked()
{
    doNegate();
}

void MainWindow::doNegate()
{
    if (errorState)
        return;

    if (qFuzzyIsNull(currentValue()))   // 0 取反没有意义
        return;

    if (operand.startsWith('-'))
        operand.remove(0, 1);
    else
        operand.prepend('-');

    ui->display->setText(operand);
}

// ============================================================
//  键盘输入：把按键映射成与按钮文字相同的字符串，
//  再调用与鼠标完全相同的核心函数，实现鼠标/键盘复用同一套逻辑
//  数字 0-9 / . / + - * / / = 或 Enter / Backspace / Esc(C) / Delete(CE) / F9(+/-)
//  一元运算：% / r(1/x) / q(x²) / s(√x)
// ============================================================
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();
    const QString txt = event->text();

    // 1) 数字 0-9（主键盘与小键盘都覆盖）
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        if (!errorState)
            inputDigit(QString(QChar('0' + (key - Qt::Key_0))));
        event->accept();
        return;
    }

    // 2) 小数点
    if (key == Qt::Key_Period || key == Qt::Key_Comma) {
        doDecimalPoint();
        event->accept();
        return;
    }

    // 3) 四则运算符（含小键盘与 Shift 组合）
    if (key == Qt::Key_Plus) {
        if (!errorState) inputOperator("+");
        event->accept();
        return;
    }
    if (key == Qt::Key_Minus) {
        if (!errorState) inputOperator("-");
        event->accept();
        return;
    }
    if (key == Qt::Key_Asterisk) {
        if (!errorState) inputOperator("*");
        event->accept();
        return;
    }
    if (key == Qt::Key_Slash) {
        if (!errorState) inputOperator("/");
        event->accept();
        return;
    }

    // 4) 等号 / 回车 / 小键盘 Enter
    if (key == Qt::Key_Equal || key == Qt::Key_Return || key == Qt::Key_Enter) {
        doEquals();
        event->accept();
        return;
    }

    // 5) 退格
    if (key == Qt::Key_Backspace) {
        doBackspace();
        event->accept();
        return;
    }

    // 6) 清除：Esc = C，Delete = CE
    if (key == Qt::Key_Escape) {
        doClear();
        event->accept();
        return;
    }
    if (key == Qt::Key_Delete) {
        doClearEntry();
        event->accept();
        return;
    }

    // 7) 正负号：F9（与 Windows 计算器一致）
    if (key == Qt::Key_F9) {
        doNegate();
        event->accept();
        return;
    }

    // 8) 一元运算：% / r(1/x) / q(x²) / s(√x)
    if (txt == "%") {
        if (!errorState) applyUnary("%");
        event->accept();
        return;
    }
    if (txt.compare("r", Qt::CaseInsensitive) == 0) {
        if (!errorState) applyUnary(QString::fromUtf8("¹⁄ₓ"));
        event->accept();
        return;
    }
    if (txt.compare("q", Qt::CaseInsensitive) == 0) {
        if (!errorState) applyUnary(QString::fromUtf8("x²"));
        event->accept();
        return;
    }
    if (txt.compare("s", Qt::CaseInsensitive) == 0) {
        if (!errorState) applyUnary(QString::fromUtf8("√x"));
        event->accept();
        return;
    }

    // 其余按键交给基类处理
    QMainWindow::keyPressEvent(event);
}
