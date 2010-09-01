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
* Description:     Stores a single bitmap animation item: image with its
*               properties (position, timing et al)
*
*/





//  INCLUDES
#include <fbs.h>
#include <bitdev.h>
#include <gdi.h>

#include "CBmpAnimItem.h"

// default 0.20 secs
const TInt KDefaultTiming = 200000;

// Public constructor, doesn't need a two-phase construction
CBmpAnimItem::CBmpAnimItem()
    : iFlags(BAIF_OWNSICON),
      iTiming(KDefaultTiming),
      iPosition(TPoint(0, 0))
    {
    }

    
// Destructor
CBmpAnimItem::~CBmpAnimItem()
    {
    if (Flags() & BAIF_OWNSICON)
        {
        delete iIcon;
        }
    }

    
// Set icon. If owned, an icon is deleted by the item
void CBmpAnimItem::SetIcon(
    CGulIcon* aIcon,
    TBool aItemOwnsIcon /*= ETrue */)
    {
    // Get rid of old icon, if any and if owned
    if ((iIcon) && (Flags() & BAIF_OWNSICON))
        {
        delete iIcon;
        }
    
    iIcon = aIcon;

    if (aItemOwnsIcon)
        {
        iFlags |= BAIF_OWNSICON;
        }
    else
        {
        iFlags &= ~BAIF_OWNSICON;
        }
    }


// Set icon from bitmaps. If owned, an icon is deleted by the item
void CBmpAnimItem::SetIconL(
    CFbsBitmap* aBitmap,
    CFbsBitmap* aMask /* = NULL */,
    TBool aItemOwnsIcon /* = ETrue */)
    {
    // Create a new icon
    CGulIcon* icon = CGulIcon::NewL();

    // Plug the bitmaps in
    icon->SetBitmap(aBitmap);
    icon->SetMask(aMask);

    // Does icon own bitmaps?
    icon->SetBitmapsOwnedExternally(!aItemOwnsIcon);

    // Store the newly created icon
    SetIcon(icon, aItemOwnsIcon);
    }
//  End of File
