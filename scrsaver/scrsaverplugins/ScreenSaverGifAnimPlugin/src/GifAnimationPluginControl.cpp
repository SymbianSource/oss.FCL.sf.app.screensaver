/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:     Screensaver GifAnimation plug-in container source file
*
*/





#include <eikenv.h>
#include <w32std.h>
#include <e32base.h>
#include <e32std.h>

#include <IHLImageFactory.h>        // IHLImageFactory
#include <IHLViewerFactory.h>       // IHLViewerFactory
#include <MIHLImageViewer.h>        // MIHLImageViewer
#include <MIHLFileImage.h>          // MIHLFileImage
#include <MIHLBitmap.h>             // MIHLBitmap
#include <MIHLImageViewer.h>        // MIHLImageViewer
#include <MIHLViewerObserver.h>
#include <bautils.h>

#include "GifAnimationPlugin.h"
#include "GifAnimationUtils.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Instance factory
// ---------------------------------------------------------------------------
//
CGifAnimationPluginControl* CGifAnimationPluginControl::NewL( 
                                        CCoeControl *aParentControl, 
                                        MPluginAdapter* aPluginAdapter )
    {
    ASSERT( aPluginAdapter );
    DBG_TRACE_FN_BEGIN;        
    CGifAnimationPluginControl* tmp = 
        new ( ELeave )CGifAnimationPluginControl();
    CleanupStack::PushL( tmp );
    tmp->ConstructL( aParentControl, aPluginAdapter );
    CleanupStack::Pop();
    DBG_TRACE_FN_END;
    return tmp;
    }

