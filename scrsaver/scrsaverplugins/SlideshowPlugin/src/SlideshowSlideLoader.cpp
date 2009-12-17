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
* Description:   Loads a slide from file
*
*/



//  INCLUDES
#include "SlideshowPluginUtils.h"
#include "SlideshowSlideLoader.h"

// ---------------------------------------------------------------------------
// DivAndRoundUp
// ---------------------------------------------------------------------------
//
TInt DivAndRoundUp(const TInt aNumber, const TInt aDivider)
    {
    TInt result = aNumber / aDivider;

    if (aNumber % aDivider)
        {
        result += 1;
        }
    
    return result;
    }

// ---------------------------------------------------------------------------
// CSlideshowSlideLoader::NewL
// ---------------------------------------------------------------------------
//
CSlideshowSlideLoader* CSlideshowSlideLoader::NewL(CSlideshowSlide *aSlide)
	{
	CSlideshowSlideLoader * self = new (ELeave)CSlideshowSlideLoader(aSlide);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

// ---------------------------------------------------------------------------
// CSlideshowSlideLoader::ConstructL
// ---------------------------------------------------------------------------
//
void CSlideshowSlideLoader::ConstructL()
	{
	// Add this object to active scheduler
	CActiveScheduler::Add(this);
	}

// ---------------------------------------------------------------------------
// CSlideshowSlideLoader::CSlideshowSlideLoader
// ---------------------------------------------------------------------------
//
CSlideshowSlideLoader::CSlideshowSlideLoader(CSlideshowSlide *aSlide)
	: CActive(CActive::EPriorityStandard),iSlide(aSlide)
	{
	// Empty
	}

// ---------------------------------------------------------------------------
// CSlideshowSlideLoader::~CSlideshowSlideLoader
// ---------------------------------------------------------------------------
//
CSlideshowSlideLoader::~CSlideshowSlideLoader()
	{
	if(IsActive())
		{
		Cancel();
		}
	
	}

// ---------------------------------------------------------------------------
// CSlideshowSlideLoader::DeleteDecoder
// ---------------------------------------------------------------------------
//
void CSlideshowSlideLoader::DeleteDecoder()
	{
	if(iDecoder)
		{
		delete iDecoder;
		iDecoder = NULL;
		}
	
	}

// ---------------------------------------------------------------------------
// CSlideshowSlideLoader::DoCancel
// ---------------------------------------------------------------------------
//
void CSlideshowSlideLoader::DoCancel()
	{
	iDecoder->Cancel();
	DeleteDecoder();
	// delete the image
	if(iSlide->iImage)
		{
		delete iSlide->iImage;
		iSlide->iImage = NULL;
		}
	}

// ---------------------------------------------------------------------------
// CSlideshowSlideLoader::RunL
// ---------------------------------------------------------------------------
//
void CSlideshowSlideLoader::RunL()
	{
	// delete the decoder to save some memory.
	DeleteDecoder();
	if(iStatus != KErrNone)
		{
		iSlide->ReleaseImage();
		// SSS_DECODEFAILED says that Image could not be decoded. Corrupted image.
		(iSlide->iFlags) |= SSS_DECODEFAILED;
		return;
		}
	
	// Image loaded, size scaled down while loading. Find out, if the
    // image still needs to be scaled smaller to fit the screen
	
    TReal aspectRatio = (TReal) iBitmapsize.iWidth / (TReal) iBitmapsize.iHeight;

    if ((iBitmapsize.iWidth <= iTargetSize.iWidth) &&
        (iBitmapsize.iHeight <= iTargetSize.iHeight))
        {
        // Fits completely - finish
        SSPLOGGER_WRITE("No resize needed");
        
        // Compress the heap after imageconversion as
        // imagedecoder seems to leave heap uncompressed
        User::Heap().Compress();
        (iSlide->iFlags) |= SSS_IMAGELOADED;
    	return;
        }

    // Figure out which dimension needs most scaling
    TReal xFactor = (TReal) iTargetSize.iWidth / (TReal) iBitmapsize.iWidth;
    TReal yFactor = (TReal) iTargetSize.iHeight / (TReal) iBitmapsize.iHeight;

    // Start with target (screen) size
    TSize finalSize = iTargetSize;
            
    if (xFactor <= yFactor)
        {
        // X-dimension needs most scaling - fit to x, re-calc y
        finalSize.iHeight = (TInt) ((TReal) finalSize.iWidth / aspectRatio);
        }
    else
        {
        // Y-dimension needs most scaling - fit to y, re-calc x
        finalSize.iWidth = (TInt) ((TReal) finalSize.iHeight * aspectRatio);
        }

    SSPLOGGER_WRITEF(_L("SSP: Final size (%d, %d)"),
                     finalSize.iWidth, finalSize.iHeight);
    
    // Create a final bitmap with the final size, and draw to it
    CFbsBitmap* finalBmp = new (ELeave) CFbsBitmap;
    CleanupStack::PushL(finalBmp);
    User::LeaveIfError(finalBmp->Create(finalSize, (iSlide->iImage)->DisplayMode()));
    
    CFbsBitmapDevice* bmpDev = CFbsBitmapDevice::NewL(finalBmp);
    CleanupStack::PushL(bmpDev);
    
    CFbsBitGc* bmpCxt;
    User::LeaveIfError(bmpDev->CreateContext(bmpCxt));
    CleanupStack::PushL(bmpCxt);
    bmpCxt->DrawBitmap(TRect(0, 0, finalSize.iWidth, finalSize.iHeight),
    					(iSlide->iImage),
    					TRect(0, 0, iBitmapsize.iWidth, iBitmapsize.iHeight));
    
    CleanupStack::PopAndDestroy(2); // bmpDev, bmpCxt
    // Return the result in the original bitmap parameter
    (iSlide->iImage)->Reset();
    
    User::LeaveIfError((iSlide->iImage)->Duplicate(finalBmp->Handle()));
    CleanupStack::PopAndDestroy(finalBmp);
    // Compress the heap after imageconversion as
    // imagedecoder seems to leave heap uncompressed
    User::Heap().Compress();
    (iSlide->iFlags) |= SSS_IMAGELOADED;
	return;
	}

// ---------------------------------------------------------------------------
// CSlideshowSlideLoader::LoadSlideL
// ---------------------------------------------------------------------------
//
void CSlideshowSlideLoader::LoadSlideL(
    const TDesC& aFilename, const TSize& aTargetSize)
    {
   
    iTargetSize = aTargetSize;
    RFs filesys;
    User::LeaveIfError(filesys.Connect());
    CleanupClosePushL(filesys);
    // this line can leave if decoder is not found..need to cacth this.
    iDecoder = CImageDecoder::FileNewL(filesys, aFilename, ContentAccess::EView,CImageDecoder::EOptionAlwaysThread);
    if (aTargetSize == TSize(-1,-1))
        {
        CleanupStack::PopAndDestroy(&filesys); 
        iDecoder = NULL;
        return;
        }

    // Get image information
    TFrameInfo frameinfo = iDecoder->FrameInfo();
    TDisplayMode mode;

    // Tone down colors if possible (saves memory)
    if (frameinfo.iFlags & TFrameInfo::ECanDither)
        {
        mode = EColor64K;
        }
    else
        {
        mode = frameinfo.iFrameDisplayMode;
        }
    
    // Find out if the image can be downscaled during loading
    iBitmapsize = frameinfo.iOverallSizeInPixels;

      
    if ((iBitmapsize.iWidth  >= aTargetSize.iWidth * 2) &&
        (iBitmapsize.iHeight >= aTargetSize.iHeight * 2))
        {
        // 1/2 size
        iBitmapsize.iWidth = DivAndRoundUp(frameinfo.iOverallSizeInPixels.iWidth, 2);
        iBitmapsize.iHeight = DivAndRoundUp(frameinfo.iOverallSizeInPixels.iHeight, 2);
        
        if ((iBitmapsize.iWidth  >= aTargetSize.iWidth * 2) &&
            (iBitmapsize.iHeight >= aTargetSize.iHeight * 2))
            {
            // 1/4 size
            iBitmapsize.iWidth = DivAndRoundUp(frameinfo.iOverallSizeInPixels.iWidth, 4);
            iBitmapsize.iHeight = DivAndRoundUp(frameinfo.iOverallSizeInPixels.iHeight, 4);
            
            if ((iBitmapsize.iWidth  >= aTargetSize.iWidth * 2) &&
                (iBitmapsize.iHeight >= aTargetSize.iHeight * 2))
                {
                // 1/8 size
                iBitmapsize.iWidth = DivAndRoundUp(frameinfo.iOverallSizeInPixels.iWidth, 8);
                iBitmapsize.iHeight = DivAndRoundUp(frameinfo.iOverallSizeInPixels.iHeight, 8);
                }
            }
        }

    // Create a target bitmap
    iSlide->iImage = new (ELeave) CFbsBitmap;
    User::LeaveIfError((iSlide->iImage)->Create(iBitmapsize, mode));
    // for filesys
    CleanupStack::PopAndDestroy(&filesys);
    
    //Start decoding the image ...
    // RunL method is called when decoding is done.
    iDecoder->Convert(&iStatus, *(iSlide->iImage),0);  
    SetActive();
 }

// ---------------------------------------------------------------------------
// CSlideshowSlideLoader::RunError
// ---------------------------------------------------------------------------
//
TInt CSlideshowSlideLoader::RunError(TInt aError)
	{
	if (NULL != iSlide->iImage)
		{
		delete iSlide->iImage;
		iSlide->iImage = NULL;
		}
	return aError;
	}
