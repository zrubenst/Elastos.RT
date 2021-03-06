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

#ifndef __ELASTOS_SECURITY_SPEC_CX509ENCODEDKEYSPEC_H__
#define __ELASTOS_SECURITY_SPEC_CX509ENCODEDKEYSPEC_H__

#include "_Elastos_Security_Spec_CX509EncodedKeySpec.h"
#include "EncodedKeySpec.h"

namespace Elastos {
namespace Security {
namespace Spec {

CarClass(CX509EncodedKeySpec)
    , public EncodedKeySpec
    , public IX509EncodedKeySpec
{
public:
    CAR_OBJECT_DECL()

    CAR_INTERFACE_DECL()

    CARAPI GetEncoded(
        /* [out, callee] */ ArrayOf<Byte> **encoded);

    CARAPI GetFormat(
        /* [out] */ String *format);

    CARAPI constructor(
        /* [in] */ ArrayOf<Byte> *encodedKey);

private:
    // TODO: Add your private member variables here.
};

}
}
}

#endif // __ELASTOS_SECURITY_SPEC_CX509ENCODEDKEYSPEC_H__
