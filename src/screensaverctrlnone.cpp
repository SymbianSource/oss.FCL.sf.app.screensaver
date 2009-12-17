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
    
    SwitchLights( KMaxLightsOnTime );
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
    
    SwitchLights( 0 );
    
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::HandleResourceChange
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlNone::HandleResourceChange(TInt /*aType*/)
    {
    // Nothing to be implemented
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlNone::SizeChanged
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlNone::SizeChanged()
    {
    // Nothing to be implemented
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


//End of file
