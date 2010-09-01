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
* Description:   Implementation of screensaver moving text display class.
*
*/

#ifdef USE_DATE_AND_TEXT

#include <e32def.h>
#include <eikdef.h>
#include <AknUtils.h>
#include <e32math.h>
#include <aknlayoutscalable_avkon.cdl.h>
#include <screensaver.rsg>

#include "screensaverappui.h"
#include "screensavershareddatai.h"
#include "ScreensaverUtils.h"
#include "screensaverutility.h"
#include "screensaverctrlmovingtext.h"

const TInt KInvertedColors = 1;

//The color for text and background
const TInt KInvertedBgColor = 215;
const TInt KInvertedTextColor = 0;
const TInt KInvertedTextBgColor = 215;

const TInt KBgColor = 0;
const TInt KTextColor = 215;
const TInt KTextBgColor = 210;

// the minimum top value
const TInt KMinTop = 4;

const TInt KRefreshRate = 60000000; // 60 sec
//
// Class CScreensaverCtrlMovingText
//
// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::NewL
// -----------------------------------------------------------------------------
//
CScreensaverCtrlMovingText* CScreensaverCtrlMovingText::NewL( TDisplayObjectType aType )
    {
    CScreensaverCtrlMovingText* obj = new( ELeave ) CScreensaverCtrlMovingText( aType );
    CleanupStack::PushL( obj );
    obj->ConstructL();
    CleanupStack::Pop();
    return obj;
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::~CScreensaverCtrlMovingText
// -----------------------------------------------------------------------------
//
CScreensaverCtrlMovingText::~CScreensaverCtrlMovingText()
    {
    iEikonEnv->RemoveFromStack( this );
    
    DeleteTimer( iBarRefreshTimer );
    DeleteTimer( iCaptureScreenTimer );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::HandleResourceChange
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::HandleResourceChange( TInt aType )
    {
    if ( aType == KEikDynamicLayoutVariantSwitch )//&& iType != EDisplayNone)
        {
        // Screen layout has changed - resize
        SetRect( iCoeEnv->ScreenDevice()->SizeInPixels() );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::SizeChanged
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::SizeChanged()
    {
    GenerateDisplayAttributesForScreensaverBar();
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::Draw
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::Draw( const TRect& aRect ) const
    {
    if ( !Model().ScreenSaverIsOn() && !Model().ScreenSaverIsPreviewing() )
        {
        return;
        }

    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::Draw, screensaver is on or previewing") );
    // Graphics context to draw on.
    CWindowGc& gc = SystemGc();

    // Fix for error ESMG-74Y4PE - S60 3.2 wk26, Power Saver: 
    // Flickering when power saver is deactivated.
    // We now clear the screen with a black brush so the screensaver 
    // background is changed to black. There will no longer be a white
    // intermediate screen and this will reduce the "flicker" effect.
    gc.SetBrushColor( KRgbBlack );

    // Start with a clear screen
    gc.Clear( aRect );
    // If there is no plugin module, indicator view overrides plugin module or
    // plugin drawing is suspended then the standard screensaver bar is shown,
    // let's draw it.

    DrawNew( gc, aRect );

    // Activate power save display mode after draw, so that
    // the image is already in screen buffer
    ScreensaverUtility::FlushDrawBuffer(); 
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::Draw, draw finished") );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::SetMoving
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::SetMoving( TBool aMove )
    {
    iMoveBar = aMove;
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::StartTimer
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::StartTimer()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::StartTimer start") );
    Model().SharedDataInterface()->SetSSForcedLightsOn( ESSForceLightsOn );

    StartCaptureScreenTimer();

    StartBarRefreshTimer();
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::StartTimer finish") );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::CancelTimer
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::CancelTimer()
    {
    DeleteTimer( iBarRefreshTimer );
    DeleteTimer( iCaptureScreenTimer );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::ClearScreen
// Date & time saver
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::ClearScreen()
    {
    
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::Refresh
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::Refresh()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::Refresh start") );
    // Currently only keylock indicator is updated, because
    // thats the only indicator whose state may change while screensaver
    // is displaying. Other indicators' state changing also dismisses
    // screensaver. Once redisplaying, the indicators are updated anyway.
    // Key lock indicator depends on status of key guard.
    TIndicatorPayload payload;
    payload.iType = EPayloadTypeInteger;
    
    Array().SetDependencyStatus( ESsKeyLockInd, !Model().SharedDataInterface()->IsKeyguardOn() );
    payload.iInteger = Model().SharedDataInterface()->UnreadMessagesNumber();
    Array().SetIndicatorPayload( ESsNewMessagesInd, payload );
    Array().SetDependencyStatus( ESsNewMessagesInd, ( payload.iInteger <= 0 ) );
    Array().SetDependencyStatus( ESsVoicemailInd, !Model().SharedDataInterface()->IsHaveNewVoicemail() );
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::Refresh move") );
    // Don't move bar during this refresh
    SetMoving( EFalse );
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::Refresh DrawObject") );
    // Cause a redraw
    DrawObject();
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::Refresh move") );
    // Allow moving
    SetMoving();
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::Refresh finish") );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::CScreensaverCtrlMovingText
// Date & time saver
// -----------------------------------------------------------------------------
//
CScreensaverCtrlMovingText::CScreensaverCtrlMovingText( TDisplayObjectType aType )
    :iType( aType )
    {
    // No implementation required
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::ConstructL()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::ConstructL start") );
    
    if ( iType == EDisplayText )
        {
        iPictoI = CAknPictographInterface::NewL( *this, *this );
        }

    iTop = KMinTop;
    iRandSeed = RandomSeed();
    iMoveBar = ETrue;

    GetTextAndBgColor();
    
    CreateWindowL();

    // Cannot do this earlier - it'll cause a SizeChanged()
    SetRect( iCoeEnv->ScreenDevice()->SizeInPixels() );

    iEikonEnv->EikAppUi()->AddToStackL(
        this, ECoeStackPriorityEnvironmentFilter, ECoeStackFlagRefusesFocus );
    ConstructAndConnectLCDL();
    ActivateL();
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::DrawNew
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::DrawNew( CWindowGc& aGc, const TRect& /* aRect */) const
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::DrawNew start") );
    // Draw bar background
    iLayoutBar.DrawRect( aGc );

    // Draw clock 
    iLayoutClock.DrawRect( aGc );
    iLayoutClockText.DrawText( aGc, iTextTime );

    // Draw date / user text
    iLayoutDate.DrawRect( aGc );

    DrawText( aGc );

    // Draw indicators
    Array().Draw( aGc );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::DrawPictoText
// Draws text with pictographs
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::DrawPictoText( CWindowGc &aGc ) const
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::DrawPictoText start") );
    // Draw text and possible pictographs
    MAknPictographDrawer* pictoDrawer = iPictoI->Interface();
    ASSERT( pictoDrawer );

    if ( ( !pictoDrawer ) || ( !pictoDrawer->ContainsPictographs( iText ) ) )
        {
        // Just draw normally
        iLayoutDateText.DrawText( aGc, iText );
        return;
        }

    // Else draw normal text with pictographs
    aGc.UseFont( iLayoutDateText.Font() );
    aGc.SetPenColor( iLayoutDateText.Color() );

    // Calculating the offset (unfortunately not public in TAknLayoutText)
    // copied from AknUtils TAknLayoutText::LayoutText()
    pictoDrawer->DrawText( aGc, *iLayoutDateText.Font(), iText,
        iLayoutDateText.TextRect(), iLayoutDateText.Font()->AscentInPixels(), 
        iLayoutDateText.Align() );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::DrawText
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::DrawText( CWindowGc& aGc ) const
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::DrawText start") );
    if ( iPictoI )
        {
        // Draw text and pictographs in the layout
        DrawPictoText( aGc );
        }
    else
        {
        // Draw just text
        iLayoutDateText.DrawText( aGc, iText );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::UpdateCurrentTimeL
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::UpdateCurrentTimeL()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::UpdateCurrentTimeL start") );
    // AVKON does not have proper format string for just time,
    // load from own resources
    FormatTextFromResourceL( iTextTime, R_SCREENSAVER_TIME_USUAL_WITHOUT_AMPM );

    if ( iType == EDisplayTime )
        {
        FormatTextFromResourceL( iText, R_QTN_DATE_USUAL_WITH_ZERO );
        }
    else
        {
        Model().SharedDataInterface()->GetDisplayObjectText( iText );
        }

    SetAmPmIndicatorPayloadL();
    // Convert numbers according to language
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::GenerateDisplayAttributesForScreensaverBar
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::GenerateDisplayAttributesForScreensaverBar( )
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::GenerateDisplayAttributesForScreensaverBar start") );
    // Zero out power save area, it should then be set before power save
    // mode can be activated (also by plugins)
    iPowerSaveDisplayActiveArea.SetRect( 0, 0, 0, 0 );

    // Cool! We got scalable layouts. Use them.
    TAknWindowComponentLayout barLayoutComponent = 
        AknLayoutScalable_Avkon::power_save_pane();
    iLayoutBar.LayoutRect( Rect(), barLayoutComponent.LayoutLine() );

    // Add bar height from layout to get total bar height
    TInt barHeight = iLayoutBar.Rect().Height();

    // Plain refresh, no move?
    if ( iMoveBar )
        {
        // Get random values for bar position and color.
        GetRandomTopValue( iLayoutBar.Rect().Height() );
        }

    // Create a rect with the new top to be used as layout rect
    TRect fakeScreen( Rect() );
    fakeScreen.iTl.iY = iTop;

    // Layout components

    // Bar itself
    iLayoutBar.LayoutRect( fakeScreen, barLayoutComponent.LayoutLine() );
    TRect barRect = iLayoutBar.Rect();

    // Clock (area + text)
    GetClockLayout( barRect );

    // Date (area + text)
    GetDateLayout( barRect );

    // Indicators (area + text)
    GetIndicatorAreaLayout( barRect );

    // Update current time
    TRAP_IGNORE( UpdateCurrentTimeL() );

    // Since it is known for sure at this point that screensaver
    // bar will be drawn the partial mode can activated for bar area
    // if it is supported by display hardware.
    const TScreensaverColorModel& colorModel = Model().GetColorModel();

    if ( colorModel.iNumberOfPartialModes > 0 )
        {
        TRect psRect( iLayoutBar.Rect() );
        SetPowerSaveDisplayActiveArea( psRect );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::GetRandomTopValue
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::GetRandomTopValue( TInt aBarHight )
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::GetRandomTopValue start") );
    TInt range( iCoeEnv->ScreenDevice()->SizeInPixels().iHeight - aBarHight - 1 - 4 );
    TInt rand = Math::Rand( iRandSeed );
    rand %= range;
    iTop = ( rand > 3 ) ? ( rand/4 ) * 4 : 4;
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::RandomValue
// -----------------------------------------------------------------------------
//
TInt CScreensaverCtrlMovingText::RandomSeed()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::RandomSeed start") );
    TTime time;
    time.HomeTime();
    return time.DateTime().MicroSecond();
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::GetTextAndBgColor
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::GetTextAndBgColor()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::GetTextAndBgColor start") );
    if( Model().SharedDataInterface()->IsInvertedColors() == KInvertedColors )
        {
        iBgColor = KInvertedBgColor;
        iTextColor = KInvertedTextColor;
        iTextBgColor = KInvertedTextBgColor;
        }
    else
        {
        iBgColor = KBgColor;
        iTextColor = KTextColor;
        iTextBgColor = KTextBgColor;
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::FormatTextL
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::FormatTextL( TDes& aText, const TDesC& aFormat )
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::FormatTextL start") );
    TTime curTime;
    curTime.HomeTime();
    // Since screensaver is updated on the minute, make sure it displays
    // the time of the passing minute by adding a few seconds to the
    // time at the moment of the update.
    curTime += TTimeIntervalSeconds( 5 );
    
    TRAPD( err, curTime.FormatL( aText, aFormat ) );
    if ( err != KErrNone )
        {
        // If time can't be displayed then display an empty text
        aText = KNullDesC;
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::SetAmPmIndicatorPayload
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::SetAmPmIndicatorPayloadL()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::SetAmPmIndicatorPayloadL start") );
    _LIT(KAmPmFormat, "%*A");
    TBuf<10> textAmPm;
    FormatTextL( textAmPm, KAmPmFormat );
    
    TIndicatorPayload payload;
    payload.iType = EPayloadTypeText;
    payload.iText.Zero();
    payload.iText.Append( textAmPm );
    Array().SetIndicatorPayload( ESsAmPmInd, payload );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::FormatTextFromResourceL
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::FormatTextFromResourceL( TDes &aText, TInt aResourceId )
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::FormatTextFromResourceL start") );
    TBuf<30> timeFormat;
    iCoeEnv->ReadResourceL( timeFormat, aResourceId );
    FormatTextL( aText, timeFormat );
    AknTextUtils::DisplayTextLanguageSpecificNumberConversion( aText );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::GetClockLayout
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::GetClockLayout( const TRect& aRect )
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::GetClockLayout start") );
    TAknWindowComponentLayout clockarea =
        AknLayoutScalable_Avkon::aid_ps_clock_pane( 0 );
    TAknWindowLineLayout clockline = clockarea.LayoutLine();
    clockline.iC = iBgColor;
    iLayoutClock.LayoutRect( aRect, clockarea.LayoutLine() );

    TAknTextComponentLayout clocktext =
        AknLayoutScalable_Avkon::power_save_t2( 0 );
    clocktext.SetC( iTextColor );
    iLayoutClockText.LayoutText( aRect, clocktext.LayoutLine() );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::GetClockLayout
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::GetDateLayout( const TRect& aRect )
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::GetDateLayout start") );
    TAknWindowComponentLayout datearea =
        AknLayoutScalable_Avkon::power_save_pane_g1( 0 );
    TAknWindowLineLayout dateline = datearea.LayoutLine();
    dateline.iC = iTextBgColor;
    iLayoutDate.LayoutRect( aRect, dateline );

    TAknTextComponentLayout datetext =
        AknLayoutScalable_Avkon::power_save_pane_t1( 0 );
    TAknTextLineLayout datetextline = datetext.LayoutLine();
    iLayoutDateText.LayoutText( aRect, datetextline );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::GetClockLayout
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::GetIndicatorAreaLayout( const TRect& aRect )
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::GetIndicatorAreaLayout start") );
    TAknWindowComponentLayout indicatorarea =
        AknLayoutScalable_Avkon::aid_ps_indicator_pane( 0 );
    TAknWindowLineLayout indicatorline = indicatorarea.LayoutLine();
    indicatorline.iC = iBgColor;
    TAknLayoutRect layoutIndicatorArea;
    layoutIndicatorArea.LayoutRect( aRect, indicatorline );

    // Indicator texts
    TAknTextComponentLayout indtext =
        AknLayoutScalable_Avkon::power_save_pane_t4( 0 );
    TAknTextLineLayout indline = indtext.LayoutLine();
    TAknLayoutText layoutIndicatorText;
    layoutIndicatorText.LayoutText( aRect, indline );

    // Indicator icons
    TAknWindowComponentLayout inds =
        AknLayoutScalable_Avkon::indicator_ps_pane( 0 );
    TAknWindowLineLayout indsline = inds.LayoutLine();
    TAknLayoutRect layoutInds;
    layoutInds.LayoutRect( aRect, indsline );

    // Tell indicator array where to draw (this should be moved to
    // the array itself)
    Array().Setup( layoutIndicatorArea, layoutIndicatorText, layoutInds );
    }

// -----------------------------------------------------------------------------
// CScreensaverCtrlMovingText::StartBarRefreshTimer
// -----------------------------------------------------------------------------
//
void CScreensaverCtrlMovingText::StartBarRefreshTimer()
    {
    SCRLOGGER_WRITEF( _L("SCR:CScreensaverCtrlMovingText::StartBarRefreshTimer start") );
    DeleteTimer( iBarRefreshTimer );
    
    TRAP_IGNORE( iBarRefreshTimer = CPeriodic::NewL( CActive::EPriorityStandard ) );
    
    TInt initialRefreshRate = InitialRefreshRate();
    
    iBarRefreshTimer->Start( initialRefreshRate, KRefreshRate, TCallBack(
        HandleRefreshTimerExpiry, this ) );
    
    SCRLOGGER_WRITEF( _L("SCR: BarRefreshTimer->Start(%d, %d, HandleRefreshTimerExpiry)"),
        initialRefreshRate, KRefreshRate );
    }

#endif //USE_DATE_AND_TEXT
// End of file.
