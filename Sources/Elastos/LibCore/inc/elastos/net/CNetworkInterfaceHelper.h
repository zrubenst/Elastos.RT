//=========================================================================
// Copyright (C) 2012 The Elastos Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#ifndef __ELASTOS_NET_CNETWORKINTERFACEHELPER_H__
#define __ELASTOS_NET_CNETWORKINTERFACEHELPER_H__

#include "_Elastos_Net_CNetworkInterfaceHelper.h"
#include "Singleton.h"
#include "NetworkInterface.h"

using Elastos::Core::Singleton;
using Elastos::Utility::IEnumeration;

namespace Elastos {
namespace Net {

CarClass(CNetworkInterfaceHelper)
    , public Singleton
    , public INetworkInterfaceHelper
{
public:
    CAR_INTERFACE_DECL()

    CAR_SINGLETON_DECL()

    CARAPI GetByName(
        /* [in] */ const String& interfaceName,
        /* [out] */ INetworkInterface** networkInterface);

    CARAPI GetByInetAddress(
        /* [in] */ IInetAddress* address,
        /* [out] */ INetworkInterface** networkInterface);

    CARAPI GetByIndex(
        /* [in] */ Int32 index,
        /* [out] */ INetworkInterface** networkInterface);

    CARAPI GetNetworkInterfaces(
        /* [out] */ IEnumeration** networkInterfaceList);
};

} // namespace Net
} // namespace Elastos

#endif //__ELASTOS_NET_CNETWORKINTERFACEHELPER_H__