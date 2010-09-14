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
* Description:   Implementation of screensaver none display object class.
*
*/



#include <screensaverplugin.h>
#include <hal.h>

#include "screensaverctrlnone.h"
#include "ScreensaverUtils.h"
#include "screensaverappui.h"
#include "screensavershareddatai.h"

//the param to set the display off/on
const TInt KDisplayOff = 0;
const TInt KDisplayOn = 1;


const TInt KTimerDelay = 60*1000*1000;  // 60 seconds
const TInt KTimerInterval = KTimerDelay; 

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::NewL
// -----------------------------------------------------------------------------
//
CScreensaverCtrlNone* CScreensaverCtrlNone::NewL()
    {
    CScreensaverCtrlNone* self = new( ELeave ) CScreensaverCtrlNone();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::~CScreensaverCtrlNone
// -----------------------------------------------------------------------------
//
CScreensaverCtrlNone::~CScreensaverCtrlNone()
    {
	if( iTimer )
		{
        iTimer->Cancel();
        delete iTimer;
		}
    
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::StartTimer
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlNone::StartTimer()
    {
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::CancelTimer
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlNone::CancelTimer()
    {
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::ClearScreen
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlNone::ClearScreen()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlNone::ClearScreen start") );
    SwitchDisplayState( KDisplayOn );
    SwitchLights( ESSForceLightsOn );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::Refresh
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlNone::Refresh()
    {
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::DrawObject
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlNone::DrawObject()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlNone::DrawObject start") );
    SwitchDisplayState( KDisplayOff );
    SwitchLights( ESSForceLightsOff );
    DrawNow();
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::HandleResourceChange
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlNone::HandleResourceChange(TInt aType)
    {
    if ( aType == KEikDynamicLayoutVariantSwitch )//&& iType != EDisplayNone)
        {
        // Screen layout has changed - resize
        SetRect( iCoeEnv->ScreenDevice()->SizeInPixels() );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::SizeChanged
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlNone::SizeChanged()
    {
    // Nothing to be implemented
    }

// ---------------------------------------------------------------------------
// CScreensaverCtrlNone::Draw
// ---------------------------------------------------------------------------
//
void CScreensaverCtrlNone::Draw( const TRect& aRect ) const
    {
    if ( !Model().ScreenSaverIsOn() && !Model().ScreenSaverIsPreviewing() )
        {
        return;
        }

    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlNone::Draw, screensaver is on or previewing") );
    CWindowGc& gc = SystemGc();
    gc.SetBrushColor( KRgbBlack );
    gc.Clear( aRect );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::CScreensaverCtrlNone
// -----------------------------------------------------------------------------
//
CScreensaverCtrlNone::CScreensaverCtrlNone()
    {
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlNone::ConstructL()
    {
    iTimer = CPeriodic::NewL( CActive::EPriorityStandard );
    iTimer->Start( KTimerDelay, KTimerInterval,
                            TCallBack( CloseDisplayResource,this ) );
	CreateWindowL();
    SetRect( iCoeEnv->ScreenDevice()->SizeInPixels() );
    ActivateL();
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::SwitchLights
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlNone::SwitchLights( TInt aSecs )
    {
    Model().SharedDataInterface()->SetSSForcedLightsOn( aSecs );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::SwitchDisplayState
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlNone::SwitchDisplayState( TInt aState )
    {
    SCRLOGGER_WRITEF(_L("SCR:CScreensaverCtrlNone::SwitchDisplayState start") );
    TInt currentState ;
    //Get the current display state
    TInt result = KErrNone;
    TRAP_IGNORE( result = HAL::Get( HALData::EDisplayState, currentState ) );
    
    if( KErrNone != result )
        {
        //if can't get the state, revert to default type
        View()->SetDisplayObject( 
            Model().SharedDataInterface()->DefaultScreensaverType() );
        return;
        }
    
    if ( aState != currentState ) //display already on/off; nothing to do
        {
        TRAP_IGNORE( HAL::Set( HALData::EDisplayState, aState ) );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::CloseDisplayResource
// -----------------------------------------------------------------------------
//
TInt CScreensaverCtrlNone::CloseDisplayResource( TAny* aPtr )
    {
    SCRLOGGER_WRITEF( _L("CScreensaverCtrlNone::CloseResource()") );
    CScreensaverCtrlNone* ctrl = static_cast<CScreensaverCtrlNone*>( aPtr );
	
    if( ctrl && ctrl->Model().ScreenSaverIsOn() && !ctrl->Model().ScreenSaverIsPreviewing() )
        {
        ctrl->SwitchDisplayState( KDisplayOff );
        ctrl->SwitchLights( ESSForceLightsOff );
        return KErrNone;
        }
    else
        {
        return KErrGeneral;
        }
    }

//End of file
