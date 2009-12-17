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
* Description:   Main include file for plugin
*
*/



#ifndef C_CBMPANIMSCRPLUGIN_H
#define C_CBMPANIMSCRPLUGIN_H

#include <e32base.h>
#include <fbs.h>
#include <eikenv.h>
#include <coecntrl.h>
#include <coeccntx.h>

#include <ScreensaverpluginIntDef.h>

#include "CBmpAnimModel.h"
#include "CBmpAnimItem.h"
#include "CBmpAnimSettings.h"

// The screensaver plugin implementation must inherit from 
// CScreensaverPluginInterfaceDefinition
class CBmpAnimScrPlugin : public CScreensaverPluginInterfaceDefinition
    {
    enum TPluginState   
        {
        EPluginStateLoading = 0,
        EPluginStateInitializing,
        EPluginStateAnimation,
        EPluginStateStoppingAnimation,
        };

    enum TLoadedAnimation
        {
        ENone,
        EPortrait,
        ELandscape,
        EPortraitRotated
        };
    
public:
    // Class instance creation
    static CBmpAnimScrPlugin* NewL();

private:
    // Construction
    CBmpAnimScrPlugin();
    
public:
    // Destruction
    ~CBmpAnimScrPlugin();

public: // from MScreensaverPlugin
    TInt InitializeL(MScreensaverPluginHost *aHost);
    TInt Draw(CWindowGc& aGc);
    const TDesC16& Name() const;
    TInt HandleScreensaverEventL(TScreensaverEvent aEvent, TAny* aData);
    TInt Capabilities();
    TInt PluginFunction(TScPluginCaps aFunction, TAny* aParam);

private:
    // Draws centered items
    void DrawCentered(CWindowGc& aGc, CBmpAnimItem* aItem);

    // Loads the animation into the model
    void LoadAnimationL(TBool aLandscape = EFalse, TBool aRotate = EFalse);

    // Re-loads the animation when display changes, if needed
    void ReloadAnimationL();

    // Starts the display timer
    void StartDisplayTimer();

    // Stops the display timer
    void StopDisplayTimer();

    // Display timer callback
    static TInt DisplayTimerCallback(TAny* aPtr);

    // Starts the lights timer
    void StartLightsTimer();

    // Stops the lights
    void StopLightsTimer();

    // Lights timer callback
    static TInt LightsTimerCallback(TAny* aPtr);
    
    // Advances the plugin state
    void HandlePluginState();

    // Requests a suitable display mode from host
    void SetDisplayMode();

    void Lights(TInt aSecs);

    // Configures the plugin
    TInt ConfigureL(TAny* aParam);

    // Sets sizes of bitmaps
    TInt ScaleBitmap(CFbsBitmap* aBmp, TBool aRotate = EFalse);

    // Returns ETrue if reload of the animation is needed
    TBool ReloadNeeded();

    // Returns ETrue if display is landscape
    TBool DisplayIsLandscape();
    
    // Returns ETrue if graphics should be rotated
    TBool RotateNeeded();
    
    // Returns ETrue if landscape graphics should be loaded
    TBool LoadLandscape();
    
    // Updates the saved display information
    void UpdateDisplayInfo();

    // Sets bitmap size and rotation (used instead of AknIconUtils function)
    // Actually an "Easy Leaving :)" wrapper around the L-version
    TInt SetSizeAndRotation(CFbsBitmap* aBmp, TSize aSize, TInt aAngle);
    
    // Sets bitmap size and rotation (used instead of AknIconUtils function)
    void SetSizeAndRotationL(CFbsBitmap* aBmp, TSize aSize, TInt aAngle);

    // Rotates and scales a source bitmap into target bitmap
    void RotateAndScaleBitmapL(
        const TRect& aTrgRect,
        CFbsBitmap* aTrgBitmap, 
        CFbsBitmap* aSrcBitmap,
        TInt aAngle);
    
private:
    // Plugin host interface
    MScreensaverPluginHost* iHost;
    
    // Plugin state
    TInt iState;

    // Needed to retrieve bitmaps from files
    CEikonEnv* iEikEnv;

    // Animator model, stores and serves animation items
    CBmpAnimModel* iModel;

    // Animator settings. Created and deleted here, but used and managed by
    // the model. Creating here enables retrieving the plugin name
    // in non-plugin-host applications (such as Themes)
    CBmpAnimSettings* iSettings;
    
    // Timer to control how long the animation is displayed
    CPeriodic* iDisplayTimer;
    
    // Flag to control animation stop. Changed from display timer callback
    TInt iStopDisplaying;

    TScreensaverDisplayInfo iDi;

    TLoadedAnimation iLoadedAnimation;
    };


#endif   // C_CBMPANIMSCRPLUGIN_H

// End of file
