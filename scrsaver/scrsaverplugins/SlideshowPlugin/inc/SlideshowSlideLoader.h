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



#ifndef C_SLIDESHOWSLIDELOADER_H
#define C_SLIDESHOWSLIDELOADER_H

#include <e32std.h>
#include <fbs.h>
#include <f32file.h>
#include <bautils.h>
#include <coemain.h>
#include <e32base.h>
#include <bitstd.h>
#include <imageconversion.h>

class CSlideshowSlide;
class CSlideshowSlideLoader : public CActive
    {
public: 
	// Creator
	static CSlideshowSlideLoader* NewL(CSlideshowSlide *aSlide);
	
	// Called when image decoding done
	void RunL();
	
	// Called if user calls Cancel() on active object
	void DoCancel();
	
	//virtual distructor
	virtual ~CSlideshowSlideLoader();
	
	//start decoding the slide
	void LoadSlideL(const TDesC& aFilename, const TSize& aTargetSize);
	
private:
	
	// Default constructor
	CSlideshowSlideLoader(CSlideshowSlide *aSlide);
	
	//Two Phase constructor
	void ConstructL();
	
	//Delete the Decoder object
	void DeleteDecoder();
	
	//From CActive
	TInt RunError(TInt aError);
	
private:
   
	// Target size (Phone screen size)
    TSize iTargetSize;
    //Bitmap size
    TSize iBitmapsize;
    //Decoder
    CImageDecoder * iDecoder;
    //Slide
    CSlideshowSlide * iSlide;
   
    };

#endif  // C_SLIDESHOWSLIDELOADER_H
