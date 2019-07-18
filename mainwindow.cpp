#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "nmeagenerator.h"

static long trackCount = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#if defined(ANDROID) || defined(__ANDROID__)
    if (!requestFineLocationPermission())
        qApp->quit();
    setScreenOrientation(SCREEN_ORIENTATION_PORTRAIT);
#endif

    ui->pushButton_Exit->setStyleSheet("font-size: 15pt; color: white;background-color: rgba(0, 102, 153, 255);");
    ui->pushButton_Start->setStyleSheet("font-size: 15pt; color: white;background-color: rgba(0, 102, 153, 255);");
    ui->pushButton_SetIP->setStyleSheet("font-size: 15pt; color: white;background-color: rgba(0, 102, 153, 255);");

    ui->text_GpsInfo->setStyleSheet("font-size: 15pt; color: white;background-color: rgba(0, 13, 26, 255);");
    ui->label_HostIp->setStyleSheet("font-size: 15pt; color: white;background-color: rgba(204, 51, 0, 255);");
    ui->label_Port->setStyleSheet("font-size: 15pt; color: white;background-color: rgba(204, 51, 0, 255);");

    connect(&m_Socket, &QUdpSocket::readyRead,this,&MainWindow::readData , Qt::UniqueConnection);
    connect(&m_Socket, &QAbstractSocket::stateChanged, this, &MainWindow::onStateChanged);
    setIpAddress();
    startGpsSource();
}

MainWindow::~MainWindow()
{
    delete ui;
}

#if defined(ANDROID) || defined(__ANDROID__)

bool MainWindow::setScreenOrientation(int orientation)
{
    QAndroidJniObject activity = QtAndroid::androidActivity();

    if(activity.isValid())
    {
        activity.callMethod<void>("setRequestedOrientation", "(I)V", orientation);
        keep_screen_on(true);
        return true;
    }
    return false;
}

void MainWindow::keyPressEvent(QKeyEvent *k)
{
    if( k->key() == Qt::Key_MediaPrevious )
    {
        return;
    }
}
#endif

bool MainWindow::startGpsSource()
{
    m_positionSource = QGeoPositionInfoSource::createDefaultSource(this);

    if (m_positionSource == nullptr)
    {
        ui->text_GpsInfo->setText("No gps source found.!");
        return false;
    }

    if(!m_positionSource->supportedPositioningMethods())
    {
        m_positionSource = nullptr;
        ui->text_GpsInfo->setText("No gps source found.!");
        return false;
    }

    QString status;

    m_positionSource->setPreferredPositioningMethods(QGeoPositionInfoSource::AllPositioningMethods);
    m_positionSource->setUpdateInterval(0);
    connect(m_positionSource, &QGeoPositionInfoSource::positionUpdated, this, &MainWindow::positionUpdated);
    connect(m_positionSource, &QGeoPositionInfoSource::updateTimeout, this, &MainWindow::updateTimeout);

    m_satelliteSource = QGeoSatelliteInfoSource::createDefaultSource(this);
    if (m_satelliteSource == nullptr)
    {
        status.append("No satellite data source found.!\n");
    }
    else
    {
        auto ok = connect(m_satelliteSource, &QGeoSatelliteInfoSource::satellitesInUseUpdated, this, &MainWindow::satellitesInUseUpdated);
        Q_ASSERT(ok);
        ok = connect(m_satelliteSource, &QGeoSatelliteInfoSource::satellitesInViewUpdated, this, &MainWindow::satellitesInViewUpdated);
        Q_ASSERT(ok);
    }


    status.append("Active Gps Source is: ");
    status.append( m_positionSource->sourceName());
    ui->text_GpsInfo->setText(status);

    return true;
}

