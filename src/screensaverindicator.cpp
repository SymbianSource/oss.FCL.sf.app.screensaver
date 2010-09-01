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
* Description:   Implementation screensaver indicator array.
*
*/



#include <AknUtils.h>
#include <barsread.h>
#include <biditext.h>
#include <AknsConstants.h>
#include <AknsUtils.h>
#include <aknlayout.cdl.h>
#include <AknLayoutFont.h>
#include <screensaver.rsg>

#include "screensaverplugin.h"
#include "screensaverindicator.h"
#include "ScreensaverUtils.h"



//
// CSCreensaverIndicator
//

// -----------------------------------------------------------------------------
// CScreensaverIndicator::~CScreensaverIndicator
// -----------------------------------------------------------------------------
//
CScreensaverIndicator::~CScreensaverIndicator()
    {    
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverIndicator::ConstructL(TResourceReader &aReader,
        TUint32 aBgColor, TUint aTextColor)
    {
    iId = (TScreensaverIndicatorId)aReader.ReadInt16();
    iDrawType = (TScreensaverDrawType)aReader.ReadInt16();
    iAlignment = (TScreensaverAlignment)aReader.ReadInt16();

    // Switch left & right alignment of indicators, if layout mirrored
    if (AknLayoutUtils::LayoutMirrored())
        {
        if (iAlignment == ESsAlignLeft)
            {
            iAlignment = ESsAlignRight;
            }
        else
            {
            iAlignment = ESsAlignLeft;
            }
        }
    iBgColor = TRgb(aBgColor);
    iTextColor = TRgb(aTextColor);
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::SetId
// -----------------------------------------------------------------------------
//
void CScreensaverIndicator::SetId(TScreensaverIndicatorId aId)
    {
    iId = aId;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::Visible
// -----------------------------------------------------------------------------
//
TBool CScreensaverIndicator::Visible() const
    {
    return iVisible;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::SetVisibility
// -----------------------------------------------------------------------------
//
void CScreensaverIndicator::SetVisibility(TBool aValue)
    {
    iVisible = aValue;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::Width()
// -----------------------------------------------------------------------------
//
TInt CScreensaverIndicator::Width() const
    {
    return iRect.Width();
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::Id
// -----------------------------------------------------------------------------
//
TScreensaverIndicatorId CScreensaverIndicator::Id() const
    {
    return iId;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::Alignment
// -----------------------------------------------------------------------------
//
TScreensaverAlignment CScreensaverIndicator::Alignment() const
    {
    return iAlignment;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::DrawType
// -----------------------------------------------------------------------------
//
TScreensaverDrawType CScreensaverIndicator::DrawType() const
    {
    return iDrawType;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::MakeSkinItemId
// -----------------------------------------------------------------------------
//
TAknsItemID CScreensaverIndicator::MakeSkinItemId(TInt aSkinBmpId)
    {
    TAknsItemID id;
    // All are AVKON generic ids
    id.Set(EAknsMajorGeneric, aSkinBmpId);
    return id;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::SetType
// -----------------------------------------------------------------------------
//
void CScreensaverIndicator::SetType(TScreensaverIndicatorType aType)
    {
    iType = aType;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::Type
// -----------------------------------------------------------------------------
//
TScreensaverIndicatorType CScreensaverIndicator::Type()
    {
    return iType;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::SetIconLayout
// -----------------------------------------------------------------------------
//
void CScreensaverIndicator::SetIconLayout(TAknLayoutRect&, TInt) 
    {
    iVisible = ETrue;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::SetTextLayout
// -----------------------------------------------------------------------------
//
void CScreensaverIndicator::SetTextLayout(TAknLayoutText&, TInt) 
    {
    iVisible = ETrue;
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::PreferredWidth
// -----------------------------------------------------------------------------
//
TInt CScreensaverIndicator::PreferredWidth() 
    { 
    return -1; 
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::MinimumWidth
// -----------------------------------------------------------------------------
//
TInt CScreensaverIndicator::MinimumWidth() 
    { 
    return 0; 
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::SetWidth
// -----------------------------------------------------------------------------
//
void CScreensaverIndicator::SetWidth(TInt) 
    {
    }

// -----------------------------------------------------------------------------
// CScreensaverIndicator::SetXPos
// -----------------------------------------------------------------------------
//
void CScreensaverIndicator::SetXPos(TInt) 
    {
    }


// End of file.
