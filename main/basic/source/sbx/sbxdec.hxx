/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/




#ifndef __SBX_SBX_DEC_HXX
#define __SBX_SBX_DEC_HXX

#ifdef WIN32

#undef WB_LEFT
#undef WB_RIGHT
#include <tools/prewin.h>
}	// close extern "C" {

#ifndef __MINGW32__
#include <comutil.h>
#endif
#include <oleauto.h>

extern "C" {	// reopen extern "C" {
#include <tools/postwin.h>

#endif
#endif
#include <basic/sbx.hxx>

#include <com/sun/star/bridge/oleautomation/Decimal.hpp>


// Decimal support
// Implementation only for windows

class SbxDecimal
{
	friend void releaseDecimalPtr( SbxDecimal*& rpDecimal );

#ifdef WIN32
	DECIMAL		maDec;
#endif
	sal_Int32		mnRefCount;

public:
	SbxDecimal( void );
	SbxDecimal( const SbxDecimal& rDec );
	SbxDecimal( const com::sun::star::bridge::oleautomation::Decimal& rAutomationDec );

	~SbxDecimal();

	void addRef( void )
		{ mnRefCount++; }

	void fillAutomationDecimal( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec );

	void setChar( sal_Unicode val );
	void setByte( sal_uInt8 val );
	void setShort( sal_Int16 val );
	void setLong( sal_Int32 val );
	void setUShort( sal_uInt16 val );
	void setULong( sal_uInt32 val );
	bool setSingle( float val );
	bool setDouble( double val );
	void setInt( int val );
	void setUInt( unsigned int val );
	bool setString( ::rtl::OUString* pOUString );
	void setDecimal( SbxDecimal* pDecimal )
	{
#ifdef WIN32
		if( pDecimal )
			maDec = pDecimal->maDec;
#else
        (void)pDecimal;
#endif
	}

	bool getChar( sal_Unicode& rVal );
	bool getByte( sal_uInt8& rVal );
	bool getShort( sal_Int16& rVal );
	bool getLong( sal_Int32& rVal );
	bool getUShort( sal_uInt16& rVal );
	bool getULong( sal_uInt32& rVal );
	bool getSingle( float& rVal );
	bool getDouble( double& rVal );
	bool getInt( int& rVal );
	bool getUInt( unsigned int& rVal );
	bool getString( ::rtl::OUString& rString );

	bool operator -= ( const SbxDecimal &r );
	bool operator += ( const SbxDecimal &r );
	bool operator /= ( const SbxDecimal &r );
	bool operator *= ( const SbxDecimal &r );
	bool neg( void );

	bool isZero( void );

	enum CmpResult { LT, EQ, GT };
	friend CmpResult compare( const SbxDecimal &rLeft, const SbxDecimal &rRight );
};

