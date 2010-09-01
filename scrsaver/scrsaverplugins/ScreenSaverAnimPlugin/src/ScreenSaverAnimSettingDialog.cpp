/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:     This file implements the Setting Dialog which
*                is used to display Settings container
*
*/





#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <uikon/eikctrlstatus.h>
#endif
#include <e32std.h>
#include <AknDialog.h>
#include <coeaui.h>
#include <eikenv.h>
#include <bautils.h>
#include <eikdialg.h>
#include <eikmenup.h>
#include <eikspane.h>

#ifdef __SERIES60_HELP
#include <hlplch.h>
#endif // __SERIES60_HELP

#ifdef __SERIES60_HELP
// Help Context UID = DLL UID3
const TUid KScreenSaverAnimPluginHelpContextUid = { 0x10207447 };
#endif // __SERIES60_HELP

#include <coeaui.h>
#include <AknWaitNoteWrapper.h>
#include <aknnotewrappers.h>
#include <akntitle.h>
#include <data_caging_path_literals.hrh>
#include <StringLoader.h>
#include <screensaveranimplugin.rsg>
//-----------
// USER INCLUDES
#include "ScreenSaverAnimPlugin.h"
#include "ScreenSaverAnimPlugin.hrh"
#include "ScreenSaverAnimSettingDialog.h"
#include "ScreenSaverAnimPluginContainer.h"
#include "screensaver.hlp.hrh"

