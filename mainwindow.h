#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void processCapturedImage(int requestId, const QImage &img);
    int displayCapturedImage(int requestId, const QImage &img);
    void capture();
    void selectCamera();


private:
    void updateCameraList();
    void setupTimer();
    void thresh_callback(int, void* );

private:
    Ui::MainWindow *ui;
    class QCamera *camera;
    class QCameraImageCapture *imageCapture;
    class QTimer *timer ;
};
#endif // MAINWINDOW_H
