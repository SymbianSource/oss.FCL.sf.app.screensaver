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
* Description:   CScreensaverIndicatorNumberAndIcon declaration.
*
*/



#ifndef C_SCREENSAVERINDICATORNUMBERANDICON_H
#define C_SCREENSAVERINDICATORNUMBERANDICON_H

#include "screensaverindicator.h"

class CScreensaverIndicatorNumberAndIcon : public CScreensaverIndicator
    {
public:
    ~CScreensaverIndicatorNumberAndIcon();
    
    /**
    * Draw the indicator. it should be implemented by the derived class
    */
    virtual void Draw( CWindowGc& aGc ) const;
    
    /**
    * two-phased constructor  
    * 
    * @param aReader the resource read of the STRUCT 
    * SCREENSAVER_PARAMETERS_ARRAY defined by ourself
    * @param aBgColor the background color of display screen
    * @param aTextColor the text color
    */
    virtual void ConstructL( TResourceReader &aReader, TUint32 aBgColor, TUint aTextColor );
    
    /**
    * Set the parameters of drawing
    * 
    * @param aCorner the position of the lefttop of the indicator
    * @param aParentRect
    */
    virtual void SetupDrawingParameters( const TPoint& aCorner, const TRect& aParentRect );
    
    /**
    * Check if indicator is visible or not
    * 
    * @return TBool
    */
    virtual TBool CheckVisibilityConditions() const;
    
    /**
    * Get the payload through the property
    * 
    * @param aPayload the reference to the payload  
    */
    virtual void Payload( TIndicatorPayload& aPayload ) const;
    
    /**
    * Set the property through the payload
    * 
    * @param aPayload
    */
    virtual void SetPayload( const TIndicatorPayload& aPayload );
    
    /**
    * Sets icon position and size
    */
    virtual void SetIconLayout( TAknLayoutRect& aLayout, TInt aX = -1 ); 

    /**
    * Sets text position and font
    */
    virtual void SetTextLayout( TAknLayoutText& aLayout, TInt aX = -1 );

    /**
    * Width function overrides
    */
    virtual TInt PreferredWidth();
    
    /**
    * Returns the icon width
    */
    virtual TInt MinimumWidth();

    /**
    * virtual void SetWidth(TInt aWidth);
    */
    virtual void SetXPos( TInt aX );

    /**
    * Initialize layout of graphics;
    */
    virtual void InitializeLayout();

    /**
    * Initialize icon;
    */
    virtual void InitializeIconL( TResourceReader &aReader );

    /**
    * draw icon;
    */
    virtual void DrawIcon( CWindowGc& aGc ) const;

private:

    /**
    * The icon will be displayer
    * 
    * Owned
    */
    CGulIcon* iIcon;
    
    /**
    *
    */
    TInt iValue;
    
    /**
    * The font of the text
    * Not owned.
    */
    const CFont *iFont;
    
    /**
    * The icon alignment
    */
    TScreensaverAlignment iIconAlign;
    
    /**
    * Text position for legacy layout
    */
    TPoint iTextPos;
    
    /**
    * Rect inside which text is drawn
    */
    TRect iTextRect;
    
    /**
    * Text baseline offset from text rect top
    */
    TInt iTextOffset;
};

#endif // C_SCREENSAVERINDICATORNUMBERANDICON_H
