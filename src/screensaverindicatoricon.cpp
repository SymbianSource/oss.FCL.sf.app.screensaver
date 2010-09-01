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
* Description:   CScreensaverIndicatorIcon implementation.
*
*/



#include <AknsUtils.h>
#include <barsread.h>

#include "screensaverindicatoricon.h"

//
// CScreensaverIconIndicator
//

// -----------------------------------------------------------------------------
// CScreensaverIconIndicator::~CScreensaverIconIndicator
// -----------------------------------------------------------------------------
//
CScreensaverIndicatorIcon::~CScreensaverIndicatorIcon()
    {
    delete iIcon;
    }

// -----------------------------------------------------------------------------
// CScreensaverIconIndicator::Draw
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorIcon::Draw(CWindowGc& aGc) const
    {
    aGc.SetPenColor(iTextColor);
    aGc.SetBrushColor(iBgColor);
    if (Visible() && iIcon)
        {
        if (!iIcon->Mask())
            {
            aGc.BitBlt(iTopLeft, iIcon->Bitmap(), iRect);
            }
        else
            {
            aGc.BitBltMasked(iTopLeft, iIcon->Bitmap(), iRect, iIcon->Mask(),
                    ETrue);
            }
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverIconIndicator::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorIcon::ConstructL(TResourceReader &aReader,
        TUint32 aBgColor, TUint aTextColor)
    {
    CScreensaverIndicator::ConstructL(aReader, aBgColor, aTextColor);

    // Icon & mask ids
    TInt id = aReader.ReadInt32();
    TInt maskid = aReader.ReadInt32();

    // Icon & mask skin identifiers
    TInt skinid = aReader.ReadInt32();
    TInt skinmaskid = aReader.ReadInt32();

    iIcon = CGulIcon::NewL();

    CFbsBitmap* bmp= NULL;
    CFbsBitmap* mask= NULL;

    // Load skinned bitmap
    AknsUtils::CreateColorIconLC(AknsUtils::SkinInstance(),
            MakeSkinItemId(skinid),
            KAknsIIDNone, // Lie so that we get default color, not skin color
            0, // No color group
            bmp, mask, AknIconUtils::AvkonIconFileName(), id, maskid,
            iTextColor);

    CleanupStack::Pop(2);

    // Save the icon
    iIcon->SetBitmap(bmp);
    iIcon->SetMask(mask);

    // Set initial size for SVG graphics, this is later altered
    // as the layout is known
    // Set desired size for SVG graphics (height from LAF, width 3 x height)
    TAknLayoutRect screenLayout;
    screenLayout.LayoutRect(TRect(0, 0, 0, 0), AknLayout::screen() );
    TRect screenRect = screenLayout.Rect();
    TAknLayoutRect powerSavePaneLayout;
    powerSavePaneLayout.LayoutRect(screenRect,
            AknLayout::Power_save_pane_descendants_Line_1() );
    TRect powerSavePaneRect = powerSavePaneLayout.Rect();
    TAknLayoutRect rect;
    rect.LayoutRect(powerSavePaneRect,
            AknLayout::Power_save_pane_descendants_Line_3() );
    TInt height = rect.Rect().Height();
    TSize iconSize(3 * height, height);
    AknIconUtils::SetSize(iIcon->Bitmap(), iconSize);
    }

// -----------------------------------------------------------------------------
// CScreensaverIconIndicator::SetupDrawingParameters
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorIcon::SetupDrawingParameters(const TPoint &aCorner,
        const TRect& aParentRect)
    {
    iVisible = EFalse;

    if (!iIcon)
        {
        return;
        }

    TSize size = iIcon->Bitmap()->SizeInPixels();
    iTopLeft.iX = aCorner.iX;
    if (iAlignment == ESsAlignRight)
        {
        // Change topright to topleft.      
        iTopLeft.iX -= size.iWidth;
        }
    // Center bitmap in y direction.
    iTopLeft.iY = aCorner.iY + (aParentRect.Size().iHeight / 2) - (size.iHeight / 2) - 2;

    iRect = TRect(0, 0, size.iWidth, size.iHeight);

    iVisible = ETrue;
    }

// -----------------------------------------------------------------------------
// CScreensaverIconIndicator::CheckVisibilityConditions
// -----------------------------------------------------------------------------
//
TBool CScreensaverIndicatorIcon::CheckVisibilityConditions() const
    {
    return (iIcon == 0 ? EFalse : ETrue);
    }

// -----------------------------------------------------------------------------
// CScreensaverIconIndicator::Payload
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorIcon::Payload(TIndicatorPayload& aPayload) const
    {
    aPayload.iType = EPayloadTypeIcon;
    aPayload.iInteger = -1;
    aPayload.iText = KNullDesC;
    aPayload.iIsDisplayed = Visible();
    aPayload.iIcon = iIcon;
    }

// -----------------------------------------------------------------------------
// CScreensaverIconIndicator::SetPayload
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorIcon::SetPayload(const TIndicatorPayload& /*aPayload*/)
    {
    // Nothing to set, payload is icon and it is created from resources.    
    }

// -----------------------------------------------------------------------------
// CScreensaverIconIndicator::SetIconLayout
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorIcon::SetIconLayout(TAknLayoutRect& aLayout, TInt aX)
    {
    if (!iIcon)
        {
        ASSERT(iIcon);
        return;
        }

    // Resize icon according to layout
    TInt height = aLayout.Rect().Height();
    TSize iconSize(KMaxTInt, height);
    AknIconUtils::SetSize(iIcon->Bitmap(), iconSize,
            EAspectRatioPreservedAndUnusedSpaceRemoved);

    // Set vertical icon position according to layout (this overrides
    // what's already there. Also set correct width in iRect
    iTopLeft.iY = aLayout.Rect().iTl.iY;
    iTopLeft.iX = aX;
    TSize size = iIcon->Bitmap()->SizeInPixels();
    iRect = TRect(0, 0, size.iWidth, size.iHeight);

    if (iAlignment == ESsAlignRight)
        {
        // Change topright to topleft.
        iTopLeft.iX -= size.iWidth;
        }

#ifdef SS_LAYOUTTRACE
    SCRLOGGER_WRITEF(_L("SCR: Icon indicator (%d):"), iId);
    SCRLOGGER_WRITEF(_L("SCR:   Rect: (%d,%d,%d,%d)"),
            iRect.iTl.iX,
            iRect.iTl.iY,
            iRect.iBr.iX,
            iRect.iBr.iY);
    SCRLOGGER_WRITEF(_L("SCR:   Size: (%d,%d)"), size.iWidth, size.iHeight);
    SCRLOGGER_WRITEF(_L("SCR:   Pos:  (%d,%d)"), iTopLeft.iX, iTopLeft.iY);
#endif
    }


// End of file
