#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QDateTime>

#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/features2d.hpp>

#include "paddle/config.h"
#include "paddle/ocr_det.h"
#include "paddle/ocr_rec.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void initWidget();

    QPixmap cvMatToPixmap(const cv::Mat imageMat);
    void performOCR(const cv::Mat& image);
    void saveTextToFile(const QString& filePath = "");

    void toggleSaveMode();
    void startPPTAutoCapture();
    void stopPPTAutoCapture();
    bool detectPPTChange(const cv::Mat& currentFrame);

private slots:
    void on_pb_selectImage_clicked();
    void on_pb_openCamera_clicked();
    void on_pb_captureImage_clicked();
    void on_pb_saveText_clicked();
    void on_pb_toggleSaveMode_clicked();
    void updateCameraFrame();
    void onSaveFormatChanged();
    void onPPTDetectionTimeout();

private:
    Ui::MainWindow *ui;

    cv::Mat m_showMat;
    PaddleOCR::DBDetector *m_det;
    PaddleOCR::CRNNRecognizer *m_rec;

    // 摄像头相关成员变量
    cv::VideoCapture *m_camera;
    QTimer *m_cameraTimer;
    bool m_isCameraOpen;
    cv::Mat m_currentFrame;
    cv::Mat m_capturedFrame;

    // 文本保存相关
    bool m_autoSaveMode;
    QString m_lastSaveDir;
    QString m_resultDir; // result文件夹路径

    // 文本保存格式相关
    bool m_singleDocumentMode;
    QString m_singleDocumentPath;
    int m_ocrCount;

    // PPT自动捕获相关
    bool m_pptAutoCapture;
    QTimer *m_pptDetectionTimer;
    cv::Mat m_prevFrame;
    cv::Mat m_background;
    double m_changeThreshold;
    int m_stableFrameCount;
    bool m_pptChangeDetected;
    int m_consecutiveChangeCount; // 连续变化计数

    // 添加检测信息显示标签
    QLabel *m_detectionInfoLabel;

    // // PPT矩形检测相关
    // cv::Rect m_pptRect;
    // bool m_pptRectValid;
    // int m_rectStableCount;
    // int m_maxRectStableFrames;

    // 图片保存相关
    bool m_saveCapturedImages; // 是否保存拍摄的照片
    QString m_imageSaveDir;    // 图片保存目录

    // // 矩形检测函数
    // cv::Rect detectPPTRectangle(const cv::Mat& frame);
    // void updatePPTRectangle(const cv::Mat& frame);
    // void drawPPTRectangle(cv::Mat& frame);

    // 初始化result文件夹的函数
    void initResultDirectory();
    // 保存图片的函数
    void saveCapturedImage(const cv::Mat& image, const QString& prefix = "capture");
};
#endif // MAINWINDOW_H
