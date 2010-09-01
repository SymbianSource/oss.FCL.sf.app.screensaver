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
* Description:   Main code file for plugin
*
*/



#include <eikenv.h>
#include <e32math.h>
#include <bitdev.h>
#include <aknutils.h>
#include <akniconutils.h>
#include <mifconvdefs.h>

#include "CBmpAnimScrPlugin.h"
#include "BmpAnimUtils.h"

#include <AknQueryDialog.h>
#include <avkon.rsg>
// "BmpAnimScrPlugin.rsg"

const TInt KDefaultViewTime = 1000000;

//
// CBmpAnimSrcPlugin
//

// Creates and returns a new instance of CBmpAnimScrPlugin
CBmpAnimScrPlugin* CBmpAnimScrPlugin::NewL()
    {
    CBmpAnimScrPlugin *plugin = new (ELeave) CBmpAnimScrPlugin();

    // Initialize settings object so that the plugin name can be retrieved
    iSettings = CBmpAnimSettings::NewL();
    
    return plugin;
    }

    
// Default constructor
CBmpAnimScrPlugin::CBmpAnimScrPlugin()
    : iState(EPluginStateLoading),
      iStopDisplaying(EFalse),
      iLoadedAnimation(ENone)
    {
    BMALOGGER_CREATE;

    }
    

// Destructor
CBmpAnimScrPlugin::~CBmpAnimScrPlugin()
    {
    delete iModel;
    delete iSettings;

    StopDisplayTimer();
    delete iDisplayTimer;
    
    BMALOGGER_DELETE;
    }

    
// --- from Screensaverplugin ---


// Initialization function. Must be called before anything but
// name query can be done
TInt CBmpAnimScrPlugin::InitializeL(MScreensaverPluginHost *aHost) 
    {
    ASSERT(aHost);
    
    // Sanity check
    if (!aHost)
        {
        return KErrArgument;
        }
    
    // Save the host interface
    iHost = aHost;

    // Start state
    iState = EPluginStateLoading;

    // Initial timing (may be overridden by settings)
    iHost->SetRefreshTimerValue(KDefaultViewTime);

    // Lie that we'll show indicators so that host does not prevent
    // plugin to be run if there are any to show. We'll stop after a
    // while anyway and the indicators are shown by normal Screensaver
    iHost->OverrideStandardIndicators();
    
    // Grab hold of the environment (this could be in the plugin host interface)
    iEikEnv = CEikonEnv::Static();

    // Create the model to store the animation in
    iModel = new(ELeave) CBmpAnimModel();
    iModel->ConstructL(iSettings);

    // Get screen info
    UpdateDisplayInfo();
    
    // Load the animation (Reload figures out which graphics should be used)
    ReloadAnimationL();

    // Create display timer
    iDisplayTimer = CPeriodic::NewL(CActive::EPriorityStandard);

    return KErrNone;
    }

    
