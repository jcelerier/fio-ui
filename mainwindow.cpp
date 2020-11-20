#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QProcess>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStorageInfo>
#include <QDirIterator>
#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    auto diskModel = new QStandardItemModel{this};
    ui->setupUi(this);
    this->centralWidget()->setLayout(ui->gridLayout_2);

    ui->diskList->setModel(diskModel);

    connect(ui->bench, &QPushButton::clicked,
            this, [=] {
        auto listElement = ui->diskList->currentIndex();
        auto drive = diskModel->data(listElement, Qt::DisplayRole).toString(); // ugly but I don't use that enough to remember the proper way...

        benchmarkDrive(drive);
    });

    // List disks... how to do this in a cross-platform way :-)
    for(auto info : QStorageInfo::mountedVolumes()) {
        diskModel->appendRow(new QStandardItem{info.rootPath()});
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::benchmarkDrive(QString drive)
{
    auto process = new QProcess{};
    process->setProgram("fio");
    process->setArguments({ // of course
                              QString("--filename=%1").arg(drive),
                              "--direct=1", "--rw=randread",
                              "--bs=4k", "--ioengine=libaio",
                              "--iodepth=256", "--runtime=120",
                              "--numjobs=4", "--time_based",
                              "--group_reporting", "--name=iops-test-job",
                              "--eta-newline=1", "--readonly",
                              "--output-format=json"
                          }); // I never used fio lol

    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, [=] (int ret, QProcess::ExitStatus status) {
        qDebug() << ret << status;
        // For now let's assume fio did not crash

        QByteArray data = process->readAllStandardOutput(); // I guess it writes on stdio

        int firstBrace = data.indexOf('{');
        if(firstBrace >= 0) {
            auto json = QJsonDocument::fromJson(data.mid(firstBrace));
            if(json.isObject()) {
                qDebug() << data; // yay, we have to look for the beginning of the json -_-
                processFioJson(json.object());
            }
        }
        process->deleteLater();
    });
    process->start(QIODevice::ReadOnly);
}

void MainWindow::processFioJson(QJsonObject obj)
{
    // In 2020 I'd use nlohmann instead, but well
    QString drive = obj["global options"].toObject()["filename"].toString();
    double perf = obj["jobs"].toArray()[0].toObject()["read"].toObject()["iops"].toDouble();

    on_benchmarkCompleted(drive, perf);
}

void MainWindow::on_benchmarkCompleted(QString drive, double perf)
{
    ui->disk->setText(drive);
    ui->performance->setText(tr("%1 iops").arg(perf));
}

