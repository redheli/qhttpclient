#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QtNetwork/QNetworkRequest"
#include "QtNetwork/QNetworkReply"
#include <QHttpMultiPart>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->download, SIGNAL(clicked(bool)), this, SLOT(test_get()));
    connect(ui->upload, SIGNAL(clicked(bool)), this, SLOT(test_upload_file()));
    manager = new QNetworkAccessManager(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QHttpPart part_parameter(QString key, QString value) {
    QHttpPart part;
    part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"" + key + "\""));
    part.setBody(value.toLatin1());
    return part;
}
void MainWindow::test_upload_file()
{
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // upload parameters data
    multiPart->append(part_parameter("drive_path", "121"));
    multiPart->append(part_parameter("drive_pose", "123"));

    // upload file data
    QHttpPart modelPart;
    modelPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("file/pcd"));
    modelPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"pcdFile\"; filename=\"/home/max/test_ground/dummy_upload.pcd\""));
    QFile *file = new QFile("/home/max/test_ground/dummy_upload.pcd");
    file->open(QIODevice::ReadOnly);
    modelPart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(modelPart);

    QUrl url("http://localhost:9899/test_upload");
    QNetworkRequest request(url);

//    QNetworkAccessManager manager;
    QNetworkReply *reply = manager->post(request, multiPart);

    connect(reply, &QNetworkReply::finished, [=](){
        qDebug("upload finished");
        QByteArray raw_data = reply->readAll();
        qDebug()<<raw_data;
    });

    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            [=](QNetworkReply::NetworkError err){
        qDebug()<<reply->errorString();
    });

    connect(reply, &QNetworkReply::sslErrors,
            [=](QList<QSslError> err){
        qDebug()<<err;
    });
}

void MainWindow::test_get()
{
    QNetworkRequest request;
    request.setUrl(QUrl("http://localhost:9899/fetch_pcd"));
    request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");

    QNetworkReply *reply = manager->get(request);
    QObject::connect(reply, &QNetworkReply::readyRead, [=](){
        qDebug("readyRead");
        this->ba.append(reply->readAll());
        qDebug()<<ba;
    });

    connect(reply, &QNetworkReply::finished, [=](){
        qDebug("finished");
        QFile outFile("/home/max/test_ground/dummy.pcd");
        if (!outFile.open(QIODevice::WriteOnly))
        {
          qDebug() << "Error opening a file\n";
        }
        else
        {
          outFile.write(ba);
          outFile.close();
        }
    });
//    static_cast<void (QNetworkReply::*)(QNetworkReply::SocketError)>
//    (&QAbstractSocket::error)

    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            [=](QNetworkReply::NetworkError err){
        qDebug()<<reply->errorString();
    });

    connect(reply, &QNetworkReply::sslErrors,
            [=](QList<QSslError> err){
        qDebug()<<err;
    });

}
