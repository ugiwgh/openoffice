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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef _DBACORE_RESULTCOLUMN_HXX_
#include "resultcolumn.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include <cppuhelper/exc_hlp.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;
using namespace dbaccess;

DBG_NAME(OResultColumn)
//--------------------------------------------------------------------------
OResultColumn::OResultColumn( const Reference < XResultSetMetaData >& _xMetaData, sal_Int32 _nPos,
        const Reference< XDatabaseMetaData >& _rxDBMeta )
    :OColumn( true )
    ,m_xMetaData( _xMetaData )
    ,m_xDBMetaData( _rxDBMeta )
    ,m_nPos( _nPos )
{
	DBG_CTOR(OResultColumn,NULL);
}
// -----------------------------------------------------------------------------
void OResultColumn::impl_determineIsRowVersion_nothrow()
{
    if ( m_aIsRowVersion.hasValue() )
        return;
    m_aIsRowVersion <<= (sal_Bool)(sal_False);

    OSL_ENSURE( m_xDBMetaData.is(), "OResultColumn::impl_determineIsRowVersion_nothrow: no DBMetaData!" );
    if ( !m_xDBMetaData.is() )
        return;
    
    try
    {
        ::rtl::OUString sCatalog, sSchema, sTable, sColumnName;
        getPropertyValue( PROPERTY_CATALOGNAME ) >>= sCatalog;
        getPropertyValue( PROPERTY_SCHEMANAME ) >>= sSchema;
        getPropertyValue( PROPERTY_TABLENAME ) >>= sTable;
        getPropertyValue( PROPERTY_NAME ) >>= sColumnName;

        try
        {
            Reference< XResultSet > xVersionColumns = m_xDBMetaData->getVersionColumns(
                makeAny( sCatalog ), sSchema, sTable );
            if ( xVersionColumns.is() ) // allowed to be NULL
            {
                Reference< XRow > xResultRow( xVersionColumns, UNO_QUERY_THROW );
                while ( xVersionColumns->next() )
                {
                    if ( xResultRow->getString( 2 ) == sColumnName )
                    {
                        m_aIsRowVersion <<= (sal_Bool)(sal_True);
                        break;
                    }
                }
            }
        }
        catch(const SQLException&)
        {
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}
// -----------------------------------------------------------------------------
OResultColumn::~OResultColumn()
{
	DBG_DTOR(OResultColumn,NULL);
}
// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OResultColumn::getImplementationId() throw (RuntimeException)
{
	static OImplementationId * pId = 0;
	if (! pId)
	{
		MutexGuard aGuard( Mutex::getGlobalMutex() );
		if (! pId)
		{
			static OImplementationId aId;
			pId = &aId;
		}
	}
	return pId->getImplementationId();
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OResultColumn::getImplementationName(  ) throw(RuntimeException)
{
	return rtl::OUString::createFromAscii("com.sun.star.sdb.OResultColumn");
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OResultColumn::getSupportedServiceNames(  ) throw (RuntimeException)
{
	Sequence< ::rtl::OUString > aSNS( 2 );
	aSNS[0] = SERVICE_SDBCX_COLUMN;
	aSNS[1] = SERVICE_SDB_RESULTCOLUMN;
	return aSNS;
}

// OComponentHelper
//------------------------------------------------------------------------------
void OResultColumn::disposing()
{
	OColumn::disposing();

	MutexGuard aGuard(m_aMutex);
	m_xMetaData = NULL;
}

// comphelper::OPropertyArrayUsageHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OResultColumn::createArrayHelper( ) const
{
	BEGIN_PROPERTY_HELPER(21)
		DECL_PROP1(CATALOGNAME,				::rtl::OUString,	READONLY);
		DECL_PROP1(DISPLAYSIZE,				sal_Int32,			READONLY);
		DECL_PROP1_BOOL(ISAUTOINCREMENT,						READONLY);
		DECL_PROP1_BOOL(ISCASESENSITIVE,						READONLY);
		DECL_PROP1_BOOL(ISCURRENCY,								READONLY);
		DECL_PROP1_BOOL(ISDEFINITELYWRITABLE,					READONLY);
		DECL_PROP1(ISNULLABLE,				sal_Int32,			READONLY);
		DECL_PROP1_BOOL(ISREADONLY,								READONLY);
		DECL_PROP1_BOOL(ISROWVERSION,                           READONLY);
		DECL_PROP1_BOOL(ISSEARCHABLE,							READONLY);
		DECL_PROP1_BOOL(ISSIGNED,								READONLY);
		DECL_PROP1_BOOL(ISWRITABLE,								READONLY);
		DECL_PROP1(LABEL,					::rtl::OUString,	READONLY);
		DECL_PROP1(NAME,					::rtl::OUString,	READONLY);
		DECL_PROP1(PRECISION,				sal_Int32,			READONLY);
		DECL_PROP1(SCALE,					sal_Int32,			READONLY);
		DECL_PROP1(SCHEMANAME,				::rtl::OUString,	READONLY);
		DECL_PROP1(SERVICENAME,				::rtl::OUString,	READONLY);
		DECL_PROP1(TABLENAME,				::rtl::OUString,	READONLY);
		DECL_PROP1(TYPE,					sal_Int32,			READONLY);
		DECL_PROP1(TYPENAME,				::rtl::OUString,	READONLY);
	END_PROPERTY_HELPER();
}

// cppu::OPropertySetHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OResultColumn::getInfoHelper()
{
	return *static_cast< ::comphelper::OPropertyArrayUsageHelper< OResultColumn >* >(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
namespace
{
    template< typename TYPE >
    void obtain( Any& _out_rValue, ::boost::optional< TYPE > _rCache, const sal_Int32 _nPos, const Reference < XResultSetMetaData >& _rxResultMeta, TYPE (SAL_CALL XResultSetMetaData::*Getter)( sal_Int32 ) )
    {
        if ( !_rCache )
            _rCache.reset( (_rxResultMeta.get()->*Getter)( _nPos ) );
        _out_rValue <<= *_rCache;
    }
}

//------------------------------------------------------------------------------
void OResultColumn::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
	try
	{
        if ( OColumn::isRegisteredProperty( nHandle ) )
        {
            OColumn::getFastPropertyValue( rValue, nHandle );
        }
        else
        {
		    switch (nHandle)
		    {
                case PROPERTY_ID_ISROWVERSION:
                    const_cast< OResultColumn* >( this )->impl_determineIsRowVersion_nothrow();
                    rValue = m_aIsRowVersion;
                    break;
			    case PROPERTY_ID_TABLENAME:
				    rValue <<= m_xMetaData->getTableName(m_nPos);
				    break;
			    case PROPERTY_ID_SCHEMANAME:
				    rValue <<= m_xMetaData->getSchemaName(m_nPos);
				    break;
			    case PROPERTY_ID_CATALOGNAME:
				    rValue <<= m_xMetaData->getCatalogName(m_nPos);
				    break;
			    case PROPERTY_ID_ISSIGNED:
                    obtain( rValue, m_isSigned, m_nPos, m_xMetaData, &XResultSetMetaData::isSigned );
			        break;
			    case PROPERTY_ID_ISCURRENCY:
                    obtain( rValue, m_isCurrency, m_nPos, m_xMetaData, &XResultSetMetaData::isCurrency );
			        break;
			    case PROPERTY_ID_ISSEARCHABLE:
                    obtain( rValue, m_bSearchable, m_nPos, m_xMetaData, &XResultSetMetaData::isSearchable );
			        break;
			    case PROPERTY_ID_ISCASESENSITIVE:
                    obtain( rValue, m_isCaseSensitive, m_nPos, m_xMetaData, &XResultSetMetaData::isCaseSensitive );
			        break;
			    case PROPERTY_ID_ISREADONLY:
                    obtain( rValue, m_isReadOnly, m_nPos, m_xMetaData, &XResultSetMetaData::isReadOnly );
			        break;
			    case PROPERTY_ID_ISWRITABLE:
                    obtain( rValue, m_isWritable, m_nPos, m_xMetaData, &XResultSetMetaData::isWritable );
			        break;
			    case PROPERTY_ID_ISDEFINITELYWRITABLE:
                    obtain( rValue, m_isDefinitelyWritable, m_nPos, m_xMetaData, &XResultSetMetaData::isDefinitelyWritable );
			        break;
			    case PROPERTY_ID_ISAUTOINCREMENT:
                    obtain( rValue, m_isAutoIncrement, m_nPos, m_xMetaData, &XResultSetMetaData::isAutoIncrement );
			        break;
			    case PROPERTY_ID_SERVICENAME:
				    rValue <<= m_xMetaData->getColumnServiceName(m_nPos);
				    break;
			    case PROPERTY_ID_LABEL:
                    obtain( rValue, m_sColumnLabel, m_nPos, m_xMetaData, &XResultSetMetaData::getColumnLabel );
				    break;
			    case PROPERTY_ID_DISPLAYSIZE:
                    obtain( rValue, m_nColumnDisplaySize, m_nPos, m_xMetaData, &XResultSetMetaData::getColumnDisplaySize );
				    break;
			    case PROPERTY_ID_TYPE:
                    obtain( rValue, m_nColumnType, m_nPos, m_xMetaData, &XResultSetMetaData::getColumnType );
				    break;
			    case PROPERTY_ID_PRECISION:
                    obtain( rValue, m_nPrecision, m_nPos, m_xMetaData, &XResultSetMetaData::getPrecision );
				    break;
			    case PROPERTY_ID_SCALE:
                    obtain( rValue, m_nScale, m_nPos, m_xMetaData, &XResultSetMetaData::getScale );
				    break;
			    case PROPERTY_ID_ISNULLABLE:
                    obtain( rValue, m_isNullable, m_nPos, m_xMetaData, &XResultSetMetaData::isNullable );
				    break;
			    case PROPERTY_ID_TYPENAME:
				    rValue <<= m_xMetaData->getColumnTypeName(m_nPos);
				    break;
                default:
                    OSL_ENSURE( false, "OResultColumn::getFastPropertyValue: unknown property handle!" );
                    break;
            }
		}
	}
	catch (SQLException& )
	{
		// default handling if we caught an exception
		switch (nHandle)
		{
			case PROPERTY_ID_LABEL:
			case PROPERTY_ID_TYPENAME:
			case PROPERTY_ID_SERVICENAME:
			case PROPERTY_ID_TABLENAME:
			case PROPERTY_ID_SCHEMANAME:
			case PROPERTY_ID_CATALOGNAME:
				// empty string'S
				rValue <<= rtl::OUString();
				break;
            case PROPERTY_ID_ISROWVERSION:
			case PROPERTY_ID_ISAUTOINCREMENT:
			case PROPERTY_ID_ISWRITABLE:
			case PROPERTY_ID_ISDEFINITELYWRITABLE:
			case PROPERTY_ID_ISCASESENSITIVE:
			case PROPERTY_ID_ISSEARCHABLE:
			case PROPERTY_ID_ISCURRENCY:
			case PROPERTY_ID_ISSIGNED:
			{
				sal_Bool bVal = sal_False;
				rValue.setValue(&bVal, getBooleanCppuType());
			}	break;
			case PROPERTY_ID_ISREADONLY:
			{
				sal_Bool bVal = sal_True;
				rValue.setValue(&bVal, getBooleanCppuType());
			}	break;
			case PROPERTY_ID_SCALE:
			case PROPERTY_ID_PRECISION:
			case PROPERTY_ID_DISPLAYSIZE:
				rValue <<= sal_Int32(0);
				break;
			case PROPERTY_ID_TYPE:
				rValue <<= sal_Int32(DataType::SQLNULL);
				break;
			case PROPERTY_ID_ISNULLABLE:
				rValue <<= ColumnValue::NULLABLE_UNKNOWN;
				break;
		}
	}
}

