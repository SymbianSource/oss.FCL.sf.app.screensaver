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
* Description:   Screensaver GifAnimation plug-in implementation header file
*
*/



#ifndef C_GIFANIMATIONPLUGIN_H
#define C_GIFANIMATIONPLUGIN_H

#include <e32std.h>
#include <coecntrl.h>
#include <coemain.h>
#include <fbs.h>
#include <barsread.h>

#include <npupp.h>
#include <pluginadapterinterface.h>
#include <cecombrowserplugininterface.h>
#include <MIHLViewerObserver.h>


// FORWARD DECLARATIONS
class MIHLFileImage;
class MIHLBitmap;
class MIHLImageViewer;

class CCoeControl;
class CGifAnimationPlugin;

// CONSTANTS
/** implementation id GifAnim Ecom Plugin */
const TInt KGifAnimationPluginImplementationValue = 0x102750B4; 

/******************************************
* This class is specific to ECom Style Plugin.
* This is used for passing plugin specific initialization information 
* to and from browser.
*******************************************/

class CGifAnimationPluginEcomMain :  public CEcomBrowserPluginInterface
    {
    public: // Functions from base classes

        /**
        * Instance factory 
        * @param aInitParam an optional parameter
        */
        static CGifAnimationPluginEcomMain* NewL(TAny* aInitParam);

        /**
        * Destructor.
        */
        virtual ~CGifAnimationPluginEcomMain();

        /**
        * @return pointer to functions struct.
        */
        NPNetscapeFuncs* Funcs() { return iNpf; }

    private: // Constructors
        CGifAnimationPluginEcomMain( NPNetscapeFuncs* aNpf );
        void Construct( NPPluginFuncs* aPluginFuncs );
    private: // new functions
        /**
         * Sets pointers to interface functions
         * @param aPpf structure to fill
         */
        NPError InitializeFuncs( NPPluginFuncs* aPpf );

    private: // Data
        // pointer to plug-in function pointers struct
        NPNetscapeFuncs* iNpf;
    };

/****************************************
Class CGifAnimationPluginControl
Class to Handle UI related actions
****************************************/
class CGifAnimationPluginControl : public CCoeControl, 
                                   public MIHLViewerObserver

    {
public: // Constructor and Destructor

    /**
     * Function:    NewL
     *
     * Description: Get instance of a CGifAnimationPluginControl object.
     * @param aParentControl a parent class to register to
     * @param aPluginAdapter pointer to plugin adapter
     */
    static CGifAnimationPluginControl* NewL( CCoeControl* aParentControl,
										     MPluginAdapter* aPluginAdapter);

    /**
    * Destructor.
    */
    ~CGifAnimationPluginControl();

public: // From MIHLViewerObserver
    /**
    * From MIHLViewerObserver Notifies client 
    * when viewer bitmap content is changed.
    * Leave situation is handled in ViewerError() callback.
    */
    void ViewerBitmapChangedL();

    /**
    * From MIHLViewerObserver Notifies client if error occurs in viewer.
    * @param aError     IN  System wide error code.
    */
    void ViewerError( TInt aError );

public: // new methods
    /**
     * Loads the image into display.
     * @param aImageFileName image file name - expected to be valid image
     */
    void LoadImageL( const TDesC& aImageFileName );
    
    /**
     * @return last error, if any
     */
    TInt GetLastError() const;
    
    /**
     * Start animation.
     */
    void StartAnimationL();
    
    /**
     * Stop animation.
     */
    void StopAnimation();
    
    /**
     * Checks that the file is a valid image file.
     * Will leave otherwise
     *
     * @param aImageFileName image file name - expected to be valid image
     */
    static void CheckFileIsValidL( const TDesC& aImageFileName );
   
   
private: // new methods

    /**
     * @return ETrue if loaded image is animation.
     */
    TBool IsAnimation() const;
    
    /**
     * Cancel animation.
     */
    void CancelAnimation();

    /**
     * Performs image loading steps.
     * Uses file name stored in LoadImageL()
     */                   
    void DoImageLoadingL();
    
    /**
     * Frees resources allocated with LoadImageL()
     */                   
    void DeleteAnimation();
    
    /**
     * If loading content fails, non-leaving HandleCallback
     * must be call always that parent can update its state
     * properly.
     */
    void HandleCallback( TInt aError );
    
    /** 
     * Informes that the plug-in wants to terminate
     */
    void InformPluginFinished();
    
    /**
     * Informs that DRM rights should not be consumed
     */
    void SetPreviewMode();

private: // from CCoeControl
    void Draw(const TRect& aRect) const;
    void SizeChanged();

private: // Constructor
    /**
     * Constructor.
     */
    CGifAnimationPluginControl();
    
    /**
     * ConstructL
     *
     * 2nd phase constructor.
     * @param aParentControl a parent class to register to
     * @param aPluginAdapter pointer to plugin adapter
     */
    void ConstructL( CCoeControl *aParentControl,
                     MPluginAdapter* aPluginAdapter );

private: // Data

    /**
     * Possible animation states
     */
    enum TAnimationState
        {
        EAnimationNotReady=1, // the file was not loaded yet or corrupt
        EAnimationLoading=2,
        EAnimationPlaying=3
        };
        
        // Source image
        MIHLFileImage*      iSourceImage;
        
        // Destination bitmap
        MIHLBitmap*         iDrawingBitmap;

        // the bitmap is used for scaling our way, IHL will not do scaling
        CFbsBitmap*         iScalingBitmap;

        // the mask for scaled bitmap 
        CFbsBitmap*         iScalingBitmapMask; 

        // size of target bitmap
        TSize               iTargetSize;
        
        // new size of target bitmap
        TSize               iTargetNewSize;
        
        // Image viewer used as engine
        MIHLImageViewer*    iEngine;
                
        // Internal animation state.
        TAnimationState     iAnimationState;

        // path and name of file being loaded
        TFileName           iFileName;
        
        // code of last loading error occured
        TInt                iLastError;        

        // pointer to adapter, we do not own the instance
        MPluginAdapter*             iPluginAdapter;
        
        // if true, no DRM rights are consumed
        TBool               isPreviewMode;
        
        // a flag to make sure the first ViewerBitmapChangedL is finished,
        // then we could draw image on screen
        TBool               isViewerBitmapChangedL;
    };


