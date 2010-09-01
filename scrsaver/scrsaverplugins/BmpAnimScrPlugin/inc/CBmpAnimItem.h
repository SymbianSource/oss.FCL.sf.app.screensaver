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
* Description:   Stores a single bitmap animation item: image with its
*               properties (position, timing et al)
*
*/



#ifndef C_CBMPANIMITEM_H
#define C_CBMPANIMITEM_H

//  INCLUDES
#include <e32base.h>  // e32def, e32std
#include <gulicon.h>

//  CONSTANTS  

//  MACROS

// Item flags
#define BAIF_OWNSICON 0x01


//  DATA TYPES  

//  EXTERNAL DATA STRUCTURES  

//  FUNCTION PROTOTYPES

//  FORWARD DECLARATIONS

//  CLASS DEFINITIONS 
class CBmpAnimItem : public CBase
    {
public:
    // Public constructor, doesn't need a two-phase construction
    CBmpAnimItem();
    
    // Destructor
    virtual ~CBmpAnimItem();
    
    // Set icon. If owned, an icon is deleted by the item
    void SetIcon(CGulIcon* aIcon, TBool aItemOwnsIcon = ETrue);
    
    // Set icon from bitmaps. If owned, an icon is deleted by the item
    void SetIconL(
        CFbsBitmap* aBitmap,
        CFbsBitmap* aMask = NULL,
        TBool aItemOwnsIcon = ETrue);
    
    // Get icon
    CGulIcon* Icon() const { return iIcon; }

    // Set timing (microsecs to display the image)
    void SetTiming(TInt aMicros) { iTiming = aMicros; }

    // Get timing
    TInt Timing() { return iTiming; }

    // Set position
    void SetPosition(TPoint aPos) { iPosition = aPos; }

    // Get position
    TPoint Position() { return iPosition; }

    // Get flags
    TInt Flags() const { return iFlags; }    
protected:
private:
    // Type flags
    TInt iFlags;

    // Icon
    CGulIcon* iIcon;

    // Timing
    TInt iTiming;
    
    // Position
    TPoint iPosition;
    };

#endif  // C_CBMPANIMITEM_H
