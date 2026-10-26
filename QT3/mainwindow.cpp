#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    statusCursorlabel.setMaximumWidth(150);
    statusCursorlabel.setText("length:"+QString::number(0)+"  lines:"+QString::number(1));
    ui->statusbar->addPermanentWidget(&statusCursorlabel);

    statuslabel.setMaximumWidth(150);
    statuslabel.setText("Ln:"+QString::number(0)+"  Col:"+QString::number(1));
    ui->statusbar->addPermanentWidget(&statuslabel);

    QLabel *author=new QLabel(ui->statusbar);
    author->setText(tr("张家富"));
    ui->statusbar->addPermanentWidget(author);
}

MainWindow::~MainWindow()
{
    delete ui;
}

