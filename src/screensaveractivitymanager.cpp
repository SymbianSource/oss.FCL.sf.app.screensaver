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

#include "screensaveractivitymanager.h"

CScreensaverActivityManager* CScreensaverActivityManager::NewL(TInt aPriority)
	{
	CScreensaverActivityManager* self = new (ELeave) CScreensaverActivityManager(aPriority);
	CleanupStack::PushL(self);
	self->ConstructL();
	CActiveScheduler::Add(self);
	CleanupStack::Pop(self);
	return self;
	}

CScreensaverActivityManager::CScreensaverActivityManager(TInt aPriority)
	: CTimer(aPriority), iInactiveCallback(0,0), iActiveCallback(0,0), iState(EStNotActive)
	{
	}


CScreensaverActivityManager::~CScreensaverActivityManager()
	{
	}


void CScreensaverActivityManager::Start(TTimeIntervalSeconds aInterval,
		TCallBack aInactiveCallback, TCallBack aActiveCallback) 
	{  	
	iInterval = aInterval;
	iInactiveCallback = aInactiveCallback;
	iActiveCallback = aActiveCallback;
			
	if	(iInterval.Int() < 0)
		{
		iInterval = TTimeIntervalSeconds(0);
		}
		
	// Cancel outstanding timer, if any
	CTimer::Cancel();
	
	if (iState == EStNotActive || iState == EStMonForInactivity) 
		{
		// Begin or contine monitoring for user inactivity
		MonitorInactivity();
		}
	else 
		{
		// Continue monitoring for user activity
		Inactivity(0); 
		}
	}

     
void CScreensaverActivityManager::SetInactivityTimeout(TTimeIntervalSeconds aInterval)
	{
	if (iState <= EStNotActive)
    	{
    	}
	Start(aInterval, iInactiveCallback, iActiveCallback); 
	}


void CScreensaverActivityManager::MonitorInactivity()
	{
	if (User::InactivityTime() >= iInterval) 
		{
		// Already inactive enough, monitor for activity
		iState = EStMonForActivity;
		// Monitor for activity, set timer before callback to avoid missing any
		// user activity should callback take some time.
		Inactivity(0); 
		// Inform client that the inactivity interval has ended.
        (void) iInactiveCallback.CallBack();
		}
	else
		{
		// Monitor for Inactivity
		iState = EStMonForInactivity;
		Inactivity(iInterval);
		}
	}


void CScreensaverActivityManager::RunL()
	{
	if (iStatus == KErrNone)
		{
		if (iState == EStMonForInactivity) 
			{
			// Looking for Inactivity, found it
	   	    // Monitor for activity, set timer before callback to avoid missing 
		    // any user activity should callback take some time.
       		iState = EStMonForActivity;
			Inactivity(0); 
			// Inform client that the inactivity interval has ended.
 			(void) iInactiveCallback.CallBack();
			}
		else if (iState == EStMonForActivity)
			{
			// Looking for activity, found it
			// Monitor for activity unless MonitorInactivity overrides it
    		iState = EStMonForInactivity; 
			MonitorInactivity();
	        (void) iActiveCallback.CallBack();
			}	
		else
            { // Must never happen, indicates internal state of object incorrect.
            
            }
		}
	}

TBool CScreensaverActivityManager::IsMonitoringForActivity()
    {
    return ( iState == EStMonForActivity );
    }