// Draw function being called repeatedly by the host
TInt CBmpAnimScrPlugin::Draw(CWindowGc& aGc) 
    {
    // If initializing, start the timer and move on to animation state
    if (iState == EPluginStateInitializing)
        {
        BMALOGGER_WRITE("First draw, initializing");
        
        StartDisplayTimer();
        HandlePluginState();
        SetDisplayMode();
        TInt nLights = iModel->Settings()->Lights();

        if (nLights > 0)
            {
            Lights(nLights);
            }

        // Make sure the animation sequence starts from the beginning
        iModel->SetCurrentItemIndex(0);
        }

    // Retrieve the next image in sequence
    TBool endOfSequence = EFalse;
    CBmpAnimItem* pItem = iModel->NextItem(endOfSequence);

    if ((endOfSequence) || (!pItem))
        {
        // End of sequence reached, see if we've shown enough (1 minute)
        if (iStopDisplaying)
            {
            // Stop the timer
            StopDisplayTimer();

            // Back to square 1
            iState = EPluginStateInitializing;

			// Set a lower refresh rate while plugin is suspended.
            // This allows the Screensaver to stop Window Server heartbeat
            // and the system is able to sleep normally
            // NOTE: Not needed anymore, Screensaver now shuts down
            // WSERV heartbeat for suspended plugins
            // iHost->SetRefreshTimerValue(KDefaultViewTime);

            TInt suspendTime = iModel->Settings()->SuspendTime();
            
            BMALOGGER_WRITEF(_L("BMA: Done drawing, suspending for %d"),
                             suspendTime);
            
            iHost->Suspend(suspendTime);

            return KErrNone;
            }
        }

    if (pItem)
        {
        // Make sure the window is empty in case the bitmap doesn't
        // fill the whole screen
        aGc.Clear();

        DrawCentered(aGc, pItem);
        }

// Activate code if centering INI-controllable
#if 0
        // Retrieve drawing information
        CGulIcon* pIcon = pItem->Icon();
        TPoint position = pItem->Position();
        CFbsBitmap* bitmap = pIcon->Bitmap();
        CFbsBitmap* mask = pIcon->Mask();

        // Draw the whole bitmap at position
        TRect rect(position, bitmap->SizeInPixels());
        
        if (mask)
            {
            // Looks like a real icon - draw masked
            aGc.BitBltMasked(position, bitmap, rect, mask, ETrue);
            }
        else
            {
            // Just the bitmap - no masked draw
            aGc.BitBlt(position, bitmap, rect);
            }

        // Wait for the specified time until next image
        //
        // TODO: The new wk28 Screensaver crashes if the next call
        // is uncommented. Maybe the timer is not stopped
        // before starting again? Hmm... doesn't seem to happen
        // anymore. I wonder what changed. Anyway, I'll have it
        // commented out for the time being, in order to control
        // all frames' rate with the single setting
        // 
        // NOTE: There was a flaw in Screensaver where it would try to
        // start refresh timer twice, if plugin changes the value during
        // the first draw. The fix is released for 2.6_wk40_FB4
        // iHost->SetRefreshTimerValue(pItem->Timing());
        }
/*
    aGc.SetPenColor(TRgb(255,0,0));
    aGc.SetPenStyle(CGraphicsContext::ESolidPen);
    aGc.DrawRect(TRect(30, 30, 100, 100));

    aGc.SetPenColor(TRgb(255,0,0));
    aGc.SetPenSize(TSize(3,3));
    aGc.DrawRect(TRect(120, 100, 200, 150));
*/
#endif
    return KErrNone;
    }


// Return the name of the plugin
const TDesC16& CBmpAnimScrPlugin::Name() const
    {
    if (iSettings)
        {
        return iSettings->PluginName();
        }

    return KPluginName;
    }


// Handles events sent by the screensaver
TInt CBmpAnimScrPlugin::HandleScreensaverEventL(
    TScreensaverEvent aEvent,
    TAny* /* aData */)
    {
    switch (aEvent)
        {
        case EScreensaverEventStarting:
            BMALOGGER_WRITE("Start event");
            break;
        case EScreensaverEventStopping:
            BMALOGGER_WRITE("Stop event");
            StopDisplayTimer();
            iState = EPluginStateInitializing;
            break;
        case EScreensaverEventDisplayChanged:
            BMALOGGER_WRITE("Display changed event");
            // Grab current screen info
            UpdateDisplayInfo();
            // Reload animation, if needed
            ReloadAnimationL();
            break;
        default:
            break;
        }

    return KErrNone;
    }


// Return plugin capabilities (configurable)
TInt CBmpAnimScrPlugin::Capabilities()
    {
    return EScpCapsConfigure;
    }


// Perform a plugin function
TInt CBmpAnimScrPlugin::PluginFunction(TScPluginCaps aFunction, TAny* aParam)
    {
    switch (aFunction)
        {
        case EScpCapsConfigure:
            {
            TRAPD(err, err = ConfigureL(aParam));
            return err;
            }
            break;
        default:
            return KErrNotSupported;
            break;
        }
    }
    

// --- private functions ---

