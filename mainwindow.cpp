#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->initWidget();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_det;
    delete m_rec;

    if (m_camera && m_camera->isOpened()) {
        m_camera->release();
    }
    delete m_camera;
    delete m_cameraTimer;
    delete m_pptDetectionTimer;
}

void MainWindow::initWidget()
{
    // 初始化result文件夹
    initResultDirectory();

    m_showMat = cv::Mat();
    ui->cb_showRect->setChecked(false);

    // 设置检测信息标签的初始状态
    ui->label_detection_info->setText("检测信息: 等待开始...");

    // 初始化摄像头相关变量
    m_camera = nullptr;
    m_cameraTimer = new QTimer(this);
    m_isCameraOpen = false;
    m_currentFrame = cv::Mat();
    m_capturedFrame = cv::Mat();

    // 初始化文本保存相关
    m_autoSaveMode = false;
    //m_lastSaveDir = QDir::currentPath();

    // 初始化文本保存格式相关
    m_singleDocumentMode = true;
    //m_singleDocumentPath = QDir(m_lastSaveDir).filePath("OCR_Results.txt");
    m_ocrCount = 0;

    // 初始化PPT自动捕获相关
    m_pptAutoCapture = false;
    m_pptDetectionTimer = new QTimer(this);
    m_prevFrame = cv::Mat();
    m_background = cv::Mat();
    m_changeThreshold = 0.4;
    m_stableFrameCount = 0;
    m_pptChangeDetected = false;
    m_consecutiveChangeCount = 0; // 初始化连续变化计数

    // 初始化图片保存相关
    m_saveCapturedImages = false; // 默认不保存图片

    connect(m_cameraTimer, &QTimer::timeout, this, &MainWindow::updateCameraFrame);
    connect(m_pptDetectionTimer, &QTimer::timeout, this, &MainWindow::onPPTDetectionTimeout);

    ui->lb_captured->setText("拍摄的照片将显示在这里");
    ui->lb_captured->setAlignment(Qt::AlignCenter);
    ui->lb_captured->setStyleSheet("border: 1px solid gray;");

    ui->lb_show->setScaledContents(false);
    ui->lb_show->setAlignment(Qt::AlignCenter);
    ui->lb_captured->setScaledContents(false);
    ui->lb_captured->setAlignment(Qt::AlignCenter);

    ui->pb_saveText->setText("保存文本");
    ui->pb_toggleSaveMode->setText("手动保存模式");

    ui->rb_singleDoc->setChecked(true);
    ui->rb_multiDoc->setChecked(false);

    connect(ui->rb_singleDoc, &QRadioButton::toggled, this, &MainWindow::onSaveFormatChanged);
    connect(ui->rb_multiDoc, &QRadioButton::toggled, this, &MainWindow::onSaveFormatChanged);

    ui->cb_saveImages->setText("保存拍摄的照片");
    ui->cb_saveImages->setChecked(false);
    connect(ui->cb_saveImages, &QCheckBox::stateChanged, this, [this](int state) {
        m_saveCapturedImages = (state == Qt::Checked);
        if (m_saveCapturedImages) {
            qDebug() << "已启用自动保存拍摄照片到:" << m_imageSaveDir;
            statusBar()->showMessage("已启用自动保存拍摄照片", 1000);
        } else {
            qDebug() << "已禁用自动保存拍摄照片";
            statusBar()->showMessage("已禁用自动保存拍摄照片", 1000);
        }
    });


    ui->pb_startPPT->setText("开始PPT自动捕获");
    connect(ui->pb_startPPT, &QPushButton::clicked, this, [this]() {
        if (!m_pptAutoCapture) {
            startPPTAutoCapture();
        } else {
            stopPPTAutoCapture();
        }
    });

    ui->hs_threshold->setRange(1, 100);
    ui->hs_threshold->setValue(40);
    connect(ui->hs_threshold, &QSlider::valueChanged, this, [this](int value) {
        m_changeThreshold = value / 100.0;
        ui->lb_thresholdValue->setText(QString("阈值: %1%").arg(value));
    });

    // 添加延迟时间设置
    ui->hs_delay->setRange(0, 5000); // 0-5秒延迟
    ui->hs_delay->setValue(1000); // 默认1秒延迟
    connect(ui->hs_delay, &QSlider::valueChanged, this, [this](int value) {
        ui->lb_delayValue->setText(QString("延迟: %1ms").arg(value));
    });
    ui->lb_delayValue->setText("延迟: 1000ms");

    //初始化PaddleOCR
    PaddleOCR::OCRConfig config("./config.txt");

    m_det = new PaddleOCR::DBDetector(config.det_model_dir, config.use_gpu, config.gpu_id,
                                      config.gpu_mem, config.cpu_math_library_num_threads,
                                      config.use_mkldnn, config.max_side_len, config.det_db_thresh,
                                      config.det_db_box_thresh, config.det_db_unclip_ratio,
                                      config.visualize, config.use_tensorrt, config.use_fp16);

    m_rec = new PaddleOCR::CRNNRecognizer(config.rec_model_dir, config.use_gpu, config.gpu_id,
                                          config.gpu_mem, config.cpu_math_library_num_threads,
                                          config.use_mkldnn, config.char_list_file,
                                          config.use_tensorrt, config.use_fp16);
}

