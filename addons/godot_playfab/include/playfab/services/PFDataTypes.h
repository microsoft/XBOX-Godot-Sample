// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFPal.h>
#include <playfab/core/PFTypes.h>
#include <playfab/services/PFTypes.h>

extern "C"
{

#pragma push_macro("IN")
#undef IN

/// <summary>
/// PFDataAbortFileUploadsRequest data model. Aborts the pending upload of the requested files.
/// </summary>
typedef struct PFDataAbortFileUploadsRequest
{
    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// The entity to perform this action on.
    /// </summary>
    PFEntityKey const* entity;

    /// <summary>
    /// Names of the files to have their pending uploads aborted.
    /// </summary>
    _Field_size_(fileNamesCount) const char* const* fileNames;

    /// <summary>
    /// Count of fileNames
    /// </summary>
    uint32_t fileNamesCount;

    /// <summary>
    /// (Optional) The expected version of the profile, if set and doesn't match the current version
    /// of the profile the operation will not be performed.
    /// </summary>
    _Maybenull_ int32_t const* profileVersion;

} PFDataAbortFileUploadsRequest;

/// <summary>
/// PFDataAbortFileUploadsResponse data model.
/// </summary>
typedef struct PFDataAbortFileUploadsResponse
{
    /// <summary>
    /// (Optional) The entity id and type.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// The current version of the profile, can be used for concurrency control during updates.
    /// </summary>
    int32_t profileVersion;

} PFDataAbortFileUploadsResponse;

/// <summary>
/// PFDataDeleteFilesRequest data model. Deletes the requested files from the entity's profile.
/// </summary>
typedef struct PFDataDeleteFilesRequest
{
    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// The entity to perform this action on.
    /// </summary>
    PFEntityKey const* entity;

    /// <summary>
    /// Names of the files to be deleted.
    /// </summary>
    _Field_size_(fileNamesCount) const char* const* fileNames;

    /// <summary>
    /// Count of fileNames
    /// </summary>
    uint32_t fileNamesCount;

    /// <summary>
    /// (Optional) The expected version of the profile, if set and doesn't match the current version
    /// of the profile the operation will not be performed.
    /// </summary>
    _Maybenull_ int32_t const* profileVersion;

} PFDataDeleteFilesRequest;

/// <summary>
/// PFDataDeleteFilesResponse data model.
/// </summary>
typedef struct PFDataDeleteFilesResponse
{
    /// <summary>
    /// (Optional) The entity id and type.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// The current version of the profile, can be used for concurrency control during updates.
    /// </summary>
    int32_t profileVersion;

} PFDataDeleteFilesResponse;

/// <summary>
/// PFDataFinalizeFileUploadsRequest data model. Finalizes the upload of the requested files. Verifies
/// that the files have been successfully uploaded and moves the file pointers from pending to live.
/// </summary>
typedef struct PFDataFinalizeFileUploadsRequest
{
    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// The entity to perform this action on.
    /// </summary>
    PFEntityKey const* entity;

    /// <summary>
    /// Names of the files to be finalized. Restricted to a-Z, 0-9, '(', ')', '_', '-' and '.'.
    /// </summary>
    _Field_size_(fileNamesCount) const char* const* fileNames;

    /// <summary>
    /// Count of fileNames
    /// </summary>
    uint32_t fileNamesCount;

    /// <summary>
    /// The current version of the profile, can be used for concurrency control during updates.
    /// </summary>
    int32_t profileVersion;

} PFDataFinalizeFileUploadsRequest;

/// <summary>
/// PFDataGetFileMetadata data model.
/// </summary>
typedef struct PFDataGetFileMetadata
{
    /// <summary>
    /// (Optional) Checksum value for the file, can be used to check if the file on the server has changed.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* checksum;

    /// <summary>
    /// (Optional) Download URL where the file can be retrieved.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* downloadUrl;

    /// <summary>
    /// (Optional) Name of the file.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* fileName;

    /// <summary>
    /// Last UTC time the file was modified.
    /// </summary>
    time_t lastModified;

    /// <summary>
    /// Storage service's reported byte count.
    /// </summary>
    int32_t size;

} PFDataGetFileMetadata;

/// <summary>
/// PFDataFinalizeFileUploadsResponse data model.
/// </summary>
typedef struct PFDataFinalizeFileUploadsResponse
{
    /// <summary>
    /// (Optional) The entity id and type.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) Collection of metadata for the entity's files.
    /// </summary>
    _Maybenull_ _Field_size_(metadataCount) struct PFDataGetFileMetadataDictionaryEntry const* metadata;

    /// <summary>
    /// Count of metadata
    /// </summary>
    uint32_t metadataCount;

    /// <summary>
    /// The current version of the profile, can be used for concurrency control during updates.
    /// </summary>
    int32_t profileVersion;

} PFDataFinalizeFileUploadsResponse;

/// <summary>
/// PFDataGetFilesRequest data model. Returns URLs that may be used to download the files for a profile
/// for a limited length of time. Only returns files that have been successfully uploaded, files that
/// are still pending will either return the old value, if it exists, or nothing.
/// </summary>
typedef struct PFDataGetFilesRequest
{
    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// The entity to perform this action on.
    /// </summary>
    PFEntityKey const* entity;

} PFDataGetFilesRequest;

/// <summary>
/// PFDataGetFilesResponse data model.
/// </summary>
typedef struct PFDataGetFilesResponse
{
    /// <summary>
    /// (Optional) The entity id and type.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) Collection of metadata for the entity's files.
    /// </summary>
    _Maybenull_ _Field_size_(metadataCount) struct PFDataGetFileMetadataDictionaryEntry const* metadata;

    /// <summary>
    /// Count of metadata
    /// </summary>
    uint32_t metadataCount;

    /// <summary>
    /// The current version of the profile, can be used for concurrency control during updates.
    /// </summary>
    int32_t profileVersion;

} PFDataGetFilesResponse;

/// <summary>
/// PFDataGetObjectsRequest data model. Gets JSON objects from an entity profile and returns it. .
/// </summary>
typedef struct PFDataGetObjectsRequest
{
    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// The entity to perform this action on.
    /// </summary>
    PFEntityKey const* entity;

    /// <summary>
    /// (Optional) Determines whether the object will be returned as an escaped JSON string or as a un-escaped
    /// JSON object. Default is JSON object.
    /// </summary>
    _Maybenull_ bool const* escapeObject;

} PFDataGetObjectsRequest;

/// <summary>
/// PFDataObjectResult data model.
/// </summary>
typedef struct PFDataObjectResult
{
    /// <summary>
    /// (Optional) Un-escaped JSON object, if EscapeObject false or default.
    /// </summary>
    PFJsonObject dataObject;

    /// <summary>
    /// (Optional) Escaped string JSON body of the object, if EscapeObject is true.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* escapedDataObject;

    /// <summary>
    /// (Optional) Name of the object. Restricted to a-Z, 0-9, '(', ')', '_', '-' and '.'.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* objectName;

} PFDataObjectResult;

/// <summary>
/// PFDataGetObjectsResponse data model.
/// </summary>
typedef struct PFDataGetObjectsResponse
{
    /// <summary>
    /// (Optional) The entity id and type.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) Requested objects that the calling entity has access to.
    /// </summary>
    _Maybenull_ _Field_size_(objectsCount) struct PFDataObjectResultDictionaryEntry const* objects;

    /// <summary>
    /// Count of objects
    /// </summary>
    uint32_t objectsCount;

    /// <summary>
    /// The current version of the profile, can be used for concurrency control during updates.
    /// </summary>
    int32_t profileVersion;

} PFDataGetObjectsResponse;

/// <summary>
/// PFDataInitiateFileUploadsRequest data model. Returns URLs that may be used to upload the files for
/// a profile 5 minutes. After using the upload calls FinalizeFileUploads must be called to move the file
/// status from pending to live.
/// </summary>
typedef struct PFDataInitiateFileUploadsRequest
{
    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// The entity to perform this action on.
    /// </summary>
    PFEntityKey const* entity;

    /// <summary>
    /// Names of the files to be set. Restricted to a-Z, 0-9, '(', ')', '_', '-' and '.'.
    /// </summary>
    _Field_size_(fileNamesCount) const char* const* fileNames;

    /// <summary>
    /// Count of fileNames
    /// </summary>
    uint32_t fileNamesCount;

    /// <summary>
    /// (Optional) The expected version of the profile, if set and doesn't match the current version
    /// of the profile the operation will not be performed.
    /// </summary>
    _Maybenull_ int32_t const* profileVersion;

} PFDataInitiateFileUploadsRequest;

/// <summary>
/// PFDataInitiateFileUploadMetadata data model.
/// </summary>
typedef struct PFDataInitiateFileUploadMetadata
{
    /// <summary>
    /// (Optional) Name of the file.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* fileName;

    /// <summary>
    /// (Optional) Location the data should be sent to via an HTTP PUT operation.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* uploadUrl;

} PFDataInitiateFileUploadMetadata;

/// <summary>
/// PFDataInitiateFileUploadsResponse data model.
/// </summary>
typedef struct PFDataInitiateFileUploadsResponse
{
    /// <summary>
    /// (Optional) The entity id and type.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// The current version of the profile, can be used for concurrency control during updates.
    /// </summary>
    int32_t profileVersion;

    /// <summary>
    /// (Optional) Collection of file names and upload urls.
    /// </summary>
    _Maybenull_ _Field_size_(uploadDetailsCount) PFDataInitiateFileUploadMetadata const* const* uploadDetails;

    /// <summary>
    /// Count of uploadDetails
    /// </summary>
    uint32_t uploadDetailsCount;

} PFDataInitiateFileUploadsResponse;

/// <summary>
/// PFDataSetObject data model.
/// </summary>
typedef struct PFDataSetObject
{
    /// <summary>
    /// (Optional) Body of the object to be saved. If empty and DeleteObject is true object will be deleted
    /// if it exists, or no operation will occur if it does not exist. Only one of Object or EscapedDataObject
    /// fields may be used.
    /// </summary>
    PFJsonObject dataObject;

    /// <summary>
    /// (Optional) Flag to indicate that this object should be deleted. Both DataObject and EscapedDataObject
    /// must not be set as well.
    /// </summary>
    _Maybenull_ bool const* deleteObject;

    /// <summary>
    /// (Optional) Body of the object to be saved as an escaped JSON string. If empty and DeleteObject
    /// is true object will be deleted if it exists, or no operation will occur if it does not exist.
    /// Only one of DataObject or EscapedDataObject fields may be used.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* escapedDataObject;

    /// <summary>
    /// Name of object. Restricted to a-Z, 0-9, '(', ')', '_', '-' and '.'.
    /// </summary>
    _Null_terminated_ const char* objectName;

} PFDataSetObject;

/// <summary>
/// PFDataSetObjectsRequest data model. Sets JSON objects on the requested entity profile. May include
/// a version number to be used to perform optimistic concurrency operations during update. If the current
/// version differs from the version in the request the request will be ignored. If no version is set
/// on the request then the value will always be updated if the values differ. Using the version value
/// does not guarantee a write though, ConcurrentEditError may still occur if multiple clients are attempting
/// to update the same profile. .
/// </summary>
typedef struct PFDataSetObjectsRequest
{
    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// The entity to perform this action on.
    /// </summary>
    PFEntityKey const* entity;

    /// <summary>
    /// (Optional) Optional field used for concurrency control. By specifying the previously returned
    /// value of ProfileVersion from GetProfile API, you can ensure that the object set will only be performed
    /// if the profile has not been updated by any other clients since the version you last loaded.
    /// </summary>
    _Maybenull_ int32_t const* expectedProfileVersion;

    /// <summary>
    /// Collection of objects to set on the profile.
    /// </summary>
    _Field_size_(objectsCount) PFDataSetObject const* const* objects;

    /// <summary>
    /// Count of objects
    /// </summary>
    uint32_t objectsCount;

} PFDataSetObjectsRequest;

/// <summary>
/// PFDataSetObjectInfo data model.
/// </summary>
typedef struct PFDataSetObjectInfo
{
    /// <summary>
    /// (Optional) Name of the object.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* objectName;

    /// <summary>
    /// (Optional) Optional reason to explain why the operation was the result that it was.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* operationReason;

    /// <summary>
    /// (Optional) Indicates which operation was completed, either Created, Updated, Deleted or None.
    /// </summary>
    _Maybenull_ PFOperationTypes const* setResult;

} PFDataSetObjectInfo;

/// <summary>
/// PFDataSetObjectsResponse data model.
/// </summary>
typedef struct PFDataSetObjectsResponse
{
    /// <summary>
    /// New version of the entity profile.
    /// </summary>
    int32_t profileVersion;

    /// <summary>
    /// (Optional) New version of the entity profile.
    /// </summary>
    _Maybenull_ _Field_size_(setResultsCount) PFDataSetObjectInfo const* const* setResults;

    /// <summary>
    /// Count of setResults
    /// </summary>
    uint32_t setResultsCount;

} PFDataSetObjectsResponse;

/// <summary>
/// Dictionary entry for an associative array with PFDataGetFileMetadata values.
/// </summary>
typedef struct PFDataGetFileMetadataDictionaryEntry
{
    _Null_terminated_ const char* key;
    PFDataGetFileMetadata const* value;
} PFDataGetFileMetadataDictionaryEntry;

/// <summary>
/// Dictionary entry for an associative array with PFDataObjectResult values.
/// </summary>
typedef struct PFDataObjectResultDictionaryEntry
{
    _Null_terminated_ const char* key;
    PFDataObjectResult const* value;
} PFDataObjectResultDictionaryEntry;

#pragma pop_macro("IN")

}
