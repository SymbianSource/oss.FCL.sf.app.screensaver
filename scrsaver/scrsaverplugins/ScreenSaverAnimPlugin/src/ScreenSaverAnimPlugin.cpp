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
* Description:     This file implements the Screen Saver Animation Plugin which
*                is used by Screen Saver application to load animation 
*                screen savers.
*
*/





#include <eikenv.h>
#include <bautils.h>
#include <apgcli.h>
#include <data_caging_path_literals.hrh>
#include <centralrepository.h>
#include <DRMHelper.h>
#include <aknnotewrappers.h> 
#include <aknnavi.h>
#include <aknnavide.h>
#include <npupp.h>
#include <cecombrowserplugininterface.h>
#include <browserplugininterface.h>
#include <mmf/common/mmfcontrollerpluginresolver.h>
#include <DRMHelperServerInternalCRKeys.h>
#include <NPNExtensions.h>
#include <screensaveranimplugin.rsg>
#include <coecntrl.h>
#include "ScreenSaverAnimPluginInternalCRKeys.h"
#include "ScreenSaverAnimPlugin.h"
#include "ScreenSaverAnimPluginContainer.h"
#include "ScreenSaverAnimSettingDialog.h"
#include "NpnImplementation.h"
#include "ScreenSaverAnimPluginHelper.h"

// Constants
#ifdef SCREENSAVER_LOG_ENABLED         
const TInt KMaxBufLen = 64;
#endif
const TInt KSixteenBytes = 128;
const TInt KUrlMaxLen = 1024;

// Animation Timeout values
const TInt KMinDurationValue = 5;
const TInt KMaxDurationValue = 60;
const TInt KDefaultDurationValue = 5;

// Backlight Values
const TInt KMinBackLightValue = 0;
const TInt KMaxBackLightValue = 30;
const TInt KDefaultBackLightValue = 0;

const TInt KScreensaverAnimPluginInterfaceUid = 0x102750CB;  


_LIT( KResourceFileName, "Z:ScreenSaverAnimPlugin.rsc" );


CScreenSaverAnimSettingObject::CScreenSaverAnimSettingObject()
    {
    iAnimationTimeout = KDefaultDurationValue;
    iLightsTimeout = KDefaultBackLightValue;
    }
// ============================ MEMBER FUNCTIONS =============================


