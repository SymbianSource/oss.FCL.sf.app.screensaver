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
* Description:   Screensaver view class implementation.
*
*/



#include <power_save_display_mode.h>
#include <screensaver.rsg>

#include "ScreensaverUtils.h"
#include "screensaverview.h"
#include "screensaverappui.h"
#include "screensaverplugin.h"
#include "screensaverutility.h"
#include "screensaverctrlmovingtext.h"
#include "screensaverctrlnone.h"
#include "screensaverctrlplugin.h"
#include "screensaverengine.h"
#include "screensavershareddatai.h"


// -----------------------------------------------------------------------------
// CScreensaverView::NewLC
// add this NewLC code called by UI
// -----------------------------------------------------------------------------
// 
CScreensaverView* CScreensaverView::NewL()
    {
    CScreensaverView* self = new( ELeave ) CScreensaverView();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CScreensaverView::CScreensaverView
// -----------------------------------------------------------------------------
//
CScreensaverView::CScreensaverView()
    {
    }


// -----------------------------------------------------------------------------
// CScreensaverView::~CScreensaverView
// -----------------------------------------------------------------------------
//
CScreensaverView::~CScreensaverView( )
    {
    DestroyDisplayObject();
    }


// -----------------------------------------------------------------------------
// CScreensaverView::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverView::ConstructL( )
    {
    // Let CAknView do its stuff
    BaseConstructL();
    
    SetDisplayObject( PslnSettingScreensaverType() );
    }


// -----------------------------------------------------------------------------
// CScreensaverView::CreateDisplayObjectL
// -----------------------------------------------------------------------------
//
void CScreensaverView::CreateDisplayObjectL( TDisplayObjectType aType )
    {
    SCRLOGGER_WRITEF(_L("SCR: CScreensaverView::CreateDisplayObjectL "),aType);
    
    DestroyDisplayObject();
    
    switch( aType )
        {
        case EDisplayPlugin:
            {
            iControl = CScreensaverCtrlPlugin::NewL();
            break;
            }
            
        case EDisplayTime:
        case EDisplayText:
            {
            iControl = CScreensaverCtrlMovingText::NewL( aType );
            break;
            }
            
        case EDisplayNone:
            {
            iControl = CScreensaverCtrlNone::NewL();
            break;
            }
            
        default:
            {
            break;
            }
        }
    
    iCurrentControlType = aType;
    }

// -----------------------------------------------------------------------------
// CScreensaverView::SetDisplayObject
// -----------------------------------------------------------------------------
//
void CScreensaverView::SetDisplayObject( TDisplayObjectType aType )
    {
    SCRLOGGER_WRITEF(_L("SCR: CScreensaverView::SetDisplayObject "));
    
    TRAPD( error, CreateDisplayObjectL( aType ) );

    if(error == KErrNone)
        return;

    SCRLOGGER_WRITE("ResetDisplayObject() setting default screensaver");

    TRAP_IGNORE( CreateDisplayObjectL( DefaultScreensaverType() ) );
    }

// -----------------------------------------------------------------------------
// CScreensaverView::CreatePreviewDisplayObjectL
// -----------------------------------------------------------------------------
//
void CScreensaverView::CreatePreviewDisplayObjectL()
    {
    SCRLOGGER_WRITEF(_L("SCR: CScreensaverView::CreatePreviewDisplayObjectL "));
    TRAPD( error, CreateDisplayObjectL( PslnSettingScreensaverType() ) );

    if(error == KErrNone)
        {
        Model().SharedDataInterface()->SetScreensaverPreviewState(
            EScreenSaverPreviewLoaded );
        return;
        }
    
    Model().SharedDataInterface()->SetScreensaverPreviewState(
        EScreenSaverPreviewError );
    }

// -----------------------------------------------------------------------------
// CScreensaverView::Id
// -----------------------------------------------------------------------------
//
TUid CScreensaverView::Id( ) const
    {
    return KUidScreensaverView;
    }


// -----------------------------------------------------------------------------
// CScreensaverView::DoActivateL
// -----------------------------------------------------------------------------
//
void CScreensaverView::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
    TUid /*aCustomMessageId*/, const TDesC8& /*aCustomMessage*/)
    {
    }


// -----------------------------------------------------------------------------
// CScreensaverView::DoDeactivate
// -----------------------------------------------------------------------------
//
void CScreensaverView::DoDeactivate( )
    {
    }

// -----------------------------------------------------------------------------
// CScreensaverView::ShowDisplayObject
// -----------------------------------------------------------------------------
//
void CScreensaverView::ShowDisplayObject()
    {
    SCRLOGGER_WRITEF(_L("SCR: CScreensaverView::ShowDisplayObject "));
    
    iControl->StartDrawObject();
    }

// -----------------------------------------------------------------------------
// CScreensaverView::HideDisplayObject
// -----------------------------------------------------------------------------
//
void CScreensaverView::HideDisplayObject()
    {
    SCRLOGGER_WRITEF(_L("SCR: CScreensaverView::HideDisplayObject "));
    iControl->StopDrawObject();
    
    ScreensaverUtility::SendToBackground();
    }

// -----------------------------------------------------------------------------
// CScreensaverView::UpdateAndRefresh
// Updates indicators and refreshes the display
// -----------------------------------------------------------------------------
//
void CScreensaverView::UpdateAndRefresh( )
    {
    SCRLOGGER_WRITEF(_L("SCR: CScreensaverView::UpdateAndRefresh "));
    iControl->Refresh();
    }

// -----------------------------------------------------------------------------
// CScreensaverView::DefaultScreensaverType
// -----------------------------------------------------------------------------
//
TDisplayObjectType CScreensaverView::DefaultScreensaverType()
    {
    return Model().SharedDataInterface()->DefaultScreensaverType();
    }

// -----------------------------------------------------------------------------
// CScreensaverView::PslnSettingScreensaverType
// -----------------------------------------------------------------------------
//
TDisplayObjectType CScreensaverView::PslnSettingScreensaverType()
    {
    return Model().SharedDataInterface()->DisplayObjectType();
    }

// -----------------------------------------------------------------------------
// CScreensaverView::Model
// -----------------------------------------------------------------------------
//
CScreensaverEngine& CScreensaverView::Model() const
    {
    return STATIC_CAST( CScreensaverAppUi*, CCoeEnv::Static()->AppUi() )->Model(); 
    }

// -----------------------------------------------------------------------------
// CScreensaverView::DestroyDisplayObject
// -----------------------------------------------------------------------------
//
void CScreensaverView::DestroyDisplayObject()
    {
    SCRLOGGER_WRITEF(_L("SCR: CScreensaverView::DestroyDisplayObject "));
    if( iControl )
        {
        delete iControl;
        iControl = NULL;
        }
    }
// End of file
