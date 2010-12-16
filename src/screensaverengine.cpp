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
* Description:   Screensaver engine class definition.
*
*/



#include <barsread.h>
#include <featmgr.h>
#include <PUAcodes.hrh>
#include <Profile.hrh>
#include <screensaver.rsg>
#include <AknUtils.h>

#include "screensaverengine.h"
#include "screensaverctrlmovingtext.h"
#include "screensavershareddatai.h"
#include "screensavershareddatamonitor.h"
#include "screensaverappui.h"
#include "ScreensaverUtils.h"
#include "screensaverutility.h"
#include "screensaveractivitymanager.h"

// Minimum plugin suspension time
const TInt KMinPluginSuspensionTime = 500000; // 0.5 sec

// Inactivity timeout in seconds when keys locked
const TInt KTimeoutShort = 5000000;
const TInt KTimeoutPreviewLocked = 2000000;

const TInt KNoPreview = 0;

const TInt KPreviewTimeout = 10000000; // 10 sec

const TText KSilentProfileInd= KPuaCodeSilentSymbol;
const TText KSilentVibraInd= KPuaCodeAprofSilentVibra;
const TText KSoundVibraInd= KPuaCodeAprofSoundVibra;
const TText KSoundInd= KPuaCodeAprofSound;

// Class CScreensaverEngine

