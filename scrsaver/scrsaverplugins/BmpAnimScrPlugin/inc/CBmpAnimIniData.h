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
* Description:   Bitmap animator INI file handler
*
*/



#ifndef C_CBMPANIMINIDATA_H
#define C_CBMPANIMINIDATA_H

#include <e32base.h>

class CBmpAnimIniData : public CBase 
	{
public:
	// Constructor, pass in name of ini file to open
	// Default search path is 'c:\system\data' on target filesystem
	// ie. 'NewL(_L("c:\\system\\data\\ttools.ini"))' is equivalent 
	// to 'NewL(_L("ttools.ini"))' 
	static CBmpAnimIniData* NewL(const TDesC& aName);
	virtual ~CBmpAnimIniData();

    // Read text value regardless of section
    // Returns: True(Success) or false(Failure)
    TBool FindVar(const TDesC8& aKeyName, TDes16& aResult);
    TBool FindVar(const TDesC8& aKeyName, TPtrC8& aResult);

    // Read integer value regardless of section
    // Returns: True(Success) or false(Failure)
    TBool FindVar(const TDesC8& aKeyName, TInt& aResult);

    // Read text value under section
    // Returns: True(Success) or false(Failure)
    TBool FindVar(
        const TDesC8& aSection, const TDesC8& aKeyName, TDes16& aResult);
    TBool FindVar(
        const TDesC8& aSection, const TDesC8& aKeyName, TPtrC8& aResult);

    // Read integer value under section
    // Returns: True(Success) or false(Failure)
    TBool FindVar(
        const TDesC8& aSection, const TDesC8& aKeyName, TInt& aResult);

    // Write integer value associated with a token
    TInt WriteVarL(
        const TDesC8& aSection, const TDesC8& aKeyName, TInt aValue);

    // Write string associated with a token
    TInt WriteVarL(
        const TDesC8& aSection, const TDesC8& aKeyName, const TDesC8& aValue);

    // Commits the changes done. NOTE if not called, the changes will NOT
    // be saved into file
    void CommitL();


protected:
    CBmpAnimIniData();
    void ConstructL(const TDesC& aName);

private:
    // Tries to locate the given section. EFalse if not found
    TBool FindSection(const TDesC8& aSection, TInt& aStart, TInt& aEnd);
    
private:
    HBufC* iName;
    HBufC8* iToken;
    TPtr8 iPtr;         // Holds the file in memory, written back in CommitL()
    };

#endif // C_CBMPANIMINIDATA_H

