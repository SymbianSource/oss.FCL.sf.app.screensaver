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
* Description:  Takes care of storing and serving the slides
*               to Slideshow plugin
*
*/


#include <e32math.h>
#include <eikenv.h>
#include <bautils.h>

#include "SlideshowModel.h"
#include "SlideshowPluginUtils.h"

//------------------------------------------------------------------------------
// CSlideshowModel::NewL
//------------------------------------------------------------------------------
//
CSlideshowModel* CSlideshowModel::NewL()
    {
    CSlideshowModel* self = new (ELeave) CSlideshowModel();
    
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    
    return self;
    }


//------------------------------------------------------------------------------
// CSlideshowModel::~CSlideshowModel
//------------------------------------------------------------------------------
//
CSlideshowModel::~CSlideshowModel()
    {
    // Delete slide array
    if (iSlides)
        {
        iSlides->ResetAndDestroy();
        }
    delete iSlides;
    }


//------------------------------------------------------------------------------
// CSlideshowModel::CSlideshowModel
//------------------------------------------------------------------------------
//
CSlideshowModel::CSlideshowModel()
    : iNextSlide(-1)
    {
    }

    
//------------------------------------------------------------------------------
// CSlideshowModel::ConstructL
//------------------------------------------------------------------------------
//
void CSlideshowModel::ConstructL()
    {
    // Create slide array
    iSlides = new(ELeave) CArrayPtrFlat<CSlideshowSlide>(
        KSlideshowSlideArrayGranularity);

    //Get random seed
    TTime time;
    time.HomeTime();
    iRandSeed = time.Int64();
    }


//------------------------------------------------------------------------------
// CSlideshowModel::NumberOfSlides
//------------------------------------------------------------------------------
//
TInt CSlideshowModel::NumberOfSlides() const
    {
    //if the slide array is not NULL, return it's size
    return iSlides ? iSlides->Count() : 0;
    }
    
  
//------------------------------------------------------------------------------
// CSlideshowModel::AppendSlideL
//------------------------------------------------------------------------------
//
void CSlideshowModel::AppendSlideL(CSlideshowSlide* aSlide)
    {
    // Sanity check
    if (!iSlides)
        {
        ASSERT(EFalse);
        return;
        }

    // Stash it in
    iSlides->AppendL(aSlide);
    }

    
//------------------------------------------------------------------------------
// CSlideshowModel::InsertSlideL
//------------------------------------------------------------------------------
//
void CSlideshowModel::InsertSlideL(TInt aIndex, CSlideshowSlide* aSlide)
    {
    if ((iSlides) && (aIndex >= 0) && (aIndex <= NumberOfSlides()))
        {
        // Squeeze it in
        iSlides->InsertL(aIndex, aSlide);
        }
    else
        {
        ASSERT(EFalse);
        }
    }
    

//------------------------------------------------------------------------------
// CSlideshowModel::DeleteSlide
//------------------------------------------------------------------------------
//
void CSlideshowModel::DeleteSlide(TInt aIndex)
    {
    //Sanity check
    if ((iSlides) && (aIndex >= 0) && (aIndex <= NumberOfSlides()))
        {
        // Grab hold of the slide for deletion, remove from array
        // and compress the array
        CSlideshowSlide* ptr = iSlides->At(aIndex);
        
        iSlides->Delete(aIndex);
        delete ptr;
        iSlides->Compress();
        
        }
    else
        {
        ASSERT(EFalse);
        }
    }


//------------------------------------------------------------------------------
// CSlideshowModel::DeleteAll
//------------------------------------------------------------------------------
//
void CSlideshowModel::DeleteAll()
    {
    //Sanity check
    if (iSlides)
        {
        iSlides->ResetAndDestroy();
        }
    
    iCurrentSlide = 0;
    iNextSlide = -1;
    }

    
//------------------------------------------------------------------------------
// CSlideshowModel::SlideAt
//------------------------------------------------------------------------------
//
CSlideshowSlide* CSlideshowModel::SlideAt(TInt aIndex) const
    {
    if ((iSlides) && (aIndex >= 0) && (aIndex < NumberOfSlides()))
        {
        //return the postion object
        return iSlides->At(aIndex);
        }
    else
        {
        return NULL;
        }
    }


//------------------------------------------------------------------------------
// CSlideshowModel::NextSlide
//------------------------------------------------------------------------------
//
CSlideshowSlide* CSlideshowModel::NextSlide(TBool /*aRandom */)
    {
    if (iNextSlide < 0)
        {
        // Next slide could not be prepared, or preparenextslide not called
        return NULL;
        }
    
    CSlideshowSlide* pSlide = SlideAt(iNextSlide);

    if (!pSlide)
        {
        // Apparently there are no more slides - wrap around
        iCurrentSlide = -1;
        iNextSlide = 0;
        pSlide = SlideAt(iNextSlide);
        }

    iCurrentSlide = iNextSlide;
    return pSlide;
    }

    
