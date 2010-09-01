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
* Description:     Bitmap animator INI file handler
*
*/





#include "CBmpAnimIniData.h"

#include <f32file.h>
#include <e32std.h>

// Default directory to look for INI file
_LIT(KIniFileDir,"C:\\private\\100056cf\\");

const TInt KTokenSize=32;

#ifdef _DEBUG
// Debugging aids
enum TIniPanic
	{
	ESectionNameTooBig,
	EKeyNameTooBig,
	};

void Panic(TIniPanic aPanic)
	{
	_LIT(CBmpAnimIniData,"CBmpAnimIniData");
	User::Panic(CBmpAnimIniData,aPanic);
	}
#endif  // _DEBUG

// d'tor
CBmpAnimIniData::~CBmpAnimIniData()
	{
	delete (TText*)iPtr.Ptr();
	delete iToken;
	delete iName;
	}

    
// Public allocation method
CBmpAnimIniData* CBmpAnimIniData::NewL(const TDesC& aName)
	{
	CBmpAnimIniData* p=new(ELeave) CBmpAnimIniData;
	CleanupStack::PushL(p);
	p->ConstructL(aName);
	CleanupStack::Pop();
	return p;
	}


// Find a key's string) value regardless of section (wide)
TBool CBmpAnimIniData::FindVar(const TDesC8& aKeyName, TDes16& aResult)
    {
    TPtrC8 tmpRes;
    if (!FindVar(aKeyName, tmpRes))
        {
        return EFalse;
        }

    // Copy the 8-bit result into the 16-bit descriptor
    aResult.Copy(tmpRes);
    return ETrue;
    }

    
// Find a key's (string) value regardless of section
TBool CBmpAnimIniData::FindVar(const TDesC8& aKeyName, TPtrC8& aResult)
    {
    _LIT8(KDummySection, "");
    // Call with no section, so starts at beginning
    if (FindVar(KDummySection, aKeyName, aResult))
        {
        return ETrue;
        }
    else
        {
        return EFalse;
        }
    }
    

// Find a key's (string) value given a section name and a key name (wide)
TBool CBmpAnimIniData::FindVar(const TDesC8& aSectName,
                               const TDesC8& aKeyName,
                               TDes16& aResult)
    {
    TPtrC8 tmpRes;
    if (!FindVar(aSectName, aKeyName, tmpRes))
        {
        return EFalse;
        }

    // Copy the 8-bit result into the 16-bit descriptor
    aResult.Copy(tmpRes);
    return ETrue;
    }

    
// Find a key's (string) value given a section name and a key name
TBool CBmpAnimIniData::FindVar(const TDesC8& aSectName,
                               const TDesC8& aKeyName,
                               TPtrC8& aResult)
    {
    __ASSERT_DEBUG(aSectName.Length() <= KTokenSize, Panic(ESectionNameTooBig));
    __ASSERT_DEBUG(aKeyName.Length() <= KTokenSize, Panic(EKeyNameTooBig));

    TInt posStartOfSection(0);
    TInt posEndOfSection(0);
    TPtrC8 SearchBuf;

    // If we have a section, set pos to section start
    if (aSectName.Length() > 0)
        {
        if (!FindSection(aSectName, posStartOfSection, posEndOfSection))
            {
            return EFalse;
            }
        }

    // Look for key in ini file data Buffer
    TInt posI = posStartOfSection; // Position in internal data Buffer
    TBool FoundKey(EFalse);
    while (!FoundKey)
        {
        // Search for next occurrence of aKeyName
        SearchBuf.Set(iPtr.Mid(posI));
        TInt posSB = SearchBuf.Find(aKeyName);

        // If not found, return
        if (posSB == KErrNotFound)
            {
            return EFalse;
            }

        // Check this is at beginning of line (ie. non-commented)
        // ie. Check preceding char was CR or LF
        if(posSB > 0)
            {
            // Create a Buffer, starting one char before found subBuf
            TPtrC8 CharBefore(SearchBuf.Right(SearchBuf.Length() - posSB + 1));
            // Check first char is end of prev
            if(CharBefore[0] == '\n')
                {
                FoundKey = ETrue;
                posI = posI + posSB;
                }
            else
                {
                posI = posI + posSB + 1;
                }
            }
        else
            {
            FoundKey = ETrue;
            }

    }   // while (!FoundKey)

    // Set pos to just after '=' sign
    SearchBuf.Set(iPtr.Mid(posI));
    TInt posSB = SearchBuf.Locate('=');
    if (posSB == KErrNotFound)     // Illegal format, should flag this...
        {
        return EFalse;
        }

    // Identify start and end of data (EOL or EOF)
    posI = posI + posSB + 1;    // 1 char after '='
    TInt posValStart = posI;
    TInt posValEnd;
    SearchBuf.Set(iPtr.Mid(posI));
    posSB = SearchBuf.Locate('\r');
    if (posSB != KErrNotFound)
        {
        posValEnd = posI + posSB;
        }
    else
        {
        posValEnd = iPtr.Length();
        }

    // Check we are still in the section requested
    if (aSectName.Length() > 0)
        {
        if (posValEnd > posEndOfSection)
            {
            return EFalse;
            }
        }

    // Parse Buffer from posn of key
    // Start one space after '='
    TLex8 lex(iPtr.Mid(posValStart, posValEnd - posValStart));
    lex.SkipSpaceAndMark();     // Should be at the start of the data
    aResult.Set(lex.MarkedToken().Ptr(),
                posValEnd - posValStart - lex.Offset());
    
    return ETrue;
    }

    