// Middle softkey control ID
const TInt KSanimMSKControlID = 3;

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::CScreenSaverAnimSettingDialog
// Default Constructor. Initialises Dialog State Variables.
// -----------------------------------------------------------------------------
//
CScreenSaverAnimSettingDialog::CScreenSaverAnimSettingDialog( 
                               CScreenSaverAnimPlugin* aCaller, 
                               CScreenSaverAnimSettingObject* aSettingObject )
    :iSettingObject( aSettingObject ),
    iCaller( aCaller )
    {
    iCustControl = NULL;
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::ConstructL
// Second Phase Constructor for CScreenSaverAnimSettingDialog. Opens resource file, reads the
// dialog menu resource and calls CAknDialog::ConstructL with the resource as
// param.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimSettingDialog::ConstructL()
    {  
    CAknDialog::ConstructL( R_SANIM_DIALOG_MENUBAR );
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::NewL
// Factory function for creating CScreenSaverAnimSettingDialog objects.
// Returns: CScreenSaverAnimSettingDialog* ; Pointer to the created object.
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
CScreenSaverAnimSettingDialog* CScreenSaverAnimSettingDialog::NewL( 
                                 CScreenSaverAnimPlugin* aCaller,
                                 CScreenSaverAnimSettingObject* aSettingObject )
    {
    CScreenSaverAnimSettingDialog* self = CScreenSaverAnimSettingDialog::NewLC(
                                             aCaller,
                                             aSettingObject );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::NewLC
// Factory function for creating CScreenSaverAnimSettingDialog objects.
// It also pushes the created dialog object onto the cleanup stack.
// Returns: CScreenSaverAnimSettingDialog* ; Pointer to the created object.
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
CScreenSaverAnimSettingDialog* CScreenSaverAnimSettingDialog::NewLC(
                                CScreenSaverAnimPlugin* aCaller,
                                CScreenSaverAnimSettingObject* aSettingObject )
    {
    CScreenSaverAnimSettingDialog* self = new ( ELeave ) 
                     CScreenSaverAnimSettingDialog( aCaller, aSettingObject );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// Destructor
CScreenSaverAnimSettingDialog::~CScreenSaverAnimSettingDialog()
    {
    if (this->IsFocused())
        {
        CEikDialog::ButtonGroupContainer().RemoveCommandObserver(
            KSanimMSKControlID );
        }

    // Custom Control will be automatically destroyed by dialog framework
    iCustControl = NULL;

    // Set the text in status pane to the stored one            
    if (iAppTitleText)
        {
        // Get the Status Pane Control
        CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();
        // Get the title pane 
        CAknTitlePane* tp = NULL;
        TRAPD( errGetControl, 
            tp = static_cast< CAknTitlePane* >( sp->ControlL( 
            TUid::Uid( EEikStatusPaneUidTitle ) ) ) );
        if ( errGetControl == KErrNone && tp )
            {
            //set application name in the title pane
            TRAPD( errSetText, 
                tp->SetTextL( *iAppTitleText ) );
            if ( errSetText != KErrNone )
                {
                // No error handling here
                }
            }

        delete iAppTitleText;
        iAppTitleText = NULL;
        }

        delete iTitlePaneString;
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::ExecuteLD
// Reimplements CAknDialog::ExecuteLD. It initialises the member variables of
// the class and calls CAknDialog::ExecuteLD() with the dialog resource.
// -----------------------------------------------------------------------------
//
TInt CScreenSaverAnimSettingDialog::ExecuteLD()
    {
#ifdef SCREENSAVER_LOG_ENABLED
    _LIT( msg, "In Executeld dlg" );
    PrintDebugMsg( msg );
#endif

    // Save this pointer since there are leaving functions in the flow
    CleanupStack::PushL( this );    

    // Get the Status Pane Control
    CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();

    // Get the Title Pane Control
    CAknTitlePane* tp = static_cast< CAknTitlePane* >( sp->ControlL(
            TUid::Uid( EEikStatusPaneUidTitle ) ) );

    // Store the Application's title text
    iAppTitleText = HBufC::NewL( tp->Text()->Length() );
    *iAppTitleText = *tp->Text();

    // Display Settings in the title pane of application
    iTitlePaneString = NULL;
    iTitlePaneString = StringLoader::LoadL( R_SANIM_TITLE_SETTINGS,
                                          iEikonEnv );
    // SetTextL also ensures that if the length exceeds the layout space
    // available the text is truncated from the end.
    tp->SetTextL( iTitlePaneString->Des() );

    // ExecuteLD will delete this, so we have to Pop it...
    CleanupStack::Pop( this ); // this

    return CAknDialog::ExecuteLD( R_SANIM_SETTING_DIALOG );

    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::DynInitMenuPaneL
// Reimplements CAknDialog::DynInitMenuPaneL inorder to support dynamic hiding
// of menu items based on current state of the viewer.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimSettingDialog::DynInitMenuPaneL(
    TInt aResourceId,           // Resource Id for which this func is called
    CEikMenuPane* aMenuPane )   // Menupane object pointer for manipulation
    {
    if ( ( aResourceId == R_SANIM_DIALOG_MENU ) && ( aMenuPane ) )
        {
        }
    }


// -----------------------------------------------------------------------------
// Layouts dialog before showing it.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimSettingDialog::PreLayoutDynInitL()
    {
    // Set middle softkey as Change. 
    SetMiddleSoftKeyLabelL( R_SANIM_MSK_CHANGE, ESanimDlgChangeCommand );

    CAknDialog::PreLayoutDynInitL();
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::OkToExitL
// Reimplements CAknDialog::OkToExitL inorder to display custom menu and
// trap the back softkey to provide restore view functionality/exit.
// -----------------------------------------------------------------------------
//
TBool CScreenSaverAnimSettingDialog::OkToExitL( TInt aButtonId ) // ButtonId of button pressed
    {
    if ( aButtonId == EAknSoftkeyOptions )
        {
        DisplayMenuL();
        return EFalse;
        }
    else
        {
        if ( aButtonId == EAknSoftkeyBack )
            {
            }
        else
            {
            // Especially handle Select Key
            if ( ( aButtonId == EAknSoftkeySelect ) ||
                 ( aButtonId == EEikBidOk ) ||
                 ( aButtonId == ESanimDlgChangeCommand ) )
                {
                // Select button pressed, handle, do not leave dialog
                ProcessCommandL( ESanimDlgChangeCommand );
                return EFalse;
                }
            if ( aButtonId == EEikCmdExit )
                {
                }
            }
        }

    return ETrue;
    }


// ---------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::HandleApplicationForegroundEvent
// This API is used by the application to give background/foreground events
// to the Dialog. The dialog inturn passes the events to custom control.
// ---------------------------------------------------------------------------
//
void CScreenSaverAnimSettingDialog::HandleApplicationForegroundEvent( TBool 
                                                 /*aForeground*/ )
    {
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::ProcessCommandL
// Reimplements CAknDialog::ProcessCommandL inorder to handle dialog menu
// commands.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimSettingDialog::ProcessCommandL( TInt aCommandId )  // Command Id
    {
    // The Commands are handled by the custom CCoe Control and
    // a boolean varialble is used inorder to keep track whether
    // the control command processing needs to be invoked or not.
    CAknDialog::ProcessCommandL( aCommandId );
    switch ( aCommandId )
        {
        case ESanimDlgChangeCommand:
        if ( iCustControl )
            {
            iCustControl->HandleListBoxSelectionL();
            }
            break;
        case ESanimDlgHelpCommand:
            {
#ifdef __SERIES60_HELP
            HlpLauncher::LaunchHelpApplicationL( iEikonEnv->WsSession(),
                iEikonEnv->EikAppUi()->AppHelpContextL() );
#endif
            }
            break;
        case EAknSoftkeyBack: // Fall Through
        case EEikCmdExit:
        case EAknCmdExit:
            // Close this dialog
            TryExitL( aCommandId );
            iAvkonAppUi->ProcessCommandL( aCommandId );
            break;
        default:
            break;
        }
    }

#ifdef __SERIES60_HELP
// -----------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::OfferKeyEventL
// Handles Key events by reimplementing CCoeControl::OfferKeyEventL.
// Returns:
//      EKeyWasConsumed: If this control uses this key.
//      EKeyWasNotConsumed: Otherwise.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimSettingDialog::GetHelpContext( TCoeHelpContext& 
                                                    aContext ) const
    {
    aContext.iMajor = KScreenSaverAnimPluginHelpContextUid;
    aContext.iContext = KSCREENSAVER_HLP_ANIM_SETTINGS;
    }
#endif // __SERIES60_HELP

// -----------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::CreateCustomControlL
// Reimplements CAknDialog::CreateCustomControlL inorder to add the 
// custom control as part of the dialog in the UI framework.
// Returns: SEikControlInfo which contains the Cust Control, if
//                          aControlType == ESanimCustControl
//                     else,
//                           the structure does not contain any control.
// -----------------------------------------------------------------------------
//
SEikControlInfo CScreenSaverAnimSettingDialog::CreateCustomControlL(
        TInt aControlType ) // Control Type
    {
#ifdef SCREENSAVER_LOG_ENABLED
    _LIT( msg, "In Create Custom Control" );
    PrintDebugMsg( msg );
#endif

    SEikControlInfo controlInfo;
    controlInfo.iControl = NULL;
    controlInfo.iTrailerTextId = 0;
    controlInfo.iFlags = EEikControlIsNonFocusing;

    switch ( aControlType )
        {
        case ESanimCustControl:
            iCustControl = CScreenSaverAnimPluginContainer::NewL( 
                                              iCaller,
                                              this,
                                              iSettingObject );
            controlInfo.iControl = iCustControl;
            break;
        default:
            break;
        }
    return controlInfo;
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::SizeChanged
// Reimplements CAknDialog::SizeChanged inorder to support the resize of the
// dialog when functions such as SetRect, SetExtent are called on the dialog.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimSettingDialog::SizeChanged()
    {
    CAknDialog::SizeChanged();
    iCustControl->SetExtent( Rect().iTl, Rect().Size() );
    DrawNow();
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::OfferKeyEventL
// Handles Key events by reimplementing CCoeControl::OfferKeyEventL.
// Returns:
//      EKeyWasConsumed: If this control uses this key.
//      EKeyWasNotConsumed: Otherwise.
// -----------------------------------------------------------------------------
//
TKeyResponse CScreenSaverAnimSettingDialog::OfferKeyEventL(
    const TKeyEvent& aKeyEvent, // Key Event
    TEventCode aType )           // Event Code
    {
    if ( iCustControl )
        {
        TKeyResponse lCcRetVal = iCustControl->OfferKeyEventL( aKeyEvent, aType );
        if ( lCcRetVal == EKeyWasConsumed )
            {
            // Key was consumed, UNLESS it was the End key, which should be
            // passed to base class. For some reason listboxes like to eat
            // end keys
            if ( aKeyEvent.iCode != EKeyEscape )
                {
                return EKeyWasConsumed;
                }
            }
        }
    return CAknDialog::OfferKeyEventL( aKeyEvent, aType );
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::HandleResourceChange
// Handles change of skin/Layout
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimSettingDialog::HandleResourceChange( TInt aType )
    {
    if ( iCustControl )
        {
        iCustControl->HandleResourceChange( aType );
        }
    CAknDialog::HandleResourceChange( aType );
    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimSettingDialog::FocusChanged
// ---------------------------------------------------------------------------
//
void CScreenSaverAnimSettingDialog::FocusChanged( TDrawNow aDrawNow )
    {
    // Pass focus changed event to listbox.
    if ( iCustControl )
        {
        iCustControl->SetFocus( IsFocused(), aDrawNow );
        }
    }

// ---------------------------------------------------------------------------
// Sets middle softkey label.
// ---------------------------------------------------------------------------
//
void CScreenSaverAnimSettingDialog::SetMiddleSoftKeyLabelL( 
    const TInt aResourceId, const TInt aCommandId )
    {
    HBufC* middleSKText = StringLoader::LoadLC( aResourceId );
    TPtr mskPtr = middleSKText->Des();
    CEikDialog::ButtonGroupContainer().AddCommandToStackL( 
        KSanimMSKControlID, 
        aCommandId, 
        mskPtr );
    CEikDialog::ButtonGroupContainer().UpdateCommandObserverL( 
        KSanimMSKControlID, 
        *this );
    CleanupStack::PopAndDestroy( middleSKText );
    }

// End of File
