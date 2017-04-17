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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

#include "clsbase.h"

#ifdef _linux
#define _alloca alloca
#endif

class CFlatBuffer
{
public:
    static int CalcBufferSize(const CLSModule *pModule);

public:
    int Flat(CLSModule *pModule, void *pBuffer);

private:

    long WriteString(const char *s);
    long WriteData(void *pData, int nSize);

    long WriteFileDirEntry(FileDirEntry *pEntry);

    long WriteKeyValuePair(KeyValuePair *pPair);

    long WriteAnnotationDescriptor(AnnotationDescriptor *pDesc);

    long WriteClassDirEntry(ClassDirEntry *pEntry);
    long WriteClassDescriptor(ClassDescriptor *pDesc);

    long WriteInterfaceDirEntry(InterfaceDirEntry *pEntry);
    long WriteInterfaceDescriptor(InterfaceDescriptor *pDesc);
    long WriteInterfaceConstDescriptor(InterfaceConstDescriptor *pDesc);
    long WriteMethodDescriptor(MethodDescriptor *pDesc);

    long WriteArrayDirEntry(ArrayDirEntry *pEntry);

    long WriteStructDirEntry(StructDirEntry *pEntry);
    long WriteStructDescriptor(StructDescriptor *pDesc);

    long WriteEnumDirEntry(EnumDirEntry *pEntry);
    long WriteEnumDescriptor(EnumDescriptor *pDesc);

    long WriteAliasDirEntry(AliasDirEntry *pEntry);

    long WriteConstDirEntry(ConstDirEntry *pEntry);

private:
    const CLSModule *m_pModule;

    char            *m_pBuffer;
    long             m_nOffset;

    int             m_cStrings;

    static const int MAXNUM = 4096 * 20;
    char            *m_ppStrings[MAXNUM];
};

long CFlatBuffer::WriteData(void *p, int size)
{
    int nBeginAddress = m_nOffset;

    memcpy(m_pBuffer + m_nOffset, p, size);
    m_nOffset += DST_ROUNDUP(size);

    return nBeginAddress;
}

long CFlatBuffer::WriteString(const char *s)
{
    int n, nSize, nBeginAddress;

    for (n = 0; n < m_cStrings; n++) {
        if (!strcmp(s, m_ppStrings[n]))
            return (long)(m_ppStrings[n]) - (long)(m_pBuffer);
    }

    nBeginAddress = m_nOffset;
    nSize = strlen(s) + 1;
    memcpy(m_pBuffer + m_nOffset, s, nSize);

    assert(m_cStrings < MAXNUM);
    m_ppStrings[m_cStrings++] = m_pBuffer + m_nOffset;

    m_nOffset += DST_ROUNDUP(nSize);
    return nBeginAddress;
}

long CFlatBuffer::WriteClassDescriptor(ClassDescriptor *pDesc)
{
    int n;
    long *p;
    ClassDescriptor d;

    memcpy(&d, pDesc, sizeof(ClassDescriptor));

    if (d.mAnnotationCount > 0) {
        p = (long *)_alloca(d.mAnnotationCount * sizeof(long));

        for (n = 0; n < d.mAnnotationCount; n++) {
            p[n] = WriteAnnotationDescriptor(d.mAnnotations[n]);
        }

        d.mAnnotations = (AnnotationDescriptor **) \
            WriteData(p, d.mAnnotationCount * sizeof(long));
    }

    if (d.mInterfaceCount > 0) {
        p = (long *)_alloca(d.mInterfaceCount * sizeof(long));

        for (n = 0; n < d.mInterfaceCount; n++)
            p[n] = WriteData(d.mInterfaces[n], sizeof(ClassInterface));

        d.mInterfaces = (ClassInterface **) \
            WriteData(p, d.mInterfaceCount * sizeof(long));
    }

    if (d.mAggregateCount > 0) {
        d.mAggrIndexes = (USHORT *)WriteData( \
            d.mAggrIndexes, d.mAggregateCount * sizeof(USHORT));
    }
    if (d.mAspectCount > 0) {
        d.mAspectIndexes = (USHORT *)WriteData( \
            d.mAspectIndexes, d.mAspectCount * sizeof(USHORT));
    }
    if (d.mClassCount > 0) {
        d.mClassIndexes = (USHORT *)WriteData( \
            d.mClassIndexes, d.mClassCount * sizeof(USHORT));
    }

    return WriteData(&d, sizeof(ClassDescriptor));
}

