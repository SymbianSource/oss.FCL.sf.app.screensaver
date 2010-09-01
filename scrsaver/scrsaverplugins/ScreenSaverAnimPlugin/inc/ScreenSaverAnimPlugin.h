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
* Description:   ScreenSaverAnimPlugin provides an implementation of 
*                MScreensaverPlugin for animation files.
*
*/



#ifndef C_SCREENSAVERANIMPLUGIN_H
#define C_SCREENSAVERANIMPLUGIN_H

#include <gdi.h>
#include <ScreensaverpluginIntDef.h>
#include <coecntrl.h>
#include <e32cmn.h>
#include <screensaverplugin.h>
#include <coecobs.h>
#include <npupp.h>
#include <ecom/implementationinformation.h>    
#include <cecombrowserplugininterface.h>
#include <pluginadapterinterface.h>
#include <flogger.h>


class CEikonEnv;
class CDRMHelper;
class CScreensaverRepositoryWatcher;
class CRepository;


_LIT( KFileLoggingDir, "ScreenSaver" );
_LIT( KFileLog, "SaveLog.txt" );
_LIT( KEmptyString, "" );

#ifdef SCREENSAVER_LOG_ENABLED

inline void PrintDebugMsg( const TDesC& aMsg ) 
    {    
    RDebug::Print( aMsg );
#if 0
    RFileLogger::Write( KFileLoggingDir, 
                        KFileLog, EFileLoggingModeAppend, aMsg );
#endif
    }
#else

inline void PrintDebugMsg( const TDesC& /*aMsg*/ ) 
    {
    }

#endif

/**
*  Encapsulates the Settings that are written into the central repository
*  @lib ScreenSaverAnimPlugin.lib
*  @since 3.1
*/
class CScreenSaverAnimSettingObject : public CBase
    {

    public:
        CScreenSaverAnimSettingObject();

    public:

        // Name of the file to be set as screensaver
        TFileName iFileName;   

        // Animation Timeout
        TInt iAnimationTimeout;  

        // Lights Timeout
        TInt iLightsTimeout;  
    };

/**
*  Provides the Animated ScreenSaverPlugin functionality which implements
*  ScreenSaver Interface and gives a Netscape Browser Plugin Interface
*  to the format specific plugins like SVG, Flash, M3G etc.
*  @lib ScreenSaverAnimPlugin.lib
*  @since 3.1
*/
 
