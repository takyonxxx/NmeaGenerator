#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QGeoCoordinate>
#include <QGeoPositionInfoSource>
#include <QGeoSatelliteInfoSource>
#include <QGeoSatelliteInfo>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool IsNan( float value )
    {
        return ((*(uint*)&value) & 0x7fffffff) > 0x7f800000;
    }

    /*QString getMacForIP(QString ipAddress)
    {
        QString MAC;
        QProcess process;
        //
        process.start(QString("arp -a %1").arg(ipAddress));
        if(process.waitForFinished())
        {
            QString result = process.readAll();
            QStringList list = result.split(QRegularExpression("\\s+"));
            if(list.contains(ipAddress))
                MAC = list.at(list.indexOf(ipAddress) + 1);
        }
        //
        return MAC;
    }*/

private:
    QUdpSocket m_Socket;
    QGeoPositionInfoSource *m_positionSource{};
    QGeoSatelliteInfoSource *m_satelliteSource{};

    void initUDP(int port);
    void deInitUDP();
    void setIpAddress();
    bool startGpsSource();

private slots:
    void readData();
    void sendData(int port, QByteArray &);
    void onStateChanged(QAbstractSocket::SocketState);
    void positionUpdated(QGeoPositionInfo);

    // This slot is invoked whenever new information about the used satellites are retrieved
    void satellitesInUseUpdated(const QList<QGeoSatelliteInfo> & satellites);

    // This slot is invoked whenever new information about the in-view satellites are retrieved
    void satellitesInViewUpdated(const QList<QGeoSatelliteInfo> & satellites);

    void updateTimeout(void);

    void on_pushButton_Start_clicked();
    void on_pushButton_Exit_clicked();
    void on_pushButton_SetIP_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
