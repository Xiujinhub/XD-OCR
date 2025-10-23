#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>

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

    // 释放摄像头资源
    if (m_camera && m_camera->isOpened()) {
        m_camera->release();
    }
    delete m_camera;
    delete m_cameraTimer;
}

//初始化界面
void MainWindow::initWidget()
{
    m_showMat = cv::Mat();
    ui->cb_showRect->setChecked(false);

    // 初始化摄像头相关变量
    m_camera = nullptr;
    m_cameraTimer = new QTimer(this);
    m_isCameraOpen = false;
    m_currentFrame = cv::Mat();
    m_capturedFrame = cv::Mat();

    // 初始化文本保存相关
    m_autoSaveMode = false; // 默认手动保存模式
    m_lastSaveDir = QDir::currentPath(); // 默认保存到程序当前目录

    // 初始化文本保存格式相关
    m_singleDocumentMode = true; // 默认单文档模式
    m_singleDocumentPath = QDir(m_lastSaveDir).filePath("OCR_Results.txt"); // 初始化单文档路径

    connect(m_cameraTimer, &QTimer::timeout, this, &MainWindow::updateCameraFrame);

    // 初始化拍摄照片显示区域
    ui->lb_captured->setText("拍摄的照片将显示在这里");
    ui->lb_captured->setAlignment(Qt::AlignCenter);
    ui->lb_captured->setStyleSheet("border: 1px solid gray;");

    // 设置QLabel的缩放策略
    ui->lb_show->setScaledContents(false);
    ui->lb_show->setAlignment(Qt::AlignCenter);
    ui->lb_captured->setScaledContents(false);
    ui->lb_captured->setAlignment(Qt::AlignCenter);

    // 初始化保存按钮文本
    ui->pb_saveText->setText("保存文本");
    ui->pb_toggleSaveMode->setText("手动保存模式");

    // 设置默认单选按钮状态
    ui->rb_singleDoc->setChecked(true); // 默认单文档模式
    ui->rb_multiDoc->setChecked(false);

    // 连接单选按钮信号
    connect(ui->rb_singleDoc, &QRadioButton::toggled, this, &MainWindow::onSaveFormatChanged);
    connect(ui->rb_multiDoc, &QRadioButton::toggled, this, &MainWindow::onSaveFormatChanged);

    //初始化PaddleOCR
    PaddleOCR::OCRConfig config("./config.txt");

    //文本检测对象
    m_det = new PaddleOCR::DBDetector(config.det_model_dir, config.use_gpu, config.gpu_id,
                                      config.gpu_mem, config.cpu_math_library_num_threads,
                                      config.use_mkldnn, config.max_side_len, config.det_db_thresh,
                                      config.det_db_box_thresh, config.det_db_unclip_ratio,
                                      config.visualize, config.use_tensorrt, config.use_fp16);

    //文本识别对象
    m_rec = new PaddleOCR::CRNNRecognizer(config.rec_model_dir, config.use_gpu, config.gpu_id,
                                          config.gpu_mem, config.cpu_math_library_num_threads,
                                          config.use_mkldnn, config.char_list_file,
                                          config.use_tensorrt, config.use_fp16);
}

// 保存格式改变槽函数
void MainWindow::onSaveFormatChanged()
{
    bool oldMode = m_singleDocumentMode;
    m_singleDocumentMode = ui->rb_singleDoc->isChecked();

    if (m_singleDocumentMode) {
        // 单文档模式
        m_singleDocumentPath = QDir(m_lastSaveDir).filePath("OCR_Results.txt");
        if (!oldMode) { // 只有当从多文档切换到单文档时才显示消息
            QMessageBox::information(this, "保存格式", "已切换到单文档模式\n所有识别结果将追加到同一个文件中");
        }
    } else {
        // 多文档模式
        if (oldMode) { // 只有当从单文档切换到多文档时才显示消息
            QMessageBox::information(this, "保存格式", "已切换到多文档模式\n每个识别结果将保存到单独的文件中");
        }
    }
}

//cv::Mat转QPixmap，支持等比缩放
QPixmap MainWindow::cvMatToPixmap(const cv::Mat imageMat)
{
    if (imageMat.empty()) {
        return QPixmap();
    }

    QImage showImage;
    if(imageMat.channels() > 1)
    {
        showImage = QImage((const uchar*)(imageMat.data), imageMat.cols, imageMat.rows,
                           imageMat.step, QImage::Format_RGB888).rgbSwapped();   //彩色图
    }
    else
    {
        cv::Mat rgbMat;
        cv::cvtColor(imageMat, rgbMat, cv::COLOR_GRAY2BGR);
        showImage = QImage((const uchar*)(rgbMat.data), rgbMat.cols, rgbMat.rows,
                           rgbMat.step, QImage::Format_RGB888).rgbSwapped();   //灰度图
    }

    return QPixmap::fromImage(showImage);
}

