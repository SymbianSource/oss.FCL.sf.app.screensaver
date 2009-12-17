/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   SlideshowPlugin utility classes and functions
*
*/



// INCLUDE FILES
#include <centralrepository.h>
#include <f32fsys.h>
#include <eikenv.h>
#include <pathinfo.h>
#include <DRMHelper.h>
#include <caf/caftypes.h>

#include "SlideshowPluginUtils.h"

_LIT(KSSPDriveLetterMC, "E");

// ============================ MEMBER FUNCTIONS ===============================

// Repository watcher

// ---------------------------------------------------------------------------
// CRepositoryWatcher::NewL
// ---------------------------------------------------------------------------
//
CRepositoryWatcher* CRepositoryWatcher::NewL(
    const TUid aUid,
    const TUint32 aKey,
    CCenRepNotifyHandler::TCenRepKeyType aKeyType,
    TCallBack aCallBack,
    CRepository* aRepository)
    {
    CRepositoryWatcher* self = new(ELeave) 
          CRepositoryWatcher(aUid, aKey, aCallBack, aRepository);

    CleanupStack::PushL(self);
    self->ConstructL(aKeyType);
    CleanupStack::Pop(self);

    return self;
    }

// ---------------------------------------------------------------------------
// CRepositoryWatcher::NewL
// ---------------------------------------------------------------------------
//
CRepositoryWatcher* CRepositoryWatcher::NewL(
    const TUid aUid,
    TCallBack aCallBack,
    CRepository* aRepository)
    {
    CRepositoryWatcher* self = new(ELeave) CRepositoryWatcher(
        aUid, NCentralRepositoryConstants::KInvalidNotificationId, aCallBack, aRepository);

    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    return self;
    }

// ---------------------------------------------------------------------------
// CRepositoryWatcher::~CRepositoryWatcher
// ---------------------------------------------------------------------------
//
CRepositoryWatcher::~CRepositoryWatcher()
    {
    if ( iNotifyHandler )
        {
        iNotifyHandler->StopListening();
        delete iNotifyHandler;      
        }
    }

// ---------------------------------------------------------------------------
// CRepositoryWatcher::CRepositoryWatcher
// ---------------------------------------------------------------------------
//
CRepositoryWatcher::CRepositoryWatcher(
    const TUid aUid,
    const TUint32 aKey,
    TCallBack aCallBack,
    CRepository* aRepository)
    :
    iUid(aUid), iKey(aKey), iCallBack(aCallBack), iRepository(aRepository)
    {
    }

// ---------------------------------------------------------------------------
// CRepositoryWatcher::ConstructL
// ---------------------------------------------------------------------------
//
void CRepositoryWatcher::ConstructL(CCenRepNotifyHandler::TCenRepKeyType aKeyType)
    {
    iNotifyHandler = CCenRepNotifyHandler::NewL(*this, *iRepository, aKeyType, iKey);
    iNotifyHandler->StartListeningL();
    }

// ---------------------------------------------------------------------------
// CRepositoryWatcher::ConstructL
// ---------------------------------------------------------------------------
//
void CRepositoryWatcher::ConstructL()
    {
    iNotifyHandler = CCenRepNotifyHandler::NewL(*this, *iRepository);
    iNotifyHandler->StartListeningL();
    }

// ---------------------------------------------------------------------------
// CRepositoryWatcher::ChangedKey
// ---------------------------------------------------------------------------
//
TUint32 CRepositoryWatcher::ChangedKey()
    {
    return iChangedKey;
    }

// ---------------------------------------------------------------------------
// CRepositoryWatcher::HandleNotifyInt
// ---------------------------------------------------------------------------
//
void CRepositoryWatcher::HandleNotifyInt( 
                                    TUint32 aKey, TInt /*aNewValue*/ )
    {
    iChangedKey = aKey;
    iCallBack.CallBack();
    iChangedKey = NCentralRepositoryConstants::KInvalidNotificationId;
    }

// ---------------------------------------------------------------------------
// CRepositoryWatcher::HandleNotifyString
// ---------------------------------------------------------------------------
//
void CRepositoryWatcher::HandleNotifyString( TUint32 aKey, 
                                                     const TDesC16& /*aNewValue*/ )
    {  
    iChangedKey = aKey;
    iCallBack.CallBack();
    iChangedKey = NCentralRepositoryConstants::KInvalidNotificationId;
    }

// ---------------------------------------------------------------------------
// CRepositoryWatcher::HandleNotifyGeneric
// ---------------------------------------------------------------------------
//
void CRepositoryWatcher::HandleNotifyGeneric(TUint32 aKey)
    {
    iChangedKey = aKey;
    iCallBack.CallBack();
    iChangedKey = NCentralRepositoryConstants::KInvalidNotificationId;
    }

