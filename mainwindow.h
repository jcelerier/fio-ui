#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void benchmarkDrive(QString drive);
    void processFioJson(QJsonObject obj);
    void on_benchmarkCompleted(QString drive, double perf);

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
