#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <list>
#include <cmath>
#include <wx/datetime.h>
#include <log4cpp/Category.hh>

class City
{
    static std::list<City> _cities;

public:
    float _latitude;
    float _longitude;
    std::string _city;
    std::string _country;
    std::string _state;
    float _timeZone;

    City(std::string country, std::string city, std::string state, float latitude, float longitude, float timezone)
    {
        _latitude = latitude;
        _longitude = longitude;
        _city = city;
        _country = country;
        _state = state;
        _timeZone = timezone;
    }

    static void Test()
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        City* city = GetCity("Sydney");
        wxDateTime date = wxDateTime::Now();
        date.SetDay(1);
        date.SetMonth(wxDateTime::Jan);
        date.SetYear(2018);
        for (int m = 0; m < 12; m++)
        {
            date.SetMonth((wxDateTime::Month)m);
            wxDateTime sunrise = city->GetSunrise(date);
            wxDateTime sunset = city->GetSunset(date);
            logger_base.debug("Date %s Sunrise %02d:%02d Sunset %02d:%02d", (const char*)date.FormatISODate().c_str(), sunrise.GetHour(), sunrise.GetMinute(), sunset.GetHour(), sunset.GetMinute());
        }
    }

    static City* GetCity(std::string city)
    {
        for (auto it = City::_cities.begin(); it != City::_cities.end(); ++it)
        {
            if (it->_city == city)
            {
                return &(*it);
            }
        }

        return nullptr;
    }

    static std::list<std::string> GetCities()
    {
        std::list<std::string> res;

        for (auto it = _cities.begin(); it != _cities.end(); ++it)
        {
            res.push_back(it->_city);
        }
        return res;
    }

    static bool GetCityLocation(std::string city, float& latitude, float& longitude)
    {
        for (auto it = City::_cities.begin(); it != City::_cities.end(); ++it)
        {
            if (it->_city == city)
            {
                latitude = it->_latitude;
                longitude = it->_longitude;
                return true;
            }
        }

        latitude = 0;
        longitude = 0;
        return false;
    }

    #define PI 3.141592653589793
    #define RADIANS(A) ((A) * PI / 180.0)
    #define DEGREES(A) ((A) * 180.0 / PI)

    wxDateTime GetSunRiseSet(wxDateTime date, bool sunset)
    {
        // static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        const float zenith = 90.83333333333333f;

        wxDateTime res = wxDateTime::Now();
        res.SetHour(0);
        res.SetMinute(0);
        res.SetSecond(0);

        // Algorithm from http://edwilliams.org/sunrise_sunset_algorithm.htm

        // int month = date.GetMonth() + 1;

        // first calculate the day of the year
        int N = date.GetDayOfYear();

        // convert the longitude to hour value and calculate an approximate time
        float lngHour = _longitude / 15.0;
        float t;
        if (sunset)
        {
            t = N + ((18 - lngHour) / 24);;
        }
        else
        {
            // THIS IS FOR SUNRISE
            t = N + ((6.0 - lngHour) / 24.0);
        }

        // calculate the Sun's mean anomaly
        float M = (0.9856 * t) - 3.289;

        // calculate the Sun's true longitude
        // NOTE: L potentially needs to be adjusted into the range[0, 360) by adding / subtracting 360
        float L = M + (1.916 * std::sin(RADIANS(M))) + (0.020 * std::sin(RADIANS(2 * M))) + 282.634;
        if (L > 360)
        {
            L -= 360.0;
        }
        else if (L < 0)
        {
            L += 360.0;
        }

        //calculate the Sun's right ascension
        // NOTE: RA potentially needs to be adjusted into the range[0, 360) by adding / subtracting 360
        float RA = DEGREES(std::atan(0.91764 * std::tan(RADIANS(L))));
        if (RA > 360)
        {
            RA -= 360.0;
        }
        else if (RA < 0)
        {
            RA += 360.0;
        }

        // right ascension value needs to be in the same quadrant as L
        int Lquadrant = std::floor(L / 90) * 90;
        int RAquadrant = std::floor(RA / 90) * 90;
        RA += (Lquadrant - RAquadrant);

        // right ascension value needs to be converted into hours
        RA /= 15.0;

        // calculate the Sun's declination
        float sinDec = 0.39782 * sin(RADIANS(L));
        float cosDec = cos(asin(sinDec));

        // calculate the Sun's local hour angle
        float cosH = (cos(RADIANS(zenith)) - (sinDec * std::sin(RADIANS(_latitude)))) / (cosDec * std::cos(RADIANS(_latitude)));
        if (cosH > 1)
        {
            res.SetHour(24);
            return res; // the sun never actually rises
        }

        // finish calculating H and convert into hours
        float H;
        if (sunset)
        {
            H = DEGREES(std::acos(cosH));
        }
        else
        {
            H = 360.0 - DEGREES(std::acos(cosH));
        }
        H /= 15.0;

        // calculate local mean time of rising/setting
        float T = H + RA - (0.06571 * t) - 6.622;

        // adjust back to UTC
        // NOTE: UT potentially needs to be adjusted into the range[0, 24) by adding / subtracting 24
        float UT = T - lngHour;
        if (UT > 24)
        {
            UT -= 24.0;
        }
        else if (UT < 0)
        {
            UT += 24.0;
        }

        wxDateTime dt = date;
        dt.SetHour(12);
        dt.SetMinute(0);
        dt.SetSecond(0);

        wxTimeSpan timezone = (dt.FromUTC() - dt);
        //logger_base.debug("Timezone %02d:%02d", timezone.GetHours(), timezone.GetMinutes() % 60);

        res.SetHour((int)UT);
        res.SetMinute((int)((UT - (int)UT) * 60));
        res.SetSecond((int)((UT * 60) - (int)(UT * 60)) * 60);

        //logger_base.debug("Time before timezone %02d:%02d", res.GetHour(), res.GetMinute());

        res += timezone;

        //logger_base.debug("Time after timezone %02d:%02d", res.GetHour(), res.GetMinute());

        return res;
    }

    wxDateTime GetSunrise(wxDateTime date)
    {
        return GetSunRiseSet(date, false);
    }

    wxDateTime GetSunset(wxDateTime date)
    {
        return GetSunRiseSet(date, true);
    }

    static bool GetDefaultCityLocation(float timezone, float& latitude, float& longitude)
    {
        for (auto it = City::_cities.begin(); it != City::_cities.end(); ++it)
        {
            if (it->_timeZone == timezone)
            {
                latitude = it->_latitude;
                longitude = it->_longitude;
                return true;
            }
        }

        latitude = 0;
        longitude = 0;
        return false;
    }
};