void MainWindow::positionUpdated(QGeoPositionInfo gpsPos)
{
    if (!gpsPos.isValid() || !gpsPos.coordinate().isValid())
        return;

    auto m_coord = gpsPos.coordinate();

    auto m_latitude = m_coord.latitude();
    auto m_longitude = m_coord.longitude();
    auto m_altitude = m_coord.altitude();

    auto m_direction = gpsPos.attribute(QGeoPositionInfo::Direction);
    if(IsNan(static_cast<float>(m_direction))) m_direction = 0;

    auto m_groundSpeed = gpsPos.attribute(QGeoPositionInfo::GroundSpeed);
    if(IsNan(static_cast<float>(m_groundSpeed))) m_groundSpeed = 0;

    auto m_verticalSpeed = gpsPos.attribute(QGeoPositionInfo::VerticalSpeed);
    if(IsNan(static_cast<float>(m_verticalSpeed))) m_verticalSpeed = 0;

    auto m_horizontalAccuracy = gpsPos.attribute(QGeoPositionInfo::HorizontalAccuracy);
    if(IsNan(static_cast<float>(m_horizontalAccuracy))) m_horizontalAccuracy = 0;

    auto m_verticalAccuracy = gpsPos.attribute(QGeoPositionInfo::VerticalAccuracy);
    if(IsNan(static_cast<float>(m_verticalAccuracy))) m_verticalAccuracy = 0;

    auto m_magneticVariation = gpsPos.attribute(QGeoPositionInfo::MagneticVariation);
    if(IsNan(static_cast<float>(m_magneticVariation))) m_magneticVariation = 0;

    auto timestamp = gpsPos.timestamp();
    auto local = timestamp.toLocalTime();
    auto dateString = timestamp.toString("ddMMyy");
    auto timeString = timestamp.toString("hhmmss");

    NmeaGenerator nmeaGenerator;
    NmeaGenerator::GpsData gpsData;
    gpsData.fixTime = timeString;
    gpsData.lat = m_latitude;
    gpsData.lng = m_longitude;
    gpsData.ground_speed = m_groundSpeed;
    gpsData.altitude = m_altitude;
    gpsData.track_angle = m_direction;
    gpsData.date = dateString;
    gpsData.magnetic_variation = m_magneticVariation;
    gpsData.number_of_satellites = 0;
    gpsData.horizontal_dilution = m_horizontalAccuracy;
    gpsData.height_of_geoid = 0;

    auto gprmc = nmeaGenerator.BuildGPRMC(gpsData);
    QByteArray data;
    data.append(gprmc);
    sendData(ui->lineEdit_Port->text().toInt(), data);

    auto gpgga = nmeaGenerator.BuildGPGGA(gpsData);
    data.clear();
    data.append(gpgga);
    sendData(ui->lineEdit_Port->text().toInt(), data);

    trackCount++;

    QString status;
    status.append(QString("Track Count: ") + QString::number(trackCount) + QString("\n\n"));
    status.append(gprmc);
    status.append("\n");
    status.append(gpgga);
    ui->text_GpsInfo->setText(status);
}

void MainWindow::satellitesInUseUpdated(const QList<QGeoSatelliteInfo> &satellites)
{
    qDebug() << tr("satellitesInUseUpdated received");
}

void MainWindow::satellitesInViewUpdated(const QList<QGeoSatelliteInfo> &satellites)
{
    qDebug() << tr("satellitesInViewUpdated received");
}

void MainWindow::updateTimeout(void)
{
    qDebug() << "updateTimeout";
}

void MainWindow::setIpAddress()
{
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) && !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            foreach (QNetworkAddressEntry entry, interface.addressEntries())
            {
                qDebug() << interface.name() + " "+ entry.ip().toString() +" " + interface.hardwareAddress();

                if ( interface.hardwareAddress() != "00:00:00:00:00:00" && entry.ip().toString().contains(".")
                     && (interface.name().contains("en0")
                         || interface.name().contains("eth0")
                         || interface.name().contains("wireless")
                         || interface.name().contains("bridge")))
                {
                    ui->lineEdit_IP->setText(entry.ip().toString());
                }
            }
        }
    }
}

void MainWindow::initUDP(int port)
{
    if(m_Socket.state() != QUdpSocket::BoundState)
    {
        m_Socket.bind(QHostAddress(ui->lineEdit_IP->text()),static_cast<quint16>(port));
    }
}

void MainWindow::deInitUDP()
{
    if(m_Socket.state() == QUdpSocket::BoundState)
    {
        m_Socket.close();
    }
}

void MainWindow::readData()
{
    QHostAddress sender;
    quint16 senderPort;

    // listener for datagrams heard...
    while (m_Socket.hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(static_cast<long>(m_Socket.pendingDatagramSize()));
        m_Socket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        if(datagram.data() != nullptr)
        {

        }
    }
}

void MainWindow::on_pushButton_Start_clicked()
{
    if(!m_positionSource)return;

    if(ui->pushButton_Start->text() == "Start")
    {
        trackCount = 0;
        ui->pushButton_Start->setText("Stop");
        m_positionSource->startUpdates();
    }
    else
    {
        ui->pushButton_Start->setText("Start");
        m_positionSource->stopUpdates();
    }
}

void MainWindow::sendData(int port, QByteArray &arrayData)
{
    QUdpSocket m_sSocket;
    m_sSocket.writeDatagram(arrayData, QHostAddress(ui->lineEdit_IP->text()), port);
    arrayData.clear();
}

void MainWindow::onStateChanged(QAbstractSocket::SocketState state)
{
    switch(state)
    {
    case QAbstractSocket::UnconnectedState:

        break;

    case QAbstractSocket::ConnectedState:

        break;

    case QAbstractSocket::BoundState:

        break;

    default:
        break;
    }
}


void MainWindow::on_pushButton_Exit_clicked()
{
    exit(0);
}


void MainWindow::on_pushButton_SetIP_clicked()
{
    setIpAddress();
}