// Find integer value regardless of section
TBool CBmpAnimIniData::FindVar(const TDesC8& aKeyName, TInt& aResult)
    {
    TPtrC8 ptr(NULL, 0);
    if (FindVar(aKeyName, ptr))
        {
        TLex8 lex(ptr);
        if (lex.Val(aResult) == KErrNone)
            {
            return ETrue;
            }
        }
    return EFalse;
    }

    
// Find integer value within a given section
TBool CBmpAnimIniData::FindVar(
    const TDesC8& aSection, const TDesC8& aKeyName, TInt& aResult)
    {
    TPtrC8 ptr(NULL, 0);
    if (FindVar(aSection, aKeyName, ptr))
        {
        TLex8 lex(ptr);
        if (lex.Val(aResult) == KErrNone)
            {
            return ETrue;
            }
        }
    return EFalse;
    }


// Write integer value in given section
TInt CBmpAnimIniData::WriteVarL(
    const TDesC8& aSection, const TDesC8& aKeyName, TInt aValue)
    {
    TBuf8<32> buf;
    buf.Num(aValue);
    return WriteVarL(aSection, aKeyName, buf);
    }


// Write string value in given section
TInt CBmpAnimIniData::WriteVarL(
    const TDesC8& aSection, const TDesC8& aKeyName, const TDesC8& aValue)
    {
    // First find the variable - this gives us a descriptor into the
    // ini data giving the bound of the item that has to be replaced
    TPtrC8 ptr;
    TInt pos;
    TBool found = FindVar(aSection, aKeyName, ptr);

    HBufC8* text = HBufC8::NewLC(
        aSection.Length() + aKeyName.Length() + aValue.Length() + 7); // +7 for '=', '[', ']' and 2*cr+lf.
    TPtr8 textp = text->Des();

    if (found)
        {
        pos = ptr.Ptr() - iPtr.Ptr();
        textp.Append(aValue);
        }
    else
        {
        TInt posStartOfSection = 0;
        TInt posEndOfSection = 0;
        
        if (FindSection(aSection, posStartOfSection, posEndOfSection))
            {
            pos = posEndOfSection;
            }
        else
            {
            pos = iPtr.Length();
            textp.Append('[');
            textp.Append(aSection);
            textp.Append(']');
            textp.Append('\r');
            textp.Append('\n');
            }
        textp.Append(aKeyName);
        textp.Append('=');
        textp.Append(aValue);
        textp.Append('\r');
        textp.Append('\n');
        }

    TInt size = (iPtr.Length() + textp.Length() - ptr.Length()) * sizeof(TText8);

    if (size > iPtr.MaxLength())
        {
        TText8* newdata = (TText8*) User::ReAllocL((TUint8*)iPtr.Ptr(), size);
        iPtr.Set(newdata, iPtr.Length(), size/sizeof(TText8));
        }

    iPtr.Replace(pos, ptr.Length(), textp);

    CleanupStack::PopAndDestroy(text);

    return KErrNone;
    }

    
