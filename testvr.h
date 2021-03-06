#ifndef TESTVR_H

#include <XPLMDisplay.h>   // for window creation and manipulation
#include "XPLMGraphics.h"  // for window drawing
#include "XPLMDataAccess.h" // for the VR dataref
#include "XPLMPlugin.h"     // for XPLM_MSG_SCENERY_LOADED message
#include "XPLMUtilities.h" // to be
#include <XPLMProcessing.h>
#include <string>
#include <glew.h>
#include <vector>
#define GLEW_BUILD

#ifndef XPLM301
    #error This is made to be compiled against the XPLM301 SDK
#endif

using std::string;

struct vertex{
    int vx,vy;
    float red,green,blue;
};

struct rectangleData{
    unsigned long long number;
    int left,bottom,width,height;
    float red, green, blue;
};

#endif
