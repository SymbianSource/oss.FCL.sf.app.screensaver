// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// CScreensaverActivityManager class
// 
//

#ifndef SCREENSAVERACTIVITYMANAGER_H
#define SCREENSAVERACTIVITYMANAGER_H

#include <e32base.h>


class CScreensaverActivityManager : public CTimer
	{
public:
 	static CScreensaverActivityManager* NewL(TInt aPriority);
	~CScreensaverActivityManager();
	void Start(TTimeIntervalSeconds aInterval,
	                    TCallBack aInactiveCallback,
	                    TCallBack aActiveCallback);
	void SetInactivityTimeout(TTimeIntervalSeconds aInterval);
	TBool IsMonitoringForActivity();
private:
	void RunL();
	CScreensaverActivityManager(TInt aPriority);
	void MonitorInactivity();
private:

    enum TState {
        EStUndefined = 0,    //< Never used, reserved
        EStNotActive,        //< Constructed but not started
        EStMonForInactivity, //< Monitoring user inactivity mode
        EStMonForActivity    //< Monitoring user activity mode
    };

    /** Inactivity interval */
    TTimeIntervalSeconds iInterval;
    
    /** Callback to be used in case that user inactivity is detected */
	TCallBack            iInactiveCallback;
	
    /** Callback to be used in case of user activity after inactivity period */
	TCallBack            iActiveCallback;
	
    /** State of object, what form of monitoring is active. */
	TState                iState;
	};

#endif // SCREENSAVERACTIVITYMANAGER_H
