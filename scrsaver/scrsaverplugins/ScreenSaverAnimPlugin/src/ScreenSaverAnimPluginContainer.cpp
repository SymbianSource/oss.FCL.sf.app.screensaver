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
* Description: 
*       Provides  ScreenSaverAnimPlugin edit list view container methods.
*
*
*/



// INCLUDE FILES

#include <aknlists.h>
#include <akntitle.h>
#include <aknslider.h> 
#include <aknsettingpage.h> 
#include <aknslidersettingpage.h> 
#include <CFLDFileListContainer.h>
#include <MGFetch.h>
#include <DRMHelper.h>
#include <screensaveranimplugin.rsg>
#include <barsread.h>
#include <centralrepository.h>

#include "ScreenSaverAnimPlugin.h"
#include "ScreenSaverAnimPluginConst.h"
#include "ScreenSaverAnimPluginHelper.h"
#include "ScreenSaverAnimPluginContainer.h"
#include "ScreenSaverAnimPlugin.hrh"

// ========================= MEMBER FUNCTIONS ================================
 CScreenSaverAnimPluginContainer* CScreenSaverAnimPluginContainer::NewLC(
                               CScreenSaverAnimPlugin* aCaller,
                               const CCoeControl* aParent, 
                               CScreenSaverAnimSettingObject* aSettingObject )
    {
    CScreenSaverAnimPluginContainer* self = 
      new ( ELeave )  CScreenSaverAnimPluginContainer( aCaller, 
                                                       aSettingObject );
    CleanupStack::PushL( self );    
    TRect rect;
    self->GetNormalScreenLayoutRect( rect );
    self->ConstructL(aParent, rect );
    return self;

    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::NewL 
// ---------------------------------------------------------------------------
//
    
CScreenSaverAnimPluginContainer* CScreenSaverAnimPluginContainer::NewL(
                               CScreenSaverAnimPlugin* aCaller,
                               const CCoeControl* aParent, 
                               CScreenSaverAnimSettingObject* aSettingObject )
    {
    CScreenSaverAnimPluginContainer* self = 
       CScreenSaverAnimPluginContainer::NewLC( aCaller,
                                               aParent,
                                               aSettingObject );
    CleanupStack::Pop( self );
    return self;  

    }

// ---------------------------------------------------------------------------
// Constructor
// 
// ---------------------------------------------------------------------------
//
CScreenSaverAnimPluginContainer::CScreenSaverAnimPluginContainer(
                               CScreenSaverAnimPlugin* aCaller, 
                               CScreenSaverAnimSettingObject* aSettingObject )
    :
    iSettingObject( aSettingObject ),
    iCaller( aCaller )    
    {
    }

 
// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// 
// ---------------------------------------------------------------------------
//
void CScreenSaverAnimPluginContainer::ConstructL( const CCoeControl* aParent,
                                                  const TRect& aRect )
    {
#ifdef SCREENSAVER_LOG_ENABLED
    _LIT( msg, "In ConstructL of container" );
    PrintDebugMsg( msg );
#endif    

    CreateWindowL( aParent ); 

#ifdef SCREENSAVER_LOG_ENABLED
    _LIT( msg1, "Window Created" );
    PrintDebugMsg( msg1 );
#endif

    BaseConstructL( aRect, NULL, R_SANIM_SETTING_LBX_ITEMS );

#ifdef SCREENSAVER_LOG_ENABLED
    _LIT( msg2, "Base ConstructL called" );
    PrintDebugMsg( msg2 );
#endif

    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::BaseConstructL

// ---------------------------------------------------------------------------
//
void CScreenSaverAnimPluginContainer::BaseConstructL( 
    const TRect& aRect, TInt /* aTitleId */, TInt aResLbxId )
    {
    // Feature support should only be read once. It should not change run-time.
    GetFeatureSupportL( iFeaturesSupport );
    CEikStatusPane* sp = static_cast< CAknAppUi* >( 
        CEikonEnv::Static()->EikAppUi() )->StatusPane();
    CAknTitlePane* title = static_cast<CAknTitlePane*>( 
        sp->ControlL( TUid::Uid( EEikStatusPaneUidTitle) ) );

    iListBox = new( ELeave ) CAknSettingStyleListBox; 

    ConstructListBoxL( aResLbxId );
    iListBox->SetContainerWindowL( *this );
    iListBox->SetRect( aRect );

    iListBox->CreateScrollBarFrameL( ETrue );
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL( 
        CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );

    SetRect( aRect );
    ActivateL();

    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::~CScreenSaverAnimPluginContainer()
// Destructor
// ---------------------------------------------------------------------------
//
CScreenSaverAnimPluginContainer::~CScreenSaverAnimPluginContainer()
    {
    delete iItems;
    delete iListBox;
    // delete iItemArray;
    }

// ---------------------------------------------------------------------------
// Sets feature support - see screensaveranimpluginconst.h for feature list.
// ---------------------------------------------------------------------------
//
void CScreenSaverAnimPluginContainer::GetFeatureSupportL( 
    TBitFlags& aFeatureBitFlags ) const
    {
    CRepository* featureRep = CRepository::NewLC( KCRUidThemesVariation );
    TInt value = KErrNone;
    featureRep->Get( KThemesLocalVariation, value );

    aFeatureBitFlags.ClearAll();

    //map value to feature flags
    if ( value & KScreenSaverRemoveAnimPluginTimeout )
        {
        aFeatureBitFlags.Set( EAnimPluginBacklightRemoved );
        }

    CleanupStack::PopAndDestroy( featureRep );
    }

// ---------------------------------------------------------------------------
// Checks if feature is supported.
// ---------------------------------------------------------------------------
//
TBool CScreenSaverAnimPluginContainer::IsFeatureSupported( TInt aFeatureID )
    {
    if ( ( aFeatureID >= 0 ) && ( aFeatureID < KAnimPluginMaxFeatureID ) )
        {
        return iFeaturesSupport.IsSet( aFeatureID );
        }
    else
        {
        // invalid feature value.
        return EFalse;
        }
    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::HandleListBoxEventL
// (other items were commented in a header).
// ---------------------------------------------------------------------------
//
void CScreenSaverAnimPluginContainer::HandleListBoxEventL( 
    CEikListBox* /*aListBox*/, TListBoxEvent aEventType )
    {
    switch ( aEventType )
        {
        case EEventEnterKeyPressed:
        case EEventItemDoubleClicked:
        case EEventItemSingleClicked:
            HandleListBoxSelectionL();
            break;
        default:
           break;
        }
    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::HandleListBoxSelectionL()  
// ---------------------------------------------------------------------------
//   
void CScreenSaverAnimPluginContainer::HandleListBoxSelectionL()
    {
     TInt index = iListBox->CurrentItemIndex();
     
     switch ( index )
        {
        case ESanimAnimFileNameId:
            if ( !iInShowFileItem )
                {                               
                iInShowFileItem = ETrue;
                TRAPD( err,ShowFileNameItemL() );
                iInShowFileItem = EFalse;
                User::LeaveIfError( err );
                }
            break;
        case ESanimAnimTimeoutId:
            ShowAnimTimeOutItemL();
            break;
        case ESanimLightsTimeoutId:
            ShowLightsTimeOutItemL();
            break;
        default:
            break;
        }
    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::ConstructListBoxL( TInt aResLbxId )
// ---------------------------------------------------------------------------
//
void CScreenSaverAnimPluginContainer::ConstructListBoxL( TInt aResLbxId )
    {
#ifdef SCREENSAVER_LOG_ENABLED
    _LIT( msg, "In Construct ListBoxL" );
    PrintDebugMsg( msg );
#endif
  
    iListBox->ConstructL( this, EAknListBoxSelectionList );
    iItems = iCoeEnv->ReadDesC16ArrayResourceL( aResLbxId );
    iItemArray = static_cast< CDesCArray* >( iListBox->Model()->ItemTextArray() );
    iListBox->SetListBoxObserver( this );
    CreateListBoxItemsL();
    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::CreateListBoxItemsL()
// 
//  
// ---------------------------------------------------------------------------
//
void CScreenSaverAnimPluginContainer::CreateListBoxItemsL()
    {
#ifdef SCREENSAVER_LOG_ENABLED 
    _LIT( msg, "Make FileName" );
    PrintDebugMsg( msg );
#endif

    MakeFileNameItem();
    iItemArray->InsertL( ESanimAnimFileNameId, iItem );
    
#ifdef SCREENSAVER_LOG_ENABLED
    _LIT( msg1, "Make Duration" );
    PrintDebugMsg( msg1 );
#endif

    MakeAnimTimeOutItemL();
    iItemArray->InsertL( ESanimAnimTimeoutId, iItem );

    if ( !IsFeatureSupported( EAnimPluginBacklightRemoved ) )
        {
        #ifdef SCREENSAVER_LOG_ENABLED
        _LIT( msg2, "Make Backlight" );
        PrintDebugMsg( msg2 );
        #endif

        MakeLightsTimeOutItemL();
        iItemArray->InsertL( ESanimLightsTimeoutId, iItem );
        }
    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::UpdateListBoxL( TInt aItemId )
// ---------------------------------------------------------------------------
//
void CScreenSaverAnimPluginContainer::UpdateListBoxL( TInt aItemId )
    {   
    switch ( aItemId )
        {
        case ESanimAnimFileNameId:
            MakeFileNameItem();
            break;
        case ESanimAnimTimeoutId:
            MakeAnimTimeOutItemL();
            break;
        case ESanimLightsTimeoutId:
            MakeLightsTimeOutItemL();
            break;
        default:
            break;
        }

    iItemArray->Delete( aItemId );
    iItemArray->InsertL( aItemId, iItem );
    iListBox->HandleItemAdditionL();
    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::HandleResourceChange( TInt aType )
// ---------------------------------------------------------------------------
//
void CScreenSaverAnimPluginContainer::HandleResourceChange( TInt aType )
    {
    CCoeControl::HandleResourceChange( aType );
    if( aType == KEikDynamicLayoutVariantSwitch )
        {
        TRect mainPaneRect;
        GetNormalScreenLayoutRect( mainPaneRect );
        SetRect(mainPaneRect);

        DrawNow();
        }
    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::MakeFileNameItem() 
//  
// ---------------------------------------------------------------------------
//
void CScreenSaverAnimPluginContainer::MakeFileNameItem()
    {
    iItem = ( *iItems )[ESanimAnimFileNameId];    
    TParsePtrC fileParser( iSettingObject->iFileName );
    TPtrC ptr( fileParser.NameAndExt() );

    // Filename may be quite long - copy only the amount there's space left
    iItem += ptr.Left( iItem.MaxLength() - iItem.Length() - 1 );
    }
    
// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::MakeAnimTimeOutItemL() 
//  
// ---------------------------------------------------------------------------
//
void CScreenSaverAnimPluginContainer::MakeAnimTimeOutItemL()
    {
    iItem = ( *iItems )[ESanimAnimTimeoutId]; 
    
    HBufC* animText = CAknSlider::CreateValueTextInHBufCL( 
                  iSettingObject->iAnimationTimeout, 
                  R_ANIM_TIMEOUT_SLIDER );

    // for A&H number conversion
    TPtr bufPtr = animText->Des();
    if( AknTextUtils::DigitModeQuery( AknTextUtils::EDigitModeShownToUser ) )
        {
        AknTextUtils::LanguageSpecificNumberConversion( bufPtr );
        }
    iItem  += bufPtr;
    delete animText;

    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::MakeLightTimeOutItemL() 
//  
// ---------------------------------------------------------------------------
//
void CScreenSaverAnimPluginContainer::MakeLightsTimeOutItemL()
    {
    iItem = ( *iItems )[ESanimLightsTimeoutId];

    HBufC* animText = NULL;

    if ( iSettingObject->iLightsTimeout == 0 )
        {
        animText = iCoeEnv->AllocReadResourceL( R_SANIM_SLIDER_OFF );
        }
    else
        {
        animText = CAknSlider::CreateValueTextInHBufCL( 
                           iSettingObject->iLightsTimeout, 
                           R_LIGHT_TIMEOUT_SLIDER );
        }

    // for A&H number conversion
    TPtr bufPtr = animText->Des();
    if( AknTextUtils::DigitModeQuery( AknTextUtils::EDigitModeShownToUser ) )
        {
        AknTextUtils::LanguageSpecificNumberConversion( bufPtr );
        }
    iItem  += bufPtr;
    delete animText;

    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::ShowFileNameItemL() 
//  
// ---------------------------------------------------------------------------
//
void CScreenSaverAnimPluginContainer::ShowFileNameItemL()
    {    
        
    CDesCArrayFlat* selectedFile = new( ELeave )CDesCArrayFlat( 1 );
    CleanupStack::PushL( selectedFile );

    TResourceReader reader;
    iCoeEnv->CreateResourceReaderLC( reader, 
                             R_MIMETYPE_SPECIFIC_PLUGIN_ARRAY );
    TInt countResource = reader.ReadInt16();

    CDesCArrayFlat* mimetypesList = new( ELeave ) 
                                    CDesCArrayFlat( countResource );

    CleanupStack::PushL( mimetypesList );

    for ( TInt i = 0; i < countResource; i++ )
        {
        HBufC* mimeType = reader.ReadHBufCL();

        // Check plugin availability before appending mimetype
        TUid uid = iCaller->FindAnimPluginL( *mimeType );
        if ( uid != KNullUid )
            {
            mimetypesList->AppendL( *mimeType );
            }
        delete mimeType;
        reader.ReadInt32();
        }

    CScreenSaverAnimPluginVerifier* verifier = 
                       CScreenSaverAnimPluginVerifier::NewL( iCaller );
    CleanupStack::PushL( verifier );
    TBool fileSelected = MGFetch::RunL( *selectedFile,
                                  EAnyMediaFile,
                                  EFalse,
                                  KNullDesC,
                                  KNullDesC,
                                  mimetypesList,
                                  verifier );

    CleanupStack::PopAndDestroy( verifier ); 
    CleanupStack::PopAndDestroy( mimetypesList );
    CleanupStack::PopAndDestroy(); // reader

    if ( fileSelected )
        {
        TPtrC fileNamePtr = selectedFile->MdcaPoint( 0 );
        TFileName tempFileName;
        tempFileName.Copy( fileNamePtr );
        iSettingObject->iFileName.Copy( tempFileName ); 
        // Write the new setting into the central repository
        iCaller->SetAnimSettings();
        UpdateListBoxL( ESanimAnimFileNameId );
        }   

    CleanupStack::PopAndDestroy( selectedFile );

    }

// ----------------------------------------------------
// CScreenSaverAnimPluginContainer::ShowAnimTimeOutItemL() 
// Display the animation time-out setting page.
// ----------------------------------------------------
//    
void CScreenSaverAnimPluginContainer::ShowAnimTimeOutItemL()
    {
    
    CAknSettingPage* dlg = new( ELeave ) CAknSliderSettingPage(
                   R_ANIM_TIMEOUT_SETTING_PAGE, 
                   iSettingObject->iAnimationTimeout );
    if ( dlg->ExecuteLD( CAknSettingPage::EUpdateWhenChanged ) )
        {
        // Write the new setting into the central repository
        iCaller->SetAnimSettings();
        UpdateListBoxL( ESanimAnimTimeoutId );
        }
    }

// ----------------------------------------------------
// CScreenSaverAnimPluginContainer::ShowLightsTimeOutItemL() 
// Display the animation time-out setting page.
// ----------------------------------------------------
//    
void CScreenSaverAnimPluginContainer::ShowLightsTimeOutItemL()
    {
     CAknSettingPage* dlg = 
        new( ELeave ) CAknSliderSettingPage( R_LIGHT_TIMEOUT_SETTING_PAGE, 
                                          iSettingObject->iLightsTimeout );
    if ( dlg->ExecuteLD( CAknSettingPage::EUpdateWhenChanged ) )
        {    
        // Write the new setting into the central repository
        iCaller->SetAnimSettings();
        UpdateListBoxL( ESanimLightsTimeoutId );
        }
    }

void CScreenSaverAnimPluginContainer::SizeChanged()
    {
    if (iListBox)
        {
        // Get size from main pane, place from this rect
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect( 
            AknLayoutUtils::EMainPane, 
            mainPaneRect );

        iListBox->SetExtent( Rect().iTl, mainPaneRect.Size() );
        }
    }

TInt CScreenSaverAnimPluginContainer::CountComponentControls() const
    {
    if ( iListBox )
        {
        return 1;
        }
    else
        {
        return 0;
        }
    }

CCoeControl* CScreenSaverAnimPluginContainer::
                   ComponentControl( TInt /*aIndex*/ ) const
    {
    if ( iListBox )
        {
        return iListBox;
        }
    else
        {
        return NULL;
        } 
    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::OfferKeyEventL
// Called when a key is pressed.
//  
// ---------------------------------------------------------------------------
TKeyResponse CScreenSaverAnimPluginContainer::OfferKeyEventL(
    const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    switch (aKeyEvent.iCode)
        {
        case EKeyLeftArrow:
        case EKeyRightArrow:
            // Listbox takes all event even if it doesn't use them
            return EKeyWasNotConsumed;
        default:
            break;
        }
    return iListBox->OfferKeyEventL( aKeyEvent, aType );
    }

void CScreenSaverAnimPluginContainer::SetContainerWindowL( const CCoeControl& 
                                   /* aContainer*/ )
    {
    }

// ---------------------------------------------------------------------------
// CScreenSaverAnimPluginContainer::FocusChanged
// ---------------------------------------------------------------------------
void CScreenSaverAnimPluginContainer::FocusChanged( TDrawNow aDrawNow )
    {
    // Pass focus changed event to listbox.
    if ( iListBox )
        {
        iListBox->SetFocus( IsFocused(), aDrawNow );
        }
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::GetNormalScreenLayoutRect
// Gets the normal screen layout rectangle for control from LAF.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimPluginContainer::GetNormalScreenLayoutRect(
    TRect& aRect )  const // Rectangle specifying extent of control
    {
    AknLayoutUtils::LayoutMetricsRect(
            AknLayoutUtils::EMainPane, 
            aRect );
    }

// End of File