// Draws centered items
void CBmpAnimScrPlugin::DrawCentered(CWindowGc& aGc, CBmpAnimItem* aItem)
    {
    CGulIcon* pIcon = aItem->Icon();
    CFbsBitmap* bitmap = pIcon->Bitmap();
    CFbsBitmap* mask = pIcon->Mask();

    if (!bitmap)
        return;

    // Center the bitmap horizontally and vertically (crop off excess)    
    TPoint pos;
    TRect rectToDraw;
    TSize sizeBmp = bitmap->SizeInPixels();
    TInt screenWidth = iDi.iRect.Width();
    TInt screenHeight = iDi.iRect.Height();

    // Horizontally
    if (sizeBmp.iWidth <= screenWidth)
        {
        // Width fits on screen - center xpos
        pos.iX = (screenWidth - sizeBmp.iWidth) / 2;
        
        // Whole width of bmp can be drawn
        rectToDraw.SetWidth(sizeBmp.iWidth);
        }
    else
        {
        // Bmp wider than screen - xpos top left
        pos.iX = 0;
        
        // Adjust draw rect position and width
        rectToDraw.iTl.iX = (sizeBmp.iWidth - screenWidth) / 2;
        rectToDraw.SetWidth(screenWidth);
        }

    // Vertically
    if (sizeBmp.iHeight <= screenHeight)
        {
        // Height fits on screen - center ypos
        pos.iY = (screenHeight - sizeBmp.iHeight) / 2;
        
        // Whole height of bmp can be drawn
        rectToDraw.SetHeight(sizeBmp.iHeight);
        }
    else
        {
        // Bmp higher than screen - ypos top left
        pos.iY = 0;
        
        // Adjust draw rect position and height
        rectToDraw.iTl.iY = (sizeBmp.iHeight - screenHeight) / 2;
        rectToDraw.SetHeight(screenHeight);
        }

    // Do the drawing
    if (mask)
        {
        // Looks like a real icon - draw masked
        aGc.BitBltMasked(pos, bitmap, rectToDraw, mask, ETrue);
        }
    else
        {
        // Just the bitmap - no masked draw
        aGc.BitBlt(pos, bitmap, rectToDraw);
        }
    }


// Loads the animation into the model
void CBmpAnimScrPlugin::LoadAnimationL(TBool aLandscape, TBool aRotate)
    {
    // Rotated landscape not supported
    ASSERT(!(aLandscape && aRotate));
    
    // Start by getting rid of a possible loaded animation
    iModel->DeleteAll();
    
    // Bitmap index. If negative, loading is finished.
    TInt nIndex = KMifIdFirst;
    
    TFileName fileName;

    if (aLandscape)
        {
        fileName = iModel->Settings()->BitmapFilenameLandscape();
        }
    else
        {
        fileName = iModel->Settings()->BitmapFilename();
        }

    BMALOGGER_WRITEF(_L("BMA: Loading from: %S"), &(fileName));
            
    while (nIndex > 0)
        {
        CFbsBitmap* pBmp = NULL;
        
        TRAPD(err, pBmp = AknIconUtils::CreateIconL(fileName, nIndex));

        if ((pBmp) && (err == KErrNone))
            {
            // Got bitmap, push and set size
            CleanupStack::PushL(pBmp);
            TInt scaleErr = ScaleBitmap(pBmp, aRotate);
            if (scaleErr == KErrNone)
                {
                // Create an item with the bitmap and store it in the model
                CBmpAnimItem* pItem = new(ELeave) CBmpAnimItem();
                CleanupStack::PushL(pItem);
            
                pItem->SetIconL(pBmp);
                iModel->AppendItemL(pItem);
                
                CleanupStack::Pop(2); // pBmp, pItem
            
                BMALOGGER_WRITEF(_L("BMA: Loaded bmp %d"), nIndex);
                
                // Try loading next bitmap (skip mask IDs)
                nIndex += 2;
                }
            else
                {
                BMALOGGER_WRITEF(_L("BMA: Bmp %d scale err %d"),
                                 nIndex, scaleErr);

                // Apparently SVG icon was not found, this is not caught
                // in CreateIconL(). Assume last image was loaded.
                CleanupStack::PopAndDestroy();  // pBmp
                nIndex = -1;
                }
            }
        else
            {
            // Loading failed - maybe reached end of bitmaps 
            nIndex = -1;
            
            BMALOGGER_WRITEF(_L("BMA: Bmp load failed: %d"), err);
            }
        }

    // Save the type of loaded animation
    if (aLandscape)
        {
        iLoadedAnimation = ELandscape;
        }
    else if (aRotate)
        {
        iLoadedAnimation = EPortraitRotated;
        }
    else
        {
        iLoadedAnimation = EPortrait;
        }

    // On to next state
    HandlePluginState();
    
    // Start animating, when appropriate
    iHost->SetRefreshTimerValue(iModel->Settings()->Timing());

    BMALOGGER_WRITE("BMA: Animation loaded");
    }


