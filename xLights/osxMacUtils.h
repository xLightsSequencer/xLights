//
//  osMacUtils.h
//  xLights
//
//  Created by Daniel Kulp on 8/17/15.
//  Copyright (c) 2015 Daniel Kulp. All rights reserved.
//

#ifndef xLights_osxMacUtils_h
#define xLights_osxMacUtils_h

class AppNapSuspenderPrivate;
class AppNapSuspender {
public:
    AppNapSuspender();
    ~AppNapSuspender();
    
    void suspend();
    void resume();
private:
    AppNapSuspenderPrivate *p;
};

#endif
