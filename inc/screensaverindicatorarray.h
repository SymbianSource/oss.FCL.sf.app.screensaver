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
* Description:   CScreensaverIndicatorArray declaration.
*
*/



#ifndef C_SCREENSAVERINDICATORARRAY_H
#define C_SCREENSAVERINDICATORARRAY_H

#include "screensaverindicator.h"

/*
*   Indicator array class.
*/
class CScreensaverIndicatorArray : public CBase
    {
    public:
        
        /**
    * Two-phased constructor
    * 
    * @param aOldLayout if construct the old layout or the new layout
    * @param aColorInversionUsed if use the inverted color or the 
    * normal color
    */
    static CScreensaverIndicatorArray* NewL( TBool aColorInversionUsed );

    /**
    * Destructor
    */
    ~CScreensaverIndicatorArray();

    /**
    * Two-phased constructor
    */
    void ConstructL( TBool aColorInversionUsed);

    /**
    * Creates indicator of given type and pushes it to
    * cleanup stack.
    *
    * @param aId  Indicator to be created.
    */
    CScreensaverIndicator *NewIndicatorLC(TScreensaverIndicatorType aType);

    /**
    * Draws indicator array to given graphics context. Setup function
    * must be called before calling Draw function.
    *
    * @param aGc  Graphics context for drawing indicator array.
    */
    void Draw(CWindowGc& aGc) const;

    /**
    * Sets up indicator array for drawing. Sets positions for
    * indicators.
    */
    void Setup(TAknLayoutRect& aLayoutRect, TAknLayoutText& aLayoutText,
               TAknLayoutRect& aLayoutInds);
    
    /*
    * Set the attributes of the indicator 
    */
    void SetIndicatorsAttribute( 
        TInt& Ind, 
        TAknLayoutText& aLayoutText, 
        TAknLayoutRect& aLayoutInds, 
        TInt& aCurrentX );

    /**
    * Checks if there is dependency flag set for given indicator.
    *
    * @param
    * @return ETrue  if there is dependency conflict.
    *         EFalse if not.
    */
    TBool CheckDependencyConflict(TScreensaverIndicatorId aId) const;

    /*
    * Sets dependency flag to given indicator to given value.
    * If dependency flag is set then the indicator is not drawn
    * even when it would be otherwise visible.
    */
    void SetDependencyStatus(TScreensaverIndicatorId aId, TBool aValue);

    /**
    * Queries payload for given indicator.
    *
    * @param aId ID of the indicator
    * @param aPayload Indicator payload to receive the information
    * @return  KErrNone if payload was set successfully.
    */
    TInt GetIndicatorPayload(
        TScreensaverIndicatorId aId,
        TIndicatorPayload& aPayload) const;

    /**
    * Sets payload for given indicator.
    * NOTE that icon and displayed indication cannot be set
    *
    * @param aType
    * @param aPayload
    * @return KErrNone if payload was set successfully.
    */
    TInt SetIndicatorPayload(
        TScreensaverIndicatorId aId,
        const TIndicatorPayload& aPayload);

    /**
    * Returns value indicating which drawing mode screensaver should select
    * based on indicator payloads and dependecies. This function aswers to question
    * "What drawing mode should screensaver select if indicator payloads and
    * dependencies were the only determining factor?". This doesn't mean
    * that screensaver will be necessarily drawn in that mode if there is
    * plugin module present or something happens while setting up indicator array
    * for drawing. Screensaver needs this information when it prepares itself for
    * drawing.
    */
    TScreensaverDrawType PreferredDrawingMode() const;

    /**
    * Returns value indicating which drawing mode screensaver should select.
    * This function is basically same as 'PreferredDrawingMode', but it
    * returns the final state of indicator array after it has been
    * set up (which may be different from PreferredDrawingMode).
    * Screensaver queries this information just before it draws the screen.
    */
    TScreensaverDrawType ActualDrawingMode() const;

    /**
    * Returns the height of indicator.
    */
    TInt Height() const;

    /**
    * To set visibility value for indicators
    */
    void SetVisibilityForIndicators();
    
private:
    
    /**
    * C++ default constructor
    */
    CScreensaverIndicatorArray();
    
    /**
    * Construct the Indicator from the resource file
    */
    void ConstructFromResourceL();
    
    /*
    * Constuct the screensaver param from the resource
    */
    void ConstructParamFromResourceL();
    
    /*
    * Construct indicator array from resource 
    */
    void ConstructIndArrayL();
    
    /*
    * Construct the indicators from resource 
    */
    void ConstructIndicatorsL( TInt aResourceId );
    
    /**
    * Returns the dependency record by the index
    */
    TBool Dependency(TInt aIndex) const;

private:
    
    /**
    * The count of the indicator
    */
    TInt iCount;
    
    /**
    * How many pixels between indicators.
    */
    TInt iGap;
    
    /** 
    * Minimum indicator distance to left and right border.
    */
    TInt iMargin;
    
    /**
    * Distance from top border of indicator bar to top border of 
    * indicator icon.
    */
    TInt iTopMargin;
    
    /**
    * The level of the text
    */
    TInt iTextLevel;
    
    /**
    * The area of the indicator
    */
    TRect iRect;
    
    /**
    * The height of the indicator bar
    */
    TInt iHeight;
    
    /**
    * The array of indicators
    */
    CArrayPtrFlat<CScreensaverIndicator>* iIndicators;
    
    /**
    * The array of dependency record
    */
    CArrayFixFlat<TBool>* iDependencyRecord;
    
    /**
    * The flag decised if use the old layout or not.
    */
    TBool iOldLayout;
    
    /**
    * Colors to be used for indicator layout
    */
    TUint32 iBgColor;
    TUint32 iTextColor;  
};

#endif // C_SCREENSAVERINDICATORARRAY_H