// 等比缩放图像以适应QLabel
QPixmap scalePixmapToLabel(const QPixmap& pixmap, QLabel* label)
{
    if (pixmap.isNull() || !label) {
        return pixmap;
    }

    return pixmap.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

//选择图像
void MainWindow::on_pb_selectImage_clicked()
{
    QString imageName = QFileDialog::getOpenFileName(this, "选择图像", "E:/photo/OCRTest/", "Image File(*.png *.jpg *.bmp)");
    if (!imageName.isEmpty())
    {
        // 加载图像并立即显示
        m_showMat = cv::imread(imageName.toLocal8Bit().toStdString(), 1);
        QPixmap showPixmap = cvMatToPixmap(m_showMat.clone());
        if (!showPixmap.isNull())
        {
            ui->tb_text->clear();
            // 等比缩放图像以适应标签
            showPixmap = scalePixmapToLabel(showPixmap, ui->lb_show);
            ui->lb_show->setPixmap(showPixmap); // 先显示原始图片

            // 使用单次定时器延迟执行OCR，确保UI先更新
            QTimer::singleShot(100, this, [this, imageName]() {
                performOCR(m_showMat);

                // 如果是自动保存模式，则自动保存文本
                if (m_autoSaveMode) {
                    saveTextToFile();
                }
            });
        }
    }
}

// 打开/关闭摄像头
void MainWindow::on_pb_openCamera_clicked()
{
    if (!m_isCameraOpen) {
        // 打开摄像头
        if (!m_camera) {
            m_camera = new cv::VideoCapture(0); // 0表示默认摄像头
        }

        if (m_camera->isOpened()) {
            m_isCameraOpen = true;
            m_cameraTimer->start(33); // 约30fps
            ui->pb_openCamera->setText("关闭摄像头");
            ui->pb_captureImage->setEnabled(true);

            // 清空拍摄的照片显示
            ui->lb_captured->clear();
            ui->lb_captured->setText("拍摄的照片将显示在这里");
        } else {
            QMessageBox::warning(this, "错误", "无法打开摄像头！");
            delete m_camera;
            m_camera = nullptr;
        }
    } else {
        // 关闭摄像头
        m_cameraTimer->stop();
        if (m_camera && m_camera->isOpened()) {
            m_camera->release();
        }
        m_isCameraOpen = false;
        ui->pb_openCamera->setText("打开摄像头");
        ui->pb_captureImage->setEnabled(false);

        // 清空显示
        ui->lb_show->clear();
    }
}

// 更新摄像头帧
void MainWindow::updateCameraFrame()
{
    if (m_camera && m_camera->isOpened()) {
        cv::Mat frame;
        *m_camera >> frame;

        if (!frame.empty()) {
            m_currentFrame = frame.clone();
            QPixmap showPixmap = cvMatToPixmap(frame);
            // 等比缩放图像以适应标签
            showPixmap = scalePixmapToLabel(showPixmap, ui->lb_show);
            ui->lb_show->setPixmap(showPixmap);
        }
    }
}

// 拍照识别
void MainWindow::on_pb_captureImage_clicked()
{
    if (!m_currentFrame.empty()) {
        // 保存拍摄的帧
        m_capturedFrame = m_currentFrame.clone();

        // 显示拍摄的照片在lb_captured中，进行等比缩放
        QPixmap capturedPixmap = cvMatToPixmap(m_capturedFrame);
        capturedPixmap = scalePixmapToLabel(capturedPixmap, ui->lb_captured);
        ui->lb_captured->setPixmap(capturedPixmap);

        // 使用单次定时器延迟执行OCR，确保UI先更新
        QTimer::singleShot(100, this, [this]() {
            performOCR(m_capturedFrame);

            // 如果是自动保存模式，则自动保存文本
            if (m_autoSaveMode) {
                saveTextToFile();
            }
        });
    }
}

// OCR文字识别函数，直接处理cv::Mat
void MainWindow::performOCR(const cv::Mat& image)
{
    cv::Mat srcMat = image.clone();
    std::vector<std::vector<std::vector<int>>> boxes;

    // 执行文本检测
    cv::Mat dstMat = m_det->Run(srcMat, boxes);

    // 执行文本识别
    std::vector<std::string> res = m_rec->Run(boxes, srcMat, nullptr);

    // 在主显示区域显示识别结果图像，进行等比缩放
    QPixmap resultPixmap;
    if (ui->cb_showRect->isChecked()) {
        // 显示带有文本框的图像
        resultPixmap = cvMatToPixmap(dstMat);
    } else {
        // 显示原始图像
        resultPixmap = cvMatToPixmap(srcMat);
    }

    resultPixmap = scalePixmapToLabel(resultPixmap, ui->lb_show);
    ui->lb_show->setPixmap(resultPixmap);

    // 显示识别结果
    ui->tb_text->clear();
    for (auto s : res) {
        ui->tb_text->append(s.data());
    }

    // 增加OCR计数
    m_ocrCount++;
}

// 保存文本到文件
void MainWindow::saveTextToFile(const QString& filePath)
{
    QString text = ui->tb_text->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::information(this, "提示", "没有文本可保存！");
        return;
    }

    QString savePath = filePath;
    QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text;

    // 如果没有指定文件路径，则根据模式决定
    if (savePath.isEmpty()) {
        if (m_autoSaveMode) {
            // 自动保存模式
            if (m_singleDocumentMode) {
                // 单文档模式：使用固定文件名
                if (m_singleDocumentPath.isEmpty()) {
                    m_singleDocumentPath = QDir(m_lastSaveDir).filePath("OCR_Results.txt");
                }
                savePath = m_singleDocumentPath;

                // 单文档模式使用追加模式，而不是覆盖
                mode = QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text;
            } else {
                // 多文档模式：使用时间戳生成文件名
                QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
                QString fileName = QString("OCR_Result_%1.txt").arg(timestamp);
                savePath = QDir(m_lastSaveDir).filePath(fileName);
                mode = QIODevice::WriteOnly | QIODevice::Text; // 多文档用覆盖模式
            }
        } else {
            // 手动保存模式：弹出文件对话框
            QString defaultFileName;
            if (m_singleDocumentMode) {
                defaultFileName = "OCR_Results.txt";
                // 手动保存时也要检查是否需要追加
                QString proposedPath = QDir(m_lastSaveDir).filePath(defaultFileName);
                if (QFile::exists(proposedPath)) {
                    mode = QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text;
                }
            } else {
                QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
                defaultFileName = QString("OCR_Result_%1.txt").arg(timestamp);
            }
            savePath = QFileDialog::getSaveFileName(this, "保存文本",
                                                    QDir(m_lastSaveDir).filePath(defaultFileName),
                                                    "文本文件 (*.txt)");
            if (savePath.isEmpty()) {
                return; // 用户取消保存
            }

            // 手动保存时，如果是单文档模式且文件已存在，则追加
            if (m_singleDocumentMode && QFile::exists(savePath)) {
                mode = QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text;
            }
        }
    }

    // 保存文件
    QFile file(savePath);
    if (file.open(mode)) {
        QTextStream out(&file);

        // 添加分隔符和时间戳（只在追加模式下添加）
        if (mode & QIODevice::Append) {
            // 如果是追加模式，先添加分隔符和时间戳
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            out << "\n\n"; // 先添加空行分隔
            out << "=== OCR识别结果 - " << timestamp << " ===\n";
        } else {
            // 如果是覆盖模式（新文件），也添加时间戳但不需要分隔符
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            out << "=== OCR识别结果 - " << timestamp << " ===\n";
        }

        out << text;
        file.close();

        // 更新最后保存目录
        m_lastSaveDir = QFileInfo(savePath).absolutePath();

        if (m_autoSaveMode) {
            QString message;
            if (m_singleDocumentMode) {
                message = QString("文本已自动追加到单文档:\n%1").arg(savePath);
            } else {
                message = QString("文本已自动保存到新文档:\n%1").arg(savePath);
            }
            // 自动保存时可以不显示消息框，避免频繁打扰
            // QMessageBox::information(this, "自动保存成功", message);
            qDebug() << message; // 使用调试输出代替消息框
        } else {
            QString message;
            if (mode & QIODevice::Append) {
                message = QString("文本已追加到单文档:\n%1").arg(savePath);
            } else {
                message = QString("文本已保存到:\n%1").arg(savePath);
            }
            QMessageBox::information(this, "保存成功", message);
        }
    } else {
        QMessageBox::warning(this, "保存失败", "无法保存文件，请检查路径和权限！");
    }
}

// 保存文本按钮点击事件
void MainWindow::on_pb_saveText_clicked()
{
    // 执行保存操作
    saveTextToFile();
}

// 切换保存模式
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
        QMessageBox::information(this, "模式切换", "已切换到手动保存模式\n"
                                                   "点击保存按钮手动保存文本");
    }
}

// 切换保存模式按钮点击事件
void MainWindow::on_pb_toggleSaveMode_clicked()
{
    toggleSaveMode();
}