// ---------------------------------------------------------------------------
// 2nd phase constructor
// ---------------------------------------------------------------------------
//
void CGifAnimationPluginControl::ConstructL( CCoeControl* aParentControl,
                                             MPluginAdapter* aPluginAdapter )
    {
    DBG_TRACE_FN_BEGIN;

    iPluginAdapter = aPluginAdapter;
    
    if ( aParentControl != NULL )
        {
        CreateWindowL( aParentControl );
        }
    else 
        {
        CreateWindowL();
        }
    ActivateL();
    
    MakeVisible( ETrue ); // make it invisible for now
    
    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CGifAnimationPluginControl::CGifAnimationPluginControl() 
    : iSourceImage( NULL ),
      iDrawingBitmap( NULL ), 
      iScalingBitmap( NULL ), 
      iScalingBitmapMask( NULL ),
      iEngine( NULL ), 
      iAnimationState( EAnimationNotReady ), 
      iLastError( KErrNone )
    {
    DBG_TRACE_FN_BEGIN;
    // nothing goes here
    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CGifAnimationPluginControl::~CGifAnimationPluginControl()
    {
    DBG_TRACE_FN_BEGIN;        

    DeleteAnimation();

    iPluginAdapter = NULL;    

    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------
// Loads the image into display.
// Param aImageFileName image file name - expected to be valid image
// ---------------------------------------------------------
//
void CGifAnimationPluginControl::LoadImageL( const TDesC& aImageFileName )
    {
    DBG_TRACE_FN_BEGIN;        
    iFileName.Copy( aImageFileName );

	iLastError = KErrNotReady; // if asked before loading is complete+Draw()
    TRAPD( error, DoImageLoadingL() );
    if ( error )    
        {
        iLastError = error; 
        DeleteAnimation();
        DBG_TRACE_FN_END;
        User::Leave( error ); // re-throw it
        }
    else
        {
        DBG_TRACE( "Animation loaded" );
        iAnimationState = EAnimationLoading;
        iLastError = KErrNone;
        }    
    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------
// Return ETrue if loaded image is animation.
// ---------------------------------------------------------
//
TBool CGifAnimationPluginControl::IsAnimation() const
    {
    DBG_TRACE_FN_BEGIN;        
    if ( iSourceImage 
      && iLastError == KErrNone ) // we did not have any error loading picture
        {
        DBG_TRACE_FN_END;
        return iSourceImage->IsAnimation(); 
        }
    DBG_TRACE_FN_END;
    return EFalse;
    }


// ---------------------------------------------------------
// Return TInt with image loading error.
// ---------------------------------------------------------
//
TBool CGifAnimationPluginControl::GetLastError() const
    {
    DBG_TRACE_FN_BEGIN;        
    // 
    DBG_TRACE_FN_END;
    return iLastError;
    }

// ---------------------------------------------------------
// Start animation.
// ---------------------------------------------------------
//
void CGifAnimationPluginControl::StartAnimationL()
    {
    DBG_TRACE_FN_BEGIN;        
    
    if ( iEngine == NULL ) // animation was stopped, 
        {                  // need to re-load animation file
        DoImageLoadingL();
        }
        
    // start animation    
    if ( iEngine && IsAnimation() )
        {
        iEngine->Play();
        iAnimationState = EAnimationPlaying;
        }

    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------
// Stop animation.
// ---------------------------------------------------------
//
void CGifAnimationPluginControl::StopAnimation()
    {
    DBG_TRACE_FN_BEGIN;        
    
    // bugfix for JPYO-6KXHRW
    MakeVisible( EFalse );
    
    
    // instead of just stopping player, we delete it. 
    // This is because of stability issues with the start-stop-destroy cycle
    DeleteAnimation();
    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------
// Called when size is changed.
// ---------------------------------------------------------
//
void CGifAnimationPluginControl::SizeChanged()
    {
    DBG_TRACE_FN_BEGIN;        
    if ( iEngine )
        {
        iEngine->SetViewerSize( Size() );
        }
    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------------------------
// Overriden CCoeControl::Draw()
// ---------------------------------------------------------------------------
//
void CGifAnimationPluginControl::Draw( const TRect& /*aRect*/ ) const
    {
    //DBG_TRACE_FN_BEGIN;        
    CWindowGc& gc = SystemGc();
    TRect rect( Rect() );

    DBG_TRACE( "Draw: Clearing background" );
    gc.Clear( Rect() );
    gc.SetBrushColor( KRgbBlack );
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush); 
    gc.DrawRect( rect );
        
    if ( iDrawingBitmap 
      && iEngine 
      && iLastError == KErrNone ) // loading was successful
        {
        TSize screenSize( rect.Size() );
        TPoint destinationPoint( 
                ( screenSize.iWidth-iTargetNewSize.iWidth ) / 2, 
                ( screenSize.iHeight-iTargetNewSize.iHeight ) / 2 );
        
        TSize drawingBitmapSize( iDrawingBitmap->Bitmap().SizeInPixels() );
        if ( drawingBitmapSize.iHeight == iTargetNewSize.iHeight
          && drawingBitmapSize.iWidth == iTargetNewSize.iWidth )
            {
            // we use unscaled version as size is Ok
            iDrawingBitmap->Draw( gc, 
                                  destinationPoint,
                                  iTargetNewSize );
            }
        else
            {
            // we use scaled version 
            if ( iDrawingBitmap->HasMask() )                
                {
                gc.BitBltMasked( destinationPoint, 
                                 iScalingBitmap, 
                                 iTargetNewSize,
                                 iScalingBitmapMask, 
                                 EFalse );
                }
            else
                {
                gc.BitBlt( destinationPoint, 
                           iScalingBitmap );
                }
            }
        }
        else // image is not ready or broken
        {
        DBG_TRACE( "image is not ready or broken" );
        }
    //DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------
// CGifAnimationPluginControl::DoImageLoadingL
// ---------------------------------------------------------
//
void CGifAnimationPluginControl::DoImageLoadingL()
    {
    DBG_TRACE_FN_BEGIN;        
    
    RFs& fs = CEikonEnv::Static()->FsSession();
    TBool fileExists = BaflUtils::FileExists( fs, iFileName );
    if ( !fileExists )
        {
        DBG_TRACE_FN_END;
        User::Leave( KErrPathNotFound );
        }
        
    
    RFile fileHandle;
    CleanupClosePushL( fileHandle );
    iLastError = fileHandle.Open( fs, iFileName, EFileRead | EFileShareReadersOnly );
    User::LeaveIfError(iLastError);
    
    // delete old instances, if any
    DeleteAnimation();

    // create new objects
    if ( iDrawingBitmap == NULL )
        {
        iDrawingBitmap = IHLBitmap::CreateL();        
        }
    
    TInt drmOption( 0 );
    if ( isPreviewMode )
        {
        drmOption = MIHLFileImage::EOptionNoDRMConsume;    
        }
    iSourceImage = IHLImageFactory::OpenFileImageL( fileHandle, 
                                                    0,  // image index
                                                    drmOption );

    // calculate target size so that picture fits the screen and centered
    TSize sourceSize( iSourceImage->Size() );
    TSize maxSize = Size();
    
    iTargetSize = TSize( Min( sourceSize.iWidth, maxSize.iWidth ),
                         Min( sourceSize.iHeight, maxSize.iHeight ) );

    if ( sourceSize.iWidth < maxSize.iWidth 
      && sourceSize.iHeight < maxSize.iHeight )
        {
        // scale up N-times
        TInt upScale = Min( maxSize.iWidth  / sourceSize.iWidth, 
                            maxSize.iHeight / sourceSize.iHeight );
        iTargetSize = TSize( sourceSize.iWidth * upScale, 
                             sourceSize.iHeight * upScale );
        const TUint32 options( 0 ); // no flags set
        // we do not want IHL do scaling, so targetSize= sourceSize
        iEngine = IHLViewerFactory::CreateImageViewerL( sourceSize, 
                                                        *iSourceImage, 
                                                        *iDrawingBitmap, 
                                                        *this, 
                                                        options ); 
        iTargetNewSize = iTargetSize;                                                
    	}
    else 
    	{
    	//The image needs to be scaled down. We pass the target size to 
    	//IHL so that it doesn't come back with an OOM situation in case
    	//the resolution is too high.
    	// scale up N-times
        TReal downScale = Min( TReal(maxSize.iWidth) / TReal(sourceSize.iWidth), 
        		               TReal(maxSize.iHeight) / TReal(sourceSize.iHeight) );
        iTargetSize = TSize( sourceSize.iWidth * downScale, 
                             sourceSize.iHeight * downScale );
        const TUint32 options( 0 ); // no flags set
        TReal widthRatio( TReal( iTargetSize.iWidth ) / TReal( sourceSize.iWidth ) );
        TReal heightRatio( TReal( iTargetSize.iHeight ) / TReal( sourceSize.iHeight ) );
        if( options & MIHLImageViewer::EOptionIgnoreAspectRatio )
            {
            downScale = ( widthRatio > heightRatio ) ? widthRatio : heightRatio;
            }
        else
            {
            downScale = ( widthRatio < heightRatio ) ? widthRatio : heightRatio;
            }
        TReal widthZoomRatio( downScale );
        TReal heightZoomRatio( downScale );
        if( options & MIHLImageViewer::EOptionIgnoreAspectRatio )
            {
            TReal widthRatio( TReal( iTargetSize.iWidth ) / TReal( sourceSize.iWidth ) );
            TReal heightRatio( TReal( iTargetSize.iHeight ) / TReal( sourceSize.iHeight ) );
            if( widthRatio < heightRatio )
                {
                widthZoomRatio = widthZoomRatio * widthRatio / heightRatio;
                }
            else
                {
                heightZoomRatio = heightZoomRatio * heightRatio / widthRatio;
                }
            }
        iTargetNewSize = TSize( sourceSize.iWidth * widthZoomRatio, 
                             sourceSize.iHeight * heightZoomRatio );
        // we do not want IHL do scaling, so targetSize= sourceSize
        iEngine = IHLViewerFactory::CreateImageViewerL( iTargetSize, 
                                                        *iSourceImage, 
                                                        *iDrawingBitmap, 
                                                        *this, 
                                                        options ); 
     	}

    // create bitmaps needed for manual scaling    
    TDisplayMode dMode = CEikonEnv::Static()->DefaultDisplayMode();
    if ( iScalingBitmap == NULL )
        {
        iScalingBitmap = new( ELeave ) CFbsBitmap;
        iScalingBitmap->Create( iTargetSize, dMode ); 
        }

    if ( iScalingBitmapMask == NULL )
        {
        iScalingBitmapMask = new( ELeave ) CFbsBitmap;
        iScalingBitmapMask->Create( iTargetSize, dMode ); 
        }

    CleanupStack::PopAndDestroy( &fileHandle );
    DBG_TRACE_FN_END;
    }
    
// ---------------------------------------------------------
// CGifAnimationPluginControl::CheckFileIsValidL
// ---------------------------------------------------------
//
void CGifAnimationPluginControl::CheckFileIsValidL( 
                                        const TDesC& aImageFileName )
    {
    DBG_TRACE_FN_BEGIN;        

    ASSERT( aImageFileName.Size() ); 
    CGifAnimationPluginControl* temp = 
                new ( ELeave )CGifAnimationPluginControl();
    CleanupStack::PushL( temp );
    temp->SetSize( TSize( 100, 100 ) );
    temp->SetPreviewMode();
    temp->LoadImageL( aImageFileName );
    temp->DeleteAnimation();
    CleanupStack::PopAndDestroy( temp );
    
    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------
// CGifAnimationPluginControl::DeleteAnimation
// ---------------------------------------------------------
//
void CGifAnimationPluginControl::DeleteAnimation()
    {
    DBG_TRACE_FN_BEGIN; 
    if ( iEngine )
        {
        iAnimationState = EAnimationNotReady;
        iEngine->Stop();
        delete iEngine;
        iEngine = NULL; 
        }
        
    if ( iSourceImage )       
        {
        delete iSourceImage;            
        iSourceImage = NULL;
        }
        
    if ( iDrawingBitmap ) 
        {
        delete iDrawingBitmap;
        iDrawingBitmap = NULL; 
        }

    if ( iScalingBitmap )
        {
        delete iScalingBitmap;
        iScalingBitmap = NULL;    
        }

    if ( iScalingBitmapMask )
        {
        delete iScalingBitmapMask;
        iScalingBitmapMask = NULL;    
        }
        
        
    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------
// Handle bitmap change notifications. State is changed accordingly
// if this is the first frame. 
// ---------------------------------------------------------
//
void CGifAnimationPluginControl::ViewerBitmapChangedL()
    {   
    if ( iSourceImage )
        {
        switch ( iAnimationState )
            {
            case EAnimationLoading:
                {
                iAnimationState = EAnimationPlaying;
                break;
                }
            case EAnimationPlaying:
                {
                // do nothing 
                break;
                }
            case EAnimationNotReady:
            default:
                {
                break;
                }
            }
        }
    
    if( iAnimationState == EAnimationPlaying )
        {
        if ( iDrawingBitmap 
          && iEngine 
          && iLastError == KErrNone ) // loading was successful
            {
            TSize drawingBitmapSize( iDrawingBitmap->Bitmap().SizeInPixels() );
            if ( drawingBitmapSize.iHeight == iTargetNewSize.iHeight
              && drawingBitmapSize.iWidth == iTargetNewSize.iWidth )
                {
                // we do not need to do scaling
                }
            else
                {
                // we need to do scaling
                CFbsBitmapDevice* bitmapDevice = CFbsBitmapDevice::NewL( iScalingBitmap );
                CleanupStack::PushL(bitmapDevice);
                CFbsBitGc* graphicsContext = NULL; 
                User::LeaveIfError( bitmapDevice->CreateContext( graphicsContext ) ); 
                CleanupStack::PushL( graphicsContext ); 
                TRect srcRect( iSourceImage->Size() );
                graphicsContext->DrawBitmap( iTargetSize, &iDrawingBitmap->Bitmap(), srcRect ); 
                CleanupStack::PopAndDestroy( 2 );//graphicsContext,bitmapDevice
                
                if ( iDrawingBitmap->HasMask() )                
                    {
                    CFbsBitmapDevice* bitmapMaskDevice = CFbsBitmapDevice::NewL( iScalingBitmapMask );
                    CleanupStack::PushL(bitmapMaskDevice);
                    CFbsBitGc* graphicsMaskContext = NULL; 
                    User::LeaveIfError( bitmapMaskDevice->CreateContext( graphicsMaskContext ) ); 
                    CleanupStack::PushL( graphicsMaskContext ); 
                    graphicsContext->DrawBitmap( iTargetSize, &iDrawingBitmap->Mask(), srcRect ); 
                    CleanupStack::PopAndDestroy( 2 );//graphicsContext,bitmapDevice
                    }
                }
            }
            
        MakeVisible( ETrue );
        DrawNow();
        }
    }

// ---------------------------------------------------------
// Handles engine errors.
// ---------------------------------------------------------
//
void CGifAnimationPluginControl::ViewerError( TInt aError )
    {
    DBG_TRACE_FN_BEGIN;      
    iLastError = aError;  
    HandleCallback( aError );
    DBG_TRACE_FN_END;
    }


// ---------------------------------------------------------
// Handles error codes; stores the error
// ---------------------------------------------------------
//
void CGifAnimationPluginControl::HandleCallback( TInt aError )
    {
    DBG_TRACE_FN_BEGIN;        

    if ( aError )
        {
        InformPluginFinished();   
        }

    DBG_TRACE_FN_END;
    }


// ---------------------------------------------------------
// Informs that plug-in wants to terminate
// ---------------------------------------------------------
//
void CGifAnimationPluginControl::InformPluginFinished()
    {
    
    ASSERT( iPluginAdapter );

    StopAnimation();
    
    TRAP_IGNORE(iPluginAdapter->PluginFinishedL());    
    }
    
void CGifAnimationPluginControl::SetPreviewMode()    
    {
    isPreviewMode = ETrue;
    }
