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
* Description:   Definitions screensaver moving text display class.
*
*/



#ifndef C_SCREENSAVERCTRLMOVINGTEXT_H
#define C_SCREENSAVERCTRLMOVINGTEXT_H

#include <AknPictographInterface.h>

#include "screensaverbase.h"

const TInt KScreensaverDataReconnectingDelay = 50000;
const TInt KScreensaverBarHeight = 24;

//
// class CScreensaverCtrlMovingText
//
class CScreensaverCtrlMovingText : public CScreensaverBase
    {
public:
    /**
    * Two-phased constructor
    * 
    * @param aType the type of display object  
    * @return CScreensaverCtrlMovingText the pointer of CScreensaverCtrlMovingText 
    * object
    */
    static CScreensaverCtrlMovingText* NewL( TDisplayObjectType aType );

     /**
     * Destructor.
     */
    ~CScreensaverCtrlMovingText();

public:
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

public:
    // Implemented here instead of shareddatainterface

    /**
    * Set moving bar on / off
    * 
    * @param aMove a bool type to decide if it can move or not
    */
    void SetMoving( TBool aMove = ETrue );

    /**
    * Start the control needed timer
    */
    void StartTimer();
    
    /**
    * Cancels the timer
    */
    void CancelTimer();
    
    /**
    * Clear the screen
    */
    void ClearScreen();
    
    /*
    * Refresh the display 
    */
    void Refresh();
    
protected:
        
    /**
    * Constructor
    * 
    * @param aType the type of the display object.  
    */
    CScreensaverCtrlMovingText( TDisplayObjectType aType );
    
    /**
    * Two-phased constructor  
    */
    void ConstructL();
    
    /**
    * Draw the object on the screen of this type  
    * 
    * @param aGc the CWindowGc object 
    * @param aRect
    */
    virtual void DrawNew( CWindowGc& aGc, const TRect& aRect ) const;
    
    /**
    * Draw the date on the bar
    */
    virtual void DrawText( CWindowGc& aGc ) const;

    /**
    * Draws text with pictographs  
    */
    void DrawPictoText( CWindowGc &aGc ) const;

    /**
    * Get the current time  
    */
    void UpdateCurrentTimeL();
    
    /**
    * Generate the screensaver bar
    */
    void GenerateDisplayAttributesForScreensaverBar();

private:
    /**
    * Get the random top value  
    * 
    * @return TInt the random value
    */
    void GetRandomTopValue( TInt aBarHight );
    
    /**
    * Get the random seed
    */
    TInt RandomSeed();
    
    /**
    * Get the text and background color
    */
    void GetTextAndBgColor();
    
    /**
    * Format the display text
    */
    void FormatTextL( TDes& aText, const TDesC& aFormat );

    /**
    * Set AmPm text payload to indicator
    */
    void SetAmPmIndicatorPayloadL();
    
    /**
    * Format the data and time text
    */
    void FormatTextFromResourceL( TDes &aDes, TInt aResourceId );
    
    /**
    * Get the clock area and text layout
    */
    void GetClockLayout( const TRect& aRect );
    
    /**
    * Get the date area and text layout
    */
    void GetDateLayout( const TRect& aRect );
    
    /**
    * Get the indicator area and text layout
    */
    void GetIndicatorAreaLayout( const TRect& aRect );
    
    /**
    * Start the indicator bar refresh timer
    */
    void StartBarRefreshTimer();

    
protected://data
    
    /**
    * The layout date and text  
    */
    TAknLayoutText iLayoutDateText;
private://data
    /**
    * The top position of the bar 
    */
    TInt iTop;
    
    /**
    * The random seed to get the random value   
    */
    TInt64 iRandSeed;

    // For new layout    
    /**
    * The time string that will be displayed on the bar  
    */
    TBuf<15> iTextTime;    
    
    /**
    * The date string that will be displayed on the bar  
    * or he text will be display the on the bar
    */
    TBuf<25> iText;
    
    /**
    * The type of the display text  
    */
    TDisplayObjectType iType;

    //Lay out
    TAknLayoutRect iLayoutBar;
    TAknLayoutRect iLayoutClock;
    TAknLayoutRect iLayoutDate; // missing
    TAknLayoutText iLayoutClockText;
    TAknLayoutText iLayoutAmPmText;
    
    // Moving bar on / off
    TBool iMoveBar;
    
    /**
    * The color for background and text
    */
    TInt iBgColor;
    TInt iTextColor;
    TInt iTextBgColor;
    
    /**
    * The indicator bar refresh timer
    */
    CPeriodic* iBarRefreshTimer;

    /**
    * Pictograph drawing interface 
    */
    CAknPictographInterface* iPictoI;
    };

#endif // C_SCREENSAVERCTRLMOVINGTEXT_H

// End of file.
