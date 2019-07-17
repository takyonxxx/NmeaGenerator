#include "nmeagenerator.h"
#define NMEA_END_CHAR_1 '\n'
#define NMEA_MAX_LENGTH 70

NmeaGenerator::NmeaGenerator()
{

}

QString NmeaGenerator::DD2NMEA(double lat, double lng)
{    
    QString nmea{};
    QString str{};
    double lata = abs(lat);
    double latd = trunc(lata);
    double latm = (lata - latd) * 60;
    QString lath = lat > 0 ? "N" : "S";
    double lnga = abs(lng);
    double lngd = trunc(lnga);
    double lngm = (lnga - lngd) * 60;
    QString lngh = lng > 0 ? "E" : "W";

    nmea += QString("%1").arg(static_cast<int>(latd), 2, 10, QChar('0'));
    str = str.sprintf("%.5f",latm)  + "," + lath + ",";
    nmea += str;

    nmea += QString("%1").arg(static_cast<int>(lngd), 3, 10, QChar('0'));
    str = str.sprintf("%.5f",lngm)  + "," + lngh;
    nmea += str;

    return nmea;
}

QString NmeaGenerator::CalculateChecksum(const QString &Sentence)
{
    QString checksum{};

    QByteArray inBytes;
    const char *cStrData;
    inBytes = Sentence.toUtf8();
    cStrData = inBytes.constData();

    int sum = 0, inx;
    char tmp;
    // All character xor:ed results in the trailing hex checksum
    // The checksum calc starts after '$' and ends before '*'
    for (inx = 1; ; inx++)
    {
        //tmp = sentence_chars[inx];
        tmp = cStrData[inx];
        // Indicates end of data and start of checksum
        if (tmp == '*')
            break;
        sum ^= tmp;    // Build checksum
    }

    // Calculated checksum converted to a 2 digit hex string
    checksum = checksum.sprintf("%02X", sum);
    return checksum;
}

QString NmeaGenerator::BuildGPRMC(const GpsData & gpsData)
{
    /* RMC - NMEA has its own version of essential gps pvt (position, velocity, time) data. It is called RMC, The Recommended Minimum, which will look similar to:
    $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
    Where:
         RMC          Recommended Minimum sentence C
         123519       Fix taken at 12:35:19 UTC
         A            Status A=active or V=Void.
         4807.038,N   Latitude 48 deg 07.038' N
         01131.000,E  Longitude 11 deg 31.000' E
         022.4        Speed over the ground in knots
         084.4        Track angle in degrees True
         230394       Date - 23rd of March 1994
         003.1,W      Magnetic Variation
         *6A          The checksum data, always begins with *
    */

    QString gprmc{};
    QString str{};

    auto coords = DD2NMEA(gpsData.lat, gpsData.lng);
    gprmc += QString("$GPRMC,");
    gprmc += gpsData.fixTime + QString(",");
    gprmc += QString("A,");
    gprmc += coords + QString(",");
    str = str.sprintf("%05.1f", gpsData.ground_speed);
    gprmc += str + QString(",");
    str = str.sprintf("%05.1f", gpsData.track_angle);
    gprmc += str + QString(",");
    gprmc += gpsData.date + QString(",");
    str = str.sprintf("%05.1f", gpsData.magnetic_variation);
    gprmc += str + QString(",W*");

    auto checksum = CalculateChecksum(gprmc);
    gprmc +=  checksum + QString("\n");

    return gprmc;
}

QString NmeaGenerator::BuildGPGGA(const GpsData & gpsData)
{
    /*GGA - essential fix data which provide 3D location and accuracy data.
    $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
    Where:
         GGA          Global Positioning System Fix Data
         123519       Fix taken at 12:35:19 UTC
         4807.038,N   Latitude 48 deg 07.038' N
         01131.000,E  Longitude 11 deg 31.000' E
         1            Fix quality: 0 = invalid
                                   1 = GPS fix (SPS)
                                   2 = DGPS fix
                                   3 = PPS fix
                       4 = Real Time Kinematic
                       5 = Float RTK
                                   6 = estimated (dead reckoning) (2.3 feature)
                       7 = Manual input mode
                       8 = Simulation mode
         08           Number of satellites being tracked
         0.9          Horizontal dilution of position
         545.4,M      Altitude, Meters, above mean sea level
         46.9,M       Height of geoid (mean sea level) above WGS84
                          ellipsoid
         (empty field) time in seconds since last DGPS update
         (empty field) DGPS station ID number
         *47          the checksum data, always begins with *
    */

    QString gpgga{};
    QString str{};

    auto coords = DD2NMEA(gpsData.lat, gpsData.lng);
    gpgga += QString("$GPGGA,");
    gpgga += gpsData.fixTime + QString(",");
    gpgga += coords + QString(",");
    gpgga += QString("1,");
    str = str.sprintf("%02d", gpsData.number_of_satellites);
    gpgga += str + QString(",");
    str = str.sprintf("%02.1f", gpsData.horizontal_dilution);
    gpgga += str + QString(",");
    str = str.sprintf("%04.1f", gpsData.altitude);
    gpgga += str + QString(",M,");
    str = str.sprintf("%03.1f", gpsData.height_of_geoid);
    gpgga += str + QString(",M,");
    gpgga += QString(",*");

    auto checksum = CalculateChecksum(gpgga);
    gpgga +=  checksum + QString("\n");

    return gpgga;
}