/*******************************************************
Class CGifAnimationPlugin
********************************************************/

class CGifAnimationPlugin : public CBase
    {
public: // Constructor and Destructor
    /** 
     * Instance factory 
     */
    static CGifAnimationPlugin* NewL( NPP anInstance );
    /**
     * Destructor
     */
    ~CGifAnimationPlugin();

    /**
     * Notifies host that plug-in has finished.
     */
    void PrepareToExitL();

    /**
     * @return ref of <code>CGifAnimationPluginControl</code>
     */
    CGifAnimationPluginControl* Control();

    /**
     * @return pointer on NPP instance
     */
    NPP Instance();

    /**
     * Sets the parent window to plug-in. If already set, only updates size.
     * @param aWindow a parent window
     * @param aRect a parent window rectangle
     */
    void SetWindowL( const NPWindow  *aWindow, const TRect& aRect );

private:
    /**
     * Construct.
     *
     * 2nd phase constructor.
     * @param anInstance NPP structure
     */
    void Construct( NPP anInstance );

private:
    // pointer to adapter
    MPluginAdapter*             iPluginAdapter;

    // pointer to Container
    CGifAnimationPluginControl* iControl;

    // structure with pointers
    NPP                         iInstance;
    };

/****************************************************************************
*
*    Plugin API methods
*
*/

/**
  * Create a plugin.
  */
NPError GifAnimationPluginNewp( NPMIMEType pluginType, 
                                NPP instance, 
                                uint16 mode, 
                                CDesCArray* argn, 
                                CDesCArray* argv, 
                                NPSavedData* saved );

/**
  * Destroy a plugin.
  */
NPError GifAnimationPluginDestroy(NPP instance, NPSavedData** save);

/**
  * This is the parent window of a plugin.
  */
NPError GifAnimationPluginSetWindow(NPP instance, NPWindow* window);

/**
  * A new data stream has been created for sending data to the plugin.
  */
NPError GifAnimationPluginNewStream( NPP instance, 
                                     NPMIMEType type, 
                                     NPStream* stream, 
                                     NPBool seekable, 
                                     uint16* stype );

/**
  * A data stream has been terminated.
  */
NPError GifAnimationPluginDestroyStream( NPP instance, 
                                         NPStream* stream, 
                                         NPReason reason );

/**
  * A data stream has been fully saved to a file.
  */
void GifAnimationPluginAsFile( NPP instance, 
                               NPStream* stream, 
                               const TDesC& fname );

/**
  * Return the maximum number of bytes this plugin can accept from the stream.
  * @param instance  - the plug-in instance
  * @param stream -  the stream to use
  */
int32 GifAnimationPluginWriteReady( NPP instance, NPStream* stream );

/**
  * Receive more data - return number of bytes processed.
  *
  * @param instance  - the plug-in instance
  * @param stream -  the stream to read from
  * @param buffer - contains the data.
  * @param len    - the number of bytes in buffer.
  * @param offset - the number of bytes already sent/processed.
  * @return TInt number of bytes processed.
  */
int32 GifAnimationPluginWrite( NPP instance, 
                               NPStream* stream, 
                               int32 offset, 
                               int32 len, 
                               void* buffer );

/**
  * Generic hook to retrieve values/attributes from the plugin.
  */
NPError GifAnimationPluginGetValue( NPP instance, 
                                    NPPVariable variable, 
                                    void *ret_value );

/**
  * Generic hook to set values/attributes within the plugin.
  */
NPError GifAnimationPluginSetValue( NPP instance, 
                                    NPNVariable variable, 
                                    void *ret_value );

/**
  * Event
  */
int16 GifAnimationPluginEvent( NPP instance, void* event );

/**
 * Notify
 */
void GifAnimationPluginURLNotify( NPP instance, 
                                  const TDesC& url, 
                                  NPReason reason, 
                                  void* notifyData );

/**
 * Print
 */
void GifAnimationPluginPrint( NPP instance, NPPrint* platformPrint );

#endif // C_GIFANIMATIONPLUGIN_H
