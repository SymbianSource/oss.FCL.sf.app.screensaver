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
* Description:   Takes care of storing and serving the slides
*               to Slideshow plugin
*
*/



#ifndef C_SLIDESHOWMODEL_H
#define C_SLIDESHOWMODEL_H

#include <e32base.h>

class CSlideshowSlide;

// Slide Array's size. 5 seems like a reasonable granularity
const TInt KSlideshowSlideArrayGranularity = 5;

//Slide status
//this is only called in startplugin, Dont call it anywhere
//Get next slide status
const TInt KStatusNoSlide = 0;                //no side
const TInt KStatusSlideDecoded = 1;           //already decoded
const TInt KStatusDecodingInProgress  =2;     //decoding in progress
const TInt KStatusSlideNotDecoding = 3;       //slide is there but not decoded
const TInt KStatusSlideDecodeingFailed = 4;   //could not decode the file

// Slideshow model. 
// Contains slideshow slides, serves the slides to Slideshow plugin
class CSlideshowModel : public CBase
    {
    
public:
	/**
	 *  Return a new CSlideshowModel object
	 */
    static CSlideshowModel* NewL();
    
    /**
     * Desconstructor
     */
    virtual ~CSlideshowModel();

private:
    /**
     * Default constructor
     */
    CSlideshowModel();
    
    /*
     *  Two-phase constructor, initialises member arrays and stuff
     */
    virtual void ConstructL();

public: // Access functions
    /*
     *  Number of slides query.
     */
    TInt NumberOfSlides() const;

    /*
     *  Add a slide to the end of the array.
     * 
     *  @param aSlide The new slide which will be added to the silde array.
     */
    void AppendSlideL(CSlideshowSlide* aSlide);

    /*
     *  Insert a slide at <aIndex>. Space must be reserved.
     * 
     *  @param aIndex The postion index which the new slide will be insert to.
     *  @param aSilde The new slide which will be insert.
     */
    void InsertSlideL(TInt aIndex, CSlideshowSlide* aSlide);

    /*
     * Delete slide at <aIndex>.
     * 
     * @param aIndex The slide at <aIndex> will be delete. 
     */
    void DeleteSlide(TInt aIndex);

    /*
     *  Delete all slides.
     */
    void DeleteAll();

    /*
     *  Get slide at position <aIndex>.
     * 
     *  @return a slide at position <aIndex>.
     */
    CSlideshowSlide* SlideAt(TInt aIndex) const;

    /*
     *  Get current slide index.
     * 
     *  @return the current slide index.
     */
    inline TInt CurrentSlideIndex() const;

    /*
     *  Set current slide index.
     * 
     *  @param aIndex The  slide index, which will be show.
     */
    inline void SetCurrentSlideIndex(TInt aIndex);

    /*
     *  Get next slide in sequence
     * 
     *  @param aRandom No used now.
     *  @return the next slide.
     */
    CSlideshowSlide* NextSlide(TBool aRandom);
    
    /*
     *  Get slide status: decoded, decoding ,not started,decoding failed,no slide.
     *  
     *  @param  aSlide the slide which status will be detected.
     *  @return status of aSlide.
     */
    TInt SlideStatus(CSlideshowSlide* aSlide); 

    /*
     *  Prepare next slide.
     *  @param aRandom Whether select next slide random.( ETrue is random, EFalse is sequential)
     *  @param aTargetSize The slide Size.
     */
    void PrepareNextSlide(TBool aRandom, const TSize& aTargetSize);

    /*
     *  Release next slide.
     */
    void ReleaseNextSlide();
    
    /*
     *  Release Current slide.
     */
    void ReleaseCurrentSlide();
    
    /*
     *  Get next slide index.
     *
     *  @return the position index which slide will be on.
     */
    inline TInt NextSlideIndex() const;

    /*
     *  Reserve space for <aCount> slides.
     * 
     *  @param aCount The slide number.
     */
    void SetReserveL(TInt aCount);

    /*
     *  Query if model is busy (decoding a slide).
     * 
     *  @return status whether the model is ready.(ETrue is ready, EFalse is not)
     */
    TBool Busy();

    /*
     *  Reset slide sequence, note that this will not release current image
     */
    inline void ResetSlideSequence();
    
private:  // Utility functions
    /*
     *  Get a random number in the range aMin - aMax (inclusive)
     * 
     *  @param aMax The upper limited.
     *  @param aMin The lower limited.
     *  @return an random number which is between aMax and aMin.
     */
    TInt Random(TInt aMax, TInt aMin = 0);
    
private:
    /*
     *  Slide array
     */
    CArrayPtrFlat<CSlideshowSlide>* iSlides;

    /*
     *  Current slide index
     */
    TInt iCurrentSlide;
    
    /*
     *  Next slide index
     */
    TInt iNextSlide;

    /* 
     *  Random seed
     */
    TInt64 iRandSeed;

    /*
     *  Busy flag
     */
    TBool iBusy;
    };

//------------------------------------------------------------------------------
// CSlideshowModel::CurrentSlideIndex
//------------------------------------------------------------------------------
//
TInt CSlideshowModel::CurrentSlideIndex() const
    {
    return iCurrentSlide;
    }


//------------------------------------------------------------------------------
// CSlideshowModel::SetCurrentSlideIndex
//------------------------------------------------------------------------------
//
void CSlideshowModel::SetCurrentSlideIndex(TInt aIndex)
    {
    iCurrentSlide = aIndex;
    }


//------------------------------------------------------------------------------
// CSlideshowModel::NextSlideIndex
//------------------------------------------------------------------------------
//
TInt CSlideshowModel::NextSlideIndex() const
    {
    return iNextSlide;
    }

//------------------------------------------------------------------------------
// CSlideshowModel::ResetSlideSequence
//------------------------------------------------------------------------------
//
void CSlideshowModel::ResetSlideSequence()
    {
    iCurrentSlide = 0;
    iNextSlide = -1;
    }

#endif   // C_CSLIDESHOWMODEL_H

// End of file
