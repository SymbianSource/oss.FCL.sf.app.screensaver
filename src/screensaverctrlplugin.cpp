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
* Description:   Implementation of screensaver plugin display object class.
*
*/



#include <AknDef.h>
#include <power_save_display_mode.h>

#include "ScreensaverpluginIntDef.h"
#include "screensaverctrlplugin.h"
#include "screensaverview.h"
#include "screensaverappui.h"
#include "screensaverutility.h"
#include "ScreensaverUtils.h"
#include "screensavershareddatai.h"

// If plugin refresh rate is lower than this threshold, wserv heartbeat
// is stopped between redraws
const TInt KStopWsHbPluginRefreshThreshold = 1000000; // 1 sec

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::NewL
// -----------------------------------------------------------------------------
//
CScreensaverCtrlPlugin* CScreensaverCtrlPlugin::NewL()
    {
    CScreensaverCtrlPlugin* self = new( ELeave ) CScreensaverCtrlPlugin();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::~CScreensaverCtrlPlugin
// -----------------------------------------------------------------------------
//
CScreensaverCtrlPlugin::~CScreensaverCtrlPlugin()
    {
    DeleteTimer( iPluginRefreshTimer );
    DeleteTimer( iPluginTimeoutTimer );
    DeletePlugin();
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::StartTimer
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::StartTimer()
    {
    // Notify plugin that screensaver is starting
    SendPluginEvent( EScreensaverEventStarting );
    
    
    StartPluginRefreshTimer();

    if ( RefreshTimerValue() >= KStopWsHbPluginRefreshThreshold )
        {
        StartCaptureScreenTimer();
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::CancelTimer
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::CancelTimer()
    {
    DeleteTimer( iPluginRefreshTimer );
    DeleteTimer( iPluginTimeoutTimer );
    
    SendPluginEvent( EScreensaverEventStopping );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::DrawObject
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::DrawObject()
    {
    CScreensaverBase::DrawObject();

    if( iPluginFlag.IsSet( EPluginFlagSuspend ) )
    	{
    	Suspend( -1 );
    	iPluginFlag.Clear( EPluginFlagSuspend );
    	}
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::ClearScreen
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::ClearScreen()
    {
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::Refresh
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::Refresh()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlPlugin::Refresh start") );
    // Currently only keylock indicator is updated, because
    // thats the only indicator whose state may change while screensaver
    // is displaying. Other indicators' state changing also dismisses
    // screensaver. Once redisplaying, the indicators are updated anyway.
    // Key lock indicator depends on status of key guard.
    Array().SetDependencyStatus( ESsKeyLockInd, !Model().SharedDataInterface()->IsKeyguardOn() );
    Array().SetVisibilityForIndicators();

    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlPlugin::Refresh DrawObject") );
    // Cause a redraw
    DrawObject();

    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlPlugin::Refresh finish") );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::SendPluginEvent
// -----------------------------------------------------------------------------
//
TInt CScreensaverCtrlPlugin::SendPluginEvent( TScreensaverEvent aEvent )
    {
    if ( iPlugin )
        {
        TRAPD( err, iPlugin->HandleScreensaverEventL( aEvent, NULL ) );
        return err;
        }

    return KErrNone;
    }

// From MScreensaverPluginHost
// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::UseStandardIndicators
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::UseStandardIndicators()
    {
    SCRLOGGER_WRITE("Host: UseStandardIndicators()");

    iPluginFlag.Clear( EPluginFlagOverrideIndicators );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::OverrideStandardIndicators
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::OverrideStandardIndicators()
    {
    SCRLOGGER_WRITE("Host: OverrideStandardIndicators()");

    iPluginFlag.Set( EPluginFlagOverrideIndicators );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::StandardIndicatorsUsed
// -----------------------------------------------------------------------------
//
TBool CScreensaverCtrlPlugin::StandardIndicatorsUsed() const
    {
    SCRLOGGER_WRITE("Host: StandardIndicatorsUsed()");

    return iPluginFlag.IsClear( EPluginFlagOverrideIndicators );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::SetRefreshTimerValue
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::SetRefreshTimerValue( TInt aValue )
    {
    SCRLOGGER_WRITEF( _L("SCR: Host: SetRefreshTimerValue(%d)"), aValue );

    iPluginRefreshRate = aValue;
    iPluginFlag.Clear( EPluginFlagTimerNotUsed );
    StartPluginRefreshTimer();
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::RefreshTimerValue
// -----------------------------------------------------------------------------
//
TInt CScreensaverCtrlPlugin::RefreshTimerValue() const
    {
    SCRLOGGER_WRITE("Host: RefreshTimerValue()");

    return iPluginRefreshRate;
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::GetIndicatorPayload
// -----------------------------------------------------------------------------
//
TInt CScreensaverCtrlPlugin::GetIndicatorPayload( 
    TScreensaverIndicatorIndex aIndex, TIndicatorPayload& aResult ) const
    {
    SCRLOGGER_WRITEF( _L("SCR: Host: GetIndicatorPayload(%d, %x)"),
        aIndex, &aResult );

    return Model().IndicatorArray().GetIndicatorPayload( ( TScreensaverIndicatorId ) aIndex, aResult );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::SetActiveDisplayArea
// -----------------------------------------------------------------------------
//
TInt CScreensaverCtrlPlugin::SetActiveDisplayArea( 
    TRect& aRect, const TScreensaverPartialMode& aMode )
    {
    SCRLOGGER_WRITEF( _L("SCR: Host: SetActiveDisplayArea(<rect>, %d)"), aMode );
    SCRLOGGER_WRITEF( _L("    -> rect: (%d, %d, %d, %d)"),
        aRect.iTl.iX, aRect.iTl.iY, aRect.iBr.iX, aRect.iBr.iY );
    // Make sure everything is in display memory
    ScreensaverUtility::FlushDrawBuffer();

    // Save the active area
    TInt err = SetPowerSaveDisplayActiveArea( aRect );
    if ( err == KErrNone )
        {
        // And activate power save display. Full mode = full colors
//        err = ActivatePowerSaveDisplay( aMode.iType
//            == EPartialModeTypeFull );
        }

    return err;
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::SetActiveDisplayArea
// -----------------------------------------------------------------------------
//
TInt CScreensaverCtrlPlugin::SetActiveDisplayArea( 
    TInt aStartRow, TInt aEndRow, const TScreensaverPartialMode& aMode )
    {
    SCRLOGGER_WRITEF( _L("SCR: Host: SetActiveDisplayArea(%d, %d, %d)"),
        aStartRow, aEndRow, aMode );
    
    TRect psRect( 0, aStartRow, 1, aEndRow);
    return SetActiveDisplayArea( psRect, aMode );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::ExitPartialMode
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::ExitPartialMode()
    {
    SCRLOGGER_WRITE("Host: ExitPartialMode()");

    LcdPartialMode()->Exit();
    // Make sure the partial area is empty
    // Make this less idiotic
    TRect psRect( 0, 0, 0, 0);
    SetPowerSaveDisplayActiveArea( psRect );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::GetColorModel
// -----------------------------------------------------------------------------
//
const TScreensaverColorModel& CScreensaverCtrlPlugin::GetColorModel() const
    {
    SCRLOGGER_WRITE("Host / Own use: GetColorModel()");

    return Model().GetColorModel();
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::Suspend
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::Suspend( TInt aTime )
    {
    SCRLOGGER_WRITEF( _L("SCR: Host: Suspend(%d)"), aTime );

    View()->SetDisplayObject( Model().SharedDataInterface()->DefaultScreensaverType() );

    CScreensaverEngine& model = MUTABLE_CAST( CScreensaverEngine&, Model() );
    model.SetExpiryTimerTimeout( KDefaultScreenSaverTimeout );

    View()->ShowDisplayObject();
    
    if ( aTime >= 0 )
        {
        model.StartSuspendTimer( aTime );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::RequestLights
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::RequestLights( TInt aSecs )
    {
    SCRLOGGER_WRITEF( _L("SCR: Host: RequestLights(%d)"), aSecs );

    if ( aSecs <= 0 )
        {
        // Turn lights off, kill lights timer
        Model().SharedDataInterface()->SetSSForcedLightsOn( 0 );
        }
    else
        {
        // Make sure nobody tries to overextend our hospitality
        TInt secs = (aSecs > KMaxLightsOnTime) ? KMaxLightsOnTime : aSecs;

        // Turn lights on, start lights timer
        Model().SharedDataInterface()->SetSSForcedLightsOn( secs );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::DisplayInfo
// -----------------------------------------------------------------------------
//
TInt CScreensaverCtrlPlugin::DisplayInfo( TScreensaverDisplayInfo* aInfo )
    {
    SCRLOGGER_WRITEF( _L("SCR: Host: DisplayInfo(%x)"), aInfo );

    if ( !aInfo )
        {
        return KErrArgument;
        }
    // Sanity check: the indicated size of the info struct should be
    // same or less than the actual size (allows for extensibility)
    if ( aInfo->iSize > sizeof( TScreensaverDisplayInfo ) )
        {
        ASSERT( EFalse );
        return KErrArgument;
        }

    // Fill our own perception of the info structure
    TScreensaverDisplayInfo info;

    info.iSize = aInfo->iSize;

    // Currently whole screen
    info.iRect = CCoeEnv::Static()->ScreenDevice()->SizeInPixels();
    info.iParent = this;

    // Copy only the size of the caller struct
    Mem::Copy( aInfo, &info, aInfo->iSize );

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::UseRefreshTimer
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::UseRefreshTimer( TBool aOn )
    {
    SCRLOGGER_WRITEF( _L("SCR: Host: UseRefreshTimer(%d)"), aOn );

    if ( aOn )
        {
        // Use normal timer, plugin timer allowed
        iPluginFlag.Clear( EPluginFlagTimerNotUsed );
        }
    else
        {
        // Plugin does not want Draw() calls, let timer tick the usual way
        iPluginFlag.Set( EPluginFlagTimerNotUsed );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::RequestTimeout
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::RequestTimeout( TInt aSecs )
    {
    CScreensaverEngine& model = MUTABLE_CAST( CScreensaverEngine&, Model() );
    model.SetExpiryTimerTimeout( 0 );
    StartPluginTimeoutTimer( aSecs );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::RevertToDefaultSaver
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::RevertToDefaultSaver()
    {
    SCRLOGGER_WRITE("Host: RevertToDefaultSaver()");

    Model().SharedDataInterface()->SetDisplayObjectType( 
        Model().SharedDataInterface()->DefaultScreensaverType() );
    }

// --- end MScreensaverPluginHost ---

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::CScreensaverCtrlPlugin
// -----------------------------------------------------------------------------
//
CScreensaverCtrlPlugin::CScreensaverCtrlPlugin()
    :iPluginFlag()
    {
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::ConstructL()
    {
    iPluginFlag.ClearAll();

    
    CreateWindowL();
    
    SetRect( iCoeEnv->ScreenDevice()->SizeInPixels() );
    ConstructAndConnectLCDL();
    LoadPluginL( this );
    ActivateL();
    
    // Notify plugin that display control has changed
    SendPluginEvent( EScreensaverEventDisplayChanged );
    
    if( Model().ScreenSaverIsPreviewing() )
        {
        SendPluginEvent( EScreensaverEventPreview );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::HandleResourceChange
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::HandleResourceChange( TInt aType )
    {
    if ( aType == KEikDynamicLayoutVariantSwitch )
        {
        // Screen layout has changed - resize
        SetRect( iCoeEnv->ScreenDevice()->SizeInPixels() );
        // Notify plugin that the display has changed
        SendPluginEvent( EScreensaverEventDisplayChanged );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::SizeChanged
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::SizeChanged()
    {
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::Draw
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::Draw( const TRect& /*aRect*/ ) const
    {
    
    if ( !Model().ScreenSaverIsOn() && !Model().ScreenSaverIsPreviewing() )
        {
        return;
        }

    // Graphics context to draw on.
    CWindowGc& gc = SystemGc();
    
    // Fix for error ESMG-74Y4PE - S60 3.2 wk26, Power Saver: 
    // Flickering when power saver is deactivated.
    // We now clear the screen with a black brush so the screensaver 
    // background is changed to black. There will no longer be a white
    // intermediate screen and this will reduce the "flicker" effect.
    gc.SetBrushColor( KRgbBlack );

    // Start with a clear screen
    // If there is no plugin module, indicator view overrides plugin module or
    // plugin drawing is suspended then the standard screensaver bar is shown,
    // let's draw it.

        // Let plugin module handle the drawing, unless not requested
    
    TInt err = KErrNone;
    if ( iPluginFlag.IsClear( EPluginFlagTimerNotUsed ) )
        {
        err = iPlugin->Draw( gc );
        }
    //Notice:add this code to shield the issue ELWG-7SF3R3.
    //Prevent screensaver plugin from being called unexpected draw function,
    //which would cause chosen images are not displayed.
    //Check the err code return by iPlugin->Draw:
    //If draw action is correct and iPluginFlag has already been set EPluginFlagSuspend,
    //then clear this EPluginFlagSuspend
    if ( KErrNone == err && iPluginFlag.IsSet( EPluginFlagSuspend ) )
        {
        iPluginFlag.Clear( EPluginFlagSuspend );
        }

    if( err != KErrNone )
        {
        iPluginFlag.Set( EPluginFlagSuspend );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::LoadPluginL
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::LoadPluginL(  MScreensaverPluginHost* /*aPluginHost*/ )
    {
    DeletePlugin();
    LoadPluginModuleL();
    User::LeaveIfNull( iPlugin );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::LoadPluginModule
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::LoadPluginModuleL()
    {
    TFileName pluginName;
        
    Model().SharedDataInterface()->GetPluginName( pluginName );
    
    // Create plugin object in the DLL
    // Convert the UID of the given screensaver plugin from text to integer
    // The string format of the UID: [12345678]
    // The number inside the brackets in hexadecimal format
    TLex lex( pluginName );
    
    // Skip the first character: '['
    lex.Get();
    
    TUint screenSaverPluginImpUid;
    
    // Get the UID
    TInt err = lex.Val( screenSaverPluginImpUid, EHex );
    
    // Bail out, if the UID is not parseable
    if ( err != KErrNone )
        {
        iPlugin = NULL;
        }
    //codescanner will crib if leaving function inside trap is called
    //after line break within the macro. Hence the following trap call
    //is made in a single line
    TRAP(err, iPlugin = STATIC_CAST( MScreensaverPlugin*, 
        CScreensaverPluginInterfaceDefinition::NewL( 
            TUid::Uid( screenSaverPluginImpUid ) ) ) );
    
    if( err != KErrNone )
        return;
    
    TRAP( err, err = iPlugin->InitializeL( this ) );
    
    if( err != KErrNone )
        {
        // Loaded OK, but failed to initialize - cannot use plugin
        delete iPlugin;
        iPlugin = NULL;
        }
    
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::DeletePlugin
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::DeletePlugin()
    {
    if( iPlugin )
        {
        delete iPlugin;
        iPlugin = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::StartPluginRefreshTimer
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::StartPluginRefreshTimer()
    {
    DeleteTimer( iPluginRefreshTimer );

    if( ( iPluginRefreshRate != 0 ) )
        {
        TRAP_IGNORE( iPluginRefreshTimer = CPeriodic::NewL( CActive::EPriorityStandard ) );
        
        iPluginRefreshTimer->Start( iPluginRefreshRate, iPluginRefreshRate,
            TCallBack( HandleRefreshTimerExpiry, this ) );
        SCRLOGGER_WRITEF( _L("SCR: iRefreshTimer->Start(%d, %d, HandleRefreshTimerExpiry)"),
            iPluginRefreshRate,iPluginRefreshRate );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::StartPluginTimeoutTimer
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlPlugin::StartPluginTimeoutTimer( TInt aSecs )
    {
    // Cancel pending timeouts
    DeleteTimer( iPluginTimeoutTimer );

    TRAP_IGNORE( iPluginTimeoutTimer = CPeriodic::NewL( CActive::EPriorityStandard ) );
    
    // Nothing more to do?
    if( ( aSecs <= 0 ) || ( aSecs > ( 35 * 60 ) ) ) // 35 mins max
        {
        return;
        }

    TInt timeOut = aSecs * 1000000; // uSecs

    iPluginTimeoutTimer->Start( timeOut, timeOut, TCallBack(
        HandlePluginTimeoutTimerExpiry, this ) );
    SCRLOGGER_WRITEF( _L("SCR: iPluginTimeoutTimer->Start(%d, %d, HandlePluginTimeoutTimerTimeout)"),
        timeOut, timeOut );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlPlugin::HandlePluginTimeoutTimerExpiry
// -----------------------------------------------------------------------------
//
TInt CScreensaverCtrlPlugin::HandlePluginTimeoutTimerExpiry( TAny* aPtr )
    {
    CScreensaverCtrlPlugin *plugin= STATIC_CAST( CScreensaverCtrlPlugin*, aPtr );
    SCRLOGGER_WRITEF( _L("SCR: Inside CScreensaverView::HandlePluginTimeoutTimerTimeout()") );
    if ( plugin )
        {
        plugin->DeleteTimer( plugin->iPluginRefreshTimer );
        plugin->DeleteTimer( plugin->iPluginTimeoutTimer );
        plugin->SendPluginEvent( EScreensaverEventTimeout );
        }

    return KErrNone;
    }
//End of file
