// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFDataTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

/// <summary>
/// Abort pending file uploads to an entity's profile.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Aborts the pending upload of the requested files. See also FileDeleteFilesAsync, FileFinalizeFileUploadsAsync,
/// FileGetFilesAsync, FileInitiateFileUploadsAsync.
///
/// When the asynchronous task is complete, call <see cref="PFDataAbortFileUploadsGetResultSize"/> and
/// <see cref="PFDataAbortFileUploadsGetResult"/> to get the result.
/// </remarks>
PF_API PFDataAbortFileUploadsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFDataAbortFileUploadsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a AbortFileUploads call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_FILE_NOT_FOUND,
/// E_PF_NO_ENTITY_FILE_OPERATION_PENDING or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFDataAbortFileUploadsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFDataAbortFileUploadsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_FILE_NOT_FOUND,
/// E_PF_NO_ENTITY_FILE_OPERATION_PENDING or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFDataAbortFileUploadsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFDataAbortFileUploadsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Delete files on an entity's profile.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Deletes the requested files from the entity's profile. See also FileAbortFileUploadsAsync, FileFinalizeFileUploadsAsync,
/// FileGetFilesAsync, FileInitiateFileUploadsAsync.
///
/// When the asynchronous task is complete, call <see cref="PFDataDeleteFilesGetResultSize"/> and <see
/// cref="PFDataDeleteFilesGetResult"/> to get the result.
/// </remarks>
PF_API PFDataDeleteFilesAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFDataDeleteFilesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a DeleteFiles call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENTITY_FILE_OPERATION_PENDING,
/// E_PF_FILE_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
PF_API PFDataDeleteFilesGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFDataDeleteFilesAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENTITY_FILE_OPERATION_PENDING,
/// E_PF_FILE_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFDataDeleteFilesGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFDataDeleteFilesResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Finalize file uploads to an entity's profile.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Finalizes the upload of the requested files. Verifies that the files have been successfully uploaded
/// and moves the file pointers from pending to live. See also FileAbortFileUploadsAsync, FileDeleteFilesAsync,
/// FileGetFilesAsync, FileInitiateFileUploadsAsync.
///
/// When the asynchronous task is complete, call <see cref="PFDataFinalizeFileUploadsGetResultSize"/>
/// and <see cref="PFDataFinalizeFileUploadsGetResult"/> to get the result.
/// </remarks>
PF_API PFDataFinalizeFileUploadsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFDataFinalizeFileUploadsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a FinalizeFileUploads call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_FILE_NOT_FOUND,
/// E_PF_NO_ENTITY_FILE_OPERATION_PENDING or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFDataFinalizeFileUploadsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFDataFinalizeFileUploadsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_FILE_NOT_FOUND,
/// E_PF_NO_ENTITY_FILE_OPERATION_PENDING or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFDataFinalizeFileUploadsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFDataFinalizeFileUploadsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Retrieves file metadata from an entity's profile.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Returns URLs that may be used to download the files for a profile for a limited length of time. Only
/// returns files that have been successfully uploaded, files that are still pending will either return
/// the old value, if it exists, or nothing. See also FileAbortFileUploadsAsync, FileDeleteFilesAsync,
/// FileFinalizeFileUploadsAsync, FileInitiateFileUploadsAsync.
///
/// When the asynchronous task is complete, call <see cref="PFDataGetFilesGetResultSize"/> and <see cref="PFDataGetFilesGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFDataGetFilesAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFDataGetFilesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetFiles call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFDataGetFilesGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFDataGetFilesAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFDataGetFilesGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFDataGetFilesResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Retrieves objects from an entity's profile.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Gets JSON objects from an entity profile and returns it.  See also ObjectSetObjectsAsync.
///
/// When the asynchronous task is complete, call <see cref="PFDataGetObjectsGetResultSize"/> and <see
/// cref="PFDataGetObjectsGetResult"/> to get the result.
/// </remarks>
PF_API PFDataGetObjectsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFDataGetObjectsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetObjects call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFDataGetObjectsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFDataGetObjectsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFDataGetObjectsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFDataGetObjectsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Initiates file uploads to an entity's profile.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Returns URLs that may be used to upload the files for a profile 5 minutes. After using the upload
/// calls FinalizeFileUploads must be called to move the file status from pending to live. See also FileAbortFileUploadsAsync,
/// FileDeleteFilesAsync, FileFinalizeFileUploadsAsync, FileGetFilesAsync.
///
/// When the asynchronous task is complete, call <see cref="PFDataInitiateFileUploadsGetResultSize"/>
/// and <see cref="PFDataInitiateFileUploadsGetResult"/> to get the result.
/// </remarks>
PF_API PFDataInitiateFileUploadsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFDataInitiateFileUploadsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a InitiateFileUploads call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENTITY_FILE_OPERATION_PENDING
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFDataInitiateFileUploadsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFDataInitiateFileUploadsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENTITY_FILE_OPERATION_PENDING
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFDataInitiateFileUploadsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFDataInitiateFileUploadsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Sets objects on an entity's profile.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Sets JSON objects on the requested entity profile. May include a version number to be used to perform
/// optimistic concurrency operations during update. If the current version differs from the version in
/// the request the request will be ignored. If no version is set on the request then the value will always
/// be updated if the values differ. Using the version value does not guarantee a write though, ConcurrentEditError
/// may still occur if multiple clients are attempting to update the same profile.  See also ObjectGetObjectsAsync.
///
/// When the asynchronous task is complete, call <see cref="PFDataSetObjectsGetResultSize"/> and <see
/// cref="PFDataSetObjectsGetResult"/> to get the result.
/// </remarks>
PF_API PFDataSetObjectsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFDataSetObjectsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a SetObjects call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_INVALID_JSON_CONTENT
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFDataSetObjectsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFDataSetObjectsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_INVALID_JSON_CONTENT
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFDataSetObjectsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFDataSetObjectsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;


}