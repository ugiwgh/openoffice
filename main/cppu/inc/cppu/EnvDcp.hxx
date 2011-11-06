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



#ifndef INCLUDED_cppu_EnvDcp_hxx
#define INCLUDED_cppu_EnvDcp_hxx

#include "rtl/ustring.hxx"
#include "uno/EnvDcp.h"


namespace cppu 
{
namespace EnvDcp
{
/** Get the OBI type part of an environment descriptor.
    (http://wiki.services.openoffice.org/wiki/Uno/Cpp/Spec/Environment_Descriptor)
    
	@param  rEnvDcp        the Environment Descriptor
	@return                the OBI type
    @since UDK 3.2.7
*/
inline rtl::OUString getTypeName(rtl::OUString const & rEnvDcp) 
{
	rtl::OUString typeName;

	uno_EnvDcp_getTypeName(rEnvDcp.pData, &typeName.pData);

	return typeName;
}

/** Get the purpose part of an environment descriptor.
    (http://wiki.services.openoffice.org/wiki/Uno/Binary/Spec/Environment_Descriptor)
    
	@param  rEnvDcp        the Environment Descriptor
	@return                the purpose
    @since UDK 3.2.7
*/
inline rtl::OUString getPurpose(rtl::OUString const & rEnvDcp)
{
	rtl::OUString purpose;

	uno_EnvDcp_getPurpose(rEnvDcp.pData, &purpose.pData);

	return purpose;
}

}
}


#endif
