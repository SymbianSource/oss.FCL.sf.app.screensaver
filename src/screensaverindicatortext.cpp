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
* Description:   CScreensaverIndicatorText implementation.
*
*/



#include <AknLayoutFont.h>
#include <barsread.h>

#include "screensaverindicatortext.h"



// -----------------------------------------------------------------------------
// CScreensaverTextIndicator::Draw
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorText::Draw( CWindowGc& aGc ) const
    {
    if ( iFont )
        {
        aGc.UseFont( iFont );
        }
    aGc.SetPenColor( iTextColor );

    // Draw normal bidi text
    TBidiText* bidi= NULL;
    TRAPD( err, bidi = TBidiText::NewL( iText, 1 ) );
    if ( err == KErrNone )
        {
        bidi->WrapText( Width(), *iFont, NULL );
        if ( iTextRect.IsEmpty() )
            {
            bidi->DrawText( aGc, iTopLeft );
            }
        else
            {
            TPoint pt = iTextRect.iTl;
            pt.iY += iTextOffset;
            bidi->DrawText( aGc, pt );
            }
        delete bidi;
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverTextIndicator::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorText::ConstructL( TResourceReader &aReader,
        TUint32 aBgColor, TUint aTextColor )
    {
    CScreensaverIndicator::ConstructL( aReader, aBgColor, aTextColor );

    iFont = AknLayoutUtils::FontFromId( aReader.ReadInt32() );
    if ( !iFont )
        {
        iFont = LatinPlain12();
        }

    }

// -----------------------------------------------------------------------------
// CScreensaverTextIndicator::SetupDrawingParameters
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorText::SetupDrawingParameters( const TPoint& aCorner,
        const TRect& aParentRect )
    {
    iVisible = EFalse;

    if ( iText.Length() <= 0 )
        {
        return;
        }

    TInt textWidth = ( iFont ? iFont->TextWidthInPixels(iText) + 1
            : iText.Length() * 11 ); // Should be enough for APAC too

    iRect = TRect( 0, 0, textWidth, aParentRect.Size().iHeight );

    iTopLeft.iX = aCorner.iX;

    if ( iAlignment == ESsAlignRight )
        {
        // Change topright to topleft.      
        iTopLeft.iX -= iRect.Size().iWidth;
        }

    iTopLeft.iY = aCorner.iY + ( aParentRect.Size().iHeight / 2 ) + ( ( aParentRect.Size().iHeight - iFont->HeightInPixels() ) / 2 ) + 2;

    // Make sure text rect is not used
    iTextRect.SetRect( 0, 0, 0, 0 );

    iVisible = ETrue;
    }

// -----------------------------------------------------------------------------
// CScreensaverTextIndicator::ConstructL
// -----------------------------------------------------------------------------
//
TBool CScreensaverIndicatorText::CheckVisibilityConditions() const
    {
    return ( iText.Length() > 0 ? ETrue : EFalse );
    }

// -----------------------------------------------------------------------------
// CScreensaverTextIndicator::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorText::Payload( TIndicatorPayload &aPayload ) const
    {
    aPayload.iType = EPayloadTypeText;
    aPayload.iText = iText;
    aPayload.iInteger = -1;
    aPayload.iIsDisplayed = Visible();
    aPayload.iIcon = NULL;
    }

// -----------------------------------------------------------------------------
// CScreensaverTextIndicator::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorText::SetPayload( const TIndicatorPayload& aPayload )
    {
    if ( aPayload.iType != EPayloadTypeText )
        {
        return;
        }

    if ( aPayload.iText.Length() > iText.MaxLength() )
        {
        return; // Don't panic.      
        }

    iText = aPayload.iText;
    }

// -----------------------------------------------------------------------------
// CScreensaverTextIndicator::SetTextLayout
// Sets text position and font
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorText::SetTextLayout( TAknLayoutText& aLayout, TInt aX )
    {
    // Font from layout
    iFont = aLayout.Font();
    // Layout MUST have a font
    ASSERT( iFont != NULL );

    // Vertical dimension of text rect from layout
    iTextRect = aLayout.TextRect();

    // Calculate text offset from rect top
    const CAknLayoutFont* layoutFont =
            CAknLayoutFont::AsCAknLayoutFontOrNull( iFont );

    if ( layoutFont )
        {
        iTextOffset = layoutFont->TextPaneTopToBaseline();
        }
    else
        {
        iTextOffset = iFont->AscentInPixels();
        }

    // X-pos from parameter
    iTextRect.iTl.iX = aX;

    // Initially set indicator rect (this may later be overridden by SetWidth())
    iRect = TRect( 0, 0, PreferredWidth(), aLayout.TextRect().Height() );

    // If right-aligned, the X-pos indicates right edge - switch to left
    if ( iAlignment == ESsAlignRight )
        {
        // Change topright to topleft.      
        iTextRect.iTl.iX -= Width(); // iRect.Size().iWidth;  
        }

    // Indicator width = text rect width
    iTextRect.SetWidth( Width() );
    }

// -----------------------------------------------------------------------------
// CScreensaverTextIndicator::PreferredWidth
// Gets preferred indicator width
// -----------------------------------------------------------------------------
//
TInt CScreensaverIndicatorText::PreferredWidth()
    {
    TInt textWidth = ( iFont ? iFont->TextWidthInPixels( iText ) + 1
            : iText.Length() * 11 ); // Should be enough for APAC too
    return textWidth;
    }

// -----------------------------------------------------------------------------
// CScreensaverTextIndicator::SetWidth
// Sets indicator width
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorText::SetWidth( TInt iWidth )
    {
    iRect.SetWidth( iWidth );
    }

// -----------------------------------------------------------------------------
// CScreensaverTextIndicator::SetXPos
// Sets indicator X-position (new layout)
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorText::SetXPos( TInt aX )
    {
    iTopLeft.iX = aX;
    if ( iAlignment == ESsAlignRight )
        {
        // Change topright to topleft.      
        iTopLeft.iX -= iRect.Size().iWidth;
        }
    }

// End of file
