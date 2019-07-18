#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QGeoCoordinate>
#include <QGeoPositionInfoSource>
#include <QGeoSatelliteInfoSource>
#include <QGeoSatelliteInfo>

#if defined(ANDROID) || defined(__ANDROID__)
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QKeyEvent>
#endif


#define SCREEN_ORIENTATION_LANDSCAPE 0
#define SCREEN_ORIENTATION_PORTRAIT 1

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

#if defined(ANDROID) || defined(__ANDROID__)
    void keep_screen_on(bool on) {
        QtAndroid::runOnAndroidThread([on]{
            QAndroidJniObject activity = QtAndroid::androidActivity();
            if (activity.isValid()) {
                QAndroidJniObject window =
                        activity.callObjectMethod("getWindow", "()Landroid/view/Window;");

                if (window.isValid()) {
                    const int FLAG_KEEP_SCREEN_ON = 128;
                    if (on) {
                        window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                    } else {
                        window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                    }
                }
            }
            QAndroidJniEnvironment env;
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
            }
        });
    }

    bool requestFineLocationPermission()
    {
        QtAndroid::PermissionResult request = QtAndroid::checkPermission("android.permission.ACCESS_FINE_LOCATION");
        if (request == QtAndroid::PermissionResult::Denied){
            QtAndroid::requestPermissionsSync(QStringList() <<  "android.permission.ACCESS_FINE_LOCATION");
            request = QtAndroid::checkPermission("android.permission.ACCESS_FINE_LOCATION");
            if (request == QtAndroid::PermissionResult::Denied)
            {
                qDebug() << "FineLocation Permission denied";
                return false;
            }
        }
        qDebug() << "FineLocation Permissions granted!";
        return true;
    }

    bool requestStorageWritePermission() {
        QtAndroid::PermissionResult request = QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE");
        if(request == QtAndroid::PermissionResult::Denied) {
            QtAndroid::requestPermissionsSync( QStringList() << "android.permission.WRITE_EXTERNAL_STORAGE" );
            request = QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE");
            if(request == QtAndroid::PermissionResult::Denied)
            {
                qDebug() << "StorageWrite Permission denied";
                return false;
            }
        }
        qDebug() << "StorageWrite Permissions granted!";
        return true;
    }
#endif

private:
    QUdpSocket m_Socket;
    QGeoPositionInfoSource *m_positionSource{};
    QGeoSatelliteInfoSource *m_satelliteSource{};

    void initUDP(int port);
    void deInitUDP();
    void setIpAddress();
    bool startGpsSource();

#if defined(ANDROID) || defined(__ANDROID__)
    bool setScreenOrientation(int orientation);
    void keyPressEvent(QKeyEvent *k);
#endif

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
