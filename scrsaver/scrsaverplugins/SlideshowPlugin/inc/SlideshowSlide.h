/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Stores a single slide
*
*/



#ifndef C_SLIDESHOWSLIDE_H
#define C_SLIDESHOWSLIDE_H

//  INCLUDES
#include <e32base.h>  // e32def, e32std
#include <e32cmn.h>
#include <fbs.h>

#include "SlideshowSlideLoader.h"

//  CONSTANTS

// Leave code when a slide is on memory card and the card is not present
const TInt KSSTryLater = (-100);

//  MACROS

// Item flags
#define SSS_NONE        	0x00
#define SSS_IMAGELOADED 	0x01
#define SSS_ISONMC      	0x02
#define SSS_DECODEFAILED    0x04


//  DATA TYPES  

//  EXTERNAL DATA STRUCTURES  

//  FUNCTION PROTOTYPES

//  FORWARD DECLARATIONS

//  CLASS DEFINITIONS 
class CSlideshowSlide : public CBase
    {
public:
    // Creator
    static CSlideshowSlide* NewL(TDesC& aFileName, TBool aIsOnMC = EFalse);

    // Destructor
    virtual ~CSlideshowSlide();
      
    // Is decoding is in progress
    TBool IsDecodingInProgress();
    
    // Could not decode the file
    TBool CouldNotDecodeFile();
    
    //
    friend class CSlideshowSlideLoader;
private:

    // Default constructor
    CSlideshowSlide(TBool aIsOnMC);

    // 2-phase
    void ConstructL(TDesC& aFileName);

public:

    // Prepares slide (loads image)
    void PrepareSlideL(const TSize& aTargetSize);

    // Get image
    CFbsBitmap* Image() const { return iImage; }

    // Get filename
    HBufC* FileName() const { return iFileName; }
    
    // Release image
    void ReleaseImage();
    
    // Get flags
    TInt Flags() const { return iFlags; }
    
    // Is slide decoded completely
    TBool IsSlideDecoded();
    
protected:
	//Empty
private:
    // Type flags
    TInt iFlags;

    // Image
    CFbsBitmap* iImage; 

    // Filename
    HBufC* iFileName;
    CSlideshowSlideLoader *iSlideLoader;
    };

#endif  // C_SLIDESHOWSLIDE_H
