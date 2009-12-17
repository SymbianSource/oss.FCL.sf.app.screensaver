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
* Description:     Takes care of storing and serving the bitmaps
*               to be animated along with their properties
*
*/





#include "CBmpAnimModel.h"


// C'tor
CBmpAnimModel::CBmpAnimModel()
    {
    }

    
// D'tor
CBmpAnimModel::~CBmpAnimModel()
    {
    // Delete item array
    if (iItems)
        {
        iItems->ResetAndDestroy();
        }
    delete iItems;
    }

    
// Two-phase constructor, initialises member arrays and stuff
void CBmpAnimModel::ConstructL(CBmpAnimSettings* aSettings)
    {
    // Create item array
    iItems = new(ELeave) CArrayPtrFlat<CBmpAnimItem>(
        KBmpAnimItemArrayGranularity);

    iSettings = aSettings;
    }


// Number of items query
TInt CBmpAnimModel::NumberOfItems() const
    {
    return iItems ? iItems->Count() : 0;
    }
    
  
// Add an item to the end of the array
void CBmpAnimModel::AppendItemL(CBmpAnimItem* aItem)
    {
    // Sanity check
    if (!iItems)
        {
        ASSERT(EFalse);
        return;
        }

    // Stash it in
    iItems->AppendL(aItem);
    }

    
// Insert an item at <aIndex>. Space must be reserved.
void CBmpAnimModel::InsertItemL(TInt aIndex, CBmpAnimItem* aItem)
    {
    if ((iItems) && (aIndex >= 0) && (aIndex <= NumberOfItems()))
        {
        // Squeeze it in
        iItems->InsertL(aIndex, aItem);
        }
    else
        {
        ASSERT(EFalse);
        }
    }
    

// Delete item at <aIndex>
void CBmpAnimModel::DeleteItemL(TInt aIndex)
    {
    if ((iItems) && (aIndex >= 0) && (aIndex <= NumberOfItems()))
        {
        // Grab hold of the item for deletion, remove from array
        // and compress the array
        CBmpAnimItem* ptr = iItems->At(aIndex);
        iItems->Delete(aIndex);
        delete ptr;
        iItems->Compress();
        }
    else
        {
        ASSERT(EFalse);
        }
    }


// Delete all Items
void CBmpAnimModel::DeleteAll()
    {
    if (iItems)
        {
        iItems->ResetAndDestroy();
        }
    }

    
// Get item at position <aIndex>
CBmpAnimItem* CBmpAnimModel::ItemAt(TInt aIndex) const
    {
    if ((iItems) && (aIndex >= 0) && (aIndex < NumberOfItems()))
        {
        return iItems->At(aIndex);
        }
    else
        {
        return NULL;
        }
    }


// Get next item in animation sequence. Returns NULL at the end of the
// sequence (or if there are no items at all), and wraps around to the
// beginning, so that next call again returns an item.
CBmpAnimItem* CBmpAnimModel::NextItem(TBool& aWrapped)
    {
    CBmpAnimItem* pItem = ItemAt(iCurrentItem++);

    if (pItem)
        {
        aWrapped = EFalse;
        }
    else
        {
        // Apparently there are no more items - wrap around and set indicator
        iCurrentItem = 0;
        aWrapped = ETrue;

        pItem = ItemAt(iCurrentItem++);
        }

    return pItem;
    }

    
// Reserve space for <aCount> Items
void CBmpAnimModel::SetReserveL(TInt aCount)
    {
    if ((iItems) && (aCount >= NumberOfItems()))
        {
        iItems->SetReserveL(aCount);
        }
    }

    
// --- private functions ---
//  End of File
