#include <iostream>

#include <QCameraInfo>
#include <QCameraImageCapture>
#include <QBuffer>
#include <QCameraViewfinder>
#include <QTimer>
#include <QComboBox>

#include <opencv2/opencv.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "mainwindow.h"
#include "cameralistmodel.h"
#include "./ui_mainwindow.h"

using namespace cv;
using namespace std;

Mat src_gray;
int thresh = 100;
RNG rng(12345);


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    updateCameraList();
    setupTimer();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::updateCameraList()
{
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    CameraListModel* listModel = new CameraListModel(cameras);
    ui->comboBox->setModel(listModel);
}

void MainWindow::setupTimer()
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this,&MainWindow::capture);
    timer->setInterval(500);
}

void MainWindow::selectCamera()
{
    // foreach (QCameraInfo cameraInfo, cameras) {
    //     names += cameraInfo.deviceName() + "\n";
    //     //Tu cherches la camera qui a le nom qu'on a choisi :
    //     //if (cameraInfo.deviceName() == "mycamera")

    // }
    // camera->setViewfinder(ui->viewfinder);
    auto infos =  ((CameraListModel*)(ui->comboBox->model()))->cameraInfo(ui->comboBox->currentIndex());
    camera = new QCamera(infos);
    imageCapture = new QCameraImageCapture(camera);
    camera->setCaptureMode(QCamera::CaptureVideo);
    //connect(imageCapture, &QCameraImageCapture::imageCaptured, this, &MainWindow::processCapturedImage);

    //const char* source_window = "Source";
    //namedWindow( source_window );

    const int max_thresh = 255;


    bool done = false;
    connect(imageCapture, &QCameraImageCapture::imageCaptured, this, &MainWindow::displayCapturedImage);
    camera->start();
    timer->start(200);

    // /*
    //     , [=](int id, const QImage &previewImage)
    //                      {
    //                          // Convertir QImage en matrice OpenCV
    //                          cv::Mat cvImage(previewImage.height(), previewImage.width(), CV_8UC4, const_cast<uchar*>(previewImage.bits()), size_t(previewImage.bytesPerLine()));
    //                         // et on fait les traitements ici pour balancer
    //                         QImage img= QImage((uchar*) cvImage.data, cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_RGB888);
    //                         QPixmap pixel = QPixmap::fromImage(img);
    //                         QByteArray ba;
    //                         QBuffer buffer(&ba);
    //                         buffer.open(QIODevice::WriteOnly);
    //                         img.save(&buffer, "PNG");
    //                     });*/
}

void MainWindow::processCapturedImage(int requestId, const QImage& img)
{
    //     Q_UNUSED(requestId);
    //     cv::Mat cvImage(img.height(), img.width(), CV_8UC1, const_cast<uchar*>(img.bits()), size_t(img.bytesPerLine()));
    //     // et on fait les traitements ici pour balancer
    //     QImage resimg= QImage((uchar*) cvImage.data, cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_ARGB32);
    //     ui->label->setPixmap(QPixmap::fromImage(resimg));

    //      std::vector<cv::Vec4i> hierarchy;
    //      std::vector<std::vector<cv::Point> > contours;
    //     // //cv::dilate(maMatrice, dilatedMat, structuringElement, cv::Point(1,1));
    //      cv::findContours(cvImage, contours, hierarchy,
    //                       cv::RETR_EXTERNAL,
    //                       cv::CHAIN_APPROX_NONE,
    //                       cv::Point(1, 1));

    //     float minContourSize = 0.1f;
    //     double segmentSize = 0.1;
    //     double max_contour = 1;
    //     uint numForme = 0;
    //     cv::RotatedRect box;

    //     std::vector<std::vector<cv::Point>>hull(contours.size());
    //     std::vector<std::vector<cv::Point>>hull2(contours.size());
    //     for (auto &&it: contours)
    //     {
    //         auto aire = cv::contourArea(it);
    //         if ( aire < minContourSize)
    //         {
    //             continue; // sauter les petites formes
    //         }

    //         // calcul du point central avec les Moments
    //         cv::Moments m = cv::moments(it);
    //         auto cx = int(m.m10 / m.m00);
    //         auto cy = int(m.m01 / m.m00);
    //         // Ok on a la position x si position x checked, et la position y si position y checked


    //          auto pv = hull2[numForme];

    //          cv::convexHull(it, hull[uint(numForme)], false);
    //          cv::approxPolyDP(hull[numForme], pv, segmentSize, true); // lit les formes dans hull et les écrit dans hull2

    //         // // il y a aussi cette fonction qui trouve le plus petit rectangle englobant la forme.
    //         // // le rectangle peut être orienté, donc ça permet de trouver l'angle
    //         // auto rect = cv::minAreaRect(max_contour);
    //         // cv::Mat boxPts;
    //         // cv::boxPoints(rect,boxPts);
    //         // ++numForme;
    //     }
    //     cv::RNG rng(12345);
    //     cv::Mat drawing = cv::Mat::zeros( cvImage.size(), CV_8UC3 );
    //     for( size_t i = 0; i< contours.size(); i++ )
    //     {
    //         cv::Scalar color = cv::Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
    //         drawContours( drawing, contours, (int)i, color );
    //         drawContours( drawing, hull, (int)i, color );
    //     }


    //     imshow( "Hull demo", drawing );
}

int MainWindow::displayCapturedImage(int requestId, const QImage &previewImage)
{
    cv::Mat cvImage(previewImage.height(), previewImage.width(),  CV_8UC4,
                    const_cast<uchar*>(previewImage.bits()), size_t(previewImage.bytesPerLine()));
    if( cvImage.empty() )
    {
        cout << "Could not open or find the image!\n" << endl;
        return -1;
    }
    cv:: Mat resizeMat(cv::Size(cvImage.cols/5, cvImage.rows/5),CV_8UC4);
    cv::resize(cvImage,resizeMat, resizeMat.size(),0,0,INTER_LINEAR);
    QImage img= QImage((uchar*) resizeMat.data, resizeMat.cols, resizeMat.rows, resizeMat.step, QImage::Format_ARGB32);
    QPixmap pixel = QPixmap::fromImage(img);
    ui->displayImage->setPixmap(pixel);
    try{
        cvtColor( resizeMat, src_gray, COLOR_BGR2GRAY );
    }

    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    blur( src_gray, src_gray, Size(3,3) );

    //imshow( source_window, cvImage );
    //if(!done){
    //createTrackbar( "Canny thresh:", source_window, &thresh, max_thresh, thresh_callback );
    thresh_callback(0,0);
    //  done= true;
    //}
    waitKey();
    return 0;
}

void MainWindow::capture()
{
    imageCapture->capture();
}



void MainWindow::thresh_callback(int, void* )
{
    Mat canny_output;
    Canny( src_gray, canny_output, thresh, thresh*2 );
    vector<vector<Point> > contours;
    findContours( canny_output, contours, RETR_TREE, CHAIN_APPROX_SIMPLE );
    vector<vector<Point> >hull( contours.size() );
    for( size_t i = 0; i < contours.size(); i++ )
    {
        convexHull( contours[i], hull[i] );
    }
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( size_t i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
        drawContours( drawing, contours, (int)i, color );
        drawContours( drawing, hull, (int)i, color );
    }
    QImage img= QImage((uchar*) drawing.data, drawing.cols, drawing.rows, drawing.step, QImage::Format_RGB888);
    QPixmap pixel = QPixmap::fromImage(img);
    ui->displayImageCleaned->setPixmap(pixel);
}
