/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   The control base class of display object.
*
*/



#include <coecntrl.h>
#include <power_save_display_mode.h>

#include "screensaverbase.h"
#include "screensaverview.h"
#include "screensaverappui.h"
#include "ScreensaverUtils.h"
#include "screensavershareddatai.h"
#include "screensaverutility.h"

const TInt KRefreshRate = 60000000; // 60 sec


// Time delayed before a wserv heartbeat stop request is issued.
// Delay is needed, because draw commands may still be handled, if
// stop request is issued right after screen refresh.
#ifdef RD_UI_TRANSITION_EFFECTS_PHASE2
const TInt KCaptureDelay  = 2000000; // 2.0 sec, used to delay the capturing operation
#else
const TInt KCaptureDelay  = 100000; // 0.1 sec
#endif

// -----------------------------------------------------------------------------
// CScreensaverBase::CScreensaverBase
// -----------------------------------------------------------------------------
//
CScreensaverBase::CScreensaverBase()
    {
    
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::~CScreensaverBase
// -----------------------------------------------------------------------------
//
CScreensaverBase::~CScreensaverBase()
    {
    DeleteTimer( iCaptureScreenTimer );

    delete iPowerSaveDisplayMode;

    delete iOffScrnBmp;
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::StartDrawObject
// -----------------------------------------------------------------------------
//
void CScreensaverBase::StartDrawObject()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverBase::StartDrawObject start") );
    StartTimer();
    
    DrawObject();
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::StopDrawObject
// -----------------------------------------------------------------------------
//
void CScreensaverBase::StopDrawObject()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverBase::StopDrawObject start") );
    if ( LcdPartialMode() && LcdPartialMode()->Status() )
        {
        SCRLOGGER_WRITE("Model: StopScreenSaver(), exit power save");
        LcdPartialMode()->Exit();
        }
    
    CancelTimer();
    
    ClearScreen();
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::DrawObject
// -----------------------------------------------------------------------------
//
void CScreensaverBase::DrawObject()
    {    
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverBase::DrawObject start") );
    SetRect( Rect() );
    
    DrawNow();
    
    StartCaptureScreenTimer();
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::View
// -----------------------------------------------------------------------------
//
CScreensaverView* CScreensaverBase::View() const
    {
    return STATIC_CAST( CScreensaverAppUi*, CCoeEnv::Static()->AppUi() )->ScreensaverView();
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::Model
// -----------------------------------------------------------------------------
//
const CScreensaverEngine& CScreensaverBase::Model() const
    { 
    return STATIC_CAST( CScreensaverAppUi*, CCoeEnv::Static()->AppUi() )->Model(); 
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::Array
// -----------------------------------------------------------------------------
//
CScreensaverIndicatorArray& CScreensaverBase::Array() const 
    { 
    return Model().IndicatorArray(); 
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::SetPowerSaveDisplayActiveArea
// -----------------------------------------------------------------------------
//
TInt CScreensaverBase::SetPowerSaveDisplayActiveArea( TRect& aRect )
    {
    // Save the requested area
    iPowerSaveDisplayActiveArea = aRect;
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::ActivatePowerSaveDisplay
// -----------------------------------------------------------------------------
//
TInt CScreensaverBase::ActivatePowerSaveDisplay( TBool aFullColors ) const
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverBase::ActivatePowerSaveDisplay start") );
    TInt err = KErrNone;

    if ( ( Model().ScreenSaverIsOn()
        && ( !iPowerSaveDisplayActiveArea.IsEmpty() ) ) )
        {
        TInt startLine = 0;
        TInt endLine = 0;

        // Normal mode - using saver bar height
        startLine = iPowerSaveDisplayActiveArea.iTl.iY;
        endLine = iPowerSaveDisplayActiveArea.iBr.iY;

        SCRLOGGER_WRITEF( _L("SCR: Powersave supports %d lines"),
            LcdPartialMode()->MaxLines() );

        // Check whether power save mode should be used
        // When local variation added, PS will override if local says
        // yes, but PS says no
        if ( Model().SharedDataInterface()->IsUsePowerSaveMode() )
            {
            SCRLOGGER_WRITEF( _L("SCR: Requesting powersave, lines: %d, %d"),
                startLine, endLine );
            TUint16* addr = ( TUint16* )iOffScrnBmp->DataAddress();
            err = LcdPartialMode()->Set( startLine, endLine, addr, aFullColors );

            Model().SharedDataInterface()->SetSSForcedLightsOn( 0 );
            }
        else
            {
            SCRLOGGER_WRITE("Power save mode not in use, not requested");
            err = KErrCancel;
            }
        }
    else
        {
        // Screensaver not on - cannot set power save mode on
        err = KErrNotReady;
        }

    return err;
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::DrawPictographArea
// -----------------------------------------------------------------------------
//
void CScreensaverBase::DrawPictographArea()
    {
    // Currently animation is not supported - do nothing
    }

// ----------------------------------------------------------------------------
// CScreensaverBase::StartCaptureScreenTimer
// -----------------------------------------------------------------------------
//
void CScreensaverBase::StartCaptureScreenTimer()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverBase::StartCaptureScreenTimer start") );
    DeleteTimer( iCaptureScreenTimer );
    
    TRAP_IGNORE( iCaptureScreenTimer = CPeriodic::NewL( CActive::EPriorityStandard ) );
    
    iCaptureScreenTimer->Start( KCaptureDelay , KCaptureDelay ,
        TCallBack( HandleCaptureTimerExpiry, this ) );
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::InitialRefreshRate
// -----------------------------------------------------------------------------
//
TInt CScreensaverBase::InitialRefreshRate()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverBase::InitialRefreshRate start") );
    TTime homeTime, timeTmp;
    TTimeIntervalSeconds interval;
    TInt initialRefresh = KRefreshRate;
    homeTime.HomeTime();
    timeTmp = homeTime;
    timeTmp.RoundUpToNextMinute();
    if ( timeTmp.SecondsFrom( homeTime, interval ) == KErrNone )
        {
        initialRefresh = interval.Int() * 1000000;
        }
    
    return initialRefresh;
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::DeleteTimer
// -----------------------------------------------------------------------------
//
void CScreensaverBase::DeleteTimer( CPeriodic*& aTimer )
    {
    if ( aTimer )
        {
        aTimer->Cancel();
        delete aTimer;
        aTimer = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::ConstructAndConnectLCDL
// -----------------------------------------------------------------------------
//
void CScreensaverBase::ConstructAndConnectLCDL()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverBase::ConstructAndConnectLCD start") );
    // User side handle object is created
    TRAPD( err, iPowerSaveDisplayMode = CPowerSaveDisplayMode::NewL() );
    if ( ( err != KErrNone ) || ( !iPowerSaveDisplayMode ) )
        {
        SCRLOGGER_WRITEF( _L("SCR: Model: No power save mode (err = %d)"), err );
        }

    iOffScrnBmp = new ( ELeave ) CFbsBitmap;
    User::LeaveIfError( iOffScrnBmp->Create( Size(), EColor64K ) );
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::LcdPartialMode
// -----------------------------------------------------------------------------
//
CPowerSaveDisplayMode* CScreensaverBase::LcdPartialMode() const
    {
    return iPowerSaveDisplayMode;
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::HandleRefreshTimerExpiry
// -----------------------------------------------------------------------------
//
TInt CScreensaverBase::HandleRefreshTimerExpiry( TAny* aPtr )
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverBase::HandleRefreshTimerExpiry start") );
    CScreensaverBase* control = STATIC_CAST(CScreensaverBase*, aPtr);
    
    SCRLOGGER_WRITEF( _L("SCR: Inside CScreensaverView::HandleRefreshTimerExpiry()") );
    
    control->DrawObject();
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::HandleCaptureTimerExpiry
// Handles capture screen timer timeout
// -----------------------------------------------------------------------------
//
TInt CScreensaverBase::HandleCaptureTimerExpiry( TAny* aPtr )
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverBase::HandleCaptureTimerExpiry start") );
    CScreensaverBase *control= STATIC_CAST( CScreensaverBase*, aPtr );
    
    SCRLOGGER_WRITEF( _L("SCR: Inside CScreensaverView::HandleWsHbStopTimerTimeout()") );
    
    if ( control )
        {
        TInt err = control->CaptureScreen();
        control->DeleteTimer( control->iCaptureScreenTimer );
        ScreensaverUtility::StopWServHeartBeat();
        
#ifdef RD_UI_TRANSITION_EFFECTS_PHASE2

        if ( KErrNone == err )
            {
            control->ActivatePowerSaveDisplay();
            }

#endif
        }
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScreensaverBase::CaptureScreen
// -----------------------------------------------------------------------------
//
TInt CScreensaverBase::CaptureScreen()
    {
    CWsScreenDevice* device = iCoeEnv->ScreenDevice();
    TInt err = device->CopyScreenToBitmap( iOffScrnBmp, iPowerSaveDisplayActiveArea );
    return err;
    }

//End of file