// -----------------------------------------------------------------------------
// CScreensaverEngine::NewL
// -----------------------------------------------------------------------------
//
CScreensaverEngine* CScreensaverEngine::NewL()
    {
    CScreensaverEngine* self = new (ELeave) CScreensaverEngine();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(); //self
    return self;
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::~CScreensaverEngine
// -----------------------------------------------------------------------------
//
CScreensaverEngine::~CScreensaverEngine( )
    {
    iAsyncCb.Cancel();
    StopActivityMonitoring( iActivityManagerScreensaver );
    DisableSharedDataAndMonitor();
    delete iIndicatorArray;
    KillTimer( iPreviewTimer );
    KillTimer( iExpiryTimer );
    iAknUiServer.Close();
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::SharedDataInterface
// -----------------------------------------------------------------------------
//
CScreensaverSharedDataI* CScreensaverEngine::SharedDataInterface( ) const
    {
    return iSharedDataI;
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::ResetInactivityTimeout
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::ResetInactivityTimeout()
    {
    if ( iActivityManagerScreensaver )
        {
        SCRLOGGER_WRITEF(_L("SCR: Model: Setting inac timeout to %d secs"),
            Timeout());
        iActivityManagerScreensaver->SetInactivityTimeout( Timeout( ) );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::ScreenSaverIsOn
// -----------------------------------------------------------------------------
//
TBool CScreensaverEngine::ScreenSaverIsOn( ) const
    {
    return iScreenSaverIsOn;
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::ScreenSaverIsPreviewing
// -----------------------------------------------------------------------------
//
TBool CScreensaverEngine::ScreenSaverIsPreviewing( ) const
    {
    return iScreenSaverIsPreviewing;
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::StartScreenSaver
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::StartScreenSaver( )
    {
    SCRLOGGER_WRITE("Model: StartScreenSaver()");

    // Stop if previewing. If keylock was activated, the pause timer will
    // be started and the selected (not necessarily previewed) screensaver
    // will start
    if ( iScreenSaverIsPreviewing )
        {
        StopScreenSaver();
        return;
        }

    // if (DisplayFlag() == 1) then screensaver can be activated
    // else screensaver can't come during phone call.
    // Also not OK to start until system startup has been completed
    if ( ( !DisplayFlag() ) && ( iSharedDataI->IsSystemStartupComplete() ) )
        {
        if ( !iScreenSaverIsOn )
            {
            // Activating is done asynchronously to prevent screensaver from
            // flashing quickly in some cases. This flashing happens e.g. when
            // a call is missed and a note is showed about it.
            iAsyncCb.CallBack();
            }
        else
            {
            SCRLOGGER_WRITE("Model: StartScreenSaver(): already on, ignore");
            }
        }
    else
        {
        SCRLOGGER_WRITE("Model: SS not displaying, not allowed.");
        ResetInactivityTimeout( );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::StopScreenSaver
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::StopScreenSaver( TBool aStoppedByKeylockOff )
    {
    SCRLOGGER_WRITE("Stopping Screensaver");
    iAsyncCb.Cancel();

    if ( !aStoppedByKeylockOff )
        {
        StartPauseTimer();
        }

    if( iScreenSaverIsOn )
        {
        // disconnect in StopScreenSaver, It makes sure other applications could  
        // receive application key event when screensaver is disactivated.
        iAknUiServer.ConnectAndSendAppsKeySuppress(EFalse);
        // Indicate we've stopped
        iSharedDataI->SetScreensaverStatus( EFalse );

        iScreenSaverIsOn = EFalse ;

        KillTimer( iExpiryTimer );

        View()->HideDisplayObject();
        }

    if( iScreenSaverIsPreviewing )
        {
        iSharedDataI->SetScreensaverPreviewMode( KNoPreview );

        iScreenSaverIsPreviewing = EFalse;

        View()->HideDisplayObject();

        // Update preview progress
        iSharedDataI->SetScreensaverPreviewState( EScreenSaverPreviewEnd );
        iSharedDataI->SetScreensaverPreviewState( EScreenSaverPreviewNone );

        KillTimer( iPreviewTimer );
        ResetInactivityTimeout();
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::StartPreviewModeL
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::StartPreviewModeL( )
    {
    SCRLOGGER_WRITEF(_L("SCR: Inside CScreensaverEngine::StartPreviewModeL()") );
    
    iScreenSaverIsPreviewing = ETrue;
    // Change the display object into what's being previewed
    View()->CreatePreviewDisplayObjectL();

    if( iSharedDataI->ScreensaverPreviewState() == EScreenSaverPreviewError )
    	{
    	iScreenSaverIsPreviewing = EFalse;
        return;
    	}

    StartPreviewTimer();
    
    UpdateIndicatorAttributes();
    
    if ( !View()->IsContentlessScreensaver() )
         {
         ScreensaverUtility::BringToForeground();
         }
    
    View()->ShowDisplayObject();
    
    iSharedDataI->SetScreensaverPreviewState( EScreenSaverPreviewStart );

    iActivityManagerScreensaver->SetInactivityTimeout(0);
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::DisplayObject
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::DisplayObject()
    {
    SCRLOGGER_WRITEF(_L("SCR: Inside CScreensaverEngine::DisplayObject()") );
    //if the display object type is plugin, we should recreate the object
    //cause when plugin is suspend status, it will be changed to the default
    //type
    if ( iSharedDataI->DisplayObjectType() == EDisplayPlugin )
        {
        View()->SetDisplayObject( EDisplayPlugin );
        SetExpiryTimerTimeout( KDefaultScreenSaverTimeout );
        }

    UpdateIndicatorAttributes();

    View()->ShowDisplayObject();

    iSharedDataI->SetScreensaverStatus( ETrue );
    }

// ----------------------------------------------------------------------------
// CScreensaverEngine::StartSuspendTimer
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::StartSuspendTimer( TInt aSec )
    {
    KillTimer( iSuspendTimer );
    
    // Let's keep this operation reasonable.
    TInt realTime = ( aSec > KMinPluginSuspensionTime ) 
        ? aSec : KMinPluginSuspensionTime;

    TRAP_IGNORE( iSuspendTimer = CPeriodic::NewL( CActive::EPriorityStandard ) );
    
    iSuspendTimer->Start( realTime, realTime, TCallBack(
        HandleSuspendTimerExpiry, this ) );
    
    SCRLOGGER_WRITEF(_L("SCR: iSuspendTimer->Start(%d, %d, HandleSuspensionTimerTimeout)"),
        realTime, realTime);
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::IndicatorArray
// -----------------------------------------------------------------------------
//
CScreensaverIndicatorArray& CScreensaverEngine::IndicatorArray() const
    {
    return *iIndicatorArray;
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::GetColorModel
// -----------------------------------------------------------------------------
//
const TScreensaverColorModel& CScreensaverEngine::GetColorModel() const
    {
    SCRLOGGER_WRITE("Host / Own use: GetColorModel()");

    return iColorModel;
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::AddPartialModeLevel
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::AddPartialModeLevel( TInt aBpp,
    TScreensaverPartialModeType aType )
    {
    TInt i;
    for (i = 0; i < iColorModel.iNumberOfPartialModes; i++ )
        {
        if ( (iColorModel.iPartialModes[i].iBpp == aBpp)
            && (iColorModel.iPartialModes[i].iType == aType) )
            {
            break;
            }
        }
    if ( i >= iColorModel.iNumberOfPartialModes )
        {
        iColorModel.iPartialModes[iColorModel.iNumberOfPartialModes].iBpp
            = aBpp;
        iColorModel.iPartialModes[iColorModel.iNumberOfPartialModes].iType
            = aType;
        iColorModel.iNumberOfPartialModes++;
        }
    }


// -----------------------------------------------------------------------------
// CScreensaverEngine::UpdateIndicatorAttributes
//  This method is for updating screensaver indicator
//  attributes (payloads and dependencies). This is where indicator
//  payload values are connected to real values.
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::UpdateIndicatorAttributes()
    {
    SCRLOGGER_WRITEF(_L("SCR: Inside CScreensaverView::UpdateIndicatorAttributes()"));

    TIndicatorPayload payload;

    // First integer types.
    payload.iType = EPayloadTypeInteger;

    // Missed calls.
    payload.iInteger = iSharedDataI->NewMissedCalls();
    iIndicatorArray->SetIndicatorPayload( ESsMissedCallsInd, payload );
    // Missed calls indicator depends on status of new contacts note.
    iIndicatorArray->SetDependencyStatus( ESsMissedCallsInd,
        !iSharedDataI->IsNCNActive() );

    // Unread messages
    payload.iInteger = iSharedDataI->UnreadMessagesNumber();
    iIndicatorArray->SetIndicatorPayload( ESsNewMessagesInd, payload );
    iIndicatorArray->SetDependencyStatus( ESsNewMessagesInd, (payload.iInteger > 0
        ? EFalse : ETrue) );

    // Then string types.
    payload.iType = EPayloadTypeText;

    // Profile information
    TBool nameDisplayed = (iSharedDataI->ActiveProfile() == EProfileGeneralId) ? EFalse : ETrue;
    TBool isSilent = iSharedDataI->IsSilentMode();
    payload.iText.Zero( );

    if ( nameDisplayed || isSilent || iShowExtendedProfileInd )
        {
        // append alert type indicator
        if ( iShowExtendedProfileInd )
            {
            TBool isVibra = iSharedDataI->IsVibraMode( );
            if ( isSilent )
                {
                payload.iText.Append( isVibra ? KSilentVibraInd
                    : KSilentProfileInd );
                }
            else
                {
                payload.iText.Append( isVibra ? KSoundVibraInd : KSoundInd );
                }
            }
        else
            {
            // Check if this is silent mode and we have to add indicator
            // character to string.
            if ( isSilent )
                {
                payload.iText.Append( KSilentProfileInd );
                }
            }

        if ( nameDisplayed )
            {
            const TDesC& profile = iSharedDataI->ProfileName( );
            // Don't panic if profile name is too long, 'less than' ensures
            // the 1 free space used for silent/vibra
            if ( profile.Length( ) < KMaxPayloadTextLength )
                {
                // append profile name
                payload.iText.Append( profile );
                }
            }

        // Profile indicator shown
        iIndicatorArray->SetDependencyStatus( ESsProfileInd, EFalse );
        }
    else
        {
        // Profile indicator not shown
        iIndicatorArray->SetDependencyStatus( ESsProfileInd, ETrue );
        }

    iIndicatorArray->SetIndicatorPayload( ESsProfileInd, payload );

    // AM/PM indicator is text but it is updated with every draw
    // in screensaver control, so that it changes properly even if
    // time rolls over between draws. Note that Am/Pm information is
    // maintained by screensaver display object, not shareddatainterface.
    if ( AknLayoutUtils::ScalableLayoutInterfaceAvailable() )
        {
        // Am/Pm indicator depends on 12-hour clock
        iIndicatorArray->SetDependencyStatus( ESsAmPmInd, !ScreensaverUtility::Query12HourClock( ) );
        }
    else
        {
        // Make sure it's not shown
        iIndicatorArray->SetDependencyStatus( ESsAmPmInd, ETrue );
        }


    // Key lock indicator depends on status of key guard.
    iIndicatorArray->SetDependencyStatus( ESsKeyLockInd, !iSharedDataI->IsKeyguardOn() );

    // Instant messaging indicator depends on existence of unseen IMs
    iIndicatorArray->SetDependencyStatus( ESsImInd, !iSharedDataI->IsHaveNewInstantMessages() );

    // Mail indicator depends on existence of unseen mails
    iIndicatorArray->SetDependencyStatus( ESsMailInd, !iSharedDataI->IsHaveNewEmail() );

    // Voicemail indicator depends on existence of unseen voicemail messages
    iIndicatorArray->SetDependencyStatus( ESsVoicemailInd, !iSharedDataI->IsHaveNewVoicemail() );

    //Set visibility for indicators if selected screensaver is a plugin 
    //which overides standard indicators
    //TODO: Convert the iControlType to the iSettingType 
//    if ( (iSharedDataI->ScreensaverObjectType() == EDisplayPlugin ) && 
//        ( AppUi()->ScreensaverView()->PluginFlags() & CScreensaverView::EPluginFlagOverrideIndicators ) )
//        {
        iIndicatorArray->SetVisibilityForIndicators();
//        }
    }


// -----------------------------------------------------------------------------
// Start screensaver expiry timer. When the timer expires the screensaver will
// be stopped and hided.
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::SetExpiryTimerTimeout( TInt aTimeout )
    {
    SCRLOGGER_WRITEF( _L( "SCR:CScreensaverEngine::SetExpiryTimerTimeout(%d) start" ), aTimeout );
    KillTimer( iExpiryTimer );

    if ( 0 < aTimeout )
        {
        TRAP_IGNORE(
            iExpiryTimer = CPeriodic::NewL( CActive::EPriorityStandard );
            iExpiryTimer->Start( aTimeout , aTimeout ,
                TCallBack( HandleExpiryTimerExpiry, this ) );
            )
        }
    }


// -----------------------------------------------------------------------------
// CScreensaverEngine::CScreensaverEngine
// -----------------------------------------------------------------------------
//
CScreensaverEngine::CScreensaverEngine() : iAsyncCb( CActive::EPriorityLow )
    {
    TCallBack callbackFunc( StartSaverCb, this );
    iAsyncCb.Set( callbackFunc );
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::ConstructL( )
    {
    SetupColorModelL();

    EnableSharedDataAndMonitorL();
    
    iIndicatorArray = CScreensaverIndicatorArray::NewL( iSharedDataI->IsInvertedColors() );
    
    StartActivityMonitoringL();
    
    FeatureManager::InitializeLibL();
    iShowExtendedProfileInd
        = FeatureManager::FeatureSupported( KFeatureIdProtocolCdma );
    FeatureManager::UnInitializeLib();

    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::EnableSharedDataAndMonitorL
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::EnableSharedDataAndMonitorL()
    {
    iSharedDataI = CScreensaverSharedDataI::NewL();
    
    iSharedDataMonitor = CScreensaverSharedDataMonitor::NewL( iSharedDataI );
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::DisableSharedDataAndMonitor
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::DisableSharedDataAndMonitor( )
    {
    delete iSharedDataMonitor;
    iSharedDataMonitor = NULL;
    
    delete iSharedDataI;
    iSharedDataI = NULL;
    
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::StartActivityMonitoringL
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::StartActivityMonitoringL( )
    {
    // Actually screensaver is the last exe to be launch at the startup,
    // so when it is being launched the elapsed time(User::InactivityTime()) 
    // is greater than the interval (15 sec) which screensaver pass to start 
    // activitymanager for inactivitycallback.
    // So it has to do to reset inactivitytime (User::ResetInactivityTime() ) 
    // once in the screensaver constructor so that the inactivity callback 
    // called after the interval time expired.
    User::ResetInactivityTime();

    // Start monitoring activity for screensaver
    iActivityManagerScreensaver
        = CScreensaverActivityManager::NewL( CActive::EPriorityUserInput );
        iActivityManagerScreensaver->Start( Timeout(), 
            TCallBack( HandleInactiveEventL,this ),
            TCallBack( HandleActiveEventL, this ) );
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::StopActivityMonitoring
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::StopActivityMonitoring( CScreensaverActivityManager*& aActivityManager )
    {
    if ( aActivityManager )
        {
        aActivityManager->Cancel();
        delete aActivityManager;
        aActivityManager = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::SetupColorModelL
// THIS IS TEMPORARY VERSION OF SetupColorModelL function. It is used until
// new version of LcdPartialMode module is integrated. 
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::SetupColorModelL()
    {
    SCRLOGGER_WRITEF(_L("SCR: Inside CScreensaverEngine::SetupColorModelL()") );
    TResourceReader reader;
    TInt i, ctmp;

    TInt suggestedDrawingMode;

    CEikonEnv::Static()->CreateResourceReaderLC( reader, R_SCREENSAVER_COLOR_MODEL );
    suggestedDrawingMode = reader.ReadInt32( );

    //
    // Let's find out how many partial modes hardware
    // actually supports.
    //
    AddPartialModeLevel( (TInt)3,
        (TScreensaverPartialModeType)EPartialModeTypeMostPowerSaving );
    AddPartialModeLevel( (TInt)3,
        (TScreensaverPartialModeType)EPartialModeTypeDefault );
    AddPartialModeLevel( (TInt)16,
        (TScreensaverPartialModeType)EPartialModeTypeFull );

    if ( suggestedDrawingMode == ESsColorModel3BitMode )
        {
        // This can always be done, just set default partial mode to
        // most power saving. First try 'most power saving' and if not supported
        // then try default. if default isn't supported leave with error code.
        iColorModel.iSystemPartialMode.iBpp = 3;
        iColorModel.iSystemPartialMode.iType
            = EPartialModeTypeMostPowerSaving;
        }

    // Read color indices for 3-bit mode.
    reader.Read( iColorModel.iColors, 8 * sizeof(TInt16) );

    // Read rgb values for gradient effect.
    for (i = 0; i < 6; i++ )
        {
        ctmp = reader.ReadInt32( );
        iColorModel.iDarkGradient[i] = TRgb( ctmp );
        }

    for (i = 0; i < 6; i++ )
        {
        ctmp = reader.ReadInt32( );
        iColorModel.iLightGradient[i] = TRgb( ctmp );
        }

    CleanupStack::PopAndDestroy( ); // reader
    }

// ----------------------------------------------------------------------------
// CScreensaverEngine::StartPreviewTimer
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::StartPreviewTimer()
    {
    SCRLOGGER_WRITEF(_L("SCR: Inside CScreensaverEngine::StartPreviewTimer()") );
    
    KillTimer( iPreviewTimer );
    
    TRAP_IGNORE( iPreviewTimer = CPeriodic::NewL( CActive::EPriorityHigh ) );
    
    iPreviewTimer->Start( KPreviewTimeout, KPreviewTimeout, TCallBack(
        HandlePreviewTimerExpiry, this ) );
    
    SCRLOGGER_WRITEF(_L("SCR: iPreviewTimer->Start(%d, %d, HandlePreviewTimerTimeout)"),
        KPreviewTimeout,KPreviewTimeout);
    }

// ----------------------------------------------------------------------------
// CScreensaverEngine::StartPauseTimer
// -----------------------------------------------------------------------------
//
void CScreensaverEngine::StartPauseTimer()
    {
    if ( !iSharedDataI->IsKeyguardOn() )
        {
        iPauseTimerStartedWithKeylockOff = ETrue;
        }
  
    TInt timeout = ( iScreenSaverIsPreviewing )? KTimeoutPreviewLocked :
                                                 KTimeoutShort;
    
    KillTimer( iPauseTimer );
    
    TRAP_IGNORE( iPauseTimer = CPeriodic::NewL( CActive::EPriorityHigh ) );
    
    if ( !iPauseTimer )
        {
        HandlePauseTimerExpiry( this );
        return;
        }
    
    iPauseTimer->Start( timeout, timeout, TCallBack(
        HandlePauseTimerExpiry, this ) );
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::KillTimer
// Stops and deletes a timer
// -----------------------------------------------------------------------------
//    
void CScreensaverEngine::KillTimer( CPeriodic*& aTimer )
    {
    if ( aTimer )
        {
        aTimer->Cancel( );
        delete aTimer;
        aTimer = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::HandlePreviewTimerExpiry
// -----------------------------------------------------------------------------
//
TInt CScreensaverEngine::HandlePreviewTimerExpiry( TAny* aPtr )
    {
    CScreensaverEngine* Engine = STATIC_CAST(CScreensaverEngine*, aPtr);
    SCRLOGGER_WRITEF(_L("SCR: Inside CScreensaverView::HandlePreviewTimerTimeout()"));
    
    Engine->KillTimer( Engine->iPreviewTimer );
    Engine->StopScreenSaver();
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CScreensaverEngine::HandlePauseTimerExpiry
// ---------------------------------------------------------------------------
//
TInt CScreensaverEngine::HandlePauseTimerExpiry( TAny* aPtr )
    {
    CScreensaverEngine* _this= STATIC_CAST(CScreensaverEngine*, aPtr);
    _this->KillTimer( _this->iPauseTimer );
    _this->iPauseTimerStartedWithKeylockOff = EFalse;

    if ( _this->iSharedDataI->IsKeyguardOn() )
        {
        SCRLOGGER_WRITE("HandleInactiveEventShortL() starting saver");
        _this->StartScreenSaver( );
        }

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::HandleActiveEventL
// -----------------------------------------------------------------------------
//
TInt CScreensaverEngine::HandleActiveEventL( TAny* aPtr )
    {
    SCRLOGGER_WRITE("HandleActiveEventL(), stop if previewing");
    CScreensaverEngine* _this= STATIC_CAST(CScreensaverEngine*, aPtr);
    if ( !_this->iSharedDataI->IsKeyguardOn() )
        {
        _this->StopScreenSaver();
        }
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::HandleInactiveEventL
// -----------------------------------------------------------------------------
//
TInt CScreensaverEngine::HandleInactiveEventL( TAny* aPtr )
    {
    SCRLOGGER_WRITE("HandleInactiveEventL(), starting screensaver");

    CScreensaverEngine* _this= STATIC_CAST(CScreensaverEngine*, aPtr);
    // Inactivity is detected immediately when preview starts
    if ( _this->iScreenSaverIsPreviewing )
        {
        _this->ResetInactivityTimeout();
        return KErrNone;
        }
    // Double-start is OK, it will be checked in StartScreenSaver()
    // This will be trigged by keylock activation after keyguard
    // timeout, or if keylock is disabled
    if ( !_this->iSharedDataI->IsOngoingCall() )
        {
        _this->StartScreenSaver( );
        }

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::HandleSuspendTimerExpiry
// -----------------------------------------------------------------------------
//
TInt CScreensaverEngine::HandleSuspendTimerExpiry( TAny* aPtr )
    {
    CScreensaverEngine* control= STATIC_CAST(CScreensaverEngine*, aPtr);
    SCRLOGGER_WRITEF(_L("SCR: Inside CScreensaverEngine::HandleSuspendTimerExpiry()"));

    control->KillTimer( control->iSuspendTimer );
    
    control->View()->SetDisplayObject( EDisplayPlugin );
    control->View()->ShowDisplayObject();

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CScreensaverEngine::HandleKeyguardStateChanged
// ---------------------------------------------------------------------------
//
void CScreensaverEngine::HandleKeyguardStateChanged( TBool aEnabled )
    {
    if ( aEnabled )
        {
        // Keys locked - if screensaver is running, this was caused by
        // automatic keyguard and screensaver should refresh the view
        // to show the keylock indicator
        if ( iScreenSaverIsOn && !iScreenSaverIsPreviewing )
            {
            View()->UpdateAndRefresh();
            }
        if ( !( iPauseTimerStartedWithKeylockOff && 
                iPauseTimer && iPauseTimer->IsActive() ) )
            {
            StartScreenSaver();
            }
        }
    else
        {
        StopScreenSaver( ETrue );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::View
// -----------------------------------------------------------------------------
//
CScreensaverView* CScreensaverEngine::View() const
    {
    return STATIC_CAST( CScreensaverAppUi*, CCoeEnv::Static()->AppUi() )->ScreensaverView();
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::Timeout
// -----------------------------------------------------------------------------
//
TInt CScreensaverEngine::Timeout( )
    {
    return iSharedDataI->ScreensaverTimeout( );
    }

// -----------------------------------------------------------------------------
// CScreensaverEngine::DisplayFlag
// -----------------------------------------------------------------------------
//
TInt CScreensaverEngine::DisplayFlag( )
    {
    return iSharedDataI->IsScreenSaverAllowed();
    }


// -----------------------------------------------------------------------------
// Handles expire timer timeout
// -----------------------------------------------------------------------------
//
TInt CScreensaverEngine::HandleExpiryTimerExpiry( TAny* aPtr )
    {
    SCRLOGGER_WRITEF( _L( "SCR:CScreensaverEngine::HandleExpiryTimerExpiry(%d) start" ), aPtr );
    CScreensaverEngine *control = STATIC_CAST( CScreensaverEngine*, aPtr );

    if ( control )
        {
        control->KillTimer( control->iExpiryTimer );
        // Revert to default screensaver.
        control->View()->SetDisplayObject(
            control->SharedDataInterface()->DefaultScreensaverType() );
        control->View()->ShowDisplayObject();
        }

    return KErrNone;
    }


// ---------------------------------------------------------------------------
// Callback to do the screensaver starting.
// ---------------------------------------------------------------------------
//
TInt CScreensaverEngine::StartSaverCb( TAny* aPtr )
    {
    CScreensaverEngine* self = static_cast<CScreensaverEngine*>( aPtr );
    // connect in StartScreenSaver, intercept and capture application 
    // key event. It makes sure this event will not be received 
    // by other applictions when screensaver is activated.
    self->iAknUiServer.ConnectAndSendAppsKeySuppress(ETrue);
    self->iScreenSaverIsOn = ETrue;

    // Report whether started from Idle BEFORE bringing to foreground
    self->iSharedDataI->SetSSStartedFromIdleStatus();

    if ( !self->View()->IsContentlessScreensaver() )
        {
        ScreensaverUtility::BringToForeground();
        }

    SCRLOGGER_WRITE("Model: SS is displaying (BringToForeground)");

    // Compress heap while displaying. No longer possible to
    // compress all heaps (User::CompressAllHeaps() is a no-op)
    User::Heap().Compress();

    self->DisplayObject();
    return KErrNone;
    }


// End of file.
