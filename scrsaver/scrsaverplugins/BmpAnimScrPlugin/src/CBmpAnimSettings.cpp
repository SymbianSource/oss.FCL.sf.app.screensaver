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
* Description:     Loads and stores the bitmap animator settings
*
*/





#include <e32std.h>

#include "CBmpAnimSettings.h"

#include "BmpAnimUtils.h"
#include <f32file.h>

// Defaults
const TInt KRunningTime = 1000000 * 60;  // 60 secs
const TInt KTiming = 200000;  // 5 fps 
const TInt KSuspendTime = -1; // Max = KMaxTInt, -1 = indefinitely
const TInt KUndefined = -1;

// Configuration file name
_LIT(KIniFile, "BmpAnimScrPlugin.ini");

// Default bitmap file name
_LIT(KMbmFileName, "BmpAnimScrPlugin.mbm");

// Default bitmap file name - lansdscape
_LIT(KMbmFileNameL, "BmpAnimScrPluginL.mbm");

// Basic info section name
_LIT8(KIniSectionInfo, "info");

// Info section keys
_LIT8(KIniKeyName, "name");  // plugin name
_LIT8(KIniKeyNumImages, "numimages");  // number of images
_LIT8(KIniKeyMbmFile, "mbmfile");  // name of imagefile
_LIT8(KIniKeyMbmFileL, "mbmfilelandscape");  // name of landscape imagefile
_LIT8(KIniKeyRunningTime, "runningtime");  // time to run animation (in millis)
_LIT8(KIniKeyLoopCount, "loopcount");  // loop count (if time is not defined)
_LIT8(KIniKeySuspendTime, "suspendtime");  // time to suspend (in millis)
_LIT8(KIniKeyTiming, "timing");  // time to display each image (RE-USED BELOW)
_LIT8(KIniKeyLights, "lights");  // lights on for 5 secs. Over 5 = number of secs
_LIT8(KIniKeyUseLandscape, "uselandscape");  // 1 = landscape image file used
_LIT8(KIniKeyScaleBmps, "scalebmps");  // 1 = bitmaps scaled to screen size

// Image section base name
_LIT8(KIniSectionImageBase, "image");  // image information section name

// Image section keys
_LIT8(KIniKeyType, "type");  // 0 = background, 1 = bitmap, 2 = icon
_LIT8(KIniKeyIndex, "index");  // index of image in mbm
_LIT8(KIniKeyMaskIndex, "maskindex");  // index of icon mask in mbm
_LIT8(KIniKeyFile, "file");  // name of image file (not yet supported)
_LIT8(KIniKeyPosX, "posx");  // display position X
_LIT8(KIniKeyPosY, "posy");  // display position Y
// _LIT8(KIniKeyTiming, "timing");  // time to display this image (NOTE RE-USE!)


// User constructor
CBmpAnimSettings* CBmpAnimSettings::NewL()
    {
    CBmpAnimSettings* self = new(ELeave) CBmpAnimSettings();
    
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    
    return self;
    }

    
// C'tor, sets defaults
CBmpAnimSettings::CBmpAnimSettings()
    : iRunningTime(KRunningTime),
      iLoopCount(KUndefined),
      iSuspendTime(KSuspendTime),
      iTiming(KTiming),
      iPluginName(KPluginName),
      iMbmFileName(KMbmFileName),
      iMbmFileNameL(KMbmFileNameL),
      iLights(KUndefined),
      iUseLandscape(0),
      iScaleBmps(0)
    {
    }


// Two-phase constructor
void CBmpAnimSettings::ConstructL()
    {
    // Try to open the settings file
    if (OpenSettings() == KErrNone)
        {
        LoadSettingsL();
        }
    }

    
// D'tor
CBmpAnimSettings::~CBmpAnimSettings()
    {
    delete iIniData;
    }


