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
    void performOCR(const QString& imageName);
    void performOCR(const cv::Mat& image); // 重载函数，直接处理cv::Mat
    void saveTextToFile(const QString& filePath = ""); // 保存文本到文件
    void toggleSaveMode(); // 切换保存模式

private slots:
    void on_pb_selectImage_clicked();
    void on_pb_openCamera_clicked(); // 打开/关闭摄像头
    void on_pb_captureImage_clicked(); // 拍照识别
    void on_pb_saveText_clicked(); // 保存文本按钮
    void on_pb_toggleSaveMode_clicked(); // 切换保存模式按钮
    void updateCameraFrame(); // 更新摄像头帧
    void onSaveFormatChanged(); // 保存格式改变

private:
    Ui::MainWindow *ui;

    cv::Mat m_showMat;   //界面显示图像
    PaddleOCR::DBDetector *m_det;       //文本检测模块
    PaddleOCR::CRNNRecognizer *m_rec;   //文本识别模块

    // 摄像头相关成员变量
    cv::VideoCapture *m_camera;
    QTimer *m_cameraTimer;
    bool m_isCameraOpen;
    cv::Mat m_currentFrame; // 当前摄像头帧
    cv::Mat m_capturedFrame; // 拍摄的照片帧

    // 文本保存相关
    bool m_autoSaveMode; // 自动保存模式
    QString m_lastSaveDir; // 最后保存的目录

    // 文本保存格式相关
    bool m_singleDocumentMode; // true:单文档模式, false:多文档模式
    QString m_singleDocumentPath; // 单文档模式下的文件路径
    int m_ocrCount; // OCR识别次数计数，用于单文档模式

};
#endif // MAINWINDOW_H
