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
* Description:   Defines screensaver shared data Subscriber.
*
*/



#ifndef C_SCREENSAVERSUBSCRIBER_H
#define C_SCREENSAVERSUBSCRIBER_H

#include <e32base.h>
#include <e32property.h>

/**
* CSubscriber
*/
class CSubscriber : public CActive
    {
public:
    CSubscriber(TCallBack aCallBack, RProperty& aProperty);
    ~CSubscriber();

public:
    // New functions
    void SubscribeL();
    void StopSubscribe();

private:
    // from CActive
    void RunL();
    void DoCancel();

private:
    TCallBack iCallBack;
    RProperty& iProperty;
    };

#endif // C_SCREENSAVERSUBSCRIBER_H
