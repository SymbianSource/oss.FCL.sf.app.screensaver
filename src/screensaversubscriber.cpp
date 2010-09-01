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
* Description:   Implementation of screensaver shared data Subscriber.
*
*/



#include "screensaversubscriber.h"


// -----------------------------------------------------------------------------
// CSubscriber::CSubscriber
// -----------------------------------------------------------------------------
//
CSubscriber::CSubscriber(TCallBack aCallBack, RProperty& aProperty)
    : CActive(EPriorityNormal), iCallBack(aCallBack), iProperty(aProperty)
    {
    CActiveScheduler::Add(this);
    }

// -----------------------------------------------------------------------------
// CSubscriber::~CSubscriber
// -----------------------------------------------------------------------------
//
CSubscriber::~CSubscriber()
    {
    }

// -----------------------------------------------------------------------------
// CSubscriber::SubscribeL
// -----------------------------------------------------------------------------
//
void CSubscriber::SubscribeL()
    {
    if (!IsActive())
        {
        iProperty.Subscribe(iStatus);
        SetActive();
        }
    }

// -----------------------------------------------------------------------------
// CSubscriber::StopSubscribe
// -----------------------------------------------------------------------------
//
void CSubscriber::StopSubscribe()
    {
    Cancel();
    }

// -----------------------------------------------------------------------------
// CSubscriber::RunL
// -----------------------------------------------------------------------------
//
void CSubscriber::RunL()
    {
    if (iStatus.Int() == KErrNone)
        {
        SubscribeL();
        iCallBack.CallBack();
        }
    }

// -----------------------------------------------------------------------------
// CSubscriber::DoCancel
// -----------------------------------------------------------------------------
//
void CSubscriber::DoCancel()
    {
    iProperty.Cancel();
    }


// End of file.