// Re-loads the animation into the model, if needed
void CBmpAnimScrPlugin::ReloadAnimationL()
    {
    // Check if the correct graphics are already loaded
    if (!ReloadNeeded())
        {
        // Done! That was easy :)
        return;
        }

    // Load correct graphics
    LoadAnimationL(LoadLandscape(), RotateNeeded());
    }

    
// Starts the display timer
void CBmpAnimScrPlugin::StartDisplayTimer()
    {
    ASSERT(iDisplayTimer);

    TInt time = iModel->Settings()->RunningTime();

    BMALOGGER_WRITEF(_L("BMA: Start display timer for %d"), time);

    iStopDisplaying = EFalse;
    iDisplayTimer->Start(
        time,
        time,
        TCallBack(DisplayTimerCallback, this));
    }

    
// Stops the display timer
void CBmpAnimScrPlugin::StopDisplayTimer()
    {
    BMALOGGER_WRITE("BMA: Stop display timer");

    if (iDisplayTimer)
        {
        iDisplayTimer->Cancel();
        }
    
    iStopDisplaying = EFalse;
    }


// Display timer callback - sets animation stop flag
TInt CBmpAnimScrPlugin::DisplayTimerCallback(TAny* aPtr)
    {
    BMALOGGER_WRITE("BMA: Display timer timeout");

    CBmpAnimScrPlugin* _this = REINTERPRET_CAST(CBmpAnimScrPlugin*, aPtr);
    _this->iStopDisplaying = ETrue;
    return KErrNone;
    }

    
// Changes the internal state flag    
void CBmpAnimScrPlugin::HandlePluginState()
    {
    switch (iState)
        {
        case EPluginStateLoading:
            iState = EPluginStateInitializing;
            break;
        case EPluginStateInitializing:          
            iState = EPluginStateAnimation;
            break;
        case EPluginStateAnimation:
             break; 
        case EPluginStateStoppingAnimation:
             iHost->SetRefreshTimerValue(KDefaultViewTime);
             iState = EPluginStateInitializing;
             break;
        }
    }


// Requests display mode from host
void CBmpAnimScrPlugin::SetDisplayMode()
    {
    if (!iHost)
        {
        return;
        }
    
    // Exit partial mode
    iHost->ExitPartialMode();
    }


void CBmpAnimScrPlugin::Lights(TInt aSecs)
    {
    BMALOGGER_WRITEF(_L("BMA: Request lights for %d secs"), aSecs);
    iHost->RequestLights(aSecs);
    }

    
