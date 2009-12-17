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
* Description:   Definitions for screensaver indicator array.
*
*/



#ifndef C_SCREENSAVERINDICATOR_H
#define C_SCREENSAVERINDICATOR_H

//#include <aknconsts.h>
#include <coecntrl.h>
#include <coeccntx.h>
#include <e32base.h>
#include <AknIconArray.h>
#include <gulicon.h>
#include <AknsItemID.h>
#include <AknUtils.h>
#include <screensaverplugin.h>

#include "screensaver.hrh"


/*
*  Base class for all indicators.
*/
class CScreensaverIndicator : public CBase
    {
public:
    
    /**
    * Destructor  
    */
    virtual ~CScreensaverIndicator();
    
    /**
    * Draw the indicator. it should be implemented by the derived class
    */
    virtual void Draw(CWindowGc& aGc) const = 0;
    
    /**
    * two-phased constructor  
    * 
    * @param aReader the resource read of the STRUCT 
    * SCREENSAVER_PARAMETERS_ARRAY defined by ourself
    * STRUCT SCREENSAVER_PARAMETERS_ARRAY 
    * {
    * WORD KIndicatorMargin;
    * WORD KIndicatorGap;
    * WORD KIndicatorTopMargin;
    * WORD KIndicatorTextLevel;
    * LONG KBackGroundColor;
    * WORD KHeight;
    * }
    * @param aBgColor the background color of display screen
    * @param aTextColor the text color
    */
    virtual void ConstructL(TResourceReader &aReader, TUint32 aBgColor, TUint aTextColor);
    
    /**
    * Set the parameters of drawing
    * 
    * @param aCorner the position of the lefttop of the indicator
    * @param aParentRect
    */
    virtual void SetupDrawingParameters(const TPoint& aCorner, const TRect& aParentRect) = 0;
    
    /**
    * Check if indicator is visible or not
    * 
    * @return TBool
    */
    virtual TBool CheckVisibilityConditions() const = 0;
    
    /**
    * Get the payload through the property
    * 
    * @param aPayload the reference to the payload  
    */
    virtual void Payload(TIndicatorPayload& aPayload) const = 0;
    
    /**
    * Set the property through the payload
    * 
    * @param aPayload
    */
    virtual void SetPayload(const TIndicatorPayload& aPayload) = 0;
    
    /**
    * Set the Id by the parameter
    * 
    * @param aId the Id value
    */        
    void SetId(TScreensaverIndicatorId aId);
    
    /**
    * Check if visible or not
    * 
    * @return TBool if visible return ETrue, or EFalse.  
    */
    virtual TBool Visible() const;
    
    /**
    * Set the visibility
    * 
    * @param aValue
    */
    void SetVisibility(TBool aValue);
    
    /**
    * Returns the indicator width
    * 
    * @return TInt
    */
    TInt Width() const;
    
    /**
    * Returns the indicator Id
    * 
    * @return TScreensaverIndicatorId
    */
    TScreensaverIndicatorId Id() const;
    
    /**
    * Returns the indicator alighment
    * enum TScreensaverAlignment
    * {
    * ESsAlignLeft,
    * ESsAlignRight
    * };
    * 
    * @return TScreensaverAlignment
    */
    TScreensaverAlignment Alignment() const;
    
    /**
    * Returns the indicator DrawType, DrawType defined as below
    *  enum TScreensaverDrawType
    * {
    * ESsDrawTypeNotDrawn,
    * ESsDrawTypeSecondary,
    * ESsDrawTypePrimary
    * };
    * 
    * @return TScreensaverDrawType
    */
    TScreensaverDrawType DrawType() const;
    
    /**
    * Makes a skin item ID from skinned bitmap id  
    * 
    * @param aSkinBmpId
    * 
    * @return TAknsItemID
    */
    TAknsItemID MakeSkinItemId(TInt aSkinBmpId);
    
    /**
    * Sets the indicator type
    * enum TScreensaverIndicatorType
    * {
    * ESsNumericIndicator,
    * ESsIconIndicator,
    * ESsTextIndicator,
    * ESsNumberAndIconIndicator
    * };
    * 
    * @param aType the type you want to set
    */
    void SetType(TScreensaverIndicatorType aType);
    
    /**
    * Returns the type of indicator
    * 
    * @return TScreensaverIndicatorType  
    */
    TScreensaverIndicatorType Type();
    
    /**
    * Depending on indicator type these may or may not be implemented
    * in derived classes. Replaces setupdrawingparameters.
    * Used for new layout         *   
    */
    virtual void SetIconLayout(TAknLayoutRect&, TInt);
    virtual void SetTextLayout(TAknLayoutText&, TInt);
    
    /**
    * Get the desired width of the indicator (where it fits completely)
    */
    virtual TInt PreferredWidth();
    
    /**
    * Get the minimum width of the indicator
    * (absolute minimum space it can be drawn in)
    */
    virtual TInt MinimumWidth();
    
    /**
    * Set the width allowed for the indicator. This has effect only on
    * indicators that have text, which may then be truncated. Bitmap
    * widths are not adjusted  
    */
    virtual void SetWidth(TInt);
    
    /**
    * Set X-position of indicator  
    */
    virtual void SetXPos(TInt);
    
protected:
    
    /**
    * The indicator Id  
    */
    TScreensaverIndicatorId iId;
    
    /**
    * The topleft position of indicator  
    */
    TPoint iTopLeft;
    
    /**
    * The flag indicates the indicator is visible or not  
    */
    TBool  iVisible;
    
    /**
    * The area of indicator  
    */
    TRect  iRect;
    
    /**
    * The indicator draw type  
    */
    TScreensaverDrawType iDrawType;
    
    /**
    * The indicator alignment  
    */
    TScreensaverAlignment iAlignment;
    
    /**
    * The indicator type.  
    */
    TScreensaverIndicatorType iType;
    
    /**
    * Colors to be used to draw indicators  
    */
    TRgb 	iBgColor;
    TRgb 	iTextColor;
    
    };


#endif // C_SCREENSAVERINDICATOR_H

// End of file.
