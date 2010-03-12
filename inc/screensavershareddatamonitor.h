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
* Description:   Defination for screensaver shared data monitoring.
*
*/



#ifndef C_SCREENSAVERSHAREDDATAMONITOR_H
#define C_SCREENSAVERSHAREDDATAMONITOR_H

// INCLUDES
#include <e32property.h>

// CLASS DECLARATION
class CScreensaverSharedDataI;
class CScreensaverRepositoryWatcher;
class CScreensaverEngine;
class CScreensaverView;
class CSubscriber;


/**
 *  Cscreensavershareddatamonitor
 * 
 */
class CScreensaverSharedDataMonitor : public CBase
    {
public:
    // Constructors and destructor

    /**
     * Destructor.
     */
    ~CScreensaverSharedDataMonitor( );

    /**
     * Two-phased constructor.
     */
    static CScreensaverSharedDataMonitor* NewL( CScreensaverSharedDataI* aData );

private:

    /**
     * Constructor for performing 1st stage construction
     */
    CScreensaverSharedDataMonitor( CScreensaverSharedDataI* aData );

    /**
     * EPOC default constructor for performing 2nd stage construction
     */
    void ConstructL();
    
    /*
    * Returns the pointer of screensaver appui 
    */
    CScreensaverView* View();
    
    /*
    * Returns the refrence of model instance
    */
    CScreensaverEngine& Model();
    
    /**
    * detele CSubscriber
    */
    void DeleteSubscriber( CSubscriber*& aSubscriber );
    
private:
    
    /**
    * Callback fuction. Called when the preview mode changed
    */
    static TInt HandlePreviewModeChanged(TAny* aPtr);

    /**
    * Callback fuction. Called when the MMC state changed
    */
    static TInt HandleMMCStateChanged(TAny* aPtr);

    /**
    * Callback fuction. Called when the USB state changed
    */
    static TInt HandleUSBStateChanged(TAny* aPtr);

    /**
    * Callback fuction. Called when the Keyguard state changed
    */
    static TInt HandleKeyguardStateChanged(TAny* aPtr);

    /**
    * Callback fuction. Called when the Call state changed
    */
    static TInt HandleCallStateChanged(TAny* aPtr);

    /**
    * Callback fuction. Called when the ShuttingDown state changed
    */
    static TInt HandleShuttingDownStateChanged(TAny* aPtr);

    static TInt HandleActivateSSChanged( TAny* aPtr );

private:
    
    /*
    * The data of model class, Not own 
    */
    CScreensaverSharedDataI* iData;
    
    /*
    * The Preview mode property and its monitor
    */
    RProperty iPreviewModeProperty;
    CSubscriber* iPreviewModeSubscriber;
    
    /*
    * The MMC property and its monitor
    */
    RProperty iMMCWatchProperty;
    CSubscriber* iMMCWatchSubscriber;
    
    /*
    * The USB property and its monitor 
    */
    RProperty iUSBWatchProperty;
    CSubscriber* iUSBWatchSubscriber;
    
    /*
    * The  KeyGuard state property and its watcher
    */
    RProperty iKeyguardStateProperty;
    CSubscriber* iKeyguardStateSubscriber;
    
    /*
    * The Call state property and its watcher 
    */
    RProperty iCallStateProperty;
    CSubscriber* iCallStateSubscriber;
    
    /*
    * The shutting down property and its watcher 
    */
    RProperty iShuttingDownProperty;
    CSubscriber* iShuttingDownSubscriber;

    RProperty iActivateSSProperty;
    CSubscriber* iActivateSSSubscriber;

    };

#endif // C_SCREENSAVERSHAREDDATAMONITOR_H
