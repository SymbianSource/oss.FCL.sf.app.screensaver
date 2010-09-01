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
* Description:   Implements the Slideshow plugin beef, especially
*                MScreensaverPlugin interface
*
*/



#ifndef C_SLIDESHOWPLUGIN_H
#define C_SLIDESHOWPLUGIN_H

#include <gdi.h>
#include <coecntrl.h>
#include <e32cmn.h>
#include <coecobs.h>
#include <AknsSrvClient.h>
#include <ScreensaverpluginIntDef.h>
#include <screensaverplugin.h>
#include <ScreensaverInternalPSKeys.h>

#include <MCLFOperationObserver.h>
#include <MCLFContentListingEngine.h>
#include <MCLFItemListModel.h>
#include <mdesession.h>
#include <mdequery.h>

#include "SlideshowModel.h"
#include "SlideshowSlide.h"
#include "SlideshowPluginUtils.h"

class CEikonEnv;
class CDRMHelper;
class CRepositoryWatcher;
class CRepository;

/**
*  Encapsulates the Settings that are written into the central repository
*  @lib ScreenSaverAnimPlugin.lib
*  @since 3.1
*/
class CSlideshowSettings : public CBase
    {
public:
    CSlideshowSettings();
        
public:
    // Slideshow time
    TInt iSlideshowTime;  
    
    // Lights time
    TInt iLightsTime;

    // Time a slide is displayed
    TInt iSlideTime;

    // Slideshow type
    TInt iSlideshowType;
    };
       
    