// ---------------------------------------------------------------------------
// CScreenSaverAnimPlugin::NewL
// ---------------------------------------------------------------------------
//
CScreenSaverAnimPlugin* CScreenSaverAnimPlugin::NewL()
    {
    CScreenSaverAnimPlugin* self = new ( ELeave ) CScreenSaverAnimPlugin;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::~CScreenSaverAnimPlugin
// ----------------------------------------------------------------------------
//   
CScreenSaverAnimPlugin::~CScreenSaverAnimPlugin()
    {    
    if ( iResourceOffset != 0 )
        {
        iEikEnv->DeleteResourceFile( iResourceOffset );
        }        
    UnregisterForDrmExpiry();       
    NotifyCenrepChangeCancel();     
    UnInitializeCenRep();

    // Final place to delete plug-in. Plug-in is deleted here
    // if plug-in notices runtime error.
    SetPluginDeleted();

    delete iDrmHelper;
    delete iSettingObject;
    delete iScreensaverName;
    }
    

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::InitializeL
// -----------------------------------------------------------------------------
//  
TInt CScreenSaverAnimPlugin::InitializeL( MScreensaverPluginHost *aHost )
    {     
    
    if ( !aHost )
        {
        return KErrCancel;
        }
#ifdef SCREENSAVER_LOG_ENABLED                 
    _LIT( msg, "InitializeL called " );    
    PrintDebugMsg( msg );
#endif    
    
    iScreenSaverHost = aHost;   
    iScreenSaverHost->UseRefreshTimer( EFalse );
    
#ifdef SCREENSAVER_LOG_ENABLED             
    _LIT( cenrepmsg, "before NotifyCenrepChangeL" );    
    PrintDebugMsg( cenrepmsg );    
#endif    
         
    NotifyCenrepChangeL();
    
#ifdef SCREENSAVER_LOG_ENABLED             
    _LIT( cenrepmsg1, "After NotifyCenrepChangeL" );    
    PrintDebugMsg( cenrepmsg1 );    
#endif    
            
    iScreenSaverHost->OverrideStandardIndicators();    
    
    iScreenSaverHost->ExitPartialMode(); 
    
#ifdef SCREENSAVER_LOG_ENABLED             
    _LIT( FindPluginLmsg, "before FindPluginL" );    
    PrintDebugMsg( FindPluginLmsg );    
#endif    
    
    TRAPD( err, FindPluginL( iSettingObject->iFileName ) )
    
    if ( err != KErrNone )
        {
        iScreenSaverHost->RevertToDefaultSaver();
        return err;
        }
    
#ifdef SCREENSAVER_LOG_ENABLED             
    _LIT( FindPluginLmsg1, "After FindPluginL" );    
    PrintDebugMsg( FindPluginLmsg1 );        
#endif    
    
    return KErrNone;
    
    }
    
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::Draw
// -----------------------------------------------------------------------------
// 
TInt CScreenSaverAnimPlugin::Draw(CWindowGc& /*aGc*/)
    {     
    return KErrNone;    
    }
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::SetWindow
// -----------------------------------------------------------------------------
//    
TInt CScreenSaverAnimPlugin::SetWindow( CCoeControl* aParent )
    {
        
    iParentControl = aParent;
    NPWindow window;
    TInt ret = KErrNone;
    
    window.window = STATIC_CAST( MPluginAdapter*, this ); 
    iNPPinstance->ndata = STATIC_CAST( MPluginAdapter*, this ); 
    window.x = 0;   /* Position of top left corner relative */
    window.y = 0;   /* to a netscape page.*/
    
    if ( aParent )
        {
        window.width = aParent->Rect().Width(); /* Max window size */
        window.height = aParent->Rect().Height();
        window.type = NPWindowTypeDrawable;                
        }
    
    if ( iPluginFuncs && !iPluginFuncs->setwindow( iNPPinstance, &window ) )
        {
        ret = KErrNone;
        }  
    else
        {
        ret = KErrGeneral;   
        }
    return ret;
    }
    
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::Name
// -----------------------------------------------------------------------------
//     
const TDesC16& CScreenSaverAnimPlugin::Name() const
    {
    if ( iScreensaverName )
        {
        return *iScreensaverName;
        }
    else
        {
        return KNullDesC;
        }
    }
    
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::Capabilities
// -----------------------------------------------------------------------------
//    
TInt CScreenSaverAnimPlugin::Capabilities()
    {
    return EScpCapsConfigure;
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::PluginFunction
// -----------------------------------------------------------------------------
// 
TInt CScreenSaverAnimPlugin::PluginFunction(
            TScPluginCaps aFunction,
            TAny* /*aParam*/)
    { 
    if ( aFunction == EScpCapsSelectionNotification  
		|| aFunction == EScpCapsPreviewNotification )
        {
        _LIT( msg, "Plugin function called Sel Notify" );    
        PrintDebugMsg( msg );
    
        if ( iSettingObject->iFileName.Length() == 0  || UpdateFileName() )
            {
            // File is not configured yet or deleted/renamed,
            // pop the settings dialog
            TInt err = KErrNone;
            TRAP( err, PopSettingsDialogL() );
            return err;
            }            
        else
            {
            return KErrNone;
            }      
        }
    if ( aFunction == EScpCapsConfigure )
        {
        _LIT( msg, "Plugin function called Configure" );    
        PrintDebugMsg( msg );   
        UpdateFileName();           
        TInt err = KErrNone;
        TRAP( err, PopSettingsDialogL() );
        return err;
        }    
    return KErrNone;                                                
    }  
    
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::HandleScreensaverEventL
// -----------------------------------------------------------------------------
//  
TInt CScreenSaverAnimPlugin::HandleScreensaverEventL(
            TScreensaverEvent aEvent,
            TAny* /*aData*/ )
    {

    if ( aEvent == EScreensaverEventDisplayChanged )
        {
        // Display Changed is called when there is a change in the size
        // of the window and when the parent control is set for the 
        // first time.        
        TScreensaverDisplayInfo* displayInfo = new( ELeave ) 
                               TScreensaverDisplayInfo;
        displayInfo->iSize = sizeof( TScreensaverDisplayInfo );
        iScreenSaverHost->DisplayInfo( displayInfo );
        iParentControl = displayInfo->iParent;
        delete displayInfo;
        }

    if ( aEvent == EScreensaverEventStopping )
        {
#ifdef SCREENSAVER_LOG_ENABLED                     
        _LIT( KStopEventMsg, "Stop Event" );
        PrintDebugMsg( KStopEventMsg );
#endif          

        // Delete the Plugin
        SendEndEvent();
        DeletePlugin();
        }

    if ( aEvent == EScreensaverEventStarting )
        {
#ifdef SCREENSAVER_LOG_ENABLED                     
        _LIT( KStartEventMsg, "Start Event" );
        PrintDebugMsg( KStartEventMsg );
#endif          
        // Check for drm rights
        // If the file is not valid, suspend and return
        if ( iDrmHelper )
            {
            TBool yes = EFalse;   
            iDrmHelper->CanSetAutomated( 
                     iSettingObject->iFileName, yes );
            if ( !yes )
                {
                iScreenSaverHost->RevertToDefaultSaver();
                return KErrNotFound;
                }
            } 

        if ( iScreenSaverHost )
            {
            iScreenSaverHost->RequestTimeout(
                iSettingObject->iAnimationTimeout );
            }        
            
#ifdef SCREENSAVER_LOG_ENABLED                     
        _LIT( KStartMsg, "Starting" );
        PrintDebugMsg( KStartMsg );
#endif        
        
        // Reset plugin creation error
        // The plugin cannot leave or return error
        // The plugin sets the error using pluginFinishedL
        iPluginError = KErrNone;
        // Create the plugin 
        TRAPD( err,CreatePluginL() );
        if ( err != KErrNone )
            {
            DeletePlugin();
            return err;
            }

        // Check whether the plugin has 
        // reported any error using PluginFinishedL    
        if ( iPluginError )
            {
            SetPluginDeleted();
            return iPluginError;
            }

#ifdef SCREENSAVER_LOG_ENABLED
        _LIT( KStartedMsg, "Started" );
        PrintDebugMsg( KStartedMsg );
#endif        
        
        // send an event to plugins       
        if ( iPluginFuncs )
            {
            NPAnimationEvent event;
            event = NPStartAnimation;
            iPluginFuncs->setvalue( iNPPinstance, 
                                    ( NPNVariable )NPNScreenSaverAnimationEvent, 
                                    &event );     
            }

        // Request lights only if set - otherwise it will turn off lights
        // (e.g. in preview). Not nice.
        TInt nLights = iSettingObject->iLightsTimeout;

        if ( nLights > 0 )
            {
            iScreenSaverHost->RequestLights( nLights );
            }
        }

    if ( aEvent == EScreensaverEventTimeout )
        {
#ifdef SCREENSAVER_LOG_ENABLED
        _LIT( KTimeoutEventMsg, "Timeout Event" );
        PrintDebugMsg( KTimeoutEventMsg );
#endif          
        // Delete the Plugin
        SendEndEvent();
        DeletePlugin();
        iScreenSaverHost->Suspend( -1) ;
        }

    return KErrNone;
    }  

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::GetAnimSettings
// -----------------------------------------------------------------------------
//  
TInt CScreenSaverAnimPlugin::GetAnimSettings()
    {   
    // Read the file last, since if it is missing the reading will be
    // stopped and settings all wrong
    TInt err = iAnimPluginSession->Get( KScreenSaverAnimationTimeOut,
                                        iSettingObject->iAnimationTimeout );

    if ( iSettingObject->iAnimationTimeout < KMinDurationValue ||
         iSettingObject->iAnimationTimeout > KMaxDurationValue )
        {
        // Out of range set it to default
        iSettingObject->iAnimationTimeout = KDefaultDurationValue;
        }

    if ( err != KErrNone )
        {
        return err;
        }

    err = iAnimPluginSession->Get( KScreenSaverLightsTimeOut,
                                   iSettingObject->iLightsTimeout );

    if ( iSettingObject->iLightsTimeout < KMinBackLightValue ||
         iSettingObject->iLightsTimeout > KMaxBackLightValue )
        {
        // Out of range set it to default
        iSettingObject->iLightsTimeout = KDefaultBackLightValue;
        }

    if ( err != KErrNone )
        {
        return err;
        }

    // read the file from central repository and return
    err = iAnimPluginSession->Get( KScreenSaverAnimatedFileName,
                           iSettingObject->iFileName );
                           
    if ( err != KErrNone )
        {
        return err;
        }

    if ( !BaflUtils::FileExists( iEikEnv->FsSession(), 
                                iSettingObject->iFileName ) )
        {
        return KErrNotFound;
        }

    TBool yes = EFalse;
    if ( iDrmHelper )
        {
        TInt err = iDrmHelper->CanSetAutomated( 
                 iSettingObject->iFileName, yes );
        if ( !yes )
            {
            return KErrNotFound;
            }
        }

    return err;
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::SetAnimSettings
// -----------------------------------------------------------------------------
//
TInt CScreenSaverAnimPlugin::SetAnimSettings() const
    {
    
    TInt err = KErrNone;
    err = iAnimPluginSession->Set( KScreenSaverAnimatedFileName, 
                          iSettingObject->iFileName );
    if ( err != KErrNone )                          
        {
        return err;
        }
    err = iAnimPluginSession->Set( KScreenSaverAnimationTimeOut,
                          iSettingObject->iAnimationTimeout );

    if ( err != KErrNone )
        {
        return err;
        }

    err = iAnimPluginSession->Set( KScreenSaverLightsTimeOut,
                          iSettingObject->iLightsTimeout );

    return err;

    } 
    
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::RegisterForDrmExpiry
// -----------------------------------------------------------------------------
//
TInt CScreenSaverAnimPlugin::RegisterForDrmExpiry()
    {
    TInt err = iDrmHelper->SetAutomatedType( 
             CDRMHelper::EAutomatedTypeScreenSaver );
    if ( err == KErrNone )
        {
        return iDrmHelper->SetAutomatedSilent( iSettingObject->iFileName,
                            ETrue ); 
        }
    else
        {
        return err;
        }
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::UnregisterForDrmExpiry
// -----------------------------------------------------------------------------
//
TInt CScreenSaverAnimPlugin::UnregisterForDrmExpiry()
    {   
    if ( iDrmHelper )
        {
        iDrmHelper->SetAutomatedType( 
             CDRMHelper::EAutomatedTypeScreenSaver );
        return iDrmHelper->RemoveAutomated( iSettingObject->iFileName );    
        }
    else
        {
        return KErrNone;
        }
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::ReadDrmNotification
// -----------------------------------------------------------------------------
//  
TInt CScreenSaverAnimPlugin::ReadDrmNotification( TDes8& aString )
    {
    return iDrmSession->Get( KDRMHelperServerNotification, aString );
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::ProcessDrmNotificationL
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimPlugin::ProcessDrmNotificationL()      
    {
         
    TInt size = KSixteenBytes;
    HBufC8* buf = HBufC8::NewLC( size );
    TPtr8 bufPtr = buf->Des();

    TInt error = ReadDrmNotification( bufPtr );
    while ( error == KErrTooBig )
        {
        size += KSixteenBytes;
        CleanupStack::PopAndDestroy( buf ); // buf
        buf = HBufC8::NewLC( size );
        bufPtr = buf->Des();
        error = ReadDrmNotification( bufPtr );
        }

    TUint8 count = ( TUint8 )( *( buf->Ptr() ) );
    TChar type = ( TUint8 )( *( buf->Ptr() + 1 ) );

    TUint8 permType = ( TUint8 )( *( buf->Ptr() + 2 ) );
    TUint8 autoType = ( TUint8 )( *( buf->Ptr() + 3 ) );

    TPtr8 ptr( ( TUint8* ) ( buf->Ptr() + 4 ) , buf->Length() - 5 , buf->Length() - 5 );
    HBufC8* content = HBufC8::NewLC( ptr.Length() );
    *content = ptr;

    TBool matched = CheckContentIdL( iSettingObject->iFileName, *content );

    CleanupStack::PopAndDestroy( content ); 
    CleanupStack::PopAndDestroy( buf ); 

    if ( !matched )
        {
        return;
        }

    if ( type == 'E')
        {
        if ( count == 0 )
            {
#ifdef SCREENSAVER_LOG_ENABLED
            _LIT( msg, "First Notification" );
            PrintDebugMsg( msg );    
#endif

            // Expire screen saver here..., no note       
            if ( iScreenSaverHost )
                {
                SendEndEvent();
                // Delete the Plugin
                DeletePlugin();
                iScreenSaverHost->Suspend(-1);
                }
            }
        else
            {
#ifdef SCREENSAVER_LOG_ENABLED
            _LIT( msg, "Second Notification" );
            PrintDebugMsg( msg );    
#endif

            // Show expiration notes...  
            iDrmHelper->SetAutomatedType( 
                CDRMHelper::EAutomatedTypeScreenSaver );
            iDrmHelper->CheckRightsAmountL( iSettingObject->iFileName );
            UnregisterForDrmExpiry();
            }
        }
    } 

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::FindAndStartPluginL
// -----------------------------------------------------------------------------
//   
void CScreenSaverAnimPlugin::FindAndStartPluginL( const TDesC& aFileName,
                                                  TBool aCheckFileMode )
    {
    FindPluginL( aFileName );
    CreatePluginL( aCheckFileMode );
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::FindPluginL
// -----------------------------------------------------------------------------
//   
void CScreenSaverAnimPlugin::FindPluginL( const TDesC& aFileName )
    {

    RApaLsSession ls;
    CleanupClosePushL( ls );
    TUid dummyUid( KNullUid );
    TDataType dataType;
    User::LeaveIfError( ls.Connect() );
    User::LeaveIfError( ls.AppForDocument( aFileName, 
                dummyUid, dataType ) );
    CleanupStack::PopAndDestroy( &ls );

#ifdef SCREENSAVER_LOG_ENABLED
    _LIT( msg, "before FindAnimPluginL" );
    PrintDebugMsg( msg );    
#endif

    iPluginUID = FindAnimPluginL( dataType.Des() );

#ifdef SCREENSAVER_LOG_ENABLED
    _LIT( msg1, "After FindAnimPluginL" );
    PrintDebugMsg( msg1 );
#endif

    if ( iPluginUID == KNullUid )
        {
        User::Leave( KErrNotFound );
        }

#ifdef SCREENSAVER_LOG_ENABLED
    _LIT( registermsg, "Before RegisterForDrmExpiry" );
    PrintDebugMsg( registermsg );
#endif    
    
    RegisterForDrmExpiry(); 

#ifdef SCREENSAVER_LOG_ENABLED
    _LIT( registermsg1, "After RegisterForDrmExpiry" );
    PrintDebugMsg( registermsg1 );
#endif

    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::CreatePluginL
// -----------------------------------------------------------------------------
//   
void CScreenSaverAnimPlugin::CreatePluginL( TBool aCheckFileMode )
    {    
    
    /* Create the Netscape plugin for the animation format.
    * Allocate memory for the plugin functions array. This
    * would be filled by the netscape plugin.
    * Initialize the plugin.
    */

    iPluginFuncs = 
         ( NPPluginFuncs* ) User::AllocL( sizeof( NPPluginFuncs ) );
    Mem::FillZ( ( void* )iPluginFuncs, sizeof( NPPluginFuncs ) );

    iNPPinstance = ( NPP ) User::AllocL( sizeof( NPP_t ) );
    Mem::FillZ( ( void* )iNPPinstance, sizeof( NPP_t ) );

    iPluginType = ( HBufC8* )User::AllocL( sizeof( NPMIMEType ) );
    Mem::FillZ( ( void* )iPluginType, sizeof( NPMIMEType ) );

    TRAPD( err, iPluginInterface = 
    CEcomBrowserPluginInterface::CreatePluginL( iPluginUID,
          ( NPNetscapeFuncs* ) ( &NpnImplementationFuncs ), 
          iPluginFuncs ) );

    if ( err != KErrNone )
        {
        SetPluginDeleted();
        User::Leave( err );
        }

#ifdef SCREENSAVER_LOG_ENABLED
    _LIT( newpmsg, "Before newp" );
    PrintDebugMsg( newpmsg );
#endif

    // Create an instance.
    // Flash Plugin needs the NPInteractionMode in the newp

    CDesCArrayFlat* attributeNames = new( ELeave ) CDesCArrayFlat( 1 );
    CDesCArrayFlat* attributeValues = new( ELeave ) CDesCArrayFlat( 1 );
    attributeNames->AppendL( KAttributeInteractionMode );
    attributeValues->AppendL( KValueScreensaver );

    User::LeaveIfError( iPluginFuncs->newp( *iPluginType, iNPPinstance, 
    0 /*mode*/, 
    attributeNames /*argn*/, 
    attributeValues /*argv*/, 
    NULL /*saved*/ ) );

    delete attributeNames;
    delete attributeValues;

#ifdef SCREENSAVER_LOG_ENABLED
    _LIT( newpmsg1, "After newp" );
    PrintDebugMsg( newpmsg1 );
#endif

    // Set ScreenSaver Mode
    if ( iPluginFuncs )
        {
        NPInteractionMode mode;
        mode = NPScreenSaverMode;
        iPluginFuncs->setvalue( iNPPinstance, 
                                ( NPNVariable )NPNInteractionMode, 
                                &mode );
        }

    if ( !aCheckFileMode )
        {
        SetWindow( iParentControl );
        AsFile();
        }

    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::FindAnimPluginL
// -----------------------------------------------------------------------------
//     
TUid CScreenSaverAnimPlugin::FindAnimPluginL( const TDesC& aDataType )
    {    

    TUid impUid = KNullUid;    

    TResourceReader reader;
    iEikEnv->CreateResourceReaderLC( reader, R_MIMETYPE_SPECIFIC_PLUGIN_ARRAY );
    TInt countResource = reader.ReadInt16();
    
    for ( TInt i = 0; i < countResource; i++ )
        {        
        HBufC* newMimeTypeEntry = reader.ReadHBufCL();            
        TPtr mimePtr( newMimeTypeEntry->Des() );
        impUid.iUid = reader.ReadInt32();
        if ( newMimeTypeEntry->Compare( aDataType ) == 0 )
            {     
            delete newMimeTypeEntry;                                                
            break;
            }
        else
            {
            delete newMimeTypeEntry;                                    
            }
        }
    CleanupStack::PopAndDestroy(); // reader      

    if ( CheckIfPluginPresentL( KBrowserPluginInterfaceUid, impUid ) ||        
         CheckIfPluginPresentL( TUid::Uid( KScreensaverAnimPluginInterfaceUid ),impUid ) )
        {
        return impUid;
        }               

    return KNullUid;   
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::CheckIfPluginPresentL
// -----------------------------------------------------------------------------
//     
TBool CScreenSaverAnimPlugin::CheckIfPluginPresentL( TUid aInterfaceUid , 
                                                    TUid aImpUid )
    {
    TBool ret( EFalse );
    RImplInfoPtrArray animPluginList;   

    REComSession::ListImplementationsL( aInterfaceUid, animPluginList );
                                        
    const TInt count = animPluginList.Count();

    for ( TInt i = 0; i < count; i++ )
        {      
        CImplementationInformation* implInfo = animPluginList[i];
        if ( aImpUid == implInfo->ImplementationUid() )
            {          
            ret = ETrue;
            break;         
            }
        }
    animPluginList.ResetAndDestroy();          
    return ret;
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::ParseMimeStringL
// -----------------------------------------------------------------------------
// 
TBool CScreenSaverAnimPlugin::ParseMimeStringL( const TDesC& aMIMEDescription,
                                               const TDesC& aDataType )    
    {
    
    if ( aMIMEDescription.Length() == 0 ||
         aDataType.Length() == 0 )
        {
        return EFalse;
        }

    _LIT( KPattern1Ptr, ",;|" );
    _LIT( KPattern2Ptr, ";|" );

    TUint    end( aMIMEDescription.Length() );
    TUint    i( 0 );
    TUint    marker( 0 );
    TUint16  mimeSeparator( '|' );
    TUint    fieldSeparator( ';' );
    HBufC*   newMimeTypeEntry = NULL;

    // Parse the aMIMEDescription string to populate the mimeTypes
    for ( ; i < end; )    // outer for loop
        {
        // Search until end of buffer or match one of the delimiters ';' or '|'.
        // We are looking for the mimeType, ie "text/html", 
        // "application/pdf", etc.
        for ( ; ( i < end ) && ( KPattern2Ptr().Locate( ( aMIMEDescription )[i] ) ==
                             KErrNotFound ); i++ )
            {
            // Walking the aMIMEDescription string
            }

        if ( i > marker )
            {
            // Create new mimeType entry, the first entry is mimeType
            newMimeTypeEntry = HBufC::NewLC( i - marker );          
            TPtr mimePtr( newMimeTypeEntry->Des() );
            mimePtr.Copy( aMIMEDescription.Mid( marker, i - marker ) );
            if ( newMimeTypeEntry->Compare( aDataType ) == 0 )
                {
                CleanupStack::PopAndDestroy( newMimeTypeEntry ); //newMimeTypeEntry
                return ETrue;
                }
            CleanupStack::PopAndDestroy( newMimeTypeEntry ); //newMimeTypeEntry
            }

        // Are we at the end of the supported mime string
        if ( i == end )
            {
            // This break with i=end means we leave outer for loop
            break;
            }

        marker = ++i;
        if ( ( aMIMEDescription )[i - 1] == mimeSeparator )
            {
            // Found a mime separator '|', get next supported mime
            continue;
            }

        // There can be multiple mimeFileExtentions per mimeType
        for ( ; ( i < end ); )  // inner for loop
            {
            // Search until end of buffer or match one of the delimiters 
            // ';' or ',' or '|'.
            for ( ; (i < end) && ( KPattern1Ptr().Locate( ( aMIMEDescription )[i] ) ==
                                    KErrNotFound ); i++ )
                {
                // Walking the aMIMEDescription string
                }

            // Are we at the end of the supported mime string
            if ( i == end )
                {
                // This break means we leave the inner loop, 
                // and with i=end means
                // we leave the outer loop
                break;
                }

            marker = ++i;
            if ( ( aMIMEDescription )[i - 1] == mimeSeparator )
                {
                // Found a mime separator '|', get next supported mime
                break;
                }

            if ( ( aMIMEDescription )[i - 1] == fieldSeparator )
                {
                // Found a field separator ';', get the mimeDescription.
                // Search until end of buffer or match one of the 
                // delimiters ';' or '|'.
                for ( ; (i < end) && 
                    ( KPattern2Ptr().Locate( ( aMIMEDescription )[i] ) ==
                                     KErrNotFound ); i++ )
                    {
                    // Walking the aMIMEDescription string
                    }
          

                // Are we at the end of the supported mime string
                if ( i == end )
                    {
                    // This break means we leave the inner loop, 
                    // and with i=end means
                    // we leave the outer loop
                    break;
                    }

                marker = ++i;
                // Make sure we start at the next mime, 
                // after we found the mimeDescription.
                // We are handling a mime string with an extra semi-colon(s),
                // ie "...mimeDescription1;|mimeType2
                for (; (i < end) && 
                    ((aMIMEDescription)[i-1] != mimeSeparator); i++)
                    {
                    // Walking the aMIMEDescription string
                    }

                // Leave the inner loop and look for the next mime
                break;
                }   // end of if fieldSeparator aka get mimeDescription

            // If we get here, we have another mimeFileExtension. 
            // Continue on the
            // inner loop to find additional mimeFileExtensions.

            }   // end of inner for (;i<end;)
        }   // end of outer for (;i<end;)        

        return EFalse;  

    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::DeletePlugin
// -----------------------------------------------------------------------------
//      
void CScreenSaverAnimPlugin::DeletePlugin()    
    {    
#ifdef SCREENSAVER_LOG_ENABLED                 
    _LIT( msg, "delete called " );    
    PrintDebugMsg( msg );
#endif  
    if ( iPluginFuncs )
        {
        iPluginFuncs->destroy( iNPPinstance, NULL );  
        }    
    SetPluginDeleted();
    }
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::SetPluginDeleted
// -----------------------------------------------------------------------------
//      
void CScreenSaverAnimPlugin::SetPluginDeleted()    
    {

    delete iPluginType;
    iPluginType = NULL;

    delete iPluginFuncs;
    iPluginFuncs = NULL;

    delete iNPPinstance;
    iNPPinstance = NULL;

    delete iPluginInterface;
    iPluginInterface = NULL;    

    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::AsFile
// -----------------------------------------------------------------------------
//    
void CScreenSaverAnimPlugin::AsFile()
    {
    
    // Set the File-name to the plugin.
    // Note that the SVG-T plugin atleast wants setwindow
    // to be called first
    iPluginFuncs->asfile( iNPPinstance, NULL, iSettingObject->iFileName );
    } 

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::UpdateFileName
// -----------------------------------------------------------------------------
//      
TBool CScreenSaverAnimPlugin::UpdateFileName()
    {
    if ( !BaflUtils::FileExists( iEikEnv->FsSession(), 
                                iSettingObject->iFileName ) )
        {
        // File is deleted now, Show empty string
        iSettingObject->iFileName.Copy( KEmptyString );        
        return ETrue;
        }

    TBool canbeautomated( EFalse );   

    TInt res = 0;
    iDrmHelper->SetAutomatedType( 
                   CDRMHelper::EAutomatedTypeScreenSaver );
    res = iDrmHelper->CanSetAutomated( iSettingObject->iFileName, 
                                       canbeautomated );

    if ( res || !canbeautomated )
        {
        iSettingObject->iFileName.Copy( KEmptyString );
        return ETrue;
        }
    return EFalse;
    }
    
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::PopSettingsDialogL
// -----------------------------------------------------------------------------
//      
void CScreenSaverAnimPlugin::PopSettingsDialogL()
    {
#ifdef SCREENSAVER_LOG_ENABLED             
    _LIT( msg, "Pop Settings Dialog" );    
    PrintDebugMsg( msg );
#endif    

    // Get the Status Pane Control
    CEikStatusPane* sp = iEikEnv->AppUiFactory()->StatusPane();

    // Fetch pointer to the default navi pane control
    CAknNavigationControlContainer* NaviPane = 
        ( CAknNavigationControlContainer* )sp->ControlL(
        TUid::Uid( EEikStatusPaneUidNavi ) );
       
    NaviPane->PushDefaultL( ETrue );                   

    CScreenSaverAnimSettingDialog* dlg = 
       CScreenSaverAnimSettingDialog::NewL( this, iSettingObject );

#ifdef SCREENSAVER_LOG_ENABLED             
    _LIT( msg1, "dlg created Call executeld" );    
    PrintDebugMsg( msg1 );
#endif    

    dlg->ExecuteLD();

    NaviPane->Pop();

#ifdef SCREENSAVER_LOG_ENABLED             
    _LIT( msg2, "Executed dlg, write to cenrep" );    
    PrintDebugMsg( msg2 );
#endif    
    
    User::LeaveIfError( SetAnimSettings() );
    
    // Check if a file is selected        
    if ( iSettingObject->iFileName.Length() == 0 )
        {
        // Don't Pop an error note
        // Just leave
        User::Leave( KErrCancel ) ;           
        }   
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::HandleRepositoryCallBack
// -----------------------------------------------------------------------------
//                
TInt CScreenSaverAnimPlugin::HandleRepositoryCallBack( TAny* aPtr )    
    {
    STATIC_CAST( CScreenSaverAnimPlugin*, aPtr )->HandleSettingsChange( aPtr );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::HandleRepositoryCallBack
// -----------------------------------------------------------------------------
//                
TInt CScreenSaverAnimPlugin::HandleDrmNotification( TAny* aPtr )    
    {
    TRAPD( err, STATIC_CAST( CScreenSaverAnimPlugin*, aPtr )
                        ->ProcessDrmNotificationL() );   
    return err;
    }    

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::HandleSettingsChange
// -----------------------------------------------------------------------------
//                
void CScreenSaverAnimPlugin::HandleSettingsChange( TAny* /*aPtr*/ )    
    {    
    if ( iScreensaverRepositoryWatcher )
        {
        if ( iScreensaverRepositoryWatcher->ChangedKey() == 
                             KScreenSaverAnimationTimeOut )
            {
            // Update the iSettingObject
            iAnimPluginSession->Get( KScreenSaverAnimationTimeOut,
                          iSettingObject->iAnimationTimeout );            
            } 
        if ( iScreensaverRepositoryWatcher->ChangedKey() ==                  
                             KScreenSaverLightsTimeOut )
            {
            // Update the iSettingObject 
            iAnimPluginSession->Get( KScreenSaverLightsTimeOut,  
                      iSettingObject->iLightsTimeout );
            } 
        
        if ( iScreensaverRepositoryWatcher->ChangedKey() ==                  
                             KScreenSaverAnimatedFileName )
            {
            UnregisterForDrmExpiry();
            iAnimPluginSession->Get( KScreenSaverAnimatedFileName,
                           iSettingObject->iFileName );
            // Find the Plugin for the new file
            TInt err;
            TRAP( err, FindPluginL( iSettingObject->iFileName ) );
            if ( err != KErrNone )
                {
                NotifyCenrepChangeCancel();
                iScreenSaverHost->RevertToDefaultSaver();
                return;
                }
            }
        }
    }
    
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::InitializeCenRepL
// -----------------------------------------------------------------------------
//  
void CScreenSaverAnimPlugin::InitializeCenRepL()
    {
    
    TRAPD( err, iAnimPluginSession = 
         CRepository::NewL( KCRUidScreenSaverAnimPlugin ) );
#ifdef SCREENSAVER_LOG_ENABLED         
    TBuf<KMaxBufLen> buffer;
    _LIT( msg, "After SS Cenrep Init : %d" );
    buffer.Format( msg, err );
    PrintDebugMsg( buffer );      
#endif    
    User::LeaveIfError( err );

    TRAPD( err1, iDrmSession = CRepository::NewL( KCRUidDRMHelperServer ) );
#ifdef SCREENSAVER_LOG_ENABLED             
    TBuf<KMaxBufLen> buffer1;
    _LIT( msg1, "After DRM Cenrep Init : %d" );
    buffer1.Format( msg1, err1 );
    PrintDebugMsg( buffer1 );     
#endif    
    User::LeaveIfError( err1 );       

    }
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::NotifyCenrepChangeL
// -----------------------------------------------------------------------------
//  
void CScreenSaverAnimPlugin::NotifyCenrepChangeL()
    { 
    
    iScreensaverRepositoryWatcher = CScreensaverRepositoryWatcher::NewL(
            KCRUidScreenSaverAnimPlugin,
            TCallBack( HandleRepositoryCallBack, this ),
            iAnimPluginSession );

    iDrmExpiryWatcher = CScreensaverRepositoryWatcher::NewL(
            KCRUidDRMHelperServer,
            KDRMHelperServerNotification,
            CCenRepNotifyHandler::EIntKey,
            TCallBack( HandleDrmNotification, this ),
            iDrmSession );                             
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::NotifyCenrepChangeCancel
// -----------------------------------------------------------------------------
//   
void CScreenSaverAnimPlugin::NotifyCenrepChangeCancel()
    {

    delete iScreensaverRepositoryWatcher;
    iScreensaverRepositoryWatcher = NULL;

    delete iDrmExpiryWatcher;     
    iDrmExpiryWatcher = NULL;

    }
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::UnInitializeCenRep
// -----------------------------------------------------------------------------
//   
void CScreenSaverAnimPlugin::UnInitializeCenRep()
    {       
    if ( iAnimPluginSession )
        {
        delete iAnimPluginSession;
        iAnimPluginSession = NULL;  
        } 
     if ( iDrmSession )
        {
        delete iDrmSession;
        iDrmSession = NULL; 
        } 
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::CScreenSaverAnimPlugin
// -----------------------------------------------------------------------------
//
CScreenSaverAnimPlugin::CScreenSaverAnimPlugin():
    iParentControl( NULL ),iPluginError( KErrNone )
    {
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::ConstructL
// -----------------------------------------------------------------------------
//    
void CScreenSaverAnimPlugin::ConstructL()
    {   
#ifdef SCREENSAVER_LOG_ENABLED             
    _LIT( msg, "ConstructL of ScreenSaverAnimPlugin" );    
    PrintDebugMsg( msg );
#endif    
    
    // Find which drive this DLL is installed.
    TFileName fileName;

    // Get the full resource file
    TParse lParse;
    
    // Add the resource dir
    lParse.Set( KResourceFileName, &KDC_RESOURCE_FILES_DIR, NULL );

    // Get the filename with full path
    fileName = lParse.FullName();

    iEikEnv = CEikonEnv::Static();

    BaflUtils::NearestLanguageFile( iEikEnv->FsSession(), fileName ); //for
                                                               // localization

    iDrmHelper = CDRMHelper::NewL( *iEikEnv );
    
#ifdef SCREENSAVER_LOG_ENABLED             
    _LIT( msg1, "Before Cenrep Init " );    
    PrintDebugMsg( msg1 );
#endif    

    InitializeCenRepL();

#ifdef SCREENSAVER_LOG_ENABLED 
    _LIT( msg2, "After Cenrep Init " );
    PrintDebugMsg( msg2 );
#endif    

    iSettingObject = new( ELeave ) CScreenSaverAnimSettingObject;

    GetAnimSettings();

    iResourceOffset = iEikEnv->AddResourceFileL( fileName );  
    
#ifdef SCREENSAVER_LOG_ENABLED             
    _LIT( msg3, "ConstructL end " );    
    PrintDebugMsg( msg3 );
#endif

    // Load localised name of screensaver to be returned for
    // Themes app in Name() query
    iScreensaverName = iEikEnv->AllocReadResourceL( R_SANIM_SCREENSAVER_NAME );
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::SendEndEvent
// -----------------------------------------------------------------------------
//       
TInt CScreenSaverAnimPlugin::SendEndEvent()
    { 
    NPAnimationEvent event;
    event = NPEndAnimation;
    if ( iPluginFuncs )
        {
        return iPluginFuncs->setvalue( iNPPinstance, 
                                ( NPNVariable )NPNScreenSaverAnimationEvent,
                                &event );
        }
    return KErrNone;

    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::CheckFileValidity
// -----------------------------------------------------------------------------
//       
TInt CScreenSaverAnimPlugin::CheckFileValidityL( const TDesC& aFileName )
    {
    FindAndStartPluginL( aFileName , ETrue /* Check File Mode */); 
    NPCheckFile checkFile;
    checkFile.fileValidityCheckError = KErrNone;
    checkFile.fileName = HBufC::NewL( aFileName.Length() );
    checkFile.fileName->Des().Copy( aFileName );
    
    if ( iPluginFuncs )
        {
        iPluginFuncs->getvalue( iNPPinstance, 
                                ( NPPVariable )NPPCheckFileIsValid,
                                &checkFile );
        } 
    delete checkFile.fileName; 
    DeletePlugin();
    return checkFile.fileValidityCheckError;

    }

/* MPluginAdapter Interface functions, The plugins call 
 * these back
 */    
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::GetVersion
// -----------------------------------------------------------------------------
//
TUint16 CScreenSaverAnimPlugin::GetVersion()
    {
    return 0;
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::PluginConstructedL
// Called when the plug-in has completed its creation.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimPlugin::PluginConstructedL( CCoeControl* 
                                                 /*aPluginControl*/ )
    {
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::PluginFinishedL
// Called when the plug-in has completed its creation.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimPlugin::PluginFinishedL()
    {
    TInt error;
    iPluginFuncs->getvalue( iNPPinstance, 
                            ( NPPVariable )NPPScreenSaverGetLastError,
                            &error );

#ifdef SCREENSAVER_LOG_ENABLED
    TBuf<KMaxBufLen> buffer1;
    _LIT( msg, "Plugin finishedL Error: %d" ); 
    buffer1.Format( msg, error ); 
    PrintDebugMsg( buffer1 );
#endif    

    if ( iScreenSaverHost )
        {
        if ( error != KErrNone )
            {
            iPluginError = error;
            // Plugin has encountered an error
            iScreenSaverHost->RevertToDefaultSaver();       
            }
        }
    }

#ifndef __SERIES60_30__
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::OpenUrlL
// Called by the plug-in to open a URL in the parent frame.
// This function is not implemented in Series 60. It is supported 
// in Series 80 and in the Nokia 7710 smartphone
// -----------------------------------------------------------------------------
//
       
void CScreenSaverAnimPlugin::OpenUrlL( const TDesC& /* aUrl */ )
    {
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::OpenUrlInTargetWindowL
// Called by the plug-in to open a URL in a named window or frame.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimPlugin::OpenUrlInTargetWindowL( const TDesC& /* aUrl */, 
    const TDesC& /* aNewTargetWindowName */ )
    {
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::HasNavigationHistoryPrevious
// Specifies whether or not the frame has a previous navigation history item.
// -----------------------------------------------------------------------------
//
TBool CScreenSaverAnimPlugin::HasNavigationHistoryPrevious() const
    {
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::LoadNavigationHistoryPreviousL
// Informs the browser to load the next navigation history URL in its parent 
// frame.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimPlugin::LoadNavigationHistoryPreviousL()
    {
    }
    
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::HasNavigationHistoryNext
// Returns whether the frame has a next navigation history item. 
// -----------------------------------------------------------------------------
//
TBool CScreenSaverAnimPlugin::HasNavigationHistoryNext() const
    {
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::LoadNavigationHistoryNextL
// Informs the browser to load the next navigation history URL in its parent 
// frame.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimPlugin::LoadNavigationHistoryNextL()
    {
    }

#endif

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::GetParentControl
// Returns the window system level control object for the plug-in.
// -----------------------------------------------------------------------------
//
CCoeControl* CScreenSaverAnimPlugin::GetParentControl()
    {
    return iParentControl;
    }
    
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::GetParentControlObserver
// Returns the observer for the plug-in control.
// -----------------------------------------------------------------------------
//
MCoeControlObserver* CScreenSaverAnimPlugin::GetParentControlObserver()
    {
    return this;
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::GetParentControlObserver
// Sets the plug-in notifier allowing the plug-in to control 
// portions of the browser.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimPlugin::SetPluginNotifier( MPluginNotifier* /*aNotifier*/ )
    {
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::SetContentInteractive
// Tells the plugin adapter if plugin content can accept user interaction.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimPlugin::SetContentInteractive( TBool /*aInteractive*/ )
    {
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::HandleControlEventL
// Handles an event from an observed control.This function is called 
// when a control for which this control is the observer calls 
// CCoeControl::ReportEventL(). It should be implemented by the observer 
// control, and should handle all events sent by controls it observes.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimPlugin::HandleControlEventL( CCoeControl* /*aControl*/,
    TCoeEvent /*aEventType*/ )
    {
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::CheckContentIdL
// -----------------------------------------------------------------------------
//    
TBool CScreenSaverAnimPlugin::CheckContentIdL( const TDesC& aFileName, 
                                               TDesC8& aContentUri )
    {

    TBool matched = EFalse;
    HBufC* contentURI = NULL;
    HBufC8* contentURI8 = NULL;

    RFile fileHandle;
    
    TInt error = fileHandle.Open( iEikEnv->FsSession(), 
                                aFileName, 
                                EFileRead | EFileShareReadersOnly );   
    if ( error != KErrNone )
        {
        return matched;
        }

    CleanupClosePushL( fileHandle ); 
    CData* content = CData::NewL( fileHandle, KDefaultContentObject, EPeek );
    CleanupStack::PopAndDestroy( &fileHandle );

    CleanupStack::PushL( content );

    contentURI = HBufC::NewLC( KUrlMaxLen );

    // create attribute set
    RStringAttributeSet stringAttributeSet;
    CleanupClosePushL( stringAttributeSet );
    // add the attributes we are interested in
    stringAttributeSet.AddL( EContentID );

    User::LeaveIfError( content->GetStringAttributeSet( stringAttributeSet ) );

    // pass on values of string attributes
    TPtr ptr = contentURI->Des();
    error = stringAttributeSet.GetValue( EContentID, ptr );
    if ( error == KErrNone )
        {
        contentURI8 = HBufC8::NewLC( contentURI->Length() );
        contentURI8->Des().Copy( contentURI->Des() );
        if ( contentURI8->Compare( aContentUri ) == 0 )
            {
            matched = ETrue;
            }
        CleanupStack::PopAndDestroy( contentURI8 ); 
        }

    CleanupStack::PopAndDestroy( 2, contentURI ); // stringAttributeSet.Close()
    CleanupStack::PopAndDestroy( content );

    return matched;
    }

// End Of file.

