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
* Description:   Definitions for base class of display object
*
*/



#ifndef C_SCREENSAVERBASE_H
#define C_SCREENSAVERBASE_H

#include <coeccntx.h>
#include <coecntrl.h>
#include <AknPictographDrawerInterface.h>

#include "screensaverplugin.h"
#include "screensaver.hrh"
#include "screensaverengine.h"
#include "screensaverindicator.h"
#include "screensaverindicatorarray.h"


/**
 *  The base class of all the different display object
 */
class CScreensaverBase : public CCoeControl, public MCoeControlContext,
    public MAknPictographAnimatorCallBack
    {
public:
    /**
     * Default c++ constructor  
     */
    CScreensaverBase();
    
    /**
    * Destructor  
    */
    virtual ~CScreensaverBase();

    /**
    * Show the control and start the refresh timer
    */
    virtual void StartDrawObject();
    
    /**
    * Stop display the control and cancel the timers
    */
    virtual void StopDrawObject();

    /**
    * Start the control needed timer
    */
    virtual void StartTimer() = 0;
    
    /**
    * Cancels the timer
    */
    virtual void CancelTimer() = 0;
    
    /**
    * Show the control
    */
    virtual void DrawObject();
    
    /**
    * Clear the screen
    */
    virtual void ClearScreen() = 0;
    
    /*
    * Refresh the display 
    */
    virtual void Refresh() = 0;
    
    /**
    * Get the screensaver view
    */
    CScreensaverView* View() const;
    
    /**
    * Returns the Screensaver Model which is owned by AppUi
    * 
    * @return CScreensaverModel
    */
    const CScreensaverEngine& Model() const;
    
    /**
    * Returns the Indicator Array which is owned by Screensaver Model
    * 
    * @return CScreensaverIndicatorArray
    */
    CScreensaverIndicatorArray& Array() const;
public:
    //From CCoeControl
    
    /** 
    * Resource change handling  
    */
    virtual void HandleResourceChange( TInt aType ) = 0;

    /**
    * Responds to changes to the size and position of the contents 
    * of this control.  
   */
    virtual void SizeChanged() = 0;
    
    // Power save mode related functions
    
    /**
    * Used to define the active area of display in power save mode
    * 
    * @param aRect
    */
    TInt SetPowerSaveDisplayActiveArea( TRect& aRect );
    
    /**
    * Activates power save display mode
    * 
    * @param aFullColors
    */
    TInt ActivatePowerSaveDisplay( TBool aFullColors = EFalse ) const;

public:
    // From MAknPictograpAnimatorCallback
    /**
    * This function is called when a redraw is needed for drawing
    * new frames of animated pictographs. It should
    *
    * 1) Draw or clear the background of pictographs
    * 2) If the pictographs are amongst text, draw the text
    * 3) Draw the pictographs using MAknPictographDrawer interface
    *
    * In many cases, it is sufficient to implement this simply by
    * calling CCoeControl::DrawNow for the appropriate control.
    */
    virtual void DrawPictographArea();

protected:
    
    /**
    * Start capturing timer
    */
    void StartCaptureScreenTimer();
    
    /**
    * Initial the initial refresh rate
    */
    TInt InitialRefreshRate();
    
    /**
    * Kill the timer
    * 
    * @param aTimer the timer you want stop
    */
    void DeleteTimer( CPeriodic*& aTimer );
    
    /**
    * for Lcd Partial mode. 
    */
    void ConstructAndConnectLCDL();
    
    /**
    * Returns the powersave display mode
    */
    CPowerSaveDisplayMode* LcdPartialMode() const;
    
    /**
    * If the timer expired, excute the refresh action 
    */
    static TInt HandleRefreshTimerExpiry( TAny* aPtr );
    
    /**
    * Callback fuction. Called when the window server heartbeat tiemr time out
    */
    static TInt HandleCaptureTimerExpiry( TAny* aPtr );
    
    /**
    * Capture screen, and get the first pixel address of the bitmap
    */
    TInt CaptureScreen();

protected:

    // Area of screen that is displayed when in power save mode 
    TRect iPowerSaveDisplayActiveArea;
    
    /**
    * The capturing screen timer
    */
    CPeriodic* iCaptureScreenTimer;
    
    
    /**
    * The pointer to the powersaver display mode
    * 
    * Owned
    */
    CPowerSaveDisplayMode* iPowerSaveDisplayMode;

    /**
    * store the bitmap data
    */
    CFbsBitmap* iOffScrnBmp; 
    
    };

#endif // C_SCREENSAVERBASE_H