long CFlatBuffer::WriteFileDirEntry(FileDirEntry *pFileDirEntry)
{
    FileDirEntry entry;

    memcpy(&entry, pFileDirEntry, sizeof(FileDirEntry));

    if (entry.mPath) {
        entry.mPath = (char *)WriteString(entry.mPath);
    }

    return WriteData(&entry, sizeof(FileDirEntry));
}

long CFlatBuffer::WriteKeyValuePair(KeyValuePair *pPair)
{
    KeyValuePair pair;

    memcpy(&pair, pPair, sizeof(KeyValuePair));

    if (pair.mKey) {
        pair.mKey = (char *)WriteString(pair.mKey);
    }
    if (pair.mValue) {
        pair.mValue = (char *)WriteString(pair.mValue);
    }

    return WriteData(&pair, sizeof(KeyValuePair));
}

long CFlatBuffer::WriteAnnotationDescriptor(AnnotationDescriptor *pDesc)
{
    int n;
    long *p;
    AnnotationDescriptor d;

    memcpy(&d, pDesc, sizeof(AnnotationDescriptor));

    d.mName = (char *)WriteString(d.mName);
    if (d.mNameSpace) {
        d.mNameSpace = (char *)WriteString(d.mNameSpace);
    }

    if (d.mKeyValuePairCount > 0) {
        p = (long *)_alloca(d.mKeyValuePairCount * sizeof(long));

        for (n = 0; n < d.mKeyValuePairCount; n++) {
            p[n] = WriteKeyValuePair(d.mKeyValuePairs[n]);
        }

        d.mKeyValuePairs = (KeyValuePair **) \
            WriteData(p, d.mKeyValuePairCount * sizeof(long));
    }

    return WriteData(&d, sizeof(AnnotationDescriptor));
}

long CFlatBuffer::WriteClassDirEntry(ClassDirEntry *pClassDirEntry)
{
    ClassDirEntry entry;

    memcpy(&entry, pClassDirEntry, sizeof(ClassDirEntry));

    entry.mDesc = (ClassDescriptor *) \
            WriteClassDescriptor(entry.mDesc);

    entry.mName = (char *)WriteString(entry.mName);
    if (entry.mNameSpace) {
        entry.mNameSpace = (char *)WriteString(entry.mNameSpace);
    }

    return WriteData(&entry, sizeof(ClassDirEntry));
}

long CFlatBuffer::WriteInterfaceConstDescriptor(InterfaceConstDescriptor *pDesc)
{
    InterfaceConstDescriptor d;

    memcpy(&d, pDesc, sizeof(InterfaceConstDescriptor));

    d.mName = (char *)WriteString(d.mName);
    if (d.mType == TYPE_STRING && d.mV.mStrValue != NULL) {
        d.mV.mStrValue = (char *)WriteString(d.mV.mStrValue);
    }

    return WriteData(&d, sizeof(InterfaceConstDescriptor));
}

long CFlatBuffer::WriteMethodDescriptor(MethodDescriptor *pDesc)
{
    int n;
    long *p;
    MethodDescriptor d;
    ParamDescriptor param;

    memcpy(&d, pDesc, sizeof(MethodDescriptor));

    if (d.mParamCount > 0) {
        p = (long *)_alloca(d.mParamCount * sizeof(long));

        for (n = 0; n < d.mParamCount; n++) {
            memcpy(&param, d.mParams[n], sizeof(ParamDescriptor));
            param.mName = (char *)WriteString(param.mName);
            if (param.mType.mNestedType) {
                param.mType.mNestedType = (TypeDescriptor *)WriteData( \
                    param.mType.mNestedType, sizeof(TypeDescriptor));
            }
            p[n] = WriteData(&param, sizeof(ParamDescriptor));
        }

        d.mParams = (ParamDescriptor **) \
            WriteData(p, d.mParamCount * sizeof(long));
    }
    d.mName = (char *)WriteString(d.mName);
    d.mSignature = (char *)WriteString(d.mSignature);

    if (d.mAnnotationCount > 0) {
        p = (long *)_alloca(d.mAnnotationCount * sizeof(long));

        for (n = 0; n < d.mAnnotationCount; n++) {
            p[n] = WriteAnnotationDescriptor(d.mAnnotations[n]);
        }

        d.mAnnotations = (AnnotationDescriptor **) \
            WriteData(p, d.mAnnotationCount * sizeof(long));
    }

    return WriteData(&d, sizeof(MethodDescriptor));
}