/**
*  Provides the Animated ScreenSaverPlugin functionality which implements
*  ScreenSaver Interface and gives a Netscape Browser Plugin Interface
*  to the format specific plugins like SVG, Flash, M3G etc.
*  @lib ScreenSaverAnimPlugin.lib
*  @since 3.1
*/
class CSlideshowPlugin: public CScreensaverPluginInterfaceDefinition,
                        public MMdESessionObserver,
                        public MMdEQueryObserver
    {
    enum TPluginState   
        {
        EPluginStateLoading = 0,
        EPluginStateInitializing,
        EPluginStateRunning,
        EPluginStateStopping
        };

public: 
    
    /**
     * Creator
     * @since 3.1
     */
    static CSlideshowPlugin* NewL();
    
    /**
     * Destructor
     */
    virtual ~CSlideshowPlugin();     

private:
    /**
     * 2nd phase constructor
     * @since 3.1
     */
    void ConstructL();

    /**
     * Default Constructor
     * @since 3.1
     */  
    CSlideshowPlugin();

public: // Functions From CScreensaverPluginInterfaceDefinition        
        
    /**
     * From MScreensaverPlugin, Method called by Screensaver
     * when plugin is being loaded into use
     * @param aHost  Plugin host (Screensaver).
     * @since 3.1
     */
    virtual TInt InitializeL(MScreensaverPluginHost *aHost);
    
    /**
     * From MScreensaverPlugin, Method called by Screensaver 
     * when plugin is allowed to draw
     * @since 3.1
     */
    virtual TInt Draw( CWindowGc& aGc );
    
    /**
     * From MScreensaverPlugin, Method called by Themes application
     * to get localised name
     * @since 3.1
     */
    virtual const TDesC16& Name() const;
    
    /**
     * From MScreensaverPlugin, Method called by Screensaver
     * to get the capabilities of screensaver plugin
     * @since 3.1
     */
    virtual TInt Capabilities();
    
    /**
     * From MScreensaverPlugin, Method called by Themes application
     * to configure the plugin or to pass e.g. selection notifications
     * to plugin
     * @since 3.1
     */
    virtual TInt PluginFunction(
        TScPluginCaps /* aFunction */,
        TAny* /* aParam */ );

    /**
     * From MScreensaverPlugin, Method called by Screensaver
     * when there are screensaver events, such as starting and stopping
     * @since 3.1
     */
    virtual TInt HandleScreensaverEventL(
        TScreensaverEvent aEvent,
        TAny* aData );                           

public: // from MMdESessionObserver
    /**
     * Called to notify the observer that opening the session has been 
     * completed and, if the opening succeeded, the session is ready for use.
     *
     * @param aSession session
     * @param aError   <code>KErrNone</code>, if opening the session succeeded;
     *                 or one of the system-wide error codes, if opening the 
     *                 session failed
     */
    void HandleSessionOpened(CMdESession& aSession, TInt aError);

    /**
     * Called to notify the observer about errors, which are not a direct 
     * consequence of the operations initiated by the client but caused by 
     * some external source (e.g., other clients). The error cannot be 
     * recovered and all on-going operations initiated by the client have been 
     * aborted. Any attempts to continue using the session will cause a panic. 
     * The client should close the session immediately and try to open a new 
     * session, if it needs to continue using the metadata engine.
     *
     * @param aSession session
     * @param aError one of the system-wide error codes
     */
    void HandleSessionError(CMdESession& aSession, TInt aError);
    
public: // from MMdEQueryObserver
    /**
     * Called to notify the observer that new results have been received 
     * in the query.
     *
     * @param aQuery              Query instance that received new results.
     * @param aFirstNewItemIndex  Index of the first new item that was added
     *                            to the result item array.
     * @param aNewItemCount       Number of items added to the result item 
     *                            array.
     */
    void HandleQueryNewResults(CMdEQuery& aQuery,
                               TInt aFirstNewItemIndex,
                               TInt aNewItemCount);

    /**
     * Called to notify the observer that the query has been completed,
     * or that an error has occured.
     *
     * @param aQuery  Query instance.
     * @param aError  <code>KErrNone</code>, if the query was completed
     *                successfully. Otherwise one of the system-wide error 
     *                codes.
     */
    void HandleQueryCompleted(CMdEQuery& aQuery, TInt aError);
    
public: // New Functions
    
    /**
     * Reads the settings from the central repository
     * @since 3.1
     * @return System wide Error value
     */
    TInt ReadSettings();  

#if 0
    /**
     * This method checks whether the set file exists currently
     * if not, it updates the filename string
     * This also checks if the rights are valid if the content is drm 
     * if not, it updates the filename with the empty file name
     * returns True, if the Filename is updated
     *         False, otherwise
     * @since 3.2
     */
     TBool UpdateFileName();
#endif
                
    /**
     * Called when there is a central repository settings change
     * @since 3.2
     */        
    void HandleCRSettingsChange(TAny* aPtr);
    
    /**
     * Called by the repository watcher when there is a central 
     * repository settings change
     * @since 3.2
     */        
    static TInt HandleRepositoryCallBack(TAny* aPtr);
        
    /**
     * Called by the P & S subscriber when the subscribed key value changes
     * @since 3.2
     */  
    static TInt HandleSettingsChanged(TAny* aPtr);
    
private: // New functions
    
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

    void StartPlugin();
    void StopPlugin();
    void DrawCentered(CWindowGc& aGc, CSlideshowSlide* aSlide);
    void LoadSlidesL();
    void LoadSlideSetL();
    void LoadRandomSlidesL();
    void SetDisplayMode();
    void Lights(TInt aSecs);
    void ConfigureL(TAny* aParam);
    void UpdateDisplayInfo();
    void FlushDrawBuffer();

    TInt SettingsChanged();

    void ConnectToMDSSessionL();
    void WaitForMDS();
    void EndWaitForMDS();
    void LoadImagesToModel(const CMdEQuery& aQuery,
                           TInt aFirstNewItemIndex = 0,
                           TInt aNewItemCount = 0);
    void AppendSlideToModelL(TDesC& aFileName, TBool aIsOnMC);
    void OpenQueryL();
    
private:
    // Pointer to the screensaver host
    MScreensaverPluginHost* iHost;

    // Display info
    TScreensaverDisplayInfo iDi;

    // Resource file offset
    TInt iResourceOffset;   

    // Handle to the central repository session
    CRepository* iSlideshowRepository;        
        
    // Environment pointer                
    CEikonEnv* iEikEnv;
        
    // Pointer to settings
    CSlideshowSettings* iSettings;

    // Slideshow model
    CSlideshowModel* iModel;
    
    // Pointer to Drm Helper
    CDRMHelper* iDrmHelper;
        
    // Cenrep watcher for settings change
    CRepositoryWatcher* iSlideshowCRWatcher;
    
    RProperty iSettingsChangedProperty;
    CPSSubscriber* iSettingsChangedSubscriber;
            
    // Localised name
    HBufC* iScreensaverName;

    // Waiter
    CActiveSchedulerWait iWaiter;
    TBool iWaitActive;

    // Draw count
    TInt iDrawCount;
    TBool iTimerUpdated;

    //loading state
    TBool iIsLoadFinished;

    // MDS session
    CMdESession* iMdESession;
    };  


#endif // C_SLIDESHOWPLUGIN_H
// End Of file
