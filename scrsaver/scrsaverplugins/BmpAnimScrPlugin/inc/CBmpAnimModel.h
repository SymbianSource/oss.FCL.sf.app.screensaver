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
* Description:   Takes care of storing and serving the bitmaps
*               to be animated along with their properties
*
*/



#ifndef C_CBMPANIMMODEL_H
#define C_CBMPANIMMODEL_H

#include "CBmpAnimItem.h"
#include "CBmpAnimSettings.h"

// 5 seems like a reasonable granularity
const TInt KBmpAnimItemArrayGranularity = 5;

// BmpAnim model. Contains BmpAnim items, serves the items to BmpAnim plugin
class CBmpAnimModel : public CBase
    {
public:
    // Default constructor & destructor
    CBmpAnimModel();
    virtual ~CBmpAnimModel();

public:
    // Two-phase constructor, initialises member arrays and stuff
    virtual void ConstructL(CBmpAnimSettings* aSettings);

public: // Access functions
    // Number of items query
    TInt NumberOfItems() const;

    // Add an item to the end of the array
    void AppendItemL(CBmpAnimItem* aItem);

    // Insert an item at <aIndex>. Space must be reserved.
    void InsertItemL(TInt aIndex, CBmpAnimItem* aItem);

    // Delete item at <anIndex>.
    void DeleteItemL(TInt aIndex);

    // Delete all items
    void DeleteAll();

    // Get item at position <aIndex>
    CBmpAnimItem* ItemAt(TInt aIndex) const;

    // Get current item index
    inline TInt CurrentItemIndex() const;

    // Set current item index
    inline void SetCurrentItemIndex(TInt aIndex);

    // Get next item in animation sequence. aWrapped is ETrue, if the
    // next item caused a wrap to the beginning of sequence
    CBmpAnimItem* NextItem(TBool& aWrapped);
    
    // Reserve space for <aCount> items
    void SetReserveL(TInt aCount);

    // Get settings
    inline CBmpAnimSettings* Settings() const;
    
private:  // Utility functions

private:
    // Item array
    CArrayPtrFlat<CBmpAnimItem>* iItems;

    // Current item index
    TInt iCurrentItem;

    CBmpAnimSettings* iSettings;
    };


// --- inlines ---

// Get current item index
inline TInt CBmpAnimModel::CurrentItemIndex() const
    { return iCurrentItem; }


// Set current item index
inline void CBmpAnimModel::SetCurrentItemIndex(TInt aIndex)
    {
    iCurrentItem = aIndex;
    }

// Get settings
inline CBmpAnimSettings* CBmpAnimModel::Settings() const
   { return iSettings; }

#endif   // C_CBMPANIMMODEL_H

// End of file