//------------------------------------------------------------------------------
// CSlideshowModel::PrepareNextSlide
//------------------------------------------------------------------------------
//
void CSlideshowModel::PrepareNextSlide(TBool aRandom, const TSize& aTargetSize)
    {
    SSPLOGGER_WRITE("Model::PrepareNextSlide(), model busy");
    
    SSPLOGGER_WRITEF(_L("SPP: iCurrentSlide %d, iNextSlide %d"),
                     iCurrentSlide, iNextSlide);
    
    TBool slideOK = EFalse;
    while (!slideOK)
        {
        TInt nSlides = NumberOfSlides();
        // Any point in continuing?
        if (nSlides == 0)
            {
            iNextSlide = -1;
            break;
            }

        // Special cases, 1 or 2 slides only
        if (nSlides == 1)
            {
            iNextSlide = 0;
            SSPLOGGER_WRITEF(_L("SPP: One slide, next slide %d"), iNextSlide);
            }
        else if (nSlides == 2)
            {
            // Switch between slides even if random
            iNextSlide = (iNextSlide == 0 ? 1 : 0);
            SSPLOGGER_WRITEF(_L("SPP: 2 slides, next slide %d"), iNextSlide);            
            }
        else
            {
            // We got at least 3 slides, and can also do random
            if (aRandom)
                {
                // Make sure we won't pick the same slide  
                do
                    {
                    iNextSlide = Random(nSlides - 1);                    
                    }
                while (iNextSlide == iCurrentSlide);

                SSPLOGGER_WRITEF(_L("SPP: Random, next slide %d"), iNextSlide);
                }
            else
                {
                iNextSlide++;

                // Wrap?
                if (iNextSlide >= nSlides)
                    {
                    SSPLOGGER_WRITE("Wrapping...");
                    iNextSlide = 0;
                    }
                SSPLOGGER_WRITEF(_L("SPP: Continuous, next slide %d"),
                                 iNextSlide);
                }
            }

        // Get the slide
        CSlideshowSlide* pSlide = SlideAt(iNextSlide);

        if (!pSlide)
            {
            // Should not have happened - start from beginning
            iNextSlide = 0;
            SSPLOGGER_WRITEF(_L("SPP: Wrapping, next slide %d"),
                    	iNextSlide);
            pSlide = SlideAt(iNextSlide);
            }

        if (pSlide)
            {
            // Try loading the image, if image does not already exist
            if (!pSlide->IsSlideDecoded())
                {
                SSPLOGGER_WRITE("Preparing next slide");
               	if(iCurrentSlide != iNextSlide)
                	{
                	// relese currently deleted image from RAM.( We dont have caching)
                	ReleaseCurrentSlide();
               	    }
               	if ((TInt)ETrue == pSlide->CouldNotDecodeFile())
               		{
               		// could not decode this file last time. so delete this slide from the 
               		// list of slide and go to the next one.
               		DeleteSlide(iNextSlide);
                    iNextSlide--;
                    continue;
               		}
                TRAPD(err, pSlide->PrepareSlideL(aTargetSize));
                if (err == KErrNone)
                	{
                    //wait for slide to load.
                	iCurrentSlide = iNextSlide;
                    SSPLOGGER_WRITE("Started Decoding Wait");                    
                    break;
                    }
                else
                    {
                    // Apparently the file was not a loadable image -
                    // remove from slideset and try again
                    SSPLOGGER_WRITEF(_L("SSP: Deleting slide %d"), iNextSlide);                    
                    DeleteSlide(iNextSlide);
                    iNextSlide--;
                    continue; // can not load this image, go to the next one.
                    }
                }
             else
                {
                // Slide is already decoded. This is cool, if the file is
                // still there. Otherwise lose image and go on.
                // should not come here as we dont any caching 
                SSPLOGGER_WRITEF(_L("Slide %d already has image"), iNextSlide);

                if (!BaflUtils::FileExists(CEikonEnv::Static()->FsSession(), 
                    *(pSlide->FileName())))
                    {
                    SSPLOGGER_WRITE("Slide image file gone, dropping image");
                    pSlide->ReleaseImage();
                    continue; // continue with next image
                    }
                else
                	{
                	if(iCurrentSlide != iNextSlide)
                		{
                		ReleaseCurrentSlide();
               	     	}
                     break;
                	}
               }
           }
        }
    SSPLOGGER_WRITE("Model::PrepareNextSlide() end, model no longer busy");    
    }


//------------------------------------------------------------------------------
// CSlideshowModel::ReleaseCurrentSlide
//------------------------------------------------------------------------------
//
void CSlideshowModel::ReleaseCurrentSlide()
	{
	CSlideshowSlide* pSlide = SlideAt(iCurrentSlide);
	if(pSlide)
		{
		pSlide->ReleaseImage();
		}
	}


//------------------------------------------------------------------------------
// CSlideshowModel::ReleaseNextSlide
//------------------------------------------------------------------------------
//
void CSlideshowModel::ReleaseNextSlide()
    {
    CSlideshowSlide* pSlide = SlideAt(iNextSlide);
    if(pSlide)
		{
		pSlide->ReleaseImage();
		}
    iNextSlide --;
    }


//------------------------------------------------------------------------------
// CSlideshowModel::SetReserveL
//------------------------------------------------------------------------------
//
void CSlideshowModel::SetReserveL(TInt aCount)
    {
    if ((iSlides) && (aCount >= NumberOfSlides()))
        {
        iSlides->SetReserveL(aCount);
        }
    }

//------------------------------------------------------------------------------
// CSlideshowModel::SlideStatus
//------------------------------------------------------------------------------
//
TInt CSlideshowModel::SlideStatus(CSlideshowSlide* aSlide)
	{
	 if(!aSlide)
		 {
		 return KStatusNoSlide;
		 }
	 else if(aSlide->IsSlideDecoded())
		 {
		 return KStatusSlideDecoded;
		 }
	 else if(aSlide->IsDecodingInProgress())
		 {
		 return KStatusDecodingInProgress;
		 }
	 else if (aSlide->CouldNotDecodeFile())
		 {
		 return KStatusSlideDecodeingFailed;
		 }
		 
	 return KStatusSlideNotDecoding;
	}
    


//------------------------------------------------------------------------------
// CSlideshowModel::Random
//------------------------------------------------------------------------------
//
TInt CSlideshowModel::Random(TInt aMax, TInt aMin /* = 0 */)
    {
    TInt range = aMax - aMin;
    TInt rand = Math::Rand(iRandSeed);  
    return (rand % range);
    }


//  End of File