long CFlatBuffer::WriteInterfaceDescriptor(
    InterfaceDescriptor *pDesc)
{
    int n;
    long *p;
    InterfaceDescriptor d;

    memcpy(&d, pDesc, sizeof(InterfaceDescriptor));

    if (d.mAnnotationCount > 0) {
        p = (long *)_alloca(d.mAnnotationCount * sizeof(long));

        for (n = 0; n < d.mAnnotationCount; n++) {
            p[n] = WriteAnnotationDescriptor(d.mAnnotations[n]);
        }

        d.mAnnotations = (AnnotationDescriptor **) \
            WriteData(p, d.mAnnotationCount * sizeof(long));
    }

    if (d.mConstCount > 0) {
        p = (long *)_alloca(d.mConstCount * sizeof(long));

        for (n = 0; n < d.mConstCount; n++) {
            p[n] = WriteInterfaceConstDescriptor(d.mConsts[n]);
        }

        d.mConsts = (InterfaceConstDescriptor **) \
            WriteData(p, d.mConstCount * sizeof(long));
    }

    if (d.mMethodCount > 0) {
        p = (long *)_alloca(d.mMethodCount * sizeof(long));

        for (n = 0; n < d.mMethodCount; n++) {
            p[n] = WriteMethodDescriptor(d.mMethods[n]);
        }

        d.mMethods = (MethodDescriptor **) \
            WriteData(p, d.mMethodCount * sizeof(long));
    }

    return WriteData(&d, sizeof(InterfaceDescriptor));
}

long CFlatBuffer::WriteInterfaceDirEntry(
    InterfaceDirEntry *pInterfaceDirEntry)
{
    InterfaceDirEntry entry;

    memcpy(&entry, pInterfaceDirEntry, sizeof(InterfaceDirEntry));

    entry.mDesc = (InterfaceDescriptor *) \
            WriteInterfaceDescriptor(entry.mDesc);

    entry.mName = (char *)WriteString(entry.mName);
    if (entry.mNameSpace) {
        entry.mNameSpace = (char *)WriteString(entry.mNameSpace);
    }

    return WriteData(&entry, sizeof(InterfaceDirEntry));
}

long CFlatBuffer::WriteArrayDirEntry(ArrayDirEntry *pArrayDirEntry)
{
    ArrayDirEntry entry;

    memcpy(&entry, pArrayDirEntry, sizeof(ArrayDirEntry));

    if (entry.mNameSpace) {
        entry.mNameSpace = (char *)WriteString(entry.mNameSpace);
    }

    if (entry.mType.mNestedType) {
        entry.mType.mNestedType = (TypeDescriptor *)WriteData( \
            entry.mType.mNestedType, sizeof(TypeDescriptor));
    }

    return WriteData(&entry, sizeof(ArrayDirEntry));
}

long CFlatBuffer::WriteStructDescriptor(StructDescriptor *pDesc)
{
    int n;
    long *p;
    StructDescriptor d;
    StructElement elem;

    memcpy(&d, pDesc, sizeof(StructDescriptor));

    if (d.mElementCount > 0) {
        p = (long *)_alloca(d.mElementCount * sizeof(long));

        for (n = 0; n < d.mElementCount; n++) {
            memcpy(&elem, d.mElements[n], sizeof(StructElement));
            elem.mName = (char *)WriteString(elem.mName);
            if (elem.mType.mNestedType) {
                elem.mType.mNestedType = (TypeDescriptor *)WriteData( \
                    elem.mType.mNestedType, sizeof(TypeDescriptor));
            }
            p[n] = WriteData(&elem, sizeof(StructElement));
        }

        d.mElements = (StructElement **) \
            WriteData(p, d.mElementCount * sizeof(long));
    }

    return WriteData(&d, sizeof(StructDescriptor));
}

long CFlatBuffer::WriteStructDirEntry(StructDirEntry *pStructDirEntry)
{
    StructDirEntry entry;

    memcpy(&entry, pStructDirEntry, sizeof(StructDirEntry));

    entry.mDesc = (StructDescriptor *) \
            WriteStructDescriptor(entry.mDesc);

    entry.mName = (char *)WriteString(entry.mName);
    if (entry.mNameSpace) {
        entry.mNameSpace = (char *)WriteString(entry.mNameSpace);
    }

    return WriteData(&entry, sizeof(StructDirEntry));
}