// --- Access functions ---
    
    
// Loads (common) settings from the settings file
TBool CBmpAnimSettings::LoadSettingsL()
    {
    // Sanity check
    if (!iIniData)
        {
        return EFalse;
        }

    // Running time (never mind the section)
    if (!(iIniData->FindVar(KIniKeyRunningTime, iRunningTime)))
        {
        // Running time not found - try loop count
        if (!(iIniData->FindVar(KIniKeyLoopCount, iLoopCount)))
            {
            // That's not there either - use default running time
            iRunningTime = KRunningTime;
            }
        else
            {
            // Loop count found - running time is undefined
            iRunningTime = KUndefined;
            }
        }

    // Suspend time
    if (!(iIniData->FindVar(KIniKeySuspendTime, iSuspendTime)))
        {
        iSuspendTime = KSuspendTime;  // TODO: KUndefined
        }

    // Default timing (need to use section as well)
    if (!(iIniData->FindVar(
              KIniSectionInfo,
              KIniKeyTiming,
              iTiming)))
        {
        iTiming = KTiming;
        }

    // Lights time
    if (!(iIniData->FindVar(KIniKeyLights, iLights)))
        {
        iLights = KUndefined;
        }

    // Use landscape
    if (!(iIniData->FindVar(KIniKeyUseLandscape, iUseLandscape)))
        {
        iUseLandscape = 0;
        }

    // Scale bmps
    if (!(iIniData->FindVar(KIniKeyScaleBmps, iScaleBmps)))
        {
        iScaleBmps = 0;
        }

    // Plugin name
    iPluginName = KNullDesC;
    if ((!(iIniData->FindVar(KIniKeyName, iPluginName))) ||
        (iPluginName.Length() <= 0))
        {
        // Not found, or empty setting - use default
        iPluginName = KPluginName;
        }

    // Load BMP file names
    LoadBitmapFileName();
    LoadBitmapFileName(ETrue);
    
    BMALOGGER_WRITEF(_L("BMA: Detected settings:"));
    BMALOGGER_WRITEF(_L(" Name: %S"), &iPluginName);
    BMALOGGER_WRITEF(_L(" Run for: %d"), RunningTime());
    BMALOGGER_WRITEF(_L(" Loops: %d"), LoopCount());
    BMALOGGER_WRITEF(_L(" Suspend: %d"), SuspendTime());
    BMALOGGER_WRITEF(_L(" Timing: %d"), Timing());
    BMALOGGER_WRITEF(_L(" Lights: %d"), Lights());
    BMALOGGER_WRITEF(_L(" Bitmap file: %S"), &iMbmFileName);
    BMALOGGER_WRITEF(_L(" Lanscape bitmap file: %S"), &iMbmFileNameL);
    BMALOGGER_WRITEF(_L(" Use landscape: %d"), UseLandscape());
    BMALOGGER_WRITEF(_L(" Scale bmps: %d"), ScaleBmps());
    BMALOGGER_WRITEF(_L("BMA: End settings"));

    return ETrue;
    }


// Saves (common) settings
TBool CBmpAnimSettings::SaveSettingsL()
    {
    // Sanity check
    if (!iIniData)
        {
        return EFalse;
        }

    // Write lights
    if (iIniData->WriteVarL(
            KIniSectionInfo, KIniKeyLights, iLights) == KErrNone)
        {
        iIniData->CommitL();
        }

    return ETrue;
    }
    
// --- private ---

    
// Opens the settings file (initialises the settings reader)
TInt CBmpAnimSettings::OpenSettings()
    {
    // Locate the dll path
    TFileName dllName;
//    Dll::FileName(dllName);
    dllName.Append(_L("\\resource\\plugins\\"));

    // Combine the dll path with the settings file name
    TParse p;
    p.SetNoWild(KIniFile, &dllName, NULL);

    BMALOGGER_WRITEF(_L("Searching INI: %S"), &(p.FullName()));

    // Try to open the INI file from plugin directory
    TRAPD(err, iIniData = CBmpAnimIniData::NewL(p.FullName()));

    if (err != KErrNone)
        {
        BMALOGGER_WRITE("INI not in plugin dir, trying \\system\\data");
        // Try \system\data instead
        TRAP(err, iIniData = CBmpAnimIniData::NewL(KIniFile));
        }
    
    if (err != KErrNone)
        {
        // Make sure the settings are not attempted to be used
        delete iIniData;
        iIniData = NULL;
        BMALOGGER_WRITE("INI not found");
        }
    else
        {
        BMALOGGER_WRITE("INI found");
        }

    return err;
    }


// Gets the bitmap file name from settings, or uses default
void CBmpAnimSettings::LoadBitmapFileName(TBool aLandscape)
    {
    // Get the DLL path name. The path is used, if only a file name
    // is specified.
    TFileName dllName;
    Dll::FileName(dllName);
    
    // Get whatever's in the settings
	TFileName fileName;
    TBool res = ETrue;
    if (aLandscape)
        {
        res = iIniData->FindVar(KIniKeyMbmFileL, fileName);
        }
    else
        {
        res = iIniData->FindVar(KIniKeyMbmFile, fileName);
        }
    
    if (!res)
        {
        // Not found - use the default name with the plugin directory
        // there's really no need to do anything here (I think)
        fileName = (_L("\\resource\\plugins\\"));
        }

    // The combination order should be:
    // 1. use whatever was defined in the settings (part or full path)
    // 2. use the default name part
    // 3. use the DLL path, if no path parts have yet been defined
    TParse p;
    if (p.SetNoWild(fileName,
                    aLandscape ? &KMbmFileNameL : &KMbmFileName,
                    &dllName) == KErrNone)
        {
        if (aLandscape)
            {
            iMbmFileNameL = p.FullName();
            }
        else
            {
            iMbmFileName = p.FullName();
            }
        }
    }

//  End of File
