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
* Description:   Implementation for screensaver shared data monitoring.
*
*/



#include <coemain.h>
#include <ScreensaverInternalCRKeys.h>
#include <UsbWatcherInternalPSKeys.h>
#include <avkondomainpskeys.h>               // kaknkeyguardstatus
#include <ctsydomainpskeys.h>                // kpsuidctsycallinformation
#include <startupdomainpskeys.h>             // kpsglobalsystemstate
#include <ScreensaverInternalPSKeys.h>
#include <UikonInternalPSKeys.h>             // kuikmmcinserted

#ifdef RD_UI_TRANSITION_EFFECTS_PHASE2
#include <akntransitionutils.h>
#endif

#include "screensavershareddatamonitor.h"
#include "screensavershareddatai.h"
#include "screensaverrepositorywatcher.h"
#include "screensaverview.h"
#include "screensaverengine.h"
#include "screensaverappui.h"
#include "screensaversubscriber.h"
#include "screensaverutility.h"

//System shuttingdown value
const TInt KSystemShuttingdown = 1;

// -----------------------------------------------------------------------------
// CScreensaverSharedDataMonitor::~CScreensaverSharedDataMonitor
// -----------------------------------------------------------------------------
//
CScreensaverSharedDataMonitor::~CScreensaverSharedDataMonitor()
    {
    DeleteSubscriber( iPreviewModeSubscriber );
    iPreviewModeProperty.Close();
    
    DeleteSubscriber( iMMCWatchSubscriber );
    iMMCWatchProperty.Close();
    
    DeleteSubscriber( iUSBWatchSubscriber );
    iUSBWatchProperty.Close();
    
    DeleteSubscriber( iKeyguardStateSubscriber );
    iKeyguardStateProperty.Close();
    
    DeleteSubscriber( iCallStateSubscriber );
    iCallStateProperty.Close();
    
    DeleteSubscriber( iShuttingDownSubscriber );
    iShuttingDownProperty.Close();
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataMonitor::NewL
// -----------------------------------------------------------------------------
//
CScreensaverSharedDataMonitor* CScreensaverSharedDataMonitor::NewL( CScreensaverSharedDataI* aData )
    {
    CScreensaverSharedDataMonitor* self = new( ELeave )CScreensaverSharedDataMonitor( aData );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(); // self;
    return self;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataMonitor::CScreensaverSharedDataMonitor
// -----------------------------------------------------------------------------
//
CScreensaverSharedDataMonitor::CScreensaverSharedDataMonitor(
    CScreensaverSharedDataI* aData )
    :iData( aData )
    {
    // No implementation required
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataMonitor::ConstructL
// -----------------------------------------------------------------------------
// 
void CScreensaverSharedDataMonitor::ConstructL()
    {
    User::LeaveIfError(iPreviewModeProperty.Attach(KPSUidScreenSaver, KScreenSaverPreviewMode));
    iPreviewModeSubscriber = new (ELeave) CSubscriber(TCallBack(HandlePreviewModeChanged, this), iPreviewModeProperty);
    iPreviewModeSubscriber->SubscribeL();
    
    
    // MMC watcher
    User::LeaveIfError(iMMCWatchProperty.Attach(KPSUidUikon, KUikMMCInserted));
    iMMCWatchSubscriber = new (ELeave) CSubscriber(
        TCallBack(HandleMMCStateChanged, this), iMMCWatchProperty);
    iMMCWatchSubscriber->SubscribeL();
    
    // USB watcher (MMC dismount)
    User::LeaveIfError(iUSBWatchProperty.Attach(
           KPSUidUsbWatcher, KUsbWatcherSelectedPersonality));
    iUSBWatchSubscriber = new (ELeave) CSubscriber(
        TCallBack(HandleUSBStateChanged, this), iUSBWatchProperty);
    iUSBWatchSubscriber->SubscribeL();

    // Keyguard state watcher
    User::LeaveIfError(iKeyguardStateProperty.Attach(
        KPSUidAvkonDomain, KAknKeyguardStatus));
    iKeyguardStateSubscriber = new (ELeave) CSubscriber(
        TCallBack(HandleKeyguardStateChanged, this), iKeyguardStateProperty);
    iKeyguardStateSubscriber->SubscribeL();

    // Call state watcher
    User::LeaveIfError(iCallStateProperty.Attach( KPSUidCtsyCallInformation, KCTsyCallState ) );
    iCallStateSubscriber = new (ELeave) CSubscriber(
        TCallBack(HandleCallStateChanged, this), iCallStateProperty);
    iCallStateSubscriber->SubscribeL();

    // Shutting down state watcher
    User::LeaveIfError( iShuttingDownProperty.Attach( KPSUidStartup, KPSGlobalSystemState ) );
    iShuttingDownSubscriber = new (ELeave) CSubscriber(
        TCallBack(HandleShuttingDownStateChanged, this), iShuttingDownProperty);
    iShuttingDownSubscriber->SubscribeL();
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataMonitor::View
// -----------------------------------------------------------------------------
// 
CScreensaverView* CScreensaverSharedDataMonitor::View()
    {
    return STATIC_CAST( CScreensaverAppUi*, CCoeEnv::Static()->AppUi() )->ScreensaverView();
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataMonitor::Model
// -----------------------------------------------------------------------------
//
CScreensaverEngine& CScreensaverSharedDataMonitor::Model()
    {
    return STATIC_CAST( CScreensaverAppUi*, CCoeEnv::Static()->AppUi() )->Model();
    }


// -----------------------------------------------------------------------------
// CScreensaverSharedDataMonitor::DeteleSubscriber
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataMonitor::DeleteSubscriber( CSubscriber*& aSubscriber )
    {
    if ( aSubscriber )
        {
        aSubscriber->StopSubscribe();
        }
    delete aSubscriber;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataMonitor::HandlePreviewModeChanged
// Handles preview mode property key change. If key is changed to 1,
// preview mode is started
// -----------------------------------------------------------------------------
//
TInt CScreensaverSharedDataMonitor::HandlePreviewModeChanged(TAny *aPtr)
    {
    CScreensaverSharedDataMonitor *data = STATIC_CAST(CScreensaverSharedDataMonitor*, aPtr);

    if ( data->iData->ScreensaverPreviewMode() == 1)
        {
        TRAP_IGNORE( data->Model().StartPreviewModeL() );
        }
    
    return KErrNone;
    }


// -----------------------------------------------------------------------------
// CScreensaverSharedDataMonitor::HandleMMCStateChanged
// Handles MMC state change callback. If current plugin is loaded from
// MMC, screensaver defaults to date & time when MMC removed
// -----------------------------------------------------------------------------
//
TInt CScreensaverSharedDataMonitor::HandleMMCStateChanged(TAny* aPtr)
    {
    STATIC_CAST(CScreensaverSharedDataMonitor*, aPtr)->Model().StopScreenSaver();
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataMonitor::HandleUSBStateChanged
// Handles USB state change callback. If current plugin is loaded from
// MMC, screensaver defaults to date & time when USB attached
// -----------------------------------------------------------------------------
//
TInt CScreensaverSharedDataMonitor::HandleUSBStateChanged(TAny* aPtr)
    {
    // Same handler as in MMC removal, parameter tells it's because of USB
    STATIC_CAST(CScreensaverSharedDataMonitor*, aPtr)->Model().StopScreenSaver();
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataMonitor::HandleKeyguardStateChanged
// Handles keyguards status change callback.
// When keys become locked, screensaver timeout is shortened.
// -----------------------------------------------------------------------------
//
TInt CScreensaverSharedDataMonitor::HandleKeyguardStateChanged(TAny* aPtr)
    {
    // If keyguard just became unlocked, inform model so that
    // short timeout use is reset. Otherwise there might be a situation
    // where short timeout was in use, user opened keys, did something
    // without letting the device idle for short timeout period and
    // locked keys again. Then short timeout would remain in use without this
    // notification.
    // Also inform of keys locked, in case Screensaver is running this
    // happened because of automatic keyguard fired, and screensaver
    // should update the display to show the keyguard indicator
    CScreensaverSharedDataMonitor* _this =
        STATIC_CAST(CScreensaverSharedDataMonitor*, aPtr);

    if ( _this->iData->IsKeyguardOn() )
        {
        // Keys locked - if screensaver is running, this was caused by
        // automatic keyguard and screensaver should refresh the view
        // to show the keylock indicator
        if ( _this->Model().ScreenSaverIsOn() )
            {
            _this->View()->UpdateAndRefresh();
            }
        }

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataMonitor::HandleCallStateChanged
// Handles call state change callback. If call state changes,
// screensaver is stopped
// -----------------------------------------------------------------------------
//
TInt CScreensaverSharedDataMonitor::HandleCallStateChanged(TAny* aPtr)
    {
#ifdef RD_UI_TRANSITION_EFFECTS_PHASE2
    // Prevent fullscreen transition from screensaver on incoming call
    CAknTransitionUtils::SetData( KScreensaverCallStateChange, (TAny*)1 );
#endif

    STATIC_CAST(CScreensaverSharedDataMonitor*, aPtr)->Model().StopScreenSaver();
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataMonitor::HandleShuttingDownStateChanged
// -----------------------------------------------------------------------------
//
TInt CScreensaverSharedDataMonitor::HandleShuttingDownStateChanged( TAny* /*aPtr*/ )
    {
    TInt startupState = -1;
    
    RProperty::Get( KPSUidStartup, KPSGlobalSystemState, startupState );
    
    if( startupState == ESwStateShuttingDown )
        {
        RProperty::Set( KPSUidScreenSaver, KScreenSaverAllowScreenSaver, KSystemShuttingdown);
        }
    
    return KErrNone;
    }

// End of file