long CFlatBuffer::WriteEnumDescriptor(EnumDescriptor *pDesc)
{
    int n;
    long *p;
    EnumDescriptor d;
    EnumElement elem;

    memcpy(&d, pDesc, sizeof(EnumDescriptor));

    if (d.mElementCount > 0) {
        p = (long *)_alloca(d.mElementCount * sizeof(long));

        for (n = 0; n < d.mElementCount; n++) {
            memcpy(&elem, d.mElements[n], sizeof(EnumElement));
            elem.mName = (char *)WriteString(elem.mName);
            p[n] = WriteData(&elem, sizeof(EnumElement));
        }

        d.mElements = (EnumElement **) \
            WriteData(p, d.mElementCount * sizeof(long));
    }

    return WriteData(&d, sizeof(EnumDescriptor));
}

long CFlatBuffer::WriteEnumDirEntry(EnumDirEntry *pEnumDirEntry)
{
    EnumDirEntry entry;

    memcpy(&entry, pEnumDirEntry, sizeof(EnumDirEntry));

    entry.mDesc = (EnumDescriptor *) \
            WriteEnumDescriptor(entry.mDesc);

    entry.mName = (char *)WriteString(entry.mName);
    if (entry.mNameSpace) {
        entry.mNameSpace = (char *)WriteString(entry.mNameSpace);
    }

    return WriteData(&entry, sizeof(EnumDirEntry));
}

long CFlatBuffer::WriteAliasDirEntry(AliasDirEntry *pAliasDirEntry)
{
    AliasDirEntry entry;

    memcpy(&entry, pAliasDirEntry, sizeof(AliasDirEntry));

    if (entry.mType.mNestedType) {
        entry.mType.mNestedType = (TypeDescriptor *)WriteData( \
            entry.mType.mNestedType, sizeof(TypeDescriptor));
    }

    entry.mName = (char *)WriteString(entry.mName);
    if (entry.mNameSpace) {
        entry.mNameSpace = (char *)WriteString(entry.mNameSpace);
    }

    return WriteData(&entry, sizeof(AliasDirEntry));
}

long CFlatBuffer::WriteConstDirEntry(ConstDirEntry *pConstDirEntry)
{
    ConstDirEntry entry;

    memcpy(&entry, pConstDirEntry, sizeof(ConstDirEntry));

    entry.mName = (char *)WriteString(entry.mName);
    if (entry.mNameSpace) {
        entry.mNameSpace = (char *)WriteString(entry.mNameSpace);
    }
    if (entry.mType == TYPE_STRING && entry.mV.mStrValue.mValue != NULL) {
        entry.mV.mStrValue.mValue = (char*)WriteString(entry.mV.mStrValue.mValue);
    }

    return WriteData(&entry, sizeof(ConstDirEntry));
}

