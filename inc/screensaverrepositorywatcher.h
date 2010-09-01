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
* Description:   CScreensaverRepositoryWatcher declaration.
*
*/



#ifndef C_SCREENSAVERREPOSITORYWATCHER_H
#define C_SCREENSAVERREPOSITORYWATCHER_H

// INCLUDES
#include <cenrepnotifyhandler.h>

// CLASS DECLARATION
class CRepository;
/**
* The class used to monitor the change of register value
*/
class CScreensaverRepositoryWatcher : public CBase,
    public MCenRepNotifyHandlerCallback
    {
public:
    
    /**
    * Two-phased constructor
    */
    static CScreensaverRepositoryWatcher* NewL(const TUid aUid,
            const TUint32 aKey, CCenRepNotifyHandler::TCenRepKeyType aKeyType,
            TCallBack aCallBack, CRepository* aRepository);

    /**
    * Two-phased constructor
    */
    static CScreensaverRepositoryWatcher* NewL(const TUid aUid,
            TCallBack aCallBack, CRepository* aRepository);

    /**
    * Destructor
    */
    ~CScreensaverRepositoryWatcher();

    /**
    * Returns the changed key
    */
    TUint32 ChangedKey();

public:
    // from MCenRepNotifyHandlerCallback
    void HandleNotifyInt(TUint32 aKey, TInt aNewValue);
    void HandleNotifyString(TUint32 aKey, const TDesC16& aNewValue);
    void HandleNotifyGeneric(TUint32 aKey);
    void HandleNotifyError(TUint32 aKey, TInt aError,
            CCenRepNotifyHandler* aHandler);

private:
    
    /**
    * C++ default constructor
    */
    CScreensaverRepositoryWatcher(const TUid aUid, const TUint32 aKey,
            TCallBack aCallBack, CRepository* aRepository);

    /**
    * Two-phased constructor
    */
    void ConstructL(CCenRepNotifyHandler::TCenRepKeyType aKeyType);

    /**
    * Two-phased constructor
    */
    void ConstructL();

private:
    TUid iUid;
    TUint32 iKey;
    TUint32 iChangedKey;
    TCallBack iCallBack;
    CRepository* iRepository;
    CCenRepNotifyHandler* iNotifyHandler;

    };

#endif // C_SCREENSAVERREPOSITORYWATCHER_H
