#include <iostream>

#include <QCameraInfo>
#include <QCameraImageCapture>
#include <QBuffer>
#include <QCameraViewfinder>
#include <QTimer>
#include <QComboBox>

#include <opencv2/opencv.hpp>
//#include "opencv2/imgcodecs.hpp"
//#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "mainwindow.h"
#include "cameralistmodel.h"
#include "./ui_mainwindow.h"

using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,rng{new RNG(12345)}
{
    ui->setupUi(this);
    ui->thresholdSlider->setValue(m_thresh);
    ui->segmentSlider->setValue(m_segmentSize);
    ui->contourSizeSlider->setValue(m_minContourSize);
    updateCameraList();
    setupTimer();
    for(int i =0; i <50;i++)
    {
        Scalar* color = new Scalar( rng->uniform(0, 256), rng->uniform(0,256), rng->uniform(0,256) );
        colors.push_back(color);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

//List all available camera in the combobox
void MainWindow::updateCameraList()
{
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    CameraListModel* listModel = new CameraListModel(cameras);
    ui->comboBox->setModel(listModel);
}

//Timer will take a capture every interval
//This capture will be treated in displayCapturedImage (connection is done in the selectCamera function)
void MainWindow::setupTimer()
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this,&MainWindow::capture);
    timer->setInterval(500);
}

void MainWindow::capture()
{
    imageCapture->capture();
}

//connect image capture with the image treatment function
void MainWindow::selectCamera()
{
    auto infos =  ((CameraListModel*)(ui->comboBox->model()))->cameraInfo(ui->comboBox->currentIndex());
    camera = new QCamera(infos);
    imageCapture = new QCameraImageCapture(camera);
    camera->setCaptureMode(QCamera::CaptureVideo);
    connect(imageCapture, &QCameraImageCapture::imageCaptured, this, &MainWindow::displayCapturedImage);
    camera->start();
    timer->start(200);
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


    //imshow( source_window, cvImage );
    processCapturedImage(0,resizeMat);

    return 0;
}

void MainWindow::changeThreshold(int newThresh)
{
    m_thresh = newThresh;
    ui->thresholdLabel->setText(QString::number(m_thresh));
}

void MainWindow::changeSegmentSize(int segmentSize)
{
    m_segmentSize = (float)segmentSize/10.f;
    ui->segmentLabel->setText(QString::number(m_segmentSize));
}

void MainWindow::changeMinContourSize(int minContourSize)
{
    m_minContourSize = (float)minContourSize/10;
    ui->contourSizeLabel->setText(QString::number(m_minContourSize));
}

void MainWindow::processCapturedImage(int, cv:: Mat& resizeMat)
{
    Mat src_gray;
    try{
        cvtColor( resizeMat, src_gray, COLOR_BGR2GRAY );
    }

    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    blur( src_gray, src_gray, Size(3,3) );



    //      std::vector<std::vector<cv::Point> > contours;
    //     // //cv::dilate(maMatrice, dilatedMat, structuringElement, cv::Point(1,1));
    //      cv::findContours(cvImage, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point(1, 1));




    Mat canny_output;
    Canny( src_gray, canny_output, m_thresh, m_thresh*2 );
    std::vector<cv::Vec4i> hierarchy;
    vector<vector<Point> > contours;
    int dilation_size = 2;
    Mat element = getStructuringElement( MORPH_CROSS,
                                        Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                        Point( dilation_size, dilation_size ) );

    Mat dilatedMat;
    cv::dilate(canny_output, dilatedMat, element, cv::Point(1,1));
    findContours(dilatedMat, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, cv::Point(1, 1));
    //findContours( dilatedMat, contours, RETR_TREE, CHAIN_APPROX_SIMPLE );


    double max_contour = 1;
    uint numForme = 0;
    cv::RotatedRect box;

    std::vector<std::vector<cv::Point>>hull(contours.size());
    std::vector<std::vector<cv::Point>>hull2;;
    for (auto &&it: contours)
    {
        auto aire = cv::contourArea(it);
        if ( aire < m_minContourSize)
        {
            continue; // sauter les petites formes
        }

        // calcul du point central avec les Moments
        cv::Moments m = cv::moments(it);
        auto cx = int(m.m10 / m.m00);
        auto cy = int(m.m01 / m.m00);
        // Ok on a la position x si position x checked, et la position y si position y checked
        auto pv = std::vector<cv::Point>();
        cv::convexHull(it, hull[uint(numForme)], false);
        cv::approxPolyDP(hull[numForme], pv, m_segmentSize, true); // lit les formes dans hull et les écrit dans hull2
        //il y a aussi cette fonction qui trouve le plus petit rectangle englobant la forme.
        //le rectangle peut être orienté, donc ça permet de trouver l'angle
        //auto rect = cv::minAreaRect(max_contour);
        //cv::Mat boxPts;
        //cv::boxPoints(rect,boxPts);
        hull2.push_back(pv);
        ++numForme;
    }

    // vector<vector<Point> >hull( contours.size() );
    // for( size_t i = 0; i < contours.size(); i++ )
    // {
    //     convexHull( contours[i], hull[i] );
    // }
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );

    for( size_t i = 0; i< numForme; i++ )
    {
        drawContours( drawing, contours, (int)i, *(colors.at(0)) );
        drawContours( drawing, hull, (int)i, *(colors.at(1)));
        //if(hull2.at(i).size() >0)
        //    drawContours( drawing, hull2, (int)i, *(colors.at(0) ));
    }
    QImage img= QImage((uchar*) drawing.data, drawing.cols, drawing.rows, drawing.step, QImage::Format_RGB888);
    QPixmap pixel = QPixmap::fromImage(img);
    ui->displayImageCleaned->setPixmap(pixel);
}
