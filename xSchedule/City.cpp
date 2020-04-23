/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "City.h"

std::list<City> City::_cities = {
    City("Canada","Vancouver","",49.25f, -123.133333f,-8),
    City("United States","Portland","OR", 45.5236111f, -122.675f, -8),
    City("United States","San Francisco","CA", 37.775f, -122.4183333f, -8),
    City("United States","Seattle","WA", 47.6063889f, -122.3308333f, -8),
    City("United States","San Jose","CA", 37.3394444f, -121.8938889f, -8),
    City("United States","Los Angeles","CA", 34.0522222f, -118.2427778f, -8),
    City("United States","Las Vegas","NV", 36.175f, -115.1363889f, -8),
    City("Canada","Calgary","",51.083333f, -114.083333f ,-7),
    City("Canada","Edmonton","", 53.55f, -113.5f,-7),
    City("United States","Phoenix","AZ",33.4483333f, -112.0733333f,-7),
    City("United States","Tucson","AZ",32.2216667f, -110.9258333f,-7),
    City("United States","Denver","CO",39.7391667f, -104.9841667f,-7),
    City("United States","Oklahoma City","OK",35.4675f,-97.5161111f,-6),
    City("Canada","Winnipeg","",49.883333f,-97.166667f,-6),
    City("United States","Dallas","TX",32.7833333f,-96.8f,-6),
    City("United States","Houston","TX",29.7630556f,-95.3630556f,-6),
    City("United States","Memphis","TN",35.1494444f,-90.0488889f,-6),
    City("United States","Milwaukee","WI",43.0388889f,-87.9063889f,-6),
    City("United States","Chicago","IL",41.85f,-87.65f,-6),
    City("United States","Nashville","TN",36.1658333f, -86.7844444f,-5),
    City("United States","Indianapolis","IN",39.7683333f, -86.1580556f,-5),
    City("United States","Detroit","MI", 42.3313889f, -83.0458333f,-5),
    City("United States","Columbus","OH", 39.9611111f, -82.9988889f,-5),
    City("United States", "Jacksonville", "FL", 30.3319444f,-81.6558333f,-5),
    City("United States", "Charlotte", "NC", 35.2269444f,-80.8433333f,-5),
    City("Canada","Hamilton", "", 43.25f, -79.833333f,-5),
    City("Canada","Toronto", "", 43.666667f, -79.416667f,-5),
    City("United States", "Washington","DC", 38.895f, -77.0366667f,-5),
    City("United States", "Baltimore","MD", 39.2902778f, -76.6125f,-5),
    City("Canada","Ottawa", "", 45.416667f, -75.7f,-5),
    City("United States","Philadelphia","PA",39.9522222f,-75.1641667f,-5),
    City("United States","New York","NY",40.7141667f,-74.0063889f,-5),
    City("Canada","Montreal","",45.5f,-73.583333f,-5),
    City("Canada","Quebec","",46.8f,-71.25f,-5),
    City("United States","Boston","MA",42.3583333f,-71.0602778f,-5),
    City("United Kingdom","Belfast","",54.583333f,-5.933333f,0),
    City("United Kingdom","Glasgow","",55.833333f,-4.25f,0),
    City("United Kingdom","Birmingham","",52.466667f,-1.916667f,0),
    City("United Kingdom","London","",51.514125f,-0.093689f,0),
    City("France","Paris","",48.866667f,2.333333f,1),
    City("Belgium","Brussels","",50.833333f,4.333333f,1),
    City("France","Marseille","",43.285413f,5.37606f,1),
    City("Germany","Duisburg","",51.433333f,6.75f,1),
    City("Germany","Dusseldorf","",51.216667f,6.766667f,1),
    City("Germany","Cologne","",50.933333f,6.95f,1),
    City("Germany","Essen","",51.45f,7.016667f,1),
    City("Germany","Dortmund","",51.516667f,7.45f,1),
    City("Germany","Frankfurt","",50.116667f,8.683333f,1),
    City("Germany","Bremen","",53.083333f,8.8f,1),
    City("Germany","Stuttgart","",48.766667f,9.183333f,1),
    City("Germany","Hanover","",52.366667f,9.716667f,1),
    City("Germany","Hamburg","",53.55f,10.0f,1),
    City("Germany","Munich","",48.15f,11.5833f,1),
    City("Germany","Berlin","",52.516667f,13.4f,1),
    City("South Africa","Johannesburg","",-26.205171f,28.049815f,2),
    City("Australia","Perth","WA",-31.95224f,115.861397f,8),
    City("Australia","Adelaide","SA",-34.928661f,138.598633f,9.5),
    City("Australia","Melbourne","VIC",-37.813938f,144.963425f,10),
    City("Australia","Sydney","NSW",-33.861481f,151.205475f,10),
    City("Australia","Brisbane","QLD",-27.47101f,153.024292f,10),
    City("New Zealand","Auckland","",-36.866667f,174.766667f,12)
};
