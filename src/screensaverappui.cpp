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
* Description:   The AppUi file for screensaver application.
*
*/



#include <eikenv.h>

#include "screensavershareddatai.h"
#include "screensaverappui.h"
#include "screensaverengine.h"

//
// CScreensaverAppUi
//
// -----------------------------------------------------------------------------
// CScreensaverAppUi::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverAppUi::ConstructL()
    {
    SCRLOGGER_WRITEF(_L("SCR:CScreensaverAppUi::ConstructL start") );
    SCRLOGGER_CREATE;

    // Choose orientation based on screensaverconfig
#if defined(SS_ALWAYSPORTRAIT)
    BaseConstructL(EAppOrientationPortrait);

#elif defined(SS_ALWAYSLANDSCAPE)
    BaseConstructL(EAppOrientationLandscape);

#else

    // Not specified - follows device setting
    BaseConstructL();
#endif

    SetKeyEventFlags( CAknAppUiBase::EDisableSendKeyShort | CAknAppUiBase::EDisableSendKeyLong );
    
    iModel = CScreensaverEngine::NewL();

    CreateViewL();
    
    iEikonEnv->SetSystem( ETrue );
    HideApplicationFromFSW();
    }

// -----------------------------------------------------------------------------
// CScreensaverAppUi::~CScreensaverAppUi
// -----------------------------------------------------------------------------
//
CScreensaverAppUi::~CScreensaverAppUi()
    {

    delete iModel;
    iModel = NULL;

    SCRLOGGER_DELETE;
    }

// -----------------------------------------------------------------------------
// CScreensaverAppUi::Model
// -----------------------------------------------------------------------------
//
CScreensaverEngine& CScreensaverAppUi::Model() const
    {
    return *iModel;
    }

// -----------------------------------------------------------------------------
// CScreensaverAppUi::HandleCommandL
// -----------------------------------------------------------------------------
// 
void CScreensaverAppUi::HandleCommandL( TInt aCommand )
    {
    switch ( aCommand )
        {
        case EEikCmdExit:
            Exit();
            break;
        default:
            break;
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverAppUi::GetView
// -----------------------------------------------------------------------------
//
CScreensaverView* CScreensaverAppUi::ScreensaverView()
    {
    return iView;
    }

// -----------------------------------------------------------------------------
// CScreensaverAppUi::CreateViewL
// -----------------------------------------------------------------------------
//
void CScreensaverAppUi::CreateViewL()
    {
    SCRLOGGER_WRITEF(_L("SCR:CScreensaverAppUi::CreateViewL start") );
    iView = CScreensaverView::NewL();

    AddViewL( iView ); // transfer ownership to CAknViewAppUi

    SetDefaultViewL( *iView );
    }

// -----------------------------------------------------------------------------
// CScreensaverAppUi::HandleKeyEventL
// -----------------------------------------------------------------------------
//
TKeyResponse CScreensaverAppUi::HandleKeyEventL( const TKeyEvent& aKeyEvent,
    TEventCode aType )
    {
    SCRLOGGER_WRITEF(_L("SCR:CScreensaverAppUi::HandleKeyEventL start") );
    // Stop on keyup instead of keydown, otherwise the underlying
    // app might get orphaned key up events and soil its pants. EXCEPT
    // in preview mode stop on keydown, because the keup from starting
    // the preview might be received here, and preview stopped before
    // it even properly started
    TBool stop = EFalse;

    if ( !iView )
        {
        return EKeyWasConsumed;
        }

    if ( aType == EEventKey && aKeyEvent.iCode == EKeyNo )
        {
        stop = ETrue;
        }
    else
        {
        if ( iModel->ScreenSaverIsPreviewing() )
            {
            if ( aType == EEventKeyDown )
                {
                stop = ETrue;
                }
            }
        else
            {
            if ( aType == EEventKeyUp )
                {
                //stop = ETrue;
                }
            }
        }

    if ( stop )
        {
        iModel->StopScreenSaver();
        }

    return EKeyWasConsumed;
    }

// -----------------------------------------------------------------------------
// CScreensaverAppUi::HandleScreenDeviceChangedL
// Stops screensaver as soon as screen device changes. Resourcechange
// about e.g. resulting layout change comes annoyingly late
// -----------------------------------------------------------------------------
//
void CScreensaverAppUi::HandleScreenDeviceChangedL()
    {
#if defined(SS_FASTSTOPONSCREENDEVICECHANGE)
    // Slightly faster stop when screendevice changes. Instead of
    // letting the created user activity stop, do it here.
    // NOTE that if forced orientation is not used and there is an
    // application that changes screen orientation for itself (e.g.
    // viewfinder), screensaver will die here when activated if fast
    // stop is used.
#if !defined(SS_ALWAYSPORTRAIT) && !defined(SS_ALWAYSLANDSCAPE)
    // Do not stop, if forced orientation is used! 
    // Screensaver may cause a screen device change when activated,
    // and will promptly commit suicide here
    iModel->StopScreenSaver();
#endif
#endif

    CAknViewAppUi::HandleScreenDeviceChangedL();
    }

// -----------------------------------------------------------------------------
// CScreensaverAppUi::HandleWsEventL
// -----------------------------------------------------------------------------
//
void CScreensaverAppUi::HandleWsEventL( const TWsEvent& aEvent,
    CCoeControl* aDestination )
    {
    SCRLOGGER_WRITEF(_L("SCR:CScreensaverAppUi::HandleWsEventL start") );
    // First, let parent class handle the event
    CAknViewAppUi::HandleWsEventL( aEvent, aDestination );

    // In case of a pointer event, also stop screensaver
    if ( aEvent.Type() == EEventPointer )
        {
        iModel->StopScreenSaver();
        }
    }

//End of file