int CFlatBuffer::Flat(CLSModule *pModule, void *pvDest)
{
    int n;
    long *p;

    m_pModule = pModule;
    m_pBuffer = (char *)pvDest;
    m_nOffset = sizeof(CLSModule);
    m_cStrings = 0;

    pModule = (CLSModule *)pvDest;
    memcpy(pModule, m_pModule, sizeof(CLSModule));

    if (pModule->mFileCount > 0) {
        p = (long *)_alloca(pModule->mFileCount * sizeof(long));

        for (n = 0; n < pModule->mFileCount; n++)
            p[n] = WriteFileDirEntry(pModule->mFileDirs[n]);

        pModule->mFileDirs = (FileDirEntry **) \
            WriteData(p, pModule->mFileCount * sizeof(long));
    }
    else {
        pModule->mFileDirs = NULL;
    }

    if (pModule->mClassCount > 0) {
        p = (long *)_alloca(pModule->mClassCount * sizeof(long));

        for (n = 0; n < pModule->mClassCount; n++)
            p[n] = WriteClassDirEntry(pModule->mClassDirs[n]);

        pModule->mClassDirs = (ClassDirEntry **) \
            WriteData(p, pModule->mClassCount * sizeof(long));
    }
    else {
        pModule->mClassDirs = NULL;
    }

    if (pModule->mInterfaceCount > 0) {
        p = (long *)_alloca(pModule->mInterfaceCount * sizeof(long));

        for (n = 0; n < pModule->mInterfaceCount; n++)
            p[n] = WriteInterfaceDirEntry(pModule->mInterfaceDirs[n]);

        pModule->mInterfaceDirs = (InterfaceDirEntry **) \
            WriteData(p, pModule->mInterfaceCount * sizeof(long));
    }
    else {
        pModule->mInterfaceDirs = NULL;
    }

    if (pModule->mDefinedInterfaceCount > 0) {
        p = (long *)_alloca(pModule->mDefinedInterfaceCount * sizeof(long));

        for (n = 0; n < pModule->mDefinedInterfaceCount; n++)
            p[n] = pModule->mDefinedInterfaceIndexes[n];

        pModule->mDefinedInterfaceIndexes = (long *) \
            WriteData(p, pModule->mDefinedInterfaceCount * sizeof(long));
    }
    else {
        pModule->mDefinedInterfaceIndexes = NULL;
    }

    if (pModule->mStructCount > 0) {
        p = (long *)_alloca(pModule->mStructCount * sizeof(long));

        for (n = 0; n < pModule->mStructCount; n++)
            p[n] = WriteStructDirEntry(pModule->mStructDirs[n]);

        pModule->mStructDirs = (StructDirEntry **) \
            WriteData(p, pModule->mStructCount * sizeof(long));
    }
    else {
        pModule->mStructDirs = NULL;
    }
    if (pModule->mArrayCount > 0) {
        p = (long *)_alloca(pModule->mArrayCount * sizeof(long));

        for (n = 0; n < pModule->mArrayCount; n++)
            p[n] = WriteArrayDirEntry(pModule->mArrayDirs[n]);

        pModule->mArrayDirs = (ArrayDirEntry **) \
            WriteData(p, pModule->mArrayCount * sizeof(long));
    }
    else {
        pModule->mArrayDirs = NULL;
    }

    if (pModule->mEnumCount > 0) {
        p = (long *)_alloca(pModule->mEnumCount * sizeof(long));

        for (n = 0; n < pModule->mEnumCount; n++)
            p[n] = WriteEnumDirEntry(pModule->mEnumDirs[n]);

        pModule->mEnumDirs = (EnumDirEntry **) \
            WriteData(p, pModule->mEnumCount * sizeof(long));
    }
    else {
        pModule->mEnumDirs = NULL;
    }

    if (pModule->mAliasCount > 0) {
        p = (long *)_alloca(pModule->mAliasCount * sizeof(long));

        for (n = 0; n < pModule->mAliasCount; n++)
            p[n] = WriteAliasDirEntry(pModule->mAliasDirs[n]);

        pModule->mAliasDirs = (AliasDirEntry **) \
            WriteData(p, pModule->mAliasCount * sizeof(long));
    }
    else {
        pModule->mAliasDirs = NULL;
    }

    if (pModule->mConstCount > 0) {
        p = (long *)_alloca(pModule->mConstCount * sizeof(long));

        for (n = 0; n < pModule->mConstCount; n++)
            p[n] = WriteConstDirEntry(pModule->mConstDirs[n]);

        pModule->mConstDirs = (ConstDirEntry **) \
            WriteData(p, pModule->mConstCount * sizeof(long));
    }
    else {
        pModule->mConstDirs = NULL;
    }

    if (pModule->mLibraryCount > 0) {
        p = (long *)_alloca(pModule->mLibraryCount * sizeof(long));

        for (n = 0; n < pModule->mLibraryCount; n++)
            p[n] = WriteString(pModule->mLibraryNames[n]);

        pModule->mLibraryNames = (char **) \
            WriteData(p, pModule->mLibraryCount * sizeof(long));
    }
    else {
        pModule->mLibraryNames = NULL;
    }

    pModule->mName = (char *)WriteString(pModule->mName);
    if (pModule->mUunm) {
        pModule->mUunm = (char *)WriteString(pModule->mUunm);
    }

    if (pModule->mServiceName) {
        pModule->mServiceName = (char *)WriteString(pModule->mServiceName);
    }

    pModule->mSize = m_nOffset;

    return m_nOffset;
}

inline int StringAlignSize(const char *s)
{
    return DST_ROUNDUP(strlen(s) + 1);
}

int CalcFileSize(FileDirEntry *p)
{
    int size = sizeof(FileDirEntry);

    if (p->mPath) size += StringAlignSize(p->mPath);

    return size;
}

int CalcKeyPairSize(KeyValuePair* p)
{
    int size = sizeof(KeyValuePair);

    if (p->mKey) size += StringAlignSize(p->mKey);
    if (p->mValue) size += StringAlignSize(p->mValue);

    return size;
}

