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
* Description:   Definitions for the display object of none type
*
*/



#ifndef C_SCREENSAVERCTRLNONE_H
#define C_SCREENSAVERCTRLNONE_H

#include "screensaverbase.h"

/**
 * The none type of the display object to cut off the screen light  
 */
class CScreensaverCtrlNone : public CScreensaverBase
    {
public:
    /**
    * The symbian c++ constructor
    */
    static CScreensaverCtrlNone* NewL();
    
    
    /**
    * destructor
    */
    ~CScreensaverCtrlNone();

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
    
public:
    // From CCoeControl
    
    /** 
    * Resource change handling  
    */
    void HandleResourceChange( TInt aType );

    /**
    * Responds to changes to the size and position of the contents 
    * of this control.  
    */
    void SizeChanged();
    
    /**
     * Draw
     * @see CCoeControl::Draw(const TRect& aRect)
     */
    void Draw( const TRect& aRect ) const;
    
private:

    /**
    *The Default C++ constructor   
    */
    CScreensaverCtrlNone();
    
    /**
    * The two-phased constructor
    */
    void ConstructL();

    /**
    * Switch off light in aSecs
    * 
    * @param aSecs after this seconds, screen will switch light
    */
    void SwitchLights( TInt aSecs );
    
    /**
    * Switch on/off the screen
    */
    void SwitchDisplayState( TInt aState );
    
    /**
     * Callback for closing the light and display resources
     * 
     * @param aPtr pointer to CScreensaverCtrlNone
     */
    static TInt CloseDisplayResource( TAny* aPtr );
    
    /**
     * Timer for check and close display&light resource after refresh.
     * There is possibility that light is turned on responding to some
     * notifications(e.g new SMS). This timer makes sure that display
     * resources are closed again after such events. 
     */
    CPeriodic* iTimer;
    
    };

#endif // C_SCREENSAVERCTRLNONE_H
