/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:     Constant definitions for Screen Saver Anim Plugin 
*
*/





#ifndef SCREENSAVERANIMPLUGINCONST_H
#define SCREENSAVERANIMPLUGINCONST_H

// Feature IDs.
enum 
    {
    // Remove Back Light Setting
    EAnimPluginBacklightRemoved,

    /*Insert new item here*/

    // last feature should be max value
    KAnimPluginMaxFeatureID
    };

/**
*******************************************************************************
* Following constants are shared between PLSN. The three constants need to be 
* sychronized in both PLSN and Screensaver Animation plugin projects when each
* has changes. 
*******************************************************************************
*/
// Same definition as in PslnVariationPrivateCRKeys.h 
const TUid KCRUidThemesVariation = { 0x102818EB };


/**
* Specifies the bitmask for Themes applications locally variated features.
* 
* Possible values are defined in PslnVariant.hrh.
*/
const TUint32 KThemesLocalVariation       = 0x00000001;


// Same definition as in pslnvariant.hrh
// Removes screensaver animation plugin's timeout setting. 
// Bitmask value in decimals: 256.
#define KScreenSaverRemoveAnimPluginTimeout 0x00000100


#endif // SCREENSAVERANIMPLUGINCONST_H
// End Of file