int CalcAnnotationSize(AnnotationDescriptor *p)
{
    int n, size = sizeof(AnnotationDescriptor);

    if (p->mName) size += StringAlignSize(p->mName);
    if (p->mNameSpace) size += StringAlignSize(p->mNameSpace);

    for (n = 0; n < p->mKeyValuePairCount; n++) {
        size += CalcKeyPairSize(p->mKeyValuePairs[n]);
    }
    size += n * sizeof(KeyValuePair*);

    return size;
}

int CalcClassSize(ClassDirEntry *p)
{
    int n, size = sizeof(ClassDirEntry);

    size += StringAlignSize(p->mName);
    if (p->mNameSpace) size += StringAlignSize(p->mNameSpace);

    size += sizeof(ClassDescriptor);

    for (n = 0; n < p->mDesc->mAnnotationCount; n++) {
        size += CalcAnnotationSize(p->mDesc->mAnnotations[n]);
    }
    size += n * sizeof(AnnotationDescriptor*);

    size += p->mDesc->mInterfaceCount * \
        (sizeof(ClassInterface) + sizeof(ClassInterface *));

    size += DST_ROUNDUP(p->mDesc->mAggregateCount * sizeof(USHORT));
    size += DST_ROUNDUP(p->mDesc->mAspectCount * sizeof(USHORT));
    size += DST_ROUNDUP(p->mDesc->mClassCount * sizeof(USHORT));

    return size;
}

int CalcInterfaceConstSize(InterfaceConstDescriptor *p)
{
    int size = sizeof(InterfaceConstDescriptor);

    size += StringAlignSize(p->mName);

    if (p->mType == TYPE_STRING && p->mV.mStrValue != NULL) {
        size += StringAlignSize(p->mV.mStrValue);
    }

    return size;
}

int CalcMethodSize(MethodDescriptor *p)
{
    int n, size = sizeof(MethodDescriptor);

    size += StringAlignSize(p->mName);
    size += StringAlignSize(p->mSignature);
    for (n = 0; n < p->mAnnotationCount; n++) {
        size += CalcAnnotationSize(p->mAnnotations[n]);
    }
    size += n * sizeof(AnnotationDescriptor*);

    for (n = 0; n < p->mParamCount; n++) {
        if (p->mParams[n]->mType.mNestedType) {
            size += sizeof(TypeDescriptor);
        }
        size += StringAlignSize(p->mParams[n]->mName);
    }
    size += n * (sizeof(ParamDescriptor) + sizeof(ParamDescriptor *));

    return size;
}

int CalcInterfaceSize(InterfaceDirEntry *p)
{
    int n, size = sizeof(InterfaceDirEntry);

    size += StringAlignSize(p->mName);
    if (p->mNameSpace) size += StringAlignSize(p->mNameSpace);

    size += sizeof(InterfaceDescriptor);

    for (n = 0; n < p->mDesc->mAnnotationCount; n++) {
        size += CalcAnnotationSize(p->mDesc->mAnnotations[n]);
    }
    size += n * sizeof(AnnotationDescriptor *);

    for (n = 0; n < p->mDesc->mConstCount; n++) {
        size += CalcInterfaceConstSize(p->mDesc->mConsts[n]);
    }
    size += n * sizeof(InterfaceConstDescriptor *);

    for (n = 0; n < p->mDesc->mMethodCount; n++) {
        size += CalcMethodSize(p->mDesc->mMethods[n]);
    }
    size += n * sizeof(MethodDescriptor *);

    return size;
}

int CalcArraySize(ArrayDirEntry *p)
{
    int size = sizeof(ArrayDirEntry);

    if (p->mNameSpace)
        size += StringAlignSize(p->mNameSpace);

    if (p->mType.mNestedType)
        size += sizeof(TypeDescriptor);

    return size;
}

int CalcStructSize(StructDirEntry *p)
{
    int n, size = sizeof(StructDirEntry);

    size += StringAlignSize(p->mName);
    if (p->mNameSpace) size += StringAlignSize(p->mNameSpace);

    size += sizeof(StructDescriptor);

    for (n = 0; n < p->mDesc->mElementCount; n++) {
        if (p->mDesc->mElements[n]->mType.mNestedType) {
            size += sizeof(TypeDescriptor);
        }
        size += StringAlignSize(p->mDesc->mElements[n]->mName);
    }
    size += n * (sizeof(StructElement) + sizeof(StructElement *));

    return size;
}