class CScreenSaverAnimPlugin: public CScreensaverPluginInterfaceDefinition,
                              public MPluginAdapter,
                              public MCoeControlObserver
    {

    public: 

        /**
        * Two-phased constructor.
        * @since 3.1
        */
        static CScreenSaverAnimPlugin* NewL();

        /**
        * Destructor.
        */
        virtual ~CScreenSaverAnimPlugin();

    public: // Functions From CScreensaverPluginInterfaceDefinition        

        /**
        * From MScreensaverPlugin, Method called by ScreenSaver Application
        * when an animation file is configured as a Screen Saver.
        * @param aHost  Screen saver plugin host.
        * @since 3.1
        */
        virtual TInt InitializeL( MScreensaverPluginHost *aHost );
            
        /**
        * From MScreensaverPlugin, Method called by ScreenSaver Application
        * when Screen Saver is activated.
        * @since 3.1
        */
        virtual TInt Draw( CWindowGc& aGc );

        /**
        * From MScreensaverPlugin, Method called by ScreenSaver Application
        * before the InitializeL.
        * @since 3.1
        */
        virtual const TDesC16& Name() const;

        /**
        * From MScreensaverPlugin, Method called by ScreenSaver Application
        * to get the capabilities of Screensaver plugin.
        * @since 3.1
        */
        virtual TInt Capabilities();

        /**
        * From MScreensaverPlugin, Method called by Psln Application
        * when the user selects Animated ScreenSaver 
        * @since 3.1
        */
        virtual TInt PluginFunction(
                TScPluginCaps /*aFunction*/,
                TAny* /*aParam*/ );

        /**
        * From MScreensaverPlugin, Method called by ScreenSaver Application
        * when there are Screen Saver events.
        * @since 3.1
        */
        virtual TInt HandleScreensaverEventL(
                TScreensaverEvent aEvent,
                TAny* aData );

    public: // New Functions

        /**
        * Reads the animation settings from the central repository
        * @since 3.1
        * @return System wide Error value
        */
        TInt GetAnimSettings();

        /**
        * Writes the animation settings to the central repository
        * @since 3.1
        * @return System wide Error value
        */
        TInt SetAnimSettings() const;

        /**
        * This method registers for expiry of drm content
        * @since 3.1
        * @return System wide Error value   
        */
        TInt RegisterForDrmExpiry();

        /**
        * This method unregisters for expiry of drm content
        * @since 3.1
        * @return System wide Error value   
        */
        TInt UnregisterForDrmExpiry();

        /**
        * This method reads and parses the central repository key 
        * for drm expiry
        * @since 3.1
        * @return System wide Error value
        */
        TInt ReadDrmNotification( TDes8& aString );

        /**
        * Called when there is a drm expiry notification
        * @since 3.1
        */
        void ProcessDrmNotificationL();

        /**
        * This method pops the settings dialog
        * @since 3.1
        */
        void PopSettingsDialogL();

        /**
        * This method checks whether the set file exists currently
        * if not, it updates the filename string
        * This also checks if the rights are valid if the content is drm 
        * if not, it updates the filename with the empty file name
        * returns True, if the Filename is updated
        *         False, otherwise
        * @since 3.1
        */
        TBool UpdateFileName();

        /**
        * Called when HandleScreenSaverEventL is called with EDisplayChanged
        * Used to set the parent control for the animation plugins
        * returns KErrGeneral when SetWindow does not go through
        * @since 3.1
        */
        TInt SetWindow( CCoeControl* aParent );

        /**
        * Called to set the filename for the animation plugins
        * @since 3.1
        */
        void AsFile();

        /**
        * Called when there is a central repository settings change
        * @since 3.1
        */
        void HandleSettingsChange( TAny* aPtr );

        /**
        * Called by the repository watcher when there is a central 
        * repository settings change
        * @since 3.1
        */
        static TInt HandleRepositoryCallBack( TAny* aPtr );

        /**
        * Called by the repository watcher when there is a 
        * drm expiry notification
        * @since 3.1
        */
        static TInt HandleDrmNotification( TAny* aPtr );

        /**
        * Sends an event to the plugins to check for the file's validity 
        * repository settings change
        * returns KErrNone if the file is valid
        * @since 3.1
        */
        TInt CheckFileValidityL( const TDesC& aFileName );

        /**
        * Parses the mime description and checks whether the given mime type
        * is present in the mime description
        * @since 3.1
        * @param aMIMEDescription mime Description which has mimetype, 
        *                extension and description
        * @param aDataType mimetype to be searched
        * @return TBool ETrue if aDataType is present in aMIMEDescription
        *               EFalse otherwise
        */
        static TBool ParseMimeStringL( const TDesC& aMIMEDescription,
                                const TDesC& aDataType );

        /**
        * Given the mimetype searches and returns the browser plugin which 
        * handles the given mimetype
        * @since 3.1
        * @param aDataType 
        * @return TUid uid of the plugin which handles the mimetype
        */
        TUid FindAnimPluginL( const TDesC& aDataType );


    public: // From MPluginAdapter
        
        /**
        * Gets the pointer to the minor version number of the plug-in version 
        * number of the Browser Plug-in API. 
        * @since 3.1
        * @return Series 60 returns 1. Series 80 and 
        * the Nokia 7710 smartphone return 101.
        */
        virtual TUint16 GetVersion();

        /**
        * Called when the plug-in has completed its creation.
        * This function is called from the constructL function of the plug-in.
        * @since 3.1
        * @param aPluginControl A pointer to the plug-in control window
        */
        virtual void PluginConstructedL( CCoeControl* aPluginControl );

        /**
        * Called after the plug-in has been deleted.
        * This function is called from the destructor of the plug-in.
        * @since 3.1        
        */
        virtual void PluginFinishedL();

#ifndef __SERIES60_30__

        /**
        * Called by the plug-in to open a URL in the parent frame.
        * This function is not implemented in Series 60. 
        * It is supported in Series 80 and in the Nokia 7710 smartphone.
        * @since 3.1
        * @param aUrl Name of the URL to open
        */
        virtual void OpenUrlL( const TDesC& aUrl );

        /**
        * Called by the plug-in to open a URL in a named window or frame.
        * This function is not implemented in Series 60. 
        * It is supported in Series 80 and in the Nokia 7710 smartphone.
        * @since 3.1
        * @param aUrl Name of the URL to open
        * @param aNewTargetWindowName The name of the target window or frame   
        */
        virtual void OpenUrlInTargetWindowL( const TDesC& aUrl, 
                                           const TDesC& aNewTargetWindowName );

        /**
        * Specifies whether or not the frame has a previous navigation 
        * history item.
        * This function is not implemented in Series 60. 
        * It is supported in Series 80 and in the Nokia 7710 smartphone.
        * @since 3.1
        * @return true if the frame has a previous navigation 
        * history item false otherwise
        */
        virtual TBool HasNavigationHistoryPrevious() const;

        /**
        * Informs the browser to load the next navigation history URL 
        * in its parent frame.
        * This function is not implemented in Series 60. 
        * It is supported in Series 80 and in the Nokia 7710 smartphone.
        * @since 3.1
        */
        virtual void LoadNavigationHistoryPreviousL();

        /**
        * Returns whether the frame has a next navigation history item. 
        * This function is not implemented in Series 60.
        * It is supported in Series 80 and in the Nokia 7710 smartphone.
        * @since 3.1
        * @return Returns whether the frame has a next navigation history item. 
        */
        virtual TBool HasNavigationHistoryNext() const;

        /**
        * Informs the browser to load the next navigation history URL 
        * in its parent frame.
        * This function is not implemented in Series 60. It is supported in
        * Series 80 and in the Nokia 7710 smartphone.
        * @since 3.1        
        */
        virtual void LoadNavigationHistoryNextL();

#endif
        /**
        * Returns the window system level control object for the plug-in.
        * @since 3.1
        * @return A pointer to the parent control window.
        */
        virtual CCoeControl* GetParentControl();

        /**
        * Returns the observer for the plug-in control.
        * @since 3.1
        * @return Observer that the plug-in can use to send events 
        * to the browser.
        */
        virtual MCoeControlObserver* GetParentControlObserver();

        /**
        * Sets the plug-in notifier allowing the plug-in to control 
        * portions of the browser.
        * @since 3.1
        * @param aNotifier Pointer to an interface that the browser can 
        * use to send events to the plug-in.        
        */
        virtual void SetPluginNotifier( MPluginNotifier *aNotifier );

        /**
        * Tells the plugin adapter if plugin content can accept user interaction.  This function
        * is called from the plugin.
        * @since Series 60 3nd Edition
        * @param None
        * @return None
        */
        virtual void SetContentInteractive( TBool /*aInteractive*/ );

    public: //MCoeControlObserver
        /** 
        * Handles an event from an observed control.
        * This function is called when a control for which this control
        * is the observer calls CCoeControl::ReportEventL(). It should be
        * implemented by the observer control, and should handle all events 
        * sent by controls it observes.     
        * @param aControl The control that sent the event.
        * @param aEventType The event type. 
        */
        virtual void HandleControlEventL( CCoeControl* aControl,
                          TCoeEvent aEventType );

    private: // New functions
        
        /**
        * Finds the mime type of the file, searches the corresponding 
        * browser plugin which handles the mime type and starts the same.
        * @since 3.1                
        */
        void FindAndStartPluginL( const TDesC& aFileName, 
                                  TBool aCheckFileMode = EFalse );

        /**
        * Makes use of ecom api to find out whether an implementation with uid
        * aImpUid is present which implements the interface aInterfaceUid.
        * @since 3.1
        */
        TBool CheckIfPluginPresentL( TUid aInterfaceUid, 
                                    TUid aImpUid );

        /**
        * Finds the mime type of the file, searches the corresponding 
        * browser plugin which handles the mime type
        * @since 3.1
        */
        void FindPluginL( const TDesC& aFileName );
        
        /**
        * Creates the format specific plugin
        * @since 3.1
        */
        void CreatePluginL( TBool aCheckFileMode = EFalse );

        /**
        * Deletes the browser plugin.
        * @since 3.1
        */
        void DeletePlugin();  

        /**
        * Deletes and Sets the browser plugin members to NULL
        * @since 3.1
        */
        void SetPluginDeleted();

        /**
        * Connects the central repository session
        * @since 3.1
        */
        void InitializeCenRepL();

        /**
        * Registers for central repository key changes
        * @since 3.1
        */
        void NotifyCenrepChangeL();

        /**
        * Cancels registration for central repository key changes
        * @since 3.1
        */
        void NotifyCenrepChangeCancel();

        /**
        * Disconnects the central repository session
        * @since 3.1
        */
        void UnInitializeCenRep();

        /**
        * Sends an animation end event to the plugins
        * @since 3.1
        */
        TInt SendEndEvent();

        /**
        * Checks whether the given content id matches the content id of the
        * file name given
        * @return ETrue if the content id matches
        * EFalse if not
        * @since 3.1
        */
        TBool CheckContentIdL( const TDesC& aFileName, TDesC8& aContentUri );

    private:

        /**
        * Default Constructor
        * @since 3.1
        */  
        CScreenSaverAnimPlugin();

        /**
        * 2nd phase constructor
        * @since 3.1
        */
        void ConstructL();

    public: 

        // Pointer to the screen saver host
        MScreensaverPluginHost* iScreenSaverHost;

    private:

        // Resource file offset
        TInt iResourceOffset;

        // Handle to the central repository session
        CRepository* iAnimPluginSession;

         // Handle to the central repository session
        CRepository* iDrmSession;

        // Environment pointer
        CEikonEnv* iEikEnv;

        // Format specific Netscape plugin UID
        TUid iPluginUID;

        // Netscape plugin functions.
        NPPluginFuncs* iPluginFuncs;

        // Netscape ECOM plugin interface.
        CEcomBrowserPluginInterface* iPluginInterface;

        // Netscape plugin instance.
        NPP iNPPinstance;

        // Plugin Type
        HBufC8* iPluginType;

        // Parent control given by the screen saver application
        CCoeControl* iParentControl;

        // Pointer to the Settings object
        CScreenSaverAnimSettingObject* iSettingObject;

        // Pointer to Drm Helper
        CDRMHelper* iDrmHelper;

        // Cenrep Watcher for settings change
        CScreensaverRepositoryWatcher* iScreensaverRepositoryWatcher;

        // Cenrep Watcher for drm expiry
        CScreensaverRepositoryWatcher* iDrmExpiryWatcher;

        // To save the error encountered by the plugin
        TInt iPluginError;

        // Screensaver plugin name
        HBufC* iScreensaverName;
    };


#endif // C_SCREENSAVERANIMPLUGIN_H
// End Of file