void MainWindow::startPPTAutoCapture()
{
    if (!m_isCameraOpen) {
        QMessageBox::warning(this, "错误", "请先打开摄像头！");
        return;
    }

    // 立即捕获当前帧作为初始参考帧
    if (!m_currentFrame.empty()) {
        m_prevFrame = m_currentFrame.clone();
        ui->label_detection_info->setText("检测信息: PPT自动捕获已启动，初始参考帧已设置");
        qDebug() << "已设置初始参考帧，尺寸:" << m_prevFrame.cols << "x" << m_prevFrame.rows;
    } else {
        QMessageBox::warning(this, "错误", "无法获取当前帧，请检查摄像头！");
        return;
    }

    m_pptAutoCapture = true;
    m_pptDetectionTimer->start(500);
    ui->pb_startPPT->setText("停止PPT自动捕获");
    ui->pb_openCamera->setEnabled(false);
    ui->pb_captureImage->setEnabled(false);

    // 重置所有检测状态
    m_stableFrameCount = 0;
    m_pptChangeDetected = false;
    m_consecutiveChangeCount = 0;
    // m_rectStableCount = 0;
    // m_pptRectValid = false;

    QMessageBox::information(this, "PPT自动捕获", "PPT自动捕获已启动\n"
                                                  "初始参考帧已设置\n"
                                                  "当检测到PPT切换时，将自动拍照并识别文字");
}

void MainWindow::stopPPTAutoCapture()
{
    m_pptAutoCapture = false;
    m_pptDetectionTimer->stop();
    ui->pb_startPPT->setText("开始PPT自动捕获");
    ui->pb_openCamera->setEnabled(true);
    ui->pb_captureImage->setEnabled(true);

    ui->label_detection_info->setText("检测信息: PPT自动捕获已停止");

    QMessageBox::information(this, "PPT自动捕获", "PPT自动捕获已停止");
}

