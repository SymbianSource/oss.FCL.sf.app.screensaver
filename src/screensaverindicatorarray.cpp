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
* Description:   CScreensaverIndicatorArray implementation.
*
*/



#include <barsread.h>
#include <screensaver.rsg>
#include <AknUtils.h>

#include "screensaverindicatorarray.h"
#include "screensaverindicatornumberandicon.h"
#include "screensaverindicatortext.h"
#include "screensaverindicatoricon.h"
#include "screensaverutility.h"

const TInt KInvertedColors = 1;


// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::NewL
// -----------------------------------------------------------------------------
//
CScreensaverIndicatorArray* CScreensaverIndicatorArray::NewL( TBool aColorInversionUsed )
    {
    CScreensaverIndicatorArray* array = new(ELeave) CScreensaverIndicatorArray();
    CleanupStack::PushL(array);
    array->ConstructL( aColorInversionUsed );
    CleanupStack::Pop(); // array

    return array;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::~CScreensaverIndicatorArray
// -----------------------------------------------------------------------------
//
CScreensaverIndicatorArray::~CScreensaverIndicatorArray()
    {
    if (iIndicators)
        {
        iIndicators->ResetAndDestroy();
        }
    delete iIndicators;
    delete iDependencyRecord;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::NewIndicatorLC
// -----------------------------------------------------------------------------
//
CScreensaverIndicator *CScreensaverIndicatorArray::NewIndicatorLC(
        TScreensaverIndicatorType aId)
    {
    CScreensaverIndicator *result= NULL;

    switch (aId)
        {
        case ESsIconIndicator:
            result = new( ELeave ) CScreensaverIndicatorIcon();
            break;
        case ESsTextIndicator:
            result = new( ELeave ) CScreensaverIndicatorText();
            break;
        case ESsNumberAndIconIndicator:
            result = new( ELeave ) CScreensaverIndicatorNumberAndIcon();
            break;
        default:
            User::Leave(KErrCorrupt);
        }

    CleanupStack::PushL( result );
    result->SetType(aId);

    return result;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorArray::ConstructL( TBool aColorInversionUsed )
    {
    if (aColorInversionUsed == KInvertedColors)
        {
        iBgColor = TRgb(KRgbBlack).Value();
        iTextColor = TRgb(KRgbWhite).Value();
        }
    else //aColorInversionUsed == KNormalColors
        {
        iBgColor = TRgb(KRgbWhite).Value();
        iTextColor = TRgb(KRgbBlack).Value();
        }
    
    ConstructFromResourceL();

    }


// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::ConstructFromResourceL
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorArray::ConstructFromResourceL()
    {
    ConstructParamFromResourceL();

    ConstructIndArrayL();
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::ConstructParamFromResourceL
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorArray::ConstructParamFromResourceL()
    {
    TResourceReader reader;

    CEikonEnv::Static()->CreateResourceReaderLC( reader, R_SCREENSAVER_PARAMETERS );
    iMargin = reader.ReadInt16();
    iGap = reader.ReadInt16();
    iTopMargin = reader.ReadInt16();
    iTextLevel = reader.ReadInt16();
    iHeight = reader.ReadInt16();
    CleanupStack::PopAndDestroy(); // reader
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::ConstructIndArrayL
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorArray::ConstructIndArrayL()
    {
    if( AknLayoutUtils::ScalableLayoutInterfaceAvailable() )
        {//use the new layout
        ConstructIndicatorsL( R_SCREENSAVER_INDICATOR_ARRAY_NEW_LAYOUT );
        }
    else
        {//use the old layout
        ConstructIndicatorsL( R_SCREENSAVER_INDICATOR_ARRAY );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::ConstructIndicatorsL
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorArray::ConstructIndicatorsL( TInt aResourceId )
    {
    TResourceReader reader;

    CEikonEnv::Static()->CreateResourceReaderLC(
        reader, aResourceId);
    
    iCount = reader.ReadInt16();

    iIndicators = new( ELeave ) CArrayPtrFlat<CScreensaverIndicator>( iCount );
    iDependencyRecord = new( ELeave ) CArrayFixFlat<TBool>( iCount );

    CScreensaverIndicator *nInd;
    TInt32 type;
    for (TInt i = 0; i < iCount; i++)
        {
        iDependencyRecord->AppendL( ETrue );
        type = reader.ReadInt32();
        nInd = NewIndicatorLC( ( TScreensaverIndicatorType ) type );
        nInd->ConstructL( reader, iBgColor, iTextColor );
        iIndicators->InsertL( 0, nInd );
        CleanupStack::Pop(); // nInd
        }

    CleanupStack::PopAndDestroy(); // reader
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::Dependency
// -----------------------------------------------------------------------------
//
TBool CScreensaverIndicatorArray::Dependency(TInt aIndex) const
    {
    return (*iDependencyRecord)[aIndex];
    }
// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::Setup
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorArray::Setup( TAknLayoutRect& aLayoutRect,
        TAknLayoutText& aLayoutText, TAknLayoutRect& aLayoutInds )
    {
    // Reset drawing parameters (do not use those defined in resources)
    iRect = aLayoutRect.Rect();

    TInt rightMargin = 0;
    // Left margin = indicators left edge - indicator area left edge
    // Right margin = 0, the AM/PM indicator needs to be on the edge
    // Except when mirrored the other way around. Right margin can be reduced
    // by 1 when mirrored, because the icons contain a border on top & right 
    if ( AknLayoutUtils::LayoutMirrored() )
        {
        iMargin = 0;
        rightMargin = aLayoutInds.Rect().iTl.iX - iRect.iTl.iX - 1;
        }
    else
        {
        iMargin = aLayoutInds.Rect().iTl.iX - iRect.iTl.iX;
        rightMargin = 0;
        }

    TInt currentX = iRect.iBr.iX - rightMargin;

    TInt leftmostRightAlignedBorder = iRect.iBr.iX - rightMargin; // iMargin;

    // This is done in two passes, first we set up right-aligned indicators
    // and then left-aligned. If any left-aligned indicator overlaps with leftmost
    // right-aligned indicator the the process is stopped (right-aligned
    // indicators have higher priority).
    for ( TInt i = 0; i < iIndicators->Count(); i++ )
        {
        if (iIndicators->At(i)->Alignment() != ESsAlignRight)
            {
            continue;
            }
        
        SetIndicatorsAttribute( i, aLayoutText, aLayoutInds, currentX );

        if (iIndicators->At(i)->Visible())
            {
            currentX -= iIndicators->At(i)->Width() + iGap;
            leftmostRightAlignedBorder = currentX;
            if (currentX < iMargin)
                {
                break;
                }
            }
        }

    // Start second round from left edge
    currentX = iRect.iTl.iX + iMargin;
    for ( TInt i = 0; i < iIndicators->Count(); i++ )
        {
        if (iIndicators->At(i)->Alignment() != ESsAlignLeft)
            {
            continue;
            }
        
        SetIndicatorsAttribute( i, aLayoutText, aLayoutInds, currentX );

        if (iIndicators->At(i)->Visible())
            {
            if (currentX + iIndicators->At(i)->Width() + iGap >= leftmostRightAlignedBorder)
                {
                iIndicators->At(i)->SetVisibility(EFalse);
                break;
                }
            currentX += iIndicators->At(i)->Width() + iGap;
            if (currentX > iRect.Width() - iMargin)
                {
                break;
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::SetIndicatorsAttribute
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorArray::SetIndicatorsAttribute( 
        TInt& Ind,
        TAknLayoutText& aLayoutText,
        TAknLayoutRect& aLayoutInds,
        TInt& aCurrentX )
    {

    // Set text layout for text containing indicators
    iIndicators->At(Ind)->SetTextLayout(aLayoutText, aCurrentX);

    // Set icon layout for icon containing indicators
    iIndicators->At(Ind)->SetIconLayout(aLayoutInds, aCurrentX);

    // Not visible, if conditions (e.g. there's icon & payload) are not met,
    // or there's no reason to show the indicator
    if (!iIndicators->At(Ind)->CheckVisibilityConditions() || Dependency(iIndicators->At(Ind)->Id()))
        {
        iIndicators->At(Ind)->SetVisibility(EFalse);        
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::Draw
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorArray::Draw(CWindowGc& aGc) const
    {
    aGc.SetBrushColor(iBgColor);
    aGc.SetPenColor(iTextColor);
    aGc.Clear(iRect);

    for (TInt i = 0; i < iIndicators->Count(); i++)
        {
        if (iIndicators->At(i)->Visible())
            {
            iIndicators->At(i)->Draw(aGc);
            }
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::CheckDependencyConflict
// -----------------------------------------------------------------------------
//
TBool CScreensaverIndicatorArray::CheckDependencyConflict(
        TScreensaverIndicatorId aId) const
    {
    if ((iDependencyRecord) && (aId < iCount))
        {
        return Dependency(aId);
        }

    return EFalse;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::SetDependencyStatus
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorArray::SetDependencyStatus(
        TScreensaverIndicatorId aId, TBool aValue)
    {
    if ((iDependencyRecord) && (aId < iCount))
        {
        (*iDependencyRecord)[aId] = aValue;
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::GetIndicatorPayload
// -----------------------------------------------------------------------------
//
TInt CScreensaverIndicatorArray::GetIndicatorPayload(
        TScreensaverIndicatorId aId, TIndicatorPayload& aPayload) const
    {
    if (aId < iCount)
        {
        for (TInt i = 0; i < iIndicators->Count(); i++)
            {
            if (aId == iIndicators->At(i)->Id())
                {
                iIndicators->At(i)->Payload(aPayload);
                return KErrNone;
                }
            }
        }

    return KErrCorrupt;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::SetIndicatorPayload
// -----------------------------------------------------------------------------
//
TInt CScreensaverIndicatorArray::SetIndicatorPayload(
        TScreensaverIndicatorId aId, const TIndicatorPayload& aPayload)
    {
    if (aId < iCount)
        {
        for (TInt i = 0; i < iIndicators->Count(); i++)
            {
            if (aId == iIndicators->At(i)->Id())
                {
                iIndicators->At(i)->SetPayload(aPayload);
                return KErrNone;
                }
            }
        }

    return KErrCorrupt;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::PreferredDrawingMode
// -----------------------------------------------------------------------------
//
TScreensaverDrawType CScreensaverIndicatorArray::PreferredDrawingMode() const
    {
    TScreensaverDrawType result = ESsDrawTypeNotDrawn; // Default mode.

    for (TInt i = 0; i < iIndicators->Count(); i++)
        {
        if ((iIndicators->At(i)->CheckVisibilityConditions() == EFalse)
                || (CheckDependencyConflict(iIndicators->At(i)->Id())))
            {
            // This indicator can't be visible or there is 
            // dependency conflict -> continue.
            continue;
            }

        switch (iIndicators->At(i)->DrawType())
            {
            case ESsDrawTypePrimary:
                // Primary draw type overides everything,
                // so we can return immediately. 
                return ESsDrawTypePrimary;
            case ESsDrawTypeSecondary:
                result = ESsDrawTypeSecondary;
                break;
            default:
                break;
            }
        }

    return result;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::ActualDrawingMode
// -----------------------------------------------------------------------------
//
TScreensaverDrawType CScreensaverIndicatorArray::ActualDrawingMode() const
    {
    TScreensaverDrawType result = ESsDrawTypeNotDrawn; // Default mode.
    
    for (TInt i = 0; i < iIndicators->Count(); i++)
        {
        if ((iIndicators->At(i)->CheckVisibilityConditions() == EFalse)
                || !( iIndicators->At(i)->Visible() ) )
            {
            // This indicator can't be visible or it is visible
            continue;
            }
        
        switch (iIndicators->At(i)->DrawType())
            {
            case ESsDrawTypePrimary:
                return ESsDrawTypePrimary; // Primary draw type overides everything.
            case ESsDrawTypeSecondary:
                result = ESsDrawTypeSecondary;
                break;
            default:
                break;
            }
        }

    return result;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::Height
// -----------------------------------------------------------------------------
//
TInt CScreensaverIndicatorArray::Height() const
    {
    return iHeight;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::SetVisibilityForIndicators
// -----------------------------------------------------------------------------
//
void CScreensaverIndicatorArray::SetVisibilityForIndicators()
    {
    for (int i = 0; i < iIndicators->Count(); i++)
        {
        // Not visible, if conditions (e.g. there's icon & payload) are not met,
        // or there's no reason to show the indicator
        if (!iIndicators->At(i)->CheckVisibilityConditions() || Dependency(iIndicators->At(i)->Id()))
            {
            iIndicators->At(i)->SetVisibility(EFalse);
            }
        else
            iIndicators->At(i)->SetVisibility(ETrue);
        }

    }

// -----------------------------------------------------------------------------
// CScreensaverIndicatorArray::CScreensaverIndicatorArray
// -----------------------------------------------------------------------------
//
CScreensaverIndicatorArray::CScreensaverIndicatorArray()
    {
    
    }

// End of files