// ---------------------------------------------------------------------------
// CRepositoryWatcher::HandleNotifyError
// ---------------------------------------------------------------------------
//
void CRepositoryWatcher::HandleNotifyError(TUint32 /*aKey*/, TInt /*aError*/, CCenRepNotifyHandler* /*aHandler*/)
    {
    }     


// P & S subscriber    

// ---------------------------------------------------------------------------
// CPSSubscriber::CPSSubscriber
// ---------------------------------------------------------------------------
//
CPSSubscriber::CPSSubscriber(TCallBack aCallBack, RProperty& aProperty)
    : CActive(EPriorityNormal), iCallBack(aCallBack), iProperty(aProperty)
    {
    CActiveScheduler::Add(this);
    }

// ---------------------------------------------------------------------------
// CPSSubscriber::~CPSSubscriber
// ---------------------------------------------------------------------------
//
CPSSubscriber::~CPSSubscriber()
    {
    Cancel();
    }

// ---------------------------------------------------------------------------
// CPSSubscriber::SubscribeL
// ---------------------------------------------------------------------------
//
void CPSSubscriber::SubscribeL()
    {
    if (!IsActive())
        {
        iProperty.Subscribe(iStatus);
        SetActive();
        }
    }

// ---------------------------------------------------------------------------
// CPSSubscriber::StopSubscribe
// ---------------------------------------------------------------------------
//
void CPSSubscriber::StopSubscribe()
    {
    Cancel();
    }

// ---------------------------------------------------------------------------
// CPSSubscriber::RunL
// ---------------------------------------------------------------------------
//
void CPSSubscriber::RunL()
    {
    if (iStatus.Int() == KErrNone)
        {
        iCallBack.CallBack();
        SubscribeL();
        }
    }

// ---------------------------------------------------------------------------
// CPSSubscriber::DoCancel
// ---------------------------------------------------------------------------
//
void CPSSubscriber::DoCancel()
    {
    iProperty.Cancel();
    }



// ========== UTILITIES CLASS ===============================

// Checks if memory card is present
TBool SlideshowUtil::IsMCPresent()
    {
    RFs& fs = CCoeEnv::Static()->FsSession();

    TInt error = KErrNone;
    TDriveInfo driveInfo;
    TInt res = 0;
    TInt err = fs.CharToDrive(PathInfo::MemoryCardRootPath()[0], res);
    error = fs.Drive(driveInfo, res );

    // Not present, locked or unknown is bad
    if ((error != KErrNone) ||
        (driveInfo.iMediaAtt & KMediaAttLocked) ||
        (driveInfo.iType == EMediaNotPresent) ||
        (driveInfo.iType == EMediaUnknown))
        {
        return EFalse;
        }

    // Read-only is fine
    return ETrue;
    }


// Checks if the given file is on memory card
TBool SlideshowUtil::IsOnMC(TFileName aFile)
    {
    // Grab drive letter from path and compare to memory card drive letter
    TBuf<1> driveLetterBuf;
    driveLetterBuf.CopyUC(aFile.Left(1));

    if (driveLetterBuf.Compare(KSSPDriveLetterMC) == 0)
        {
        return ETrue;
        }
    
    return EFalse;
    }


// Checks that the file has enough DRM rights to be displayed
TBool SlideshowUtil::DRMCheck(CDRMHelper *aDRMHelper, TDesC& /* aFileName */)
    {
    if (!aDRMHelper)
        {
        return EFalse;
        }

    return ETrue;
    }

    
// Checks that the slide has enough DRM rights to be displayed
TBool SlideshowUtil::DRMCheck(CDRMHelper *aDRMHelper, CSlideshowSlide* aSlide)
    {
    if (!aDRMHelper || !aSlide)
        {
        return EFalse;
        }

    return (DRMCheck(aDRMHelper, *(aSlide->FileName())));
    }

    
// Consumes the slide's DRM rights
void SlideshowUtil::DRMConsume(CDRMHelper* aDRMHelper, CSlideshowSlide* aSlide)
    {
    if (!aDRMHelper || !aSlide)
        {
        return;
        }
    
    aDRMHelper->ConsumeFile2(*(aSlide->FileName()),
                             ContentAccess::EView,
                             CDRMHelper::EStart);
    aDRMHelper->ConsumeFile2(*(aSlide->FileName()),
                             ContentAccess::EView,
                             CDRMHelper::EFinish);
    }


// End of File
