/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:   Loads and stores the bitmap animator settings
*
*/



#ifndef C_CBMPANIMSETTINGS_H
#define C_CBMPANIMSETTINGS_H

#include <e32std.h>
#include "CBmpAnimIniData.h"

// Default plugin name
_LIT(KPluginName, "BitmapAnimator");
const TInt KMaxPluginNameLength = 30;

// BmpAnim settings
class CBmpAnimSettings : public CBase
    {
public:
    // User constructor
    static CBmpAnimSettings* NewL();

    // Construction & destruction    
private:
    CBmpAnimSettings();
    void ConstructL();
public:
    virtual ~CBmpAnimSettings();

public:  // Access functions
    // Load the current settings from file (may be used from outside,
    // hence public). Usually just called from ConstructL(), tho
    TBool LoadSettingsL();

    // Save settings
    TBool SaveSettingsL();
    
    // Get running time (-1 = not defined)
    inline TInt RunningTime();
    
    // Get loop count (-1 = not defined)
    inline TInt LoopCount();
    
    // Get suspend time (-1 = indefinitely)
    inline TInt SuspendTime();
    
    // Get / set default timing
    inline TInt Timing();
    inline void SetTiming(TInt aTiming);

    // Get plugin name
    inline TDesC& PluginName();
    
    // Get bitmap file name
    inline TDesC& BitmapFilename();

    // Get landscape bitmap file name
    inline TDesC& BitmapFilenameLandscape();

    // Get/set lights on time (secs)
    inline TInt Lights();
    inline void SetLights(TInt aSecs);

    // Landscape used?
    inline TInt UseLandscape();

    // Should bitmaps be scaled to screen
    inline TInt ScaleBmps();
    
private:  // Utility functions
    // Opens the settings file (initialises the settings reader)
    TInt OpenSettings();
    
    // Figures out the bitmap file name
    void LoadBitmapFileName(TBool aLandscape = EFalse);
    
private:
    // Settings file handler
    CBmpAnimIniData* iIniData;
    
    // Time to run the animation. Overrides loop count
    TInt iRunningTime;

    // Number of loops to be run. Used if run time is undefined
    TInt iLoopCount;
        
    // Time to suspend the animation. -1 = indefinitely
    TInt iSuspendTime;
    
    // Time to display each image. Individual image timing overrides this.
    TInt iTiming;

    // Plugin name
    TBuf<KMaxPluginNameLength> iPluginName;
            
    // Bitmap file name
    TFileName iMbmFileName;

    // Landscale bitmap file name
    TFileName iMbmFileNameL;

    // Lights on time (secs)
    TInt iLights;

    // Landscape images used (0 = no)
    TInt iUseLandscape;
    
    // Bitmaps scaled to screen size (0 = no)
    TInt iScaleBmps;
    };


// --- inlines ---

// Get run time (-1 = not defined)
inline TInt CBmpAnimSettings::RunningTime()
    { return iRunningTime; }


// Get loop count (-1 = not defined)
inline TInt CBmpAnimSettings::LoopCount()
    { return iLoopCount; }


// Get suspend time (-1 = indefinitely)
inline TInt CBmpAnimSettings::SuspendTime()
    { return iSuspendTime; }


// Get default timing
inline TInt CBmpAnimSettings::Timing()
    { return iTiming; }

// Set default timing
inline void CBmpAnimSettings::SetTiming(TInt aTiming)
    { iTiming = aTiming; }


// Get default bitmap file
inline TDesC16& CBmpAnimSettings::PluginName()
    { return iPluginName; }
    
    
// Get default bitmap file
inline TDesC16& CBmpAnimSettings::BitmapFilename()
    { return iMbmFileName; }
    
// Get landscape bitmap file
inline TDesC16& CBmpAnimSettings::BitmapFilenameLandscape()
    { return iMbmFileNameL; }

// Get lights on time (secs)
inline TInt CBmpAnimSettings::Lights()
    { return iLights; }

    
// Set lights on time (secs)
inline void CBmpAnimSettings::SetLights(TInt aSecs)
    { iLights = aSecs; }

// Use landscape?
inline TInt CBmpAnimSettings::UseLandscape()
    { return iUseLandscape; }

// Scale bitmaps?
inline TInt CBmpAnimSettings::ScaleBmps()
    { return iScaleBmps; }

#endif  // C_CBMPANIMSETTINGS_H