bool MainWindow::detectPPTChange(const cv::Mat& currentFrame)
{
    if (m_prevFrame.empty() || currentFrame.empty()) {
        m_prevFrame = currentFrame.clone();
        ui->label_detection_info->setText("检测信息: 初始化参考帧...");
        return false;
    }

    // 全局检测，不再使用PPT矩形区域
    cv::Mat currentROI = currentFrame;
    cv::Mat prevROI = m_prevFrame;

    // 转换为灰度图
    cv::Mat grayCurrent, grayPrev;
    cv::cvtColor(currentROI, grayCurrent, cv::COLOR_BGR2GRAY);
    cv::cvtColor(prevROI, grayPrev, cv::COLOR_BGR2GRAY);

    // 使用ORB特征检测器
    auto orb = cv::ORB::create(1000);
    std::vector<cv::KeyPoint> kp1, kp2;
    cv::Mat desc1, desc2;
    orb->detectAndCompute(grayCurrent, cv::noArray(), kp1, desc1);
    orb->detectAndCompute(grayPrev, cv::noArray(), kp2, desc2);

    if (kp1.empty() || kp2.empty() || desc1.empty() || desc2.empty()) {
        ui->label_detection_info->setText("检测信息: 特征点检测失败");
        m_prevFrame = currentFrame.clone();
        return false;
    }

    // 特征点匹配
    auto matcher = cv::BFMatcher::create(cv::NORM_HAMMING);
    std::vector<cv::DMatch> matches;
    matcher->match(desc1, desc2, matches);

    if (matches.empty()) {
        ui->label_detection_info->setText("检测信息: 特征点匹配失败");
        m_prevFrame = currentFrame.clone();
        return false;
    }

    // 计算匹配质量
    double min_dist = 1000, max_dist = 0;
    for (const auto& match : matches) {
        if (match.distance < min_dist) min_dist = match.distance;
        if (match.distance > max_dist) max_dist = match.distance;
    }

    // 筛选优质匹配
    std::vector<cv::DMatch> good_matches;
    double match_threshold = std::max(2.5 * min_dist, 30.0);

    for (const auto& match : matches) {
        if (match.distance <= match_threshold) {
            good_matches.push_back(match);
        }
    }

    // 计算变化比例
    double changeRatio = 0.0;
    size_t min_keypoints = std::min(kp1.size(), kp2.size());
    if (min_keypoints == 0) {
        changeRatio = 1.0;
    } else {
        changeRatio = 1.0 - (double)good_matches.size() / min_keypoints;
    }

    // 更新检测信息显示 - 改为全局检测信息
    QString detectionInfo = QString("检测信息: 全局检测 变化率: %1% 优质匹配: %2 / %3 特征点: %4 - %5 阈值: %6")
                                .arg(QString::number(changeRatio * 100, 'f', 1))
                                .arg(good_matches.size())
                                .arg(matches.size())
                                .arg(kp1.size())
                                .arg(kp2.size())
                                .arg(m_changeThreshold);

    ui->label_detection_info->setText(detectionInfo);

    qDebug() << detectionInfo;

    // 更新前一帧
    m_prevFrame = currentFrame.clone();

    return changeRatio > m_changeThreshold;
}
void MainWindow::onPPTDetectionTimeout()
{
    if (!m_pptAutoCapture || !m_isCameraOpen || m_currentFrame.empty()) {
        return;
    }

    // 如果正在等待延迟拍照，跳过检测
    if (m_pptChangeDetected) {
        return;
    }

    // 检测PPT切换
    bool pptChanged = detectPPTChange(m_currentFrame);

    if (pptChanged) {
        m_consecutiveChangeCount++;
        m_stableFrameCount = 0;

        qDebug() << "检测到PPT变化，连续次数:" << m_consecutiveChangeCount;

        if (m_consecutiveChangeCount >= 1) {
            m_pptChangeDetected = true;

            // 获取延迟时间
            int delayMs = ui->hs_delay->value();
            qDebug() << "确认PPT切换，延迟" << delayMs << "ms后拍照";

            // 延迟拍照，确保PPT完全切换
            QTimer::singleShot(delayMs, this, [this]() {
                if (m_pptAutoCapture && m_pptChangeDetected) {
                    qDebug() << "执行自动拍照和OCR识别";
                    // 自动拍照并识别
                    on_pb_captureImage_clicked();

                    // 重置状态，允许下一次检测
                    m_pptChangeDetected = false;
                    m_consecutiveChangeCount = 0;

                    // 设置保护期，避免重复检测同一变化
                    QTimer::singleShot(2000, this, [this]() {
                        // 重新设置参考帧
                        if (!m_currentFrame.empty()) {
                            m_prevFrame = m_currentFrame.clone();
                            qDebug() << "保护期结束，重新设置参考帧";
                        }
                    });
                }
            });
        }
    } else {
        // 如果没有检测到变化，缓慢减少连续计数
        if (m_consecutiveChangeCount > 0) {
            m_consecutiveChangeCount--;
        }
        m_stableFrameCount++;

        // 稳定帧数足够多时完全重置
        if (m_stableFrameCount > 10) {
            m_consecutiveChangeCount = 0;
        }
    }
}

void MainWindow::onSaveFormatChanged()
{
    bool oldMode = m_singleDocumentMode;
    m_singleDocumentMode = ui->rb_singleDoc->isChecked();

    if (m_singleDocumentMode) {
        // 单文档模式：使用result文件夹下的OCR_Results.txt
        m_singleDocumentPath = QDir(m_resultDir).filePath("OCR_Results.txt");
        if (!oldMode) {
            QMessageBox::information(this, "保存格式",
                                     QString("已切换到单文档模式\n所有识别结果将追加到同一个文件中:\n%1").arg(m_singleDocumentPath));
        }
    } else {
        if (oldMode) {
            QMessageBox::information(this, "保存格式",
                                     "已切换到多文档模式\n每个识别结果将保存到result文件夹下的单独文件中");
        }
    }
}

