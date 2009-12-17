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
* Description:   Screensaver engine class definition.
*
*/



#ifndef C_SCREENSAVERENGINE_H
#define C_SCREENSAVERENGINE_H

#include <e32base.h>
#include <AknCapServerClient.h>

#include "screensaverindicator.h"
#include "screensaverindicatorarray.h"
#include "screensaverplugin.h"

//delay before wserv timer is turned off
#define KWSERVHEARTBEATTIMEOUT 15

// FORWARD DECLARATIONS
class CUserActivityManager;
class CScreensaverSharedDataI;
class CScreensaverSharedDataMonitor;
class CScreensaverAppUi;
class CScreensaverView;
class CPowerSaveDisplayMode;

/**
* CScreensaverModel
*/
class CScreensaverEngine : public CBase 
    {
public:
    
    /**
    * Two-phased constructor
    * 
    * @param aAppUi the AppUi of the application
    */
    static CScreensaverEngine* NewL();
    
    /**
    * Destructor
    */
    ~CScreensaverEngine();

    /**
    * Return the pointer to the CScreensaverSharedDataI
    * 
    * @return CScreensaverSharedDataI*
    */
    CScreensaverSharedDataI* SharedDataInterface() const;
    
    /**
    * Reset the time during which the user did not do anything
    */
    void ResetInactivityTimeout();

    /**
    * Returns if the screensaver is on or not
    */
    TBool ScreenSaverIsOn() const;
    
    /**
    * Returns if the screensaver is previewing or not
    */
    TBool ScreenSaverIsPreviewing() const;

    /**
    * Starts screensaver
    */
    void StartScreenSaver();
    
    /**
    * Stops the screensaver
    */
    void StopScreenSaver();

    /**
    * Starts the preview model
    */
    void StartPreviewModeL();
    
    /*
    * Display the Object(); 
    */
    void DisplayObject();

    /**
    * Start the suspend timer
    */
    void StartSuspendTimer( TInt aSec );

    /**
    * Returns the indicator array
    */
    CScreensaverIndicatorArray& IndicatorArray() const;
    
    /**
    * Returns the color model
    */
    const TScreensaverColorModel& GetColorModel( ) const;
    
    /**
    * Adds the partial model
    * 
    * @param aBpp the bits per pixels of the partial model
    * @param aType the type of the partial type
    */
    void AddPartialModeLevel( TInt aBpp, TScreensaverPartialModeType aType );

    /**
    * Updates the indicator attributes.
    */
    void UpdateIndicatorAttributes( );
    
private:
    
    /**
    * C++ default constructor
    * 
    * @param aAppUi the AppUi of the application
    */
    CScreensaverEngine();

private:
    
    /**
    * Two-phased constructor
    */
    void ConstructL();
    
    /**
    * Constructs the shared data member
    */
    void EnableSharedDataAndMonitorL();
    
    /**
    * Destruct the shared data member
    */
    void DisableSharedDataAndMonitor();

    /**
    * Starts to monitor the user activity
    */
    void StartActivityMonitoringL();
    
    /**
    * Stops monitoring the user activity
    */
    void StopActivityMonitoring( CUserActivityManager*& aActivityManager );

    /**
    * Gets the color model from the resource
    */
    void SetupColorModelL();

    /**
    * Start the preview timer
    */
    void StartPreviewTimer();
    
    /**
    * Kill the timer
    * 
    * @param aTimer the timer you want stop
    */
    void KillTimer( CPeriodic*& aTimer );

    
    /**
    * Callback fuction. Called when the preivew tiemr time out
    */
    static TInt HandlePreviewTimerExpiry( TAny* aPtr );
    
    /**
    * Called when the time that the user did activity is out.
    */
    static TInt HandleActiveEventL(TAny* aPtr);
    
    /**
    * Called when the time that the user did no activity is out.
    */
    static TInt HandleInactiveEventL(TAny* aPtr);

    /**
    * Called when the time that the user did activity is out.
    */
    static TInt HandleActiveEventShortL(TAny* aPtr);    
    
    /**
    * Called when the time that the user did no activity is out.
    */
    static TInt HandleInactiveEventShortL(TAny* aPtr);
    
    /**
    * Callback fuction. Called when the suspension tiemr time out
    */
    static TInt HandleSuspendTimerExpiry( TAny* aPtr );

    
    /**
    * Returns the CScreensaverView
    */
    CScreensaverView* View() const;

    /**
    * Returns the timeout.
    */
    TInt Timeout();
    
    /**
    * Returns the display flag.
    */
    TInt DisplayFlag();


private:
    
    /**
    * The flag if append alert type indicator
    */
    TBool iShowExtendedProfileInd;
    
    /**
    * The flag if the screensaver is on or not
    */
    TBool iScreenSaverIsOn;
    
    /**
    * The flag if the screensaver is previewing or not
    */
    TBool iScreenSaverIsPreviewing;
    
    /**
    * The trigger for screensaver activation
    */
    CUserActivityManager* iActivityManagerScreensaver;

    
    /**
    * The pointer to CScreensaverSharedDataI
    * 
    * Owned
    */
    CScreensaverSharedDataI* iSharedDataI;
    
    /*
    * The pointer to the CScreensaverSharedDataMonitor 
    */
    CScreensaverSharedDataMonitor* iSharedDataMonitor;
    
    /**
    * The pointer to the indicator array
    * 
    * Owned
    */
    CScreensaverIndicatorArray* iIndicatorArray;
    
    /**
    * The trigger for screensaver activation, short timeout
    */
    CUserActivityManager* iActivityManagerScreensaverShort;
    
    /**
    * moved from view class
    */
    TScreensaverColorModel iColorModel;
    
    
    /**
    * The preview timer
    */
    CPeriodic* iPreviewTimer;
    
    /**
    * The suspend timer
    */
    CPeriodic* iSuspendTimer;

    /**
    * used to intercept and capture application key event.
    */
    RAknUiServer iAknUiServer;
    };



#endif // C_SCREENSAVERENGINE_H

// End of file