int CalcEnumSize(EnumDirEntry *p)
{
    int n, size = sizeof(EnumDirEntry);

    size += StringAlignSize(p->mName);
    if (p->mNameSpace) size += StringAlignSize(p->mNameSpace);

    size += sizeof(EnumDescriptor);

    for (n = 0; n < p->mDesc->mElementCount; n++) {
        size += StringAlignSize(p->mDesc->mElements[n]->mName);
    }
    size += n * (sizeof(EnumElement) + sizeof(EnumElement *));

    return size;
}

int CalcAliasSize(AliasDirEntry *p)
{
    int size = sizeof(AliasDirEntry);

    size += StringAlignSize(p->mName);
    if (p->mNameSpace) size += StringAlignSize(p->mNameSpace);
    if (p->mType.mNestedType) size += sizeof(TypeDescriptor);

    return size;
}

int CalcConstSize(ConstDirEntry *p)
{
    int size = sizeof(ConstDirEntry);

    size += StringAlignSize(p->mName);
    if (p->mNameSpace) size += StringAlignSize(p->mNameSpace);

    if (p->mType == TYPE_STRING && p->mV.mStrValue.mValue != NULL) {
        size += StringAlignSize(p->mV.mStrValue.mValue);
    }

    return size;
}

int CFlatBuffer::CalcBufferSize(const CLSModule *pModule)
{
    int n, size = sizeof(CLSModule);

    for (n = 0; n < pModule->mFileCount; n++) {
        size += CalcFileSize(pModule->mFileDirs[n]);
    }
    size += n * sizeof(FileDirEntry *);

    for (n = 0; n < pModule->mClassCount; n++) {
        size += CalcClassSize(pModule->mClassDirs[n]);
    }
    size += n * sizeof(ClassDirEntry *);

    for (n = 0; n < pModule->mInterfaceCount; n++) {
        size += CalcInterfaceSize(pModule->mInterfaceDirs[n]);
    }
    size += n * sizeof(InterfaceDirEntry *);

    if (pModule->mDefinedInterfaceCount) {
        size += pModule->mDefinedInterfaceCount * sizeof(long);
    }

    for (n = 0; n < pModule->mArrayCount; n++) {
        size += CalcArraySize(pModule->mArrayDirs[n]);
    }
    size += n * sizeof(ArrayDirEntry *);

    for (n = 0; n < pModule->mStructCount; n++) {
        size += CalcStructSize(pModule->mStructDirs[n]);
    }
    size += n * sizeof(StructDirEntry *);

    for (n = 0; n < pModule->mEnumCount; n++) {
        size += CalcEnumSize(pModule->mEnumDirs[n]);
    }
    size += n * sizeof(EnumDirEntry *);

    for (n = 0; n < pModule->mAliasCount; n++) {
        size += CalcAliasSize(pModule->mAliasDirs[n]);
    }
    size += n * sizeof(AliasDirEntry *);

    for (n = 0; n < pModule->mConstCount; n++) {
        size += CalcConstSize(pModule->mConstDirs[n]);
    }
    size += n * sizeof(ConstDirEntry *);

    for (n = 0; n < pModule->mLibraryCount; n++) {
        size += StringAlignSize(pModule->mLibraryNames[n]);
    }
    size += n * sizeof(char *);

    size += StringAlignSize(pModule->mName);
    if (pModule->mUunm) size += StringAlignSize(pModule->mUunm);
    if (pModule->mServiceName) {
        size += StringAlignSize(pModule->mServiceName);
    }

    return size + 4;
}

int FlatCLS(const CLSModule *pSrc, void **ppDest)
{
    int nSize;
    CLSModule *pDest;
    CFlatBuffer buffer;

    nSize = CFlatBuffer::CalcBufferSize(pSrc);
    pDest = (CLSModule *)new char[nSize];
    if (!pDest) _ReturnError (CLSError_OutOfMemory);

    nSize = buffer.Flat((CLSModule *)pSrc, pDest);
    pDest->mAttribs |= CARAttrib_inheap;

    if (pDest->mAttribs & CARAttrib_compress) {
        nSize = CompressCLS(pDest);
        if (nSize < 0) {
            delete [] (char *)pDest;
            _Return (nSize);
        }
    }

    *ppDest = (void *)pDest;
    _ReturnOK (nSize);
}
