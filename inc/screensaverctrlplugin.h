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
* Description:   Definitions for the display object of plugin type
*
*/



#ifndef C_SCREENSAVERCTRLPLUGIN_H
#define C_SCREENSAVERCTRLPLUGIN_H

#include <babitflags.h>

#include "screensaverbase.h"

/**
 * The plugin type of display object which is derived from the 
 * CScreensaverBase class
 */
class CScreensaverCtrlPlugin : public CScreensaverBase,
    public MScreensaverPluginHost
    {
public:
    enum TScreensaverPluginFlag
        {
        // Indicates that plugin module wants to draw indicators
        EPluginFlagOverrideIndicators = 0,
        // Indicates that plugin does not use refresh timer and should not get
        // Draw() calls
        EPluginFlagTimerNotUsed,
        EPluginFlagSuspend
        };
public:
    /**
    * Two-phased constructor.  
    */
    static CScreensaverCtrlPlugin* NewL();

    /**
    * Destructor  
    */
    ~CScreensaverCtrlPlugin();

    /**
    * Start the control needed timer
    */
    void StartTimer();
    
    /**
    * Cancels the timer
    */
    void CancelTimer();
    
    
    /**
    * Show the control
    */
    void DrawObject();
    
    /**
    * Clear the screen
    */
    void ClearScreen();
    
    /*
    * Refresh the display 
    */
    void Refresh();
    
    /**
    * Send the event to plugin to deal with
    * 
    * @param aEvent the event send to plugin
    * @return TInt  
    */
    TInt SendPluginEvent( TScreensaverEvent aEvent );

public:
    // From MScreensaverPluginHost
    void UseStandardIndicators();
    void OverrideStandardIndicators();
    TBool StandardIndicatorsUsed() const;
    void SetRefreshTimerValue( TInt aValue );
    TInt RefreshTimerValue() const;
    TInt GetIndicatorPayload( TScreensaverIndicatorIndex aIndex,
        TIndicatorPayload& aResult ) const;
    TInt SetActiveDisplayArea( TRect& aRect,
        const TScreensaverPartialMode& aMode );
    TInt SetActiveDisplayArea( TInt aStartRow, TInt aEndRow,
        const TScreensaverPartialMode& aMode );
    void ExitPartialMode();
    const TScreensaverColorModel& GetColorModel() const;
    void Suspend( TInt aTime );
    void RequestLights( TInt aSecs );
    TInt DisplayInfo( TScreensaverDisplayInfo* aDisplayInfo );
    void UseRefreshTimer( TBool aOn = ETrue );
    void RequestTimeout( TInt aSecs );
    void RevertToDefaultSaver();
    // End MScreensaverPluginHost

protected:
    /**
    * The default C++ constuctor
    * 
    * @param aType the type of the display object.
    * @param aPlugin the plugin which the screensaver is setted.  
    */
    CScreensaverCtrlPlugin();

    /**
    * Symbian default constructor.
    */
    void ConstructL();

protected:
    // From CCoeControl
    
    /** 
    * Resource change handling  
    */
    virtual void HandleResourceChange( TInt aType );
    
    /**
    * Responds to changes to the size and position of the contents 
    * of this control.  
    */
    virtual void SizeChanged();
    
    /**
    * Drawing (override this to draw)
    */
    virtual void Draw( const TRect& aRect ) const;
    
private:
    /*
    * LoadPluginL()
    */
    void LoadPluginL( MScreensaverPluginHost* aPluginHost );
    
    /*
    * Get the poiter of the plugin
    */
    void LoadPluginModuleL();
    
    /*
    * Delete the plugin
    */
    void DeletePlugin();
    
    /**
    * Starts the plugin refresh timer
    */
    void StartPluginRefreshTimer();

    /**
    * Starts the plugin time out timer
    */
    void StartPluginTimeoutTimer( TInt aSecs );

    
    /**
    * Callback fuction. Called when the Plugin tiemr time out
    */
    static TInt HandlePluginTimeoutTimerExpiry( TAny* aPtr );


private://data
    /**
    * Plugin interface.
    */
    MScreensaverPlugin* iPlugin;
    
    /**
    * The plugin refresh timer
    */
    CPeriodic* iPluginRefreshTimer;
    
    /**
    * The plugin timeout timer
    */
    CPeriodic* iPluginTimeoutTimer;

    
    /**
    * The plugin flag
    */
    mutable TBitFlagsT<TUint32> iPluginFlag;
    
    /**
    * The refresh rate of plugin
    */
    TInt iPluginRefreshRate;
    };
#endif // C_SCREENSAVERCTRLPLUGIN_H
