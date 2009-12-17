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
* Description:   CScreensaverIndicatorNumberAndIcon implementation.
*
*/



#include <barsread.h>
#include <AknsUtils.h>
#include <AknLayoutFont.h>

#include "screensaverindicatornumberandicon.h"


// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::~CScreensaverNumberAndIconIndicator
// -----------------------------------------------------------------------------
//
CScreensaverIndicatorNumberAndIcon::~CScreensaverIndicatorNumberAndIcon()
    {
    delete iIcon;
    }

// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::Draw
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorNumberAndIcon::Draw( CWindowGc& aGc ) const
    {
    TBuf<10> nStr;
    nStr.AppendNum( iValue );

    aGc.SetPenColor( iTextColor );
    aGc.SetBrushColor( iBgColor );

    if ( iFont )
        {
        aGc.UseFont( iFont );
        }
    else
        {
        ASSERT( iFont != NULL );
        aGc.UseFont( CEikonEnv::Static()->AnnotationFont() );
        }

    if ( Visible() && iIcon )
        {
        // Convert number according to language
        AknTextUtils::DisplayTextLanguageSpecificNumberConversion( nStr );

        DrawIcon( aGc );

        // Draw number beside icon
        if ( ( iTextRect.IsEmpty() ) || ( !iFont ) )
            {
            // Draw using text position (old layout)
            aGc.DrawText( nStr, iTextPos );
            }
        else
            {
            // Use text rect
            ASSERT( iTextOffset> 0 );
            aGc.DrawText( nStr, iTextRect, iTextOffset );
            }
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorNumberAndIcon::ConstructL( TResourceReader &aReader,
        TUint32 aBgColor, TUint aTextColor )
    {
    CScreensaverIndicator::ConstructL( aReader, aBgColor, aTextColor );

    iIconAlign = ( TScreensaverAlignment )aReader.ReadInt32();
    iFont = AknLayoutUtils::FontFromId( aReader.ReadInt32() );
    if ( !iFont )
        {
        iFont = LatinPlain12();
        }

    // create icon
    InitializeIconL( aReader );

    // Set desired size for SVG graphics (height from LAF, width 3 x height)
    InitializeLayout();
    }

// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::SetupDrawingParameters
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorNumberAndIcon::SetupDrawingParameters(
        const TPoint& aCorner, const TRect& aParentRect )
    {
    iVisible = EFalse;

    TBuf<10> nStr;

    if ( !iIcon )
        {
        return;
        }

    nStr.AppendNum( iValue );

    TSize bmsize = iIcon->Bitmap()->SizeInPixels();
    TInt textWidth = iFont ? iFont->TextWidthInPixels( nStr ) + 1 : nStr.Length()
            * 11; // Should be enough for APAC too

    iTopLeft.iX = aCorner.iX;
    if ( iAlignment == ESsAlignRight )
        {
        // Change topright to topleft.      
        iTopLeft.iX -= bmsize.iWidth + textWidth;
        }
    // Center bitmap in y direction.
    iTopLeft.iY = aCorner.iY + ( aParentRect.Size().iHeight / 2 ) - ( bmsize.iHeight / 2 ) - 2;

    // Calculate text corner
    if ( iIconAlign == ESsAlignLeft )
        {
        iTextPos.iX = iTopLeft.iX + bmsize.iWidth + 1;
        }
    else
        {
        iTextPos.iX = iTopLeft.iX;
        }

    iTextPos.iY = aCorner.iY + iRect.Size().iHeight / 2 + 4;

    iRect = TRect( 0, 0, bmsize.iWidth + textWidth, aParentRect.Size().iHeight );

    // Make sure text rect is not used
    iTextRect.SetRect( 0, 0, 0, 0 );

    iVisible = ETrue;
    }

// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::CheckVisibilityConditions
// -----------------------------------------------------------------------------
//
TBool CScreensaverIndicatorNumberAndIcon::CheckVisibilityConditions() const
    {
    if ( ( iIcon ) && ( iValue > 0 ) )
        {
        return ETrue;
        }

    return EFalse;
    }

// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::Payload
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorNumberAndIcon::Payload( TIndicatorPayload& aPayload ) const
    {
    aPayload.iType = EPayloadTypeInteger;
    aPayload.iInteger = iValue;
    aPayload.iText = KNullDesC;
    aPayload.iIsDisplayed = Visible();
    aPayload.iIcon = iIcon;
    }

// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::SetPayload
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorNumberAndIcon::SetPayload(
        const TIndicatorPayload& aPayload )
    {
    if ( aPayload.iType != EPayloadTypeInteger )
        {
        return;
        }

    iValue = aPayload.iInteger;
    }

// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::SetIconLayout
// Sets icon position and size
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorNumberAndIcon::SetIconLayout( TAknLayoutRect& aLayout,
        TInt aX )
    {
    if ( !iIcon )
        {
        ASSERT( iIcon );
        return;
        }

    // Resize icon according to layout
    TInt height = aLayout.Rect().Height();
    TSize iconSize(KMaxTInt, height);
    AknIconUtils::SetSize( iIcon->Bitmap(), iconSize,
            EAspectRatioPreservedAndUnusedSpaceRemoved );

    // Overrides vertical icon position set earlier
    iTopLeft.iY = aLayout.Rect().iTl.iY;

    // Set initial rect
    iRect = TRect( 0, 0, PreferredWidth(), height );

    // Set X-position
    SetXPos( aX );

#ifdef SS_LAYOUTTRACE
    SCRLOGGER_WRITEF( _L("SCR: NumberAndIcon indicator (%d):"), iId );
    SCRLOGGER_WRITEF( _L("SCR:   Icon rect: (%d,%d,%d,%d)"),
            iRect.iTl.iX,
            iRect.iTl.iY,
            iRect.iBr.iX,
            iRect.iBr.iY );
    TSize size = iIcon->Bitmap()->SizeInPixels();
    SCRLOGGER_WRITEF( _L("SCR:   Icon size: (%d,%d)"), size.iWidth, size.iHeight );
    SCRLOGGER_WRITEF( _L("SCR:   Icon pos:  (%d,%d)"), iTopLeft.iX, iTopLeft.iY );
#endif
    }

// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::SetTextLayout
// Sets text position and font
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorNumberAndIcon::SetTextLayout( TAknLayoutText& aLayout,
        TInt /* aX */ )
    {
    // Font from layout
    iFont = aLayout.Font();
    // Layout MUST have a font
    ASSERT( iFont != NULL );

    // Vertical dimension of text rect from layout
    // (x-dims will be set separately)
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
    }

// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::PreferredWidth
// Return width where the whole indicator can be drawn
// -----------------------------------------------------------------------------
//
TInt CScreensaverIndicatorNumberAndIcon::PreferredWidth()
    {
    // Preferred width = icon width + text width + a little gap in between
    TBuf<10> nStr;
    nStr.AppendNum( iValue );

    TInt width = iFont ? iFont->TextWidthInPixels( nStr ) : nStr.Length() * 11; // Should be enough for APAC too

    // Add icon width
    width += MinimumWidth();

    // Add a little gap between text and icon
    return width + 1;
    }

// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::MinimumWidth
// Get minimum width 
// -----------------------------------------------------------------------------
//
TInt CScreensaverIndicatorNumberAndIcon::MinimumWidth()
    {
    // Minimum width is icon width
    return ( iIcon ? ( iIcon->Bitmap()->SizeInPixels()).iWidth : 0 );
    }

// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::SetXPos
// Sets the x-position for the indicator
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorNumberAndIcon::SetXPos( TInt aX )
    {
    iVisible = EFalse;

    if ( !iIcon )
        {
        return;
        }

    TBuf<10> nStr;
    nStr.AppendNum( iValue );

    TSize bmsize = iIcon->Bitmap()->SizeInPixels();
    TInt textWidth = iFont ? iFont->TextWidthInPixels( nStr ) + 1 : nStr.Length()
            * 11; // Should be enough for APAC too

    iTopLeft.iX = aX;
    if ( iAlignment == ESsAlignRight )
        {
        // Change topright to topleft.      
        iTopLeft.iX -= bmsize.iWidth + textWidth;
        }

    // Calculate text corner
    if ( iIconAlign == ESsAlignLeft )
        {
        iTextRect.iTl.iX = iTopLeft.iX + bmsize.iWidth;
        }
    else
        {
        iTextRect.iTl.iX = iTopLeft.iX;
        }

    iTextRect.SetWidth( textWidth );

    iVisible = ETrue;
    }

// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::InitializeLayout
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorNumberAndIcon::InitializeLayout()
    {
    // Set desired size for SVG graphics (height from LAF, width 3 x height)
    TAknLayoutRect screenLayout;
    screenLayout.LayoutRect( TRect(0, 0, 0, 0), AknLayout::screen() );
    TRect screenRect = screenLayout.Rect();
    TAknLayoutRect powerSavePaneLayout;
    powerSavePaneLayout.LayoutRect( screenRect,
            AknLayout::Power_save_pane_descendants_Line_1() );
    TRect powerSavePaneRect = powerSavePaneLayout.Rect();
    TAknLayoutRect rect;
    rect.LayoutRect( powerSavePaneRect,
            AknLayout::Power_save_pane_descendants_Line_3() );
    TInt height = rect.Rect().Height();
    TSize iconSize( 2 * height, height );
    AknIconUtils::SetSize( iIcon->Bitmap(), iconSize );
    }

// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::InitializeIconL
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorNumberAndIcon::InitializeIconL( TResourceReader &aReader )
    {
    // Icon & mask ids
    TInt id = aReader.ReadInt32();
    TInt maskid = aReader.ReadInt32();

    // Icon & mask skin identifiers 
    TInt skinid = aReader.ReadInt32();
    TInt skinmask = aReader.ReadInt32();

    iIcon = CGulIcon::NewL();

    CFbsBitmap* bmp= NULL;
    CFbsBitmap* mask= NULL;

    // Load skinned bitmap
    AknsUtils::CreateColorIconLC( AknsUtils::SkinInstance(),
            MakeSkinItemId( skinid ),
            KAknsIIDNone, // Lie so that we get default color, not skin color
            0, // No color group
            bmp, mask, AknIconUtils::AvkonIconFileName(), id, maskid,
            iTextColor );

    CleanupStack::Pop( 2 );

    // Save the icon
    iIcon->SetBitmap( bmp );
    iIcon->SetMask( mask );
    }

// -----------------------------------------------------------------------------
// CScreensaverNumberAndIconIndicator::DrawIcon
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorNumberAndIcon::DrawIcon( CWindowGc& aGc ) const
    {
    if ( iIconAlign == ESsAlignLeft )
        {
        if ( !iIcon->Mask() )
            {
            aGc.BitBlt( iTopLeft, iIcon->Bitmap(), iRect );
            }
        else
            {
            aGc.BitBltMasked( iTopLeft, iIcon->Bitmap(), iRect,
                    iIcon->Mask(), ETrue );
            }
        }
    else
        {
        // Calculate icon left point (right edge - icon width)
        TPoint pTmp = TPoint( iTopLeft.iX + iRect.Size().iWidth - iIcon->Bitmap()->SizeInPixels().iWidth, iTopLeft.iY );

        if ( !iIcon->Mask() )
            {
            aGc.BitBlt( pTmp, iIcon->Bitmap(), iRect );
            }
        else
            {
            aGc.BitBltMasked( pTmp, iIcon->Bitmap(), iRect, iIcon->Mask(),
                    ETrue );
            }
        }
    }

// End of file
