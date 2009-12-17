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
* Description:   This file declares all helper classes 
*                for ScreenSaverAnimPlugin
*
*/



#ifndef C_SCREENSAVERANIMPLUGINHELPER_H
#define C_SCREENSAVERANIMPLUGINHELPER_H

//  INCLUDES
#include <e32base.h>
#include <MMGFetchVerifier.h>
#include <cenrepnotifyhandler.h>

// FORWARD DECLARATIONS
class CDRMHelper;
class CCoeEnv;
class CScreenSaverAnimPlugin;
class CRepository;

// CLASS DECLARATION

/**
* Implementation for File Validity Check.
* @since 3.1
* @internal
*/
class CScreenSaverAnimPluginVerifier : public CBase, public MMGFetchVerifier
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CScreenSaverAnimPluginVerifier* NewL( 
                             CScreenSaverAnimPlugin* aFileVerify );

        /**
        * Destructor.
        */
        ~CScreenSaverAnimPluginVerifier();

    public: // Functions from base classes

        /**
        * From MFLDFileObserver.
        */
        TBool VerifySelectionL( const MDesCArray* aSelectedFiles );

    private:

        /**
        * C++ default constructor.
        */
        CScreenSaverAnimPluginVerifier( CScreenSaverAnimPlugin* aFileVerify );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
        void ShowErrorNoteL( TInt  aResourceId  ) const;

    private:    // Data
        // Own: DRM helper
        CDRMHelper* iDRMHelper;
        
        // Environment pointer
        CCoeEnv* iCoeEnv;

        CScreenSaverAnimPlugin* iFileVerify;

    };
    
    
// CLASS DECLARATION

/**
* Helper Class to watch Central Repository Changes.
* @since 3.1
* @internal
*/    
class CScreensaverRepositoryWatcher : public CBase, public MCenRepNotifyHandlerCallback
    {
public:
    static CScreensaverRepositoryWatcher* NewL(
        const TUid aUid,
        const TUint32 aKey,
        CCenRepNotifyHandler::TCenRepKeyType aKeyType,
        TCallBack aCallBack,
        CRepository* aRepository );

    static CScreensaverRepositoryWatcher* NewL(
        const TUid aUid,
        TCallBack aCallBack,
        CRepository* aRepository );

    ~CScreensaverRepositoryWatcher();

    TUint32 ChangedKey();

public: // from MCenRepNotifyHandlerCallback
    void HandleNotifyInt    ( TUint32 aKey, TInt aNewValue );
    void HandleNotifyString ( TUint32 aKey, const TDesC16& aNewValue );
    void HandleNotifyGeneric( TUint32 aKey );
    void HandleNotifyError  ( TUint32 aKey, TInt aError, CCenRepNotifyHandler* aHandler );

private:
    CScreensaverRepositoryWatcher(
        const TUid aUid,
        const TUint32 aKey,
        TCallBack aCallBack,
        CRepository* aRepository );

    void ConstructL( CCenRepNotifyHandler::TCenRepKeyType aKeyType );

    void ConstructL();

private:
    TUid                    iUid;
    TUint32                 iKey;
    TUint32                 iChangedKey;
    TCallBack               iCallBack;
    CRepository*            iRepository;
    CCenRepNotifyHandler*   iNotifyHandler;

    };

#endif // C_SCREENSAVERANIMPLUGINHELPER_H
            
// End of File