// Configure the plugin
TInt CBmpAnimScrPlugin::ConfigureL(TAny* aParam)
    {
    if (!iSettings)
        {
        return KErrNotFound;
        }
    
    // Grab the parameter (CEikonEnv in this case)
    CEikonEnv* eikEnv = NULL;

    if (aParam)
        {
        // The host was kind enough to provide us with a param - use it
        eikEnv = REINTERPRET_CAST(CEikonEnv*, aParam);
        }
    else if (iEikEnv)
        {
        // Use own env, if initialized
        eikEnv = iEikEnv;
        }

    TInt setting = iSettings->Lights();
    
    CAknNumberQueryDialog* dlg = CAknNumberQueryDialog::NewL(setting);
    CleanupStack::PushL(dlg);
    _LIT(KPrompt, "Lights time (sec)");
    dlg->SetPromptL(KPrompt);
    dlg->SetMinimumAndMaximum(0, 30);
    CleanupStack::Pop();
    
    if (dlg->ExecuteLD(R_AVKON_DIALOG_QUERY_VALUE_NUMBER))
        {
        iSettings->SetLights(setting);
        iSettings->SaveSettingsL();
        }
    
    // All was swell!
    return KErrNone;
    }


// Scale bitmap to screen size, set size of SVG bitmaps
TInt CBmpAnimScrPlugin::ScaleBitmap(CFbsBitmap* aBmp, TBool aRotate)
    {
    TInt ret = KErrNone;

    // SVG size always screen size
    TSize size = iDi.iRect.Size();

    if (!AknIconUtils::IsMifIcon(aBmp))
        {
        // Bitmaps maintain their original size, unless scaling requested, in which
        // case screen size is OK 
        if (!iSettings->ScaleBmps())
            {
            // No scaling, use original size
            size = aBmp->SizeInPixels();

            if (aRotate)
                {
                // Lie the target size, otherwise IconUitls will think
                // the image needs scaling (this won't work perfectly
                // either, the image gets clipped a little :(
                //size.SetSize(size.iHeight, size.iHeight);

                // Flip size for rotation
                size.SetSize(size.iHeight, size.iWidth);
                }
            }
        }
    
    if (aRotate)
        {
        // Set image to screen size and rotate 90 deg left (270 right)
        // ret = SetSizeAndRotation(aBmp, size, 270);
        ret = AknIconUtils::SetSizeAndRotation(
            aBmp, size, EAspectRatioPreservedSlice, 270);
        }
    else
        {
        // Just set image to size
        ret = AknIconUtils::SetSize(aBmp, size, EAspectRatioPreserved);
        }

    return ret;
    }


// Returns ETrue if reload of the animation is needed
TBool CBmpAnimScrPlugin::ReloadNeeded()
    {
    // Assume reload needed
    TBool needed = ETrue;

    switch (iLoadedAnimation)
        {
        case EPortrait:
            // No reload if display portrait
            if (!DisplayIsLandscape())
                {
                needed = EFalse;
                }
            break;

        case ELandscape:
        case EPortraitRotated:
            // No reload if display landscape
            if (DisplayIsLandscape())
                {
                needed = EFalse;
                }
            break;
            
        case ENone:
        default:
            // Reload
            break;
        }

    return needed;
    }

    
// Returns ETrue if display in landscape
TBool CBmpAnimScrPlugin::DisplayIsLandscape()
    {
    // Should actually check the rotation and stuff, but what the hey...
    return (iDi.iRect.Width() > iDi.iRect.Height());
    }


// Returns ETrue if graphics should be rotated
TBool CBmpAnimScrPlugin::RotateNeeded()
    {
    // Rotate needed, if only portrait graphics are used, and
    // display is landscape
    return ((!iSettings->UseLandscape()) && (DisplayIsLandscape()));
    }


// Returns ETrue if landscape graphics should be loaded
TBool CBmpAnimScrPlugin::LoadLandscape()
    {
    // Landscape, if only available and display is landscape
    return ((iSettings->UseLandscape()) && (DisplayIsLandscape()));
    }
    
    
// Updates the saved information about display
void CBmpAnimScrPlugin::UpdateDisplayInfo()
    {
    iDi.iSize = sizeof(TScreensaverDisplayInfo);
    iHost->DisplayInfo(&iDi);
    }