// Commits the changes in variables, ie writes the file data (held in iPtr)
// into the actual disk file
void CBmpAnimIniData::CommitL()
    {
    TAutoClose<RFs> fs;
    User::LeaveIfError(fs.iObj.Connect());
    fs.PushL();

    // Just replace the file with the data, no seeking necessary
    TAutoClose<RFile> file;
    file.iObj.Replace(fs.iObj, *iName, EFileStreamText|EFileWrite);
    file.PushL();
    
    TPtrC8 ptrc8((TUint8*)iPtr.Ptr(), iPtr.Size());
    User::LeaveIfError(file.iObj.Write(ptrc8));
    
    file.Pop();
    fs.Pop();
    }


// --- protected --- 

// c'tor
CBmpAnimIniData::CBmpAnimIniData() 
: iPtr(NULL, 0)
    {
    __DECLARE_NAME(_S("CBmpAnimIniData"));
    }


// Allocate a buffer and Read file's contents into iPtr
void CBmpAnimIniData::ConstructL(const TDesC& aName)
    {
    // Allocate space for token
    iToken = HBufC8::NewL(KTokenSize + 2);   // 2 extra chars for [tokenName]

    // Connect to file server
    TAutoClose<RFs> fs;
    User::LeaveIfError(fs.iObj.Connect());
    fs.PushL();

    // Find file, given name
    TFindFile ff(fs.iObj);
    TAutoClose<RFile> file;    
    TInt err = ff.FindByDir(aName, KIniFileDir);
    
    // If found - cool, use the found file's name and open it
    if (err == KErrNone)
        {
        iName = ff.File().AllocL();
        err = file.iObj.Open(fs.iObj, *iName, EFileStreamText | EFileRead);
        }

    // If not found - create the file using the given name
    if (err == KErrNotFound)
        {
        err = file.iObj.Create(fs.iObj, aName, EFileStreamText);
        iName = aName.AllocL();
        }

    file.PushL();

    // Get file size and read in as 8-bit data
    TInt size = 0;
    User::LeaveIfError(file.iObj.Size(size));
    TText8* data = (TText8*) User::AllocL(size);
    iPtr.Set(data, size, size);
    TPtr8 dest((TUint8*)data, 0, size);
    User::LeaveIfError(file.iObj.Read(dest));

    file.Pop();
    fs.Pop();
    }
    

// --- private --- 


// Find a section in the INIfile
TBool CBmpAnimIniData::FindSection(
    const TDesC8& aSection, TInt& aStart, TInt& aEnd)
    {
    TInt posI = 0;  // Position in internal databuffer
    TBool FoundSection = EFalse;
    TPtrC8 SearchBuf;
    
    while (!FoundSection)
        {
        // Move search buffer to next area of interest
        SearchBuf.Set(iPtr.Mid(posI));
        
        // Make up token "[SECTIONNAME]", to search for
        TPtr8 sectionToken = iToken->Des();
        _LIT8(sectionTokenFmtString,"[%S]");
        sectionToken.Format(sectionTokenFmtString, &aSection);
        
        // Search for next occurrence of aSection
        TInt posSB = SearchBuf.Find(sectionToken);
        
        // If not found, return
        if (posSB == KErrNotFound)
            {
            return EFalse;
            }
        
        // Check this is at beginning of line (ie. non-commented)
        // ie. Check preceding char was LF
        if (posSB > 0)
            {
            // Create a Buffer, starting one char before found subBuf
            TPtrC8 CharBefore(SearchBuf.Right(SearchBuf.Length() - posSB + 1));
            // Check first char is end of prev
            if (CharBefore[0] == '\n')
                {
                FoundSection = ETrue;       // found
                posI = posI + posSB;
                }
            else
                {
                posI = posI + posSB + 1;    // try again
                }
            }
        else
            {
            FoundSection = ETrue;
            }
        
        }   // while (!FoundSection)
    
    // Set start of section, after section name, (incl '[' and ']')
    aStart = posI + aSection.Length() + 2;
    
    // Set end of section, by finding begin of next section or end
    SearchBuf.Set(iPtr.Mid(posI));
    
    _LIT8(nextSectionBuf,"\n[");
    
    TInt posSB = SearchBuf.Find(nextSectionBuf);
    if (posSB != KErrNotFound)
        {
        aEnd = posI + posSB + 1;
        }
    else
        {
        aEnd = iPtr.Length();
        }
    
    return ETrue;
    }

// EOF
