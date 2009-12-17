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
* Description:   This file implements all helper classes 
*                for ScreenSaverAnimPlugin
*
*/



// INCLUDE FILES

#include    <DRMHelper.h>
#include    <coemain.h>
#include    <apgcli.h>
#include    <barsread.h>
#include    <aknnotewrappers.h>
#include    <centralrepository.h>

#include    <screensaveranimplugin.rsg>

#include    "ScreenSaverAnimPluginHelper.h"
#include    "ScreenSaverAnimPlugin.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScreenSaverAnimPluginVerifier::CScreenSaverAnimPluginVerifier
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CScreenSaverAnimPluginVerifier::CScreenSaverAnimPluginVerifier(
             CScreenSaverAnimPlugin* aFileVerify ) :
             iFileVerify( aFileVerify )
    {
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPluginVerifier::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimPluginVerifier::ConstructL()
    {
    iCoeEnv = CCoeEnv::Static();
    iDRMHelper = CDRMHelper::NewL( *iCoeEnv );
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPluginVerifier::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScreenSaverAnimPluginVerifier* CScreenSaverAnimPluginVerifier::NewL(
                                CScreenSaverAnimPlugin* aFileVerify )
    {
    CScreenSaverAnimPluginVerifier* self = new( ELeave ) 
                            CScreenSaverAnimPluginVerifier( aFileVerify );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }
    
// -----------------------------------------------------------------------------
// CScreenSaverAnimPluginVerifier::~CScreenSaverAnimPluginVerifier
// Destructor.
// -----------------------------------------------------------------------------
//
CScreenSaverAnimPluginVerifier::~CScreenSaverAnimPluginVerifier()
    {
    delete iDRMHelper;
    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPluginVerifier::IsFileValidL
// Empty implementation.
// -----------------------------------------------------------------------------
//
TBool CScreenSaverAnimPluginVerifier::VerifySelectionL( const MDesCArray* 
                                                        aSelectedFiles )
    {    

    TBool canbeautomated( EFalse );

    if( aSelectedFiles->MdcaCount()<1 )
        {
        // Download item
        return ETrue;
        }

    // 1. Check whether the drm rights are ok
    TInt res = 0;
    iDRMHelper->SetAutomatedType( 
                   CDRMHelper::EAutomatedTypeScreenSaver );
    res = iDRMHelper->CanSetAutomated( aSelectedFiles->MdcaPoint( 0 ), canbeautomated );

    if ( res )
        {
        iDRMHelper->HandleErrorL( res, aSelectedFiles->MdcaPoint( 0 ) );        
        return canbeautomated;
        }   
    else if ( !canbeautomated )
        {
        ShowErrorNoteL( R_DRM_PREV_RIGHTS_SET );
        return canbeautomated;
        }

    // 2. Check whether the file is supported
    RApaLsSession ls;
    CleanupClosePushL( ls );
    TUid dummyUid( KNullUid );
    TDataType dataType;
    User::LeaveIfError( ls.Connect() );
    // find the mimetype
    User::LeaveIfError( ls.AppForDocument( aSelectedFiles->MdcaPoint( 0 ), 
                dummyUid, dataType ) );               
                    
    CleanupStack::PopAndDestroy( &ls );

    TResourceReader reader;
    TBool valid( EFalse );
    iCoeEnv->CreateResourceReaderLC( reader, 
                             R_MIMETYPE_SPECIFIC_PLUGIN_ARRAY );
    TInt countResource = reader.ReadInt16();

    for ( TInt i = 0; i < countResource; i++ )
        {
        HBufC* newMimeTypeEntry = reader.ReadHBufCL();        
        TPtr mimePtr( newMimeTypeEntry->Des() );
        reader.ReadInt32();
        if ( newMimeTypeEntry->Compare( dataType.Des() ) == 0 )
            {            
            valid =  ETrue;
            }
        delete newMimeTypeEntry;        
        }
    CleanupStack::PopAndDestroy(); // reader  

    if ( !valid )
        {
        // Pop a not supported error message             
        ShowErrorNoteL( R_SANIM_NOT_SUPPORTED );    
        return EFalse;
        }

    //3. Check file for Validity
    
    //Display the confirmation query when setting 
    //a DRM protected image as screensaver in General settings.
    TInt isCancel;
    isCancel = iDRMHelper->ShowAutomatedNote( aSelectedFiles->MdcaPoint( 0 ) ); 
    if ( KErrCancel == isCancel )
        {
        return EFalse;
        }

    
    TInt validity = KErrNone;
    TRAPD( err, 
       validity = iFileVerify->CheckFileValidityL( aSelectedFiles->MdcaPoint( 0 ) ));     

    if ( err )
        {
        // Dll may not be present
        ShowErrorNoteL( R_SANIM_NOT_SUPPORTED );
        return EFalse;
        }           

    if ( validity == KErrNoMemory )
        {
        // Pop up a Corrupted File message
        CEikonEnv::Static()->HandleError( validity );
        return EFalse;        
        }   
    else  if ( validity != KErrNone )
        {
        TInt resourceId = R_SANIM_ERROR_WRONG_FORMAT;
        // Show information note
        ShowErrorNoteL( resourceId );    
        return EFalse;        
        }

    return valid;

    }

// -----------------------------------------------------------------------------
// CScreenSaverAnimPluginVerifier::ShowErrorNoteL
// Displays the error note
// -----------------------------------------------------------------------------
//
void CScreenSaverAnimPluginVerifier::ShowErrorNoteL( TInt  aResourceId  ) const
    {
    HBufC* errorText = 
        iCoeEnv->AllocReadResourceLC( aResourceId );
    CAknInformationNote* note = new( ELeave ) CAknInformationNote( EFalse );
    note->ExecuteLD( *errorText );
    CleanupStack::PopAndDestroy( errorText ); // errorText
    }

// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::NewL
// -----------------------------------------------------------------------------
//
CScreensaverRepositoryWatcher* CScreensaverRepositoryWatcher::NewL(
    const TUid aUid,
    const TUint32 aKey,
    CCenRepNotifyHandler::TCenRepKeyType aKeyType,
    TCallBack aCallBack,
    CRepository* aRepository )
    {
    CScreensaverRepositoryWatcher* self = new( ELeave ) 
          CScreensaverRepositoryWatcher( aUid, aKey, aCallBack, aRepository );

    CleanupStack::PushL( self );
    self->ConstructL( aKeyType );
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::NewL
// -----------------------------------------------------------------------------
//
CScreensaverRepositoryWatcher* CScreensaverRepositoryWatcher::NewL(
    const TUid aUid,
    TCallBack aCallBack,
    CRepository* aRepository )
    {
    CScreensaverRepositoryWatcher* self = new( ELeave ) CScreensaverRepositoryWatcher(
        aUid, NCentralRepositoryConstants::KInvalidNotificationId, aCallBack, aRepository );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }


// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::~CScreensaverRepositoryWatcher
// -----------------------------------------------------------------------------
//
CScreensaverRepositoryWatcher::~CScreensaverRepositoryWatcher()
    {
    if ( iNotifyHandler )
        {
        iNotifyHandler->StopListening();
        delete iNotifyHandler;      
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::CScreensaverRepositoryWatcher
// -----------------------------------------------------------------------------
//
CScreensaverRepositoryWatcher::CScreensaverRepositoryWatcher(
    const TUid aUid,
    const TUint32 aKey,
    TCallBack aCallBack,
    CRepository* aRepository )
    :
    iUid( aUid ), iKey( aKey ), iCallBack( aCallBack ), iRepository( aRepository )
    {
    }


// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverRepositoryWatcher::ConstructL(CCenRepNotifyHandler::TCenRepKeyType aKeyType)
    {
    iNotifyHandler = CCenRepNotifyHandler::NewL( *this, *iRepository, aKeyType, iKey );
    iNotifyHandler->StartListeningL();
    }


// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverRepositoryWatcher::ConstructL()
    {
    iNotifyHandler = CCenRepNotifyHandler::NewL( *this, *iRepository );
    iNotifyHandler->StartListeningL();
    }


// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::ChangedKey
// -----------------------------------------------------------------------------
//
TUint32 CScreensaverRepositoryWatcher::ChangedKey()
    {
    return iChangedKey;
    }

// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::HandleNotifyInt
// -----------------------------------------------------------------------------
//
void CScreensaverRepositoryWatcher::HandleNotifyInt( 
                                    TUint32 aKey, TInt /*aNewValue*/ )
    {
    iChangedKey = aKey;
    iCallBack.CallBack();
    iChangedKey = NCentralRepositoryConstants::KInvalidNotificationId;
    }

// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::HandleNotifyString
// -----------------------------------------------------------------------------
//
void CScreensaverRepositoryWatcher::HandleNotifyString( TUint32 aKey, 
                                                     const TDesC16& /*aNewValue*/ )
    {  
    iChangedKey = aKey;
    iCallBack.CallBack();
    iChangedKey = NCentralRepositoryConstants::KInvalidNotificationId;
    }

// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::HandleNotifyGeneric
// -----------------------------------------------------------------------------
//
void CScreensaverRepositoryWatcher::HandleNotifyGeneric(TUint32 aKey)
    {
    iChangedKey = aKey;
    iCallBack.CallBack();
    iChangedKey = NCentralRepositoryConstants::KInvalidNotificationId;
    }

// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::HandleNotifyError
// -----------------------------------------------------------------------------
//
void CScreensaverRepositoryWatcher::HandleNotifyError(TUint32 /*aKey*/, TInt /*aError*/, CCenRepNotifyHandler* /*aHandler*/)
    {
    }     

// End of File