QPixmap MainWindow::cvMatToPixmap(const cv::Mat imageMat)
{
    if (imageMat.empty()) {
        return QPixmap();
    }

    QImage showImage;
    if(imageMat.channels() > 1)
    {
        showImage = QImage((const uchar*)(imageMat.data), imageMat.cols, imageMat.rows,
                           imageMat.step, QImage::Format_RGB888).rgbSwapped();
    }
    else
    {
        cv::Mat rgbMat;
        cv::cvtColor(imageMat, rgbMat, cv::COLOR_GRAY2BGR);
        showImage = QImage((const uchar*)(rgbMat.data), rgbMat.cols, rgbMat.rows,
                           rgbMat.step, QImage::Format_RGB888).rgbSwapped();
    }

    return QPixmap::fromImage(showImage);
}

QPixmap scalePixmapToLabel(const QPixmap& pixmap, QLabel* label)
{
    if (pixmap.isNull() || !label) {
        return pixmap;
    }

    return pixmap.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void MainWindow::on_pb_selectImage_clicked()
{
    QString imageName = QFileDialog::getOpenFileName(this, "选择图像", "E:/photo/OCRTest/", "Image File(*.png *.jpg *.bmp)");
    if (!imageName.isEmpty())
    {
        m_showMat = cv::imread(imageName.toLocal8Bit().toStdString(), 1);
        QPixmap showPixmap = cvMatToPixmap(m_showMat.clone());

        if (!showPixmap.isNull())
        {
            ui->tb_text->clear();
            showPixmap = scalePixmapToLabel(showPixmap, ui->lb_show);
            ui->lb_show->setPixmap(showPixmap);

            QTimer::singleShot(100, this, [this, imageName]() {

                performOCR(m_showMat);

                if (m_autoSaveMode) {
                    saveTextToFile();
                }
            });
        }
    }
}

void MainWindow::on_pb_openCamera_clicked()
{
    if (!m_isCameraOpen) {
        if (!m_camera) {
            m_camera = new cv::VideoCapture(0);
        }

        if (m_camera->isOpened()) {
            m_isCameraOpen = true;
            m_cameraTimer->start(33);
            ui->pb_openCamera->setText("关闭摄像头");
            ui->pb_captureImage->setEnabled(true);

            ui->lb_captured->clear();
            ui->lb_captured->setText("拍摄的照片将显示在这里");

            if (m_pptAutoCapture) {
                m_pptDetectionTimer->start(500);
            }
        } else {
            QMessageBox::warning(this, "错误", "无法打开摄像头！");
            delete m_camera;
            m_camera = nullptr;
        }
    } else {
        if (m_pptAutoCapture) {
            stopPPTAutoCapture();
        }

        m_cameraTimer->stop();
        if (m_camera && m_camera->isOpened()) {
            m_camera->release();
        }
        m_isCameraOpen = false;
        ui->pb_openCamera->setText("打开摄像头");
        ui->pb_captureImage->setEnabled(false);

        ui->lb_show->clear();
    }
}

void MainWindow::updateCameraFrame()
{
    if (m_camera && m_camera->isOpened()) {
        cv::Mat frame;
        *m_camera >> frame;

        if (!frame.empty()) {
            m_currentFrame = frame.clone();

            // 在调试模式下绘制PPT矩形和状态信息
            cv::Mat displayFrame = frame.clone();
            if (m_pptAutoCapture) {
                //drawPPTRectangle(displayFrame);

                // 显示变化检测状态
                std::string status = "Monitoring";
                cv::Scalar color(0, 255, 0); // 绿色

                if (m_pptChangeDetected) {
                    status = "CAPTURING...";
                    color = cv::Scalar(0, 0, 255); // 红色
                } else if (m_consecutiveChangeCount > 0) {
                    status = "Change Detected: " + std::to_string(m_consecutiveChangeCount);
                    color = cv::Scalar(0, 165, 255); // 橙色
                }

                cv::putText(displayFrame, status,
                            cv::Point(10, 30),
                            cv::FONT_HERSHEY_SIMPLEX, 0.7, color, 2);

                // 显示阈值信息
                std::string thresholdInfo = "Threshold: " +
                                            std::to_string(static_cast<int>(m_changeThreshold * 100)) + "%";
                cv::putText(displayFrame, thresholdInfo,
                            cv::Point(10, 60),
                            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
            }

            QPixmap showPixmap = cvMatToPixmap(displayFrame);
            showPixmap = scalePixmapToLabel(showPixmap, ui->lb_show);
            ui->lb_show->setPixmap(showPixmap);
        }
    }
}

void MainWindow::on_pb_captureImage_clicked()
{
    if (!m_currentFrame.empty()) {
        m_capturedFrame = m_currentFrame.clone();

        QPixmap capturedPixmap = cvMatToPixmap(m_capturedFrame);
        capturedPixmap = scalePixmapToLabel(capturedPixmap, ui->lb_captured);
        ui->lb_captured->setPixmap(capturedPixmap);

        // 保存拍摄的图片
        saveCapturedImage(m_capturedFrame, "capture");

        QTimer::singleShot(100, this, [this]() {
            performOCR(m_capturedFrame);

            if (m_autoSaveMode) {
                saveTextToFile();
            }
        });
    }
}

void MainWindow::performOCR(const cv::Mat& image)
{
    // 识别前显示"正在识别..."
    ui->label_result->setText("正在识别...");
    // 添加短暂延迟，让用户能看到"正在识别..."状态
    QEventLoop loop;
    QTimer::singleShot(100, &loop, &QEventLoop::quit);
    loop.exec();

    cv::Mat srcMat = image.clone();
    std::vector<std::vector<std::vector<int>>> boxes;

    cv::Mat dstMat = m_det->Run(srcMat, boxes);
    std::vector<std::string> res = m_rec->Run(boxes, srcMat, nullptr);

    QPixmap resultPixmap;
    if (ui->cb_showRect->isChecked()) {
        resultPixmap = cvMatToPixmap(dstMat);
    } else {
        resultPixmap = cvMatToPixmap(srcMat);
    }

    resultPixmap = scalePixmapToLabel(resultPixmap, ui->lb_show);
    ui->lb_show->setPixmap(resultPixmap);

    ui->tb_text->clear();
    // 识别完成后显示"识别结果："
    ui->label_result->setText("识别结果：");

    for (auto s : res) {
        ui->tb_text->append(s.data());
    }

    m_ocrCount++;
}

void MainWindow::saveTextToFile(const QString& filePath)
{
    QString text = ui->tb_text->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::information(this, "提示", "没有文本可保存！");
        return;
    }

    QString savePath = filePath;
    QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text;

    if (savePath.isEmpty()) {
        if (m_autoSaveMode) {
            // 自动保存模式：强制使用result文件夹
            if (m_singleDocumentMode) {
                savePath = m_singleDocumentPath;
                mode = QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text;
            } else {
                QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
                QString fileName = QString("OCR_Result_%1.txt").arg(timestamp);
                savePath = QDir(m_resultDir).filePath(fileName);
                mode = QIODevice::WriteOnly | QIODevice::Text;
            }
        } else {
            // 手动保存模式：允许用户选择，但默认使用result文件夹
            QString defaultFileName;
            if (m_singleDocumentMode) {
                defaultFileName = "OCR_Results.txt";
                QString proposedPath = QDir(m_resultDir).filePath(defaultFileName);
                if (QFile::exists(proposedPath)) {
                    mode = QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text;
                }
                savePath = QFileDialog::getSaveFileName(this, "保存文本",
                                                        proposedPath,
                                                        "文本文件 (*.txt)");
            } else {
                QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
                defaultFileName = QString("OCR_Result_%1.txt").arg(timestamp);
                savePath = QFileDialog::getSaveFileName(this, "保存文本",
                                                        QDir(m_resultDir).filePath(defaultFileName),
                                                        "文本文件 (*.txt)");
            }

            if (savePath.isEmpty()) {
                return;
            }

            if (m_singleDocumentMode && QFile::exists(savePath)) {
                mode = QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text;
            }
        }
    }

    QFile file(savePath);
    if (file.open(mode)) {
        QTextStream out(&file);

        if (mode & QIODevice::Append) {
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            out << "\n\n=== OCR识别结果 - " << timestamp << " ===\n";
        } else {
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            out << "=== OCR识别结果 - " << timestamp << " ===\n";
        }

        out << text;
        file.close();

        // 只在手动保存时更新lastSaveDir，自动保存始终使用result文件夹
        if (!m_autoSaveMode) {
            m_lastSaveDir = QFileInfo(savePath).absolutePath();
        }

        QString message;
        if (m_autoSaveMode) {
            if (m_singleDocumentMode) {
                message = QString("文本已自动追加到单文档:\n%1").arg(savePath);
            } else {
                message = QString("文本已自动保存到新文档:\n%1").arg(savePath);
            }
            // 自动保存模式下只显示在调试信息中，不弹窗打扰用户
            qDebug() << message;
            // 可以在状态栏显示短暂提示
            statusBar()->showMessage(message, 3000);
        } else {
            if (mode & QIODevice::Append) {
                message = QString("文本已追加到单文档:\n%1").arg(savePath);
            } else {
                message = QString("文本已保存到:\n%1").arg(savePath);
            }
            QMessageBox::information(this, "保存成功", message);
        }
    } else {
        QMessageBox::warning(this, "保存失败", QString("无法保存文件到:\n%1\n请检查路径和权限！").arg(savePath));
    }
}

void MainWindow::on_pb_saveText_clicked()
{
    saveTextToFile();
}

void MainWindow::toggleSaveMode()
{
    m_autoSaveMode = !m_autoSaveMode;

    if (m_autoSaveMode) {
        ui->pb_toggleSaveMode->setText("自动保存模式");
        QString message;
        if (m_singleDocumentMode) {
            message = "已切换到自动保存模式（单文档）\n每次识别后文本将自动追加到同一文件";
        } else {
            message = "已切换到自动保存模式（多文档）\n每次识别后文本将保存到单独文件";
        }
        QMessageBox::information(this, "模式切换", message);
    } else {
        ui->pb_toggleSaveMode->setText("手动保存模式");
        QMessageBox::information(this, "模式切换", "已切换到手动保存模式\n点击保存按钮手动保存文本");
    }
}

void MainWindow::on_pb_toggleSaveMode_clicked()
{
    toggleSaveMode();
}

// 初始化result文件夹
// 初始化result文件夹
void MainWindow::initResultDirectory()
{
    // 获取应用程序所在目录
    QString appDir = QCoreApplication::applicationDirPath();
    m_resultDir = QDir(appDir).filePath("result");

    QDir resultDir(m_resultDir);
    if (!resultDir.exists()) {
        if (resultDir.mkpath(".")) {
            qDebug() << "创建result文件夹成功:" << m_resultDir;
        } else {
            qDebug() << "创建result文件夹失败，使用当前目录";
            m_resultDir = appDir;
        }
    }

    // 创建images子文件夹
    m_imageSaveDir = QDir(m_resultDir).filePath("images");
    QDir imageDir(m_imageSaveDir);
    if (!imageDir.exists()) {
        if (imageDir.mkpath(".")) {
            qDebug() << "创建images文件夹成功:" << m_imageSaveDir;
        } else {
            qDebug() << "创建images文件夹失败，使用result目录";
            m_imageSaveDir = m_resultDir;
        }
    }

    // 设置初始保存目录为result文件夹
    m_lastSaveDir = m_resultDir;

    // 更新单文档路径
    m_singleDocumentPath = QDir(m_resultDir).filePath("OCR_Results.txt");

    qDebug() << "Result文件夹路径:" << m_resultDir;
    qDebug() << "Images文件夹路径:" << m_imageSaveDir;
}

// 保存拍摄的图片
void MainWindow::saveCapturedImage(const cv::Mat& image, const QString& prefix)
{
    if (image.empty()) {
        qDebug() << "无法保存空图像";
        return;
    }

    if (!m_saveCapturedImages) {
        return; // 如果未启用保存图片，直接返回
    }

    // 生成文件名：前缀_时间戳.jpg
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    QString fileName;

    if (m_pptAutoCapture && prefix == "capture") {
        fileName = QString("PPT_Auto_%1.jpg").arg(timestamp);
    } else {
        fileName = QString("%1_%2.jpg").arg(prefix).arg(timestamp);
    }

    QString filePath = QDir(m_imageSaveDir).filePath(fileName);

    // 保存图片
    bool success = cv::imwrite(filePath.toLocal8Bit().toStdString(), image);

    if (success) {
        qDebug() << "图片保存成功:" << filePath;
        if (m_pptAutoCapture) {
            statusBar()->showMessage(QString("PPT自动捕获: 图片已保存 - %1").arg(fileName), 3000);
        }
    } else {
        qDebug() << "图片保存失败:" << filePath;
        statusBar()->showMessage("图片保存失败，请检查权限或磁盘空间", 3000);
    }
}
