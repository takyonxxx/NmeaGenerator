#ifndef NMEAGENERATOR_H
#define NMEAGENERATOR_H

#include <QtCore>

class NmeaGenerator
{
public:
    NmeaGenerator();

    double convertKnotsToKilometersPerHour(double knots){
        return knots * 1.852;
    }

    struct GpsData
    {
        QString fixTime;            //123519       Fix taken at 12:35:19 UTC
        qreal lat;                  //deg
        qreal lng;                  //deg
        qreal ground_speed;         //knots
        qreal altitude;             //meters
        qreal track_angle;          //deg
        QString date;               //ddmmyy
        int number_of_satellites;   // Number of satellites being tracked
        qreal magnetic_variation;
        qreal height_of_geoid;      // (mean sea level) above WGS84 ellipsoid
        qreal horizontal_dilution;  // of position
    };

public:
    QString DD2NMEA(double lat, double lng);
    QString CalculateChecksum(const QString &);
    QString BuildGPRMC(const GpsData &);
    QString BuildGPGGA(const GpsData &);
};

#endif // NMEAGENERATOR_H