#if 0
// Rotates and scales a source bitmap into target bitmap (non-leaving wrapper)
TInt CBmpAnimScrPlugin::SetSizeAndRotation(
    CFbsBitmap* aBmp, TSize aSize, TInt aAngle)
    {
    // Anything to do?
    if ((aBmp) && (aBmp->SizeInPixels() == aSize) && ((aAngle % 360) == 0))
        {
        // Duh, the bitmap is already as requested
        return KErrNone;
        }

    // Call the actual workhorse
    TRAPD(err, SetSizeAndRotationL(aBmp, aSize, aAngle));

    return err;
    }


// Rotates and scales a source bitmap into target bitmap (leaving version)
void CBmpAnimScrPlugin::SetSizeAndRotationL(
    CFbsBitmap* aBmp, TSize aSize, TInt aAngle)
    {
    // Make a copy of the source bitmap, and use the original source as target
    CFbsBitmap* tmpBmp = new (ELeave) CFbsBitmap;
    CleanupStack::PushL(tmpBmp);

    User::LeaveIfError(tmpBmp->Duplicate(aBmp->Handle()));
    
    // Discard original bitmap
    aBmp->Reset();

    // Create new target bitmap in the original object
    User::LeaveIfError(aBmp->Create(aSize, tmpBmp->DisplayMode()));

    // Let the workhorse do its work
    RotateAndScaleBitmapL(TRect(aSize), aBmp, tmpBmp, aAngle);

    // Not interested in original anymore
    CleanupStack::PopAndDestroy(tmpBmp);
    }
    

