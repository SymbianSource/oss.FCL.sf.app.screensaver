/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Defines screensaver utility tools.
*
*/



#ifndef C_SCREENSAVERUTILITY_H
#define C_SCREENSAVERUTILITY_H

// INCLUDES
#include <e32base.h>

// CLASS DECLARATION

/**
* The uitility class of screensaver
*/
class ScreensaverUtility
    {
public:
    
    /**
    * Panic
    */
    static void Panic( TInt aPanic );

    /**
    * Flushes the draw buffer (before activating power save) 
    */
    static void FlushDrawBuffer();
    
    /**
    * Checks whether screen has been rotated
    */
    static TBool ScreenRotated();
    
    /**
    * Query whether 12-hour clock is displayed
    */
    static TBool Query12HourClock();

    /**
    * Brings the application to foreground
    */
    static void BringToForeground(); 
    
    /**
    * Sends the application to background
    */
    static void SendToBackground();
    
    /**
    * Drive info helpers
    */
    static TBool IsDrivePresent(TInt aDrive);
    
    /**
    * Loads the bitmap
    */
    static CGulIcon* LoadBitmapL();

    /**
    * Stop the server heartbeat
    */
    static void StopWServHeartBeat();
    };

#endif // C_SCREENSAVERUTILITY_H
