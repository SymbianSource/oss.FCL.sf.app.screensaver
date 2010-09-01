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
* Description:     Stores a single slide
*
*/





//  INCLUDES
#include "SlideshowSlide.h"
#include "SlideshowSlideLoader.h"
#include "SlideshowPluginUtils.h"
    
// Creator
CSlideshowSlide* CSlideshowSlide::NewL(TDesC& aFileName, TBool aIsOnMC)
    {
    CSlideshowSlide* self = new (ELeave) CSlideshowSlide(aIsOnMC);
    CleanupStack::PushL(self);
    self->ConstructL(aFileName);
    CleanupStack::Pop(self);
    return self;
    }

// Destructor
CSlideshowSlide::~CSlideshowSlide()
    {
    ReleaseImage();
    delete iSlideLoader;
    delete iFileName;
    }


// Default C'tor
CSlideshowSlide::CSlideshowSlide(TBool aIsOnMC)
    : iFlags(SSS_NONE)
    {
    if (aIsOnMC)
        {
        iFlags |= SSS_ISONMC;
        }
    }

// is decoding in progress
TBool CSlideshowSlide::IsDecodingInProgress()
	{
	if(iSlideLoader->IsActive())
		{
		return ETrue;
		}
	return EFalse;
	}
// Could not decode the file
TBool CSlideshowSlide::CouldNotDecodeFile()
	{
	if (iFlags & SSS_DECODEFAILED)
        {
        return ETrue;
        }
	return EFalse;
	}
// Two-phase constructor
void CSlideshowSlide::ConstructL(TDesC& aFileName)
    {
    // Allocate space for the filename and save it
    iFileName = HBufC::NewL(aFileName.Length());
    *iFileName = aFileName;
    iSlideLoader = CSlideshowSlideLoader::NewL(this);
    }
// Prepares the slide (loads image)
void CSlideshowSlide::PrepareSlideL(const TSize& aTargetSize)
    {
   	if (iFlags & SSS_IMAGELOADED)
      {
      	  return;
      }
    if( iSlideLoader->IsActive())
    	{
    	// slide is being decoded ... should wait .. dont call LoadSlideL here
    	// should not come here..
    	return;
    	}
    iSlideLoader->LoadSlideL(*iFileName, aTargetSize);
    }

// Is slide decoded completely
TBool CSlideshowSlide::IsSlideDecoded()
	{
	if (iFlags & SSS_IMAGELOADED)
        {
        return ETrue;
        }
	return EFalse;
	}
// Releases the slide's image
void CSlideshowSlide::ReleaseImage()
    {
    // if decoding is in progress... at any cost dont delete the image before Canceling the decoding
    if(IsDecodingInProgress())
    	{
    	iSlideLoader->Cancel();
    	}
    else if(iImage )
    	{
    	delete iImage;
    	iImage = NULL;
    	}
    //image deleted, revert back to not decoded state
    iFlags &= ~(SSS_IMAGELOADED);
    }
//  End of File
