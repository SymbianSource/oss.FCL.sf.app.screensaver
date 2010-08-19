/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Defines screensaver shared data connections.
*
*/



#ifndef _SCREENSAVERSHAREDDATAI_H__
#define _SCREENSAVERSHAREDDATAI_H__

#include <msvapi.h>         // for MMsvSessionObserver
#include <cenrepnotifyhandler.h>

#include "screensaverplugin.h"
#include "ScreensaverUtils.h"
#include "ScreensaverInternalPSKeys.h"
#include "ScreensaverInternalCRKeys.h"
#include "screensaverengine.h"

enum
    {
    ESSForceLightsOff = 0,
    ESSForceLightsOn
    };

// CLASS DECLARATION
class CRepository;
class CScreensaverRepositoryWatcher;
class CSubscriber;
class MProfile;
class MProfileEngine;

class CScreensaverSharedDataI : public CBase, public MMsvEntryObserver,
    public MMsvSessionObserver
    {
public:
    /**
    * Two-phased constructor
    */
    static CScreensaverSharedDataI* NewL();

    /**
    * Destructor
    */
    ~CScreensaverSharedDataI();

public:
    /**
    * Get the screensaver object type
    */
    TDisplayObjectType DisplayObjectType() const;

    /*
    * Change the display object type of screensaver
    */
    void SetDisplayObjectType( const TDisplayObjectType aType );
    
    /**
    * Gets the display text
    */
    void GetDisplayObjectText( TDes& aDes ) const;

    /*
    * Get the plug-in name
    */
    void GetPluginName( TFileName& aFileName ) const;
    
    /*
    * Get the flag if uses the inversive color 
    */
    TBool IsInvertedColors() const;
    
    /**
    * Gets the timeout
    */
    TInt ScreensaverTimeout() const;
    
    /*
    * Is the New Contact Note is actived or not
    */
    TBool IsNCNActive() const;
    
    /*
    * Get the count of the missed calls 
    */
    TInt NewMissedCalls() const;    

    /**
    * Request screen lights to be turned on or off
    *
    * @param sSecs - lights on for aSecs seconds, 0 = off, max 30.
    */
    void SetSSForcedLightsOn( const TInt aSecs );

    /**
    * Publish Screensaver on/off status
    *
    * @param aOn ETrue - saver on, EFalse - off
    */
    void SetScreensaverStatus(const TBool aOn = ETrue);

    /**
    * Publish "Screensaver started from Idle" status
    */
    void SetSSStartedFromIdleStatus();
    
    /*
    * Get the flag that if the screensaver is allowed or not
    */
    TInt IsScreenSaverAllowed() const;
    
    /*
    * Get the Preview mode of screensaver 
    */
    TInt ScreensaverPreviewMode() const;
    
    /*
    * Set the preview mode 
    */
    void SetScreensaverPreviewMode( const TInt aMode );

    /**
    * Get preview state
    */
    TScreenSaverPreviewState ScreensaverPreviewState() const;
    
    /**
    * Report preview state
    */
    void SetScreensaverPreviewState( const TScreenSaverPreviewState aState );
    
    /*
    * Get keyguard status 
    *
    * @return ETrue  key guard is active.
    *         EFalse key guard is inactive.
    */
    TBool IsKeyguardOn() const;

    /**
    * Query power save mode usage
    *
    * @return ETrue, if power save mode should be used
    */
    TBool IsUsePowerSaveMode() const;

    /**
    * Query system startup completion status
    *
    * @return ETrue, if system startup is complete
    */
    TBool IsSystemStartupComplete();
    
    /**
    * Gets the default screensaver type
    */
    TDisplayObjectType DefaultScreensaverType() const;

    /**
    * Returns number of unread messages in inbox.
    *
    * @return  Number of unread messages in inbox.
    */
    TInt UnreadMessagesNumber();

    /**
    * Return the name of currently selected profile
    *
    * @return Profile name descriptor.
    */
    const TDesC& ProfileName();

    /**
    * Query active profile.
    *
    * @return Id of active profile, or system error code.
    */
    TInt ActiveProfile();

    /**
    * Query new email status
    *
    * @return ETrue if there is new email
    */
    TBool IsHaveNewEmail() const;

    /**
    * Query new voice mail status
    *
    * @return ETrue, if there is new voice mail
    */
    TBool IsHaveNewVoicemail() const;
    
    /**
    * Query new instant message status
    *
    * @return ETrue, if there are instant messages pending
    */
    TBool IsHaveNewInstantMessages() const;

    /**
    * Query silent mode status
    *
    * @return ETrue, if silent mode is on
    */
    TBool IsSilentMode();

    /**
    * Query vibra mode status
    *
    * @return ETrue, if vibra mode is on
    */
    TBool IsVibraMode();
    
    /**
    * Query call state
    * 
    * @return ETrue if a call is ongoing 
    */
    TBool IsOngoingCall();

public:
    /**
    * From MMsvSessionObserver. Not used.
    */
    void HandleSessionEventL(TMsvSessionEvent /*aEvent*/, TAny* /*aArg1*/,
            TAny* /*aArg2*/, TAny* /*aArg3*/);

    /**
    * From MMsvEntryObserver. Callback function. Handles global In-box events.
    * @param aEvent An entry event.
    */
    void HandleEntryEventL(TMsvEntryEvent aEvent, TAny* aArg1, TAny* aArg2,
            TAny* aArg3);

    /*
    * Returns the pointer of the screensaver repository 
    */
    CRepository* ScreensaverRepository() const;
    
public:
    /*
    * Get the flag of display changed 
    */
    TBool GetDisplayObjChanged() const;
    /*
    * Set the flag of display changed 
    */
    void SetDisplayObjChanged( TBool aChanged );

private:
    
    /**
    * C++ default constructor
    */
    CScreensaverSharedDataI();
    
    /**
    * Two-phased constructor
    */
    void ConstructL();
    
    /**
    * Connect to the screensaver central repository
    */
    void ConnectToSSCRL();
    
    /**
    * Connect to the Psln Setting central repository
    */
    void ConnectToPslnSettingCRL();
    void ConnectToCommonTsyCRL();
    void ConnectToInboxL();

    /**
    * Connects to Profile engine. Returns ETrue if successfully connected
    * iProfileEngine will then point to a valid MProfileEngine
    */
    TBool ConnectToProfileEngine();

    /**
     * Connects to active profile. Returns ETrue if successfully connected
     * iActiveProfile will then point to a valid MProfile
     */
    TBool ConnectToActiveProfile();
    
    /*
    * Define the screensaver properties 
    */
    void DefineScreensaverProperties( TUint32 aKey, const TSecurityPolicy& aWritePolicy );

private:
    // Shared data key state handlers.
    
    /**
    * Callback fuction. Called when the timeout changed
    */
    static TInt HandleTimeoutChanged(TAny* aPtr);

    /**
    * Resets the inactivity timeout
    */
    void ResetInactivityTimeout();

    /**
    * Callback fuction. Called when the display object changed
    */
    static TInt HandleDisplayObjChanged(TAny* aPtr);
   
    /*
    * Get the default type
    */
    void GetDefaultType();

    /*
     * Get the default type string from the CR
     */
    TInt GetDefaultTypeString( TDes& DefaultScreenSaver );
    
    /*
    * Get the count of the voice mail 
    */
    TInt VoiceMailCount( const TUint32 aKey ) const;

    /**
    * Invalidate cached active profile
    */
    void InvalidateActiveProfile();
    
    /**
    * Returns the AppUi
    */
    CScreensaverAppUi *AppUi() const;

private:
    
    /*
    * The type of the display object
    */
    TDisplayObjectType iDefaultType;
    /**
    * not owned!
    */
    CScreensaverAppUi* iAppUi;

    /**
    * Screensaver specific values.
    */
    CRepository* iScreensaverRepository;
    CScreensaverRepositoryWatcher* iScreensaverRepositoryWatcher;

    /**
    * General Settings.
    */
    CRepository* iSettingsRepository;
    CScreensaverRepositoryWatcher* iSettingsRepositoryWatcher;

    /**
    * Common tsy repository (voice message waiting)
    */
    CRepository* iCTsyRepository;


    /**
    * Access to the profile engine
    */
    MProfileEngine* iProfileEngine;
    
    /**
    * Need to store also the profile, so that it's name
    * can be used at any time
    */
    MProfile* iActiveProfile;

    /**
    * The Msv session pointer.
    */
    CMsvSession* iMsvSession;
    
    /**
    *  In-box folder entry. Note that the entry is not owned by this class.
    */
    CMsvEntry* iInboxFolder;

    /**
    * System startup completion flag
    */
    TBool iStartupComplete;
    
    /**
   * Record display changed flag
   */
    TBool iDisplayChange;
    };

#endif  // C_SCREENSAVERSHAREDDATAI_H
// End of file.
