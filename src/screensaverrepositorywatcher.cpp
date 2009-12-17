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
* Description:   CScreensaverRepositoryWatcher implementation.
*
*/



#include <centralrepository.h>

#include "screensaverrepositorywatcher.h"
#include "ScreensaverUtils.h"
//
// CScreensaverRepositoryWatcher
//
// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::NewL
// -----------------------------------------------------------------------------
//
CScreensaverRepositoryWatcher* CScreensaverRepositoryWatcher::NewL(
    const TUid aUid,
    const TUint32 aKey,
    CCenRepNotifyHandler::TCenRepKeyType aKeyType,
    TCallBack aCallBack,
    CRepository* aRepository)
    {
    CScreensaverRepositoryWatcher* self = new(ELeave) CScreensaverRepositoryWatcher(aUid, aKey, aCallBack, aRepository);

    CleanupStack::PushL(self);
    self->ConstructL(aKeyType);
    CleanupStack::Pop(self);

    return self;
    }


// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::NewL
// -----------------------------------------------------------------------------
//
CScreensaverRepositoryWatcher* CScreensaverRepositoryWatcher::NewL(
    const TUid aUid,
    TCallBack aCallBack,
    CRepository* aRepository)
    {
    CScreensaverRepositoryWatcher* self = new(ELeave) CScreensaverRepositoryWatcher(
        aUid, NCentralRepositoryConstants::KInvalidNotificationId, aCallBack, aRepository);

    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    return self;
    }


// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::CScreensaverRepositoryWatcher
// -----------------------------------------------------------------------------
//
CScreensaverRepositoryWatcher::~CScreensaverRepositoryWatcher()
    {
    if (iNotifyHandler)
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
    CRepository* aRepository)
    :iUid(aUid), iKey(aKey), iCallBack(aCallBack), iRepository(aRepository)
    {
    }


// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverRepositoryWatcher::ConstructL(CCenRepNotifyHandler::TCenRepKeyType aKeyType)
    {
    iNotifyHandler = CCenRepNotifyHandler::NewL(*this, *iRepository, aKeyType, iKey);
    iNotifyHandler->StartListeningL();
    }

// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverRepositoryWatcher::ConstructL()
    {
    iNotifyHandler = CCenRepNotifyHandler::NewL(*this, *iRepository);
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
void CScreensaverRepositoryWatcher::HandleNotifyInt(TUint32 aKey, TInt aNewValue)
    {
    SCRLOGGER_WRITEF(_L("SCR: HandleNotifyInt aKey=%d, aNewValue=%d"), aKey, aNewValue);

    iChangedKey = aKey;
    iCallBack.CallBack();
    iChangedKey = NCentralRepositoryConstants::KInvalidNotificationId;
    }

// -----------------------------------------------------------------------------
// CScreensaverRepositoryWatcher::HandleNotifyString
// -----------------------------------------------------------------------------
//
void CScreensaverRepositoryWatcher::HandleNotifyString(TUint32 aKey, const TDesC16& aNewValue)
    {
    SCRLOGGER_WRITEF(_L("SCR: HandleNotifyString aKey=%d, aNewValue=%S"), aKey, &aNewValue);

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


// End of file.
