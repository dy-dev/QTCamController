#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "opencv2/core/types.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

namespace cv {
class Mat;
class RNG;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectCamera();
    int displayCapturedImage(int requestId, const QImage &img);
    void processCapturedImage(int, cv:: Mat& resizeMat);
    void capture();
    void changeThreshold(int);
    void changeSegmentSize(int);
    void changeMinContourSize(int);

private:
    void updateCameraList();
    void setupTimer();

private:
    Ui::MainWindow *ui;
    int m_thresh{100};
    cv::RNG* rng;
    std::vector<cv::Scalar*> colors;

    double m_segmentSize {0.1};
    float m_minContourSize{1.f};

    class QCamera *camera;
    class QCameraImageCapture *imageCapture;
    class QTimer *timer ;
};
#endif // MAINWINDOW_H