// Rotates and scales a source bitmap into target bitmap
void CBmpAnimScrPlugin::RotateAndScaleBitmapL(
    const TRect& aTrgRect,
    CFbsBitmap* aTrgBitmap, 
    CFbsBitmap* aSrcBitmap,
    TInt aAngle)
    {
    aAngle = aAngle % 360;
    if (aAngle < 0)
        {
        aAngle+=360;
        }

    if (!aSrcBitmap) User::Leave(KErrArgument);
    if (!aTrgBitmap) User::Leave(KErrArgument);
    if (aSrcBitmap->DisplayMode() != aTrgBitmap->DisplayMode()) 
        User::Leave(KErrArgument);
    
    TSize trgBitmapSize = aTrgBitmap->SizeInPixels();
    if ((trgBitmapSize.iHeight < aTrgRect.iBr.iY) ||
        (trgBitmapSize.iWidth < aTrgRect.iBr.iX))
        {
        User::Leave(KErrArgument);
        }

    if (aTrgRect.IsEmpty())
        return;

    TSize srcSize = aSrcBitmap->SizeInPixels();

    TInt centerX = srcSize.iWidth / 2;
    TInt centerY = srcSize.iHeight / 2;

    TInt trgWidth = aTrgRect.Width();
    TInt trgHeight = aTrgRect.Height();
    
    TInt scalefactor = 65536;
    TInt xscalefactor = (srcSize.iWidth << 16) / trgWidth;
    TInt yscalefactor = (srcSize.iHeight << 16) / trgHeight;

    // Check if rotating 90 left or right, no need to scale
    if (((aAngle == 270) || (aAngle == 90)) &&
        (srcSize.iWidth == trgHeight) &&
        (srcSize.iHeight == trgWidth))
            {
            scalefactor = 65535;
            }
        else
            {
            if (xscalefactor < yscalefactor)
                {
                scalefactor = yscalefactor;
                }
            else
                {
                scalefactor = xscalefactor;
                }
            }
    
        TBool srcTemporary = EFalse;
        TBool hardMask = EFalse;
        if (aSrcBitmap->IsRomBitmap())
            {
            srcTemporary = ETrue;
            }
        if (aSrcBitmap->IsCompressedInRAM())
            {
            srcTemporary = ETrue;
            }

        TBool fallbackOnly = EFalse;
        TDisplayMode displayMode = aSrcBitmap->DisplayMode();
        TUint8 fillColor = 0;
        
        switch(displayMode)
            {
            case EGray2:
                srcTemporary = ETrue;
                hardMask = ETrue;
                fillColor = 0xff; // white
                break;
            case EGray4:
            case EGray16:
            case EColor16:
            case EColor16M:
            case ERgb:
                fallbackOnly = ETrue;
                break;
            case EColor256:
                fillColor = 0xff; // should be black in our indexed palette....
            case EGray256:
            case EColor4K:
            case EColor64K:

            case EColor16MU:
            // These are the supported modes
                break;
            default:
                fallbackOnly = ETrue;
            }

        if (fallbackOnly)
            {
            // Color mode not supported
            User::Leave(KErrNotSupported);
            }

        CFbsBitmap* realSource = aSrcBitmap;
        CFbsBitmap* realTarget = aTrgBitmap;
        if (srcTemporary)
            {
            realSource = new (ELeave) CFbsBitmap();
            CleanupStack::PushL(realSource);
            if (hardMask)
                {
                realTarget = new (ELeave) CFbsBitmap();
                CleanupStack::PushL(realTarget);
                User::LeaveIfError(realSource->Create(srcSize, EGray256));
                displayMode = EGray256;
                User::LeaveIfError(realTarget->Create(
                    aTrgBitmap->SizeInPixels(), EGray256));
                }
            else
                {
                User::LeaveIfError(realSource->Create(
                    srcSize, aSrcBitmap->DisplayMode()));
                }

            CFbsBitmapDevice* dev = CFbsBitmapDevice::NewL(realSource);
            CleanupStack::PushL(dev);
            CFbsBitGc* gc = NULL;
            User::LeaveIfError(dev->CreateContext(gc));
            CleanupStack::PushL(gc);
            gc->BitBlt(TPoint(0,0), aSrcBitmap);
            CleanupStack::PopAndDestroy(2); // dev, gc
            }

        // Heap lock for FBServ large chunk is only needed with large bitmaps.
        if (realSource->IsLargeBitmap() || realTarget->IsLargeBitmap())
            {
            realTarget->LockHeapLC(ETrue); // fbsheaplock
            }
        else
            {
            // Bogus push so we can pop() anyway
            CleanupStack::PushL((TAny*)NULL);
            }

        TUint32* srcAddress = realSource->DataAddress();
        TUint32* trgAddress = realTarget->DataAddress();

        TReal realsin;
        TReal realcos;
        TInt sin;
        TInt cos;

        User::LeaveIfError(Math::Sin(realsin, ((2*KPi)/360) * aAngle));
        User::LeaveIfError(Math::Cos(realcos, ((2*KPi)/360) * aAngle));

        sin = ((TInt)(realsin * scalefactor));
        cos = ((TInt)(realcos * scalefactor));

        TInt xx = ((trgWidth)/2) - ((srcSize.iWidth/2) - centerX);
        TInt yy = ((trgHeight)/2) - ((srcSize.iHeight/2) - centerY);

        TInt x = 0;
        TInt y = 0;
        TInt u = 0;
        TInt v = 0;

        if( (displayMode==EGray256) || (displayMode==EColor256) )
            {
            TInt srcScanLen8 = CFbsBitmap::ScanLineLength(
                srcSize.iWidth, displayMode);
            TInt trgScanLen8 = CFbsBitmap::ScanLineLength(
                trgBitmapSize.iWidth, displayMode);
            TUint8* srcAddr8 = reinterpret_cast<TUint8*>(srcAddress);
            TUint8* trgAddr8 = reinterpret_cast<TUint8*>(trgAddress);

            // skip left and top margins in the beginning
            trgAddr8 += trgScanLen8 * aTrgRect.iTl.iY + aTrgRect.iTl.iX;

            for (y = 0; y < trgHeight; y++)
                {
                u = (-xx) * cos + (y-yy) * sin + (centerX<<16);
                v = (y-yy) * cos - (-xx) * sin + (centerY<<16);
                for (x = 0; x < trgWidth; x++)
                    {
                    if (((u>>16)>=srcSize.iWidth) ||
                         ((v>>16)>=srcSize.iHeight) ||
                         ((u>>16)<0) ||
                         ((v>>16)<0)) 
                        {
                        *trgAddr8++ = fillColor;
                        }
                    else
                        {            	
                        *trgAddr8++ = srcAddr8[(u>>16)+(((v>>16))*srcScanLen8)];
                        }
                    u += cos;
                    v -= sin;
                    }
                trgAddr8 += trgScanLen8 - trgWidth;
                }
            }
        else if( displayMode == EColor64K || displayMode == EColor4K)
            {
            TInt srcScanLen16 = CFbsBitmap::ScanLineLength(
                srcSize.iWidth, displayMode) / 2;
            TInt trgScanLen16 = CFbsBitmap::ScanLineLength(
                trgBitmapSize.iWidth, displayMode) / 2;
            TUint16* srcAddr16 = reinterpret_cast<TUint16*>(srcAddress);
            TUint16* trgAddr16 = reinterpret_cast<TUint16*>(trgAddress);

            // skip left and top margins in the beginning
            trgAddr16 += trgScanLen16 * aTrgRect.iTl.iY + aTrgRect.iTl.iX;

            for (y = 0; y < trgHeight; y++)
                {
                u = (-xx) * cos + (y-yy) * sin + (centerX<<16);
                v = (y-yy) * cos - (-xx) * sin + (centerY<<16);
                for (x = 0; x < trgWidth; x++)
                    {
                    if (((u>>16)>=srcSize.iWidth) ||
                        ((v>>16)>=srcSize.iHeight) ||
                        ((u>>16)<0) ||
                        ((v>>16)<0)) 
                        {
                        *trgAddr16++ = 0;
                        }
                    else
                        {
                        *trgAddr16++ =
                            srcAddr16[(u>>16)+(((v>>16))*srcScanLen16)];
                        }
                    u += cos;
                    v -= sin;
                    }
                trgAddr16 += trgScanLen16 - trgWidth;
                }        
            }
        else if(displayMode == EColor16MU)
            {
            TInt srcScanLen32 = CFbsBitmap::ScanLineLength(
                srcSize.iWidth, displayMode) / 4;
            TInt trgScanLen32 = CFbsBitmap::ScanLineLength(
                trgBitmapSize.iWidth, displayMode) / 4;
            TUint32* srcAddr32 = srcAddress;
            TUint32* trgAddr32 = trgAddress;

            // skip left and top margins in the beginning
            trgAddr32 += trgScanLen32 * aTrgRect.iTl.iY + aTrgRect.iTl.iX;

            for (y = 0; y < trgHeight; y++)
                {
                u = (-xx) * cos + (y-yy) * sin + (centerX<<16);
                v = (y-yy) * cos - (-xx) * sin + (centerY<<16);
                for (x = 0; x < trgWidth; x++)
                    {
                    if (((u>>16)>=srcSize.iWidth) ||
                        ((v>>16)>=srcSize.iHeight) ||
                        ((u>>16)<0) ||
                        ((v>>16)<0)) 
                        {
                        *trgAddr32++ = 0;
                        }
                    else
                        {
                        *trgAddr32++ =
                            srcAddr32[(u>>16)+(((v>>16))*srcScanLen32)];
                        } 
                    u += cos;
                    v -= sin;
                    }
                trgAddr32 += trgScanLen32 - trgWidth;
                }
            }
        else
            {
            // Display mode not supported - but this should've been caught
            // already earlier
            User::Leave(KErrUnknown);
            }

        CleanupStack::PopAndDestroy(); // fbsheaplock

        if (srcTemporary)
            {
            if (hardMask)
                {
                CFbsBitmapDevice* dev = CFbsBitmapDevice::NewL(aTrgBitmap);
                CleanupStack::PushL(dev);
                CFbsBitGc* gc = NULL;
                User::LeaveIfError(dev->CreateContext(gc));
                CleanupStack::PushL(gc);  
                gc->BitBlt(TPoint(0,0), realTarget);
                CleanupStack::PopAndDestroy(3); // dev, gc, realtarget
                }
            CleanupStack::PopAndDestroy(); // realSource
            }
        }
#endif
    
// End of file
