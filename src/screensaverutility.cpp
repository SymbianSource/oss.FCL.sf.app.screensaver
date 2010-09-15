/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Defines screensaver utility tools.
*
*/



#include <coecntrl.h>
#include <akntranseffect.h>
#include <data_caging_path_literals.hrh>
#include <gulicon.h>

#ifdef RD_UI_TRANSITION_EFFECTS_PHASE2
#include <akntransitionutils.h>
#endif

#include "screensaverutility.h"
#include "screensaverappui.h"
#include "ScreensaverUtils.h"


// Screensaver window group priority boost (to ensure it's on top)
const TInt KSSWinPriorityBoost = 2;

_LIT(KSsBitmapName, "\\ssbackground.mbm");
_LIT(KSsBitmapUserDrive, "c:");
_LIT(KSsBitmapSystemDrive, "z:");

// -----------------------------------------------------------------------------
// CScreensaverUtility::Panic( TInt aPanic )
// -----------------------------------------------------------------------------
//
void ScreensaverUtility::Panic( TInt aPanic )
    {
    User::Panic( _L("Screensaver"),aPanic );
    }


// -----------------------------------------------------------------------------
// CScreensaverUtility::FlushDrawBuffer( TInt aPanic )
// Flushes pending draw commands. A "must" before activating power save
// mode, so that the image can be copied to power save memory
// -----------------------------------------------------------------------------
//
void ScreensaverUtility::FlushDrawBuffer()
    {
    CCoeEnv::Static()->WsSession().Flush();
    }

// -----------------------------------------------------------------------------
// CScreensaverUtility::ScreenRotated( TInt aPanic )
// Checks if screen has been rotated
// -----------------------------------------------------------------------------
//
TBool ScreensaverUtility::ScreenRotated()
    {
    TPixelsAndRotation pxRot;
    CCoeEnv::Static()->ScreenDevice()->GetDefaultScreenSizeAndRotation(pxRot);

    // Rotated, if 90 or 270 degrees
    return ((pxRot.iRotation == CFbsBitGc::EGraphicsOrientationRotated90)
            || (pxRot.iRotation == CFbsBitGc::EGraphicsOrientationRotated270));
    }

// -----------------------------------------------------------------------------
// ScreensaverUtility::Query12HourClock
// Query whether 12-hour clock is displayed
// -----------------------------------------------------------------------------
//
TBool ScreensaverUtility::Query12HourClock( )
    {
    TLocale locale;
    return (locale.TimeFormat() == ETime12);
    }


// -----------------------------------------------------------------------------
// ScreensaverUtility::BringToForeground
// -----------------------------------------------------------------------------
//
void ScreensaverUtility::BringToForeground()
    {
#ifdef RD_UI_TRANSITION_EFFECTS_PHASE2
    const TInt KSsaCustomActivateContextNum = 1002;

    GfxTransEffect::BeginFullScreen( KSsaCustomActivateContextNum, TRect( ),
        AknTransEffect::EParameterType, AknTransEffect::GfxTransParam(
            KUidScreensaverApp,
            AknTransEffect::TParameter::EActivateExplicitContinue ) );
#endif
    CEikonEnv::Static()->SetAutoForwarding( ETrue );
    CEikonEnv::Static()->BringForwards( ETrue, ECoeWinPriorityAlwaysAtFront
        + KSSWinPriorityBoost );
    }

// -----------------------------------------------------------------------------
// ScreensaverUtility::SendToBackground
// -----------------------------------------------------------------------------
//
void ScreensaverUtility::SendToBackground()
    {
#ifdef RD_UI_TRANSITION_EFFECTS_PHASE2

    // Start the exit effect context even if this is actually application switch. The exit context
    // makes possible to map effect by using the *previous* application uid (=Screen Saver).
    // Note: Not allowed to call GfxTransEffect::EndFullScreen() as AVKON takes care of that when
    // EApplicationExit context is used! 
    const TInt KSsaCustomDeactivateContextNum = 1001;

    //condition takes care of special case of incomeing call (should not have effect then)
    if ( CAknTransitionUtils::GetData( KScreensaverCallStateChange ) == NULL )
        {
        if ( static_cast<CScreensaverAppUi*>(CCoeEnv::Static()->AppUi())->IsForeground() )
            {
            GfxTransEffect::BeginFullScreen( KSsaCustomDeactivateContextNum,
            TRect( ), AknTransEffect::EParameterType,
            AknTransEffect::GfxTransParam( KUidScreensaverApp,
                AknTransEffect::TParameter::EActivateExplicitContinue ) );
            }
        }
    CAknTransitionUtils::RemoveData( KScreensaverCallStateChange );
#endif
    CEikonEnv::Static()->RootWin().SetOrdinalPosition(
                    0, ECoeWinPriorityNeverAtFront );
    CEikonEnv::Static()->BringForwards( EFalse );
    }

// -----------------------------------------------------------------------------
// ScreensaverUtility::IsDrivePresent
// -----------------------------------------------------------------------------
//
TBool ScreensaverUtility::IsDrivePresent(TInt aDrive)
    {
    RFs& fs = CCoeEnv::Static()->FsSession();

    TInt error = KErrNone;
    TDriveInfo driveInfo;
    error = fs.Drive(driveInfo, aDrive );

    // Not present, locked or unknown is bad
    if ((error != KErrNone) ||
        (driveInfo.iMediaAtt & KMediaAttLocked) ||
        (driveInfo.iType == EMediaNotPresent) ||
        (driveInfo.iType == EMediaUnknown))
        {
        return EFalse;
        }

    // Read-only is fine
    return ETrue;
    }

// -----------------------------------------------------------------------------
// ScreensaverUtility::LoadBitmapL
// -----------------------------------------------------------------------------
//
CGulIcon* ScreensaverUtility::LoadBitmapL()
    {
    CGulIcon *icon = CGulIcon::NewL();
    CleanupStack::PushL( icon );
    icon->SetBitmap(new (ELeave) CFbsBitmap());
    CFbsBitmap* mask = new (ELeave) CFbsBitmap();

    // First try user path and then system path.
    TInt error;
    TFileName path;
    path.Copy(KSsBitmapUserDrive);
    path.Append(KDC_APP_BITMAP_DIR);
    path.Append(KSsBitmapName);

    for (TInt i = 0; i < 2; i++)
        {
        error = icon->Bitmap()->Load(path, 0);
        if (error == KErrNone)
            {
            error = mask->Load(path, 1);
            if (error != KErrNone)
                {
                delete mask;
                }
            else
                {
                icon->SetMask( mask );
                }
            CleanupStack::Pop(); // icon
            return icon;
            }
            path.Copy(KSsBitmapSystemDrive);
            path.Append(KDC_APP_BITMAP_DIR);
            path.Append(KSsBitmapName);
        }

    CleanupStack::PopAndDestroy(); // icon
    return NULL;
    }


// -----------------------------------------------------------------------------
// ScreensaverUtility::StopWServHeartBeat
// -----------------------------------------------------------------------------
//
void ScreensaverUtility::StopWServHeartBeat()
    {
    SCRLOGGER_WRITE("Model: Stopping WServ HB, snoozing...");
    RWsSession ws;
    TInt err = ws.Connect( );
    if ( !err )
        {
        ws.PrepareForSwitchOff( );
        ws.Close( );
        }
    }

// End of file
