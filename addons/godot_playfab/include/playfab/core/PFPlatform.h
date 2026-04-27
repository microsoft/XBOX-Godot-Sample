// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFPal.h>
#include <XAsync.h>

extern "C"
{

//-----------------------------------------------------------------------------
// Hooks for platform specific behavior
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Memory (optional on all platforms)
//-----------------------------------------------------------------------------

/// <summary>
/// A callback invoked every time a new memory buffer must be dynamically allocated by the library.
/// This callback is optionally installed by calling PFMemSetFunctions()
/// 
/// The callback must allocate and return a pointer to a contiguous block of memory of the 
/// specified size that will remain valid until the app's corresponding PFMemFreeFunction 
/// callback is invoked to release it.
/// 
/// Every non-null pointer returned by this method will be subsequently passed to the corresponding
/// PFMemFreeFunction callback once the memory is no longer needed.
/// </summary>
/// <returns>A pointer to an allocated block of memory of the specified size, or a null 
/// pointer if allocation failed.</returns>
/// <param name="size">The size of the allocation to be made. This value will never be zero.</param>
/// <param name="memoryTypeId">An opaque identifier representing the internal category of
/// memory being allocated. This value should be ignored.</param>
typedef _Ret_maybenull_ _Post_writable_byte_size_(size) void* STDAPIVCALLTYPE PFMemAllocFunction(_In_ size_t size, _In_ uint32_t memoryTypeId);

/// <summary>
/// A callback invoked every time a previously allocated memory buffer is no longer needed by 
/// the library and can be freed. This callback is optionally installed by calling PFMemSetFunctions()
///
/// The callback is invoked whenever the library has finished using a memory buffer previously 
/// returned by the app's corresponding PFMemAllocFunction such that the application can free the
/// memory buffer.
/// </summary>
/// <param name="pointer">The pointer to the memory buffer previously allocated. This value will
/// never be a null pointer.</param>
/// <param name="memoryTypeId">An opaque identifier representing the internal category of
/// memory being allocated. This value should be ignored.</param>
typedef void STDAPIVCALLTYPE PFMemFreeFunction(_In_ _Post_invalid_ void* pointer, _In_ uint32_t memoryTypeId);

/// <summary>
/// Struct encapsulating memory hooks.
/// </summary>
/// <remarks>
/// Both hooks must be set at the same time.
/// </remarks>
typedef struct PFMemoryHooks
{
    /// <summary>
    /// Custom allocation hook.
    /// </summary>
    PFMemAllocFunction* alloc;

    /// <summary>
    /// Custom free hook.
    /// </summary>
    PFMemFreeFunction* free;
} PFMemoryHooks;

/// <summary>
/// Optionally sets the memory hook functions to allow callers to control route memory 
/// allocations to their own memory manager. This must be called before PFInitialize() 
/// and can not be called again once memory hooks have been set.
///
/// This method allows the application to install custom memory allocation routines in order 
/// to service all requests for new memory buffers instead of using default allocation routines.
/// </summary>
/// <param name="hooks">Custom memory hooks to be used by the SDK.</param>
/// <returns>HRESULT return code for this API operation.</returns>
/// <remarks>
/// Must be called before PFInitialize.
/// </remarks>
PF_API PFMemSetFunctions(
    _In_ PFMemoryHooks* hooks
) noexcept;

/// <summary>
/// Gets the memory hook functions to allow callers to control route memory allocations to their 
/// own memory manager. This method allows the application get the default memory allocation routines.
/// This can be used along with PFMemSetFunctions() to monitor all memory allocations.
/// </summary>
/// <param name="hooks">Struct to be populated with the current memory hooks. Defaults will be returned
/// if custom hooks weren't previously set.</param>
/// <returns>HRESULT return code for this API operation.</returns>
PF_API PFMemGetFunctions(
    _Out_ PFMemoryHooks* hooks
) noexcept;

/// <summary>
/// Determines if custom memory functions are being used.
/// </summary>
/// <param name="isUsingCustomMemoryFunctions">Indicates if custom memory functions are in use.</param>
/// <returns>HRESULT return code for this API operation.</returns>
PF_API PFMemIsUsingCustomMemoryFunctions(
    _Out_ bool* isUsingCustomMemoryFunctions
) noexcept;

//-----------------------------------------------------------------------------
// Storage. Required on some platforms, see online documentation for more details.
//-----------------------------------------------------------------------------

/// <summary>
/// Read from storage event handler. Should implement an <see cref="XAsyncProvider"/>.
/// </summary>
/// <param name="context">Optional pointer to data used by the event handler.</param>
/// <param name="key">Identifies the data being written. Typically a filepath.</param>
/// <param name="async">XAsyncBlock for the operation.</param>
/// <remarks>
/// This event handler will be invoked when PlayFab needs to read data from storage.
/// The XAsyncBlock should be completed with data size (in bytes) that was read.
/// All arguments are owned by the SDK and are guaranteed to be valid until the operation is complete (except context).
/// </remarks>
typedef HRESULT STDAPIVCALLTYPE PFPlatformLocalStorageReadAsyncHandler(
    _In_opt_ void* context,
    _In_z_ const char* key,
    _Inout_ XAsyncBlock* async
);

/// <summary>
/// Write to storage event handler. Should implement an <see cref="XAsyncProvider"/>.
/// </summary>
/// <param name="context">Optional pointer to data used by the event handler.</param>
/// <param name="key">Identifies the data being written. Typically a filepath.</param>
/// <param name="dataSize">The size (in bytes) of the data.</param>
/// <param name="data">The data to write.</param>
/// <param name="async">XAsyncBlock for the operation.</param>
/// <remarks>
/// This event handler will be invoked when PlayFab needs to write data to storage.
/// All arguments are owned by the SDK and are guaranteed to be valid until the operation is complete (except context).
/// </remarks>
typedef HRESULT STDAPIVCALLTYPE PFPlatformLocalStorageWriteAsyncHandler(
    _In_opt_ void* context,
    _In_z_ const char* key,
    _In_ size_t dataSize,
    _In_reads_bytes_(dataSize) void const* data,
    _Inout_ XAsyncBlock* async
);

/// <summary>
/// Write from storage event handler. Should implement an <see cref="XAsyncProvider"/>.
/// </summary>
/// <param name="context">Optional pointer to data used by the event handler.</param>
/// <param name="key">Identifies the data being written. Typically a filepath.</param>
/// <param name="async">XAsyncBlock for the operation.</param>
/// <remarks>
/// This event handler will be invoked when PlayFab needs to clear data from storage.
/// All arguments are owned by the SDK and are guaranteed to be valid until the operation is complete (except context).
/// </remarks>
typedef HRESULT STDAPIVCALLTYPE PFPlatformLocalStorageClearAsyncHandler(
    _In_opt_ void* context,
    _In_z_ const char* key,
    _Inout_ XAsyncBlock* async
);

/// <summary>
/// Struct encapsulating the storage hooks.
/// </summary>
/// <remarks>
/// All 3 handlers must be set at the same time.
/// </remarks>
typedef struct PFLocalStorageHooks
{
    /// <summary>
    /// (Optional) Queue that the hooks should be invoked on. If not specified, the callbacks will be invoked
    /// on the background queue provided to PFInitialize.
    /// </summary>
    _Maybenull_ XTaskQueueHandle queueHandle;

    /// <summary>
    /// Custom read hook.
    /// </summary>
    PFPlatformLocalStorageReadAsyncHandler* read;

    /// <summary>
    /// Custom write hook.
    /// </summary>
    PFPlatformLocalStorageWriteAsyncHandler* write;

    /// <summary>
    /// Custom clear hook.
    /// </summary>
    PFPlatformLocalStorageClearAsyncHandler* clear;

    /// <summary>
    /// (Optional) Client context that will be passed to storage hooks.
    /// </summary>
    _Maybenull_ void* context;
} PFLocalStorageHooks;

/// <summary>
/// Sets the storage event handlers. If custom storage handlers are not set, a default implementation
/// based on cstdio and std::fstream objects will be used. 
/// </summary>
/// <param name="hooks">Custom local storage hooks to be used by the SDK.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Must be called before PFInitialize.
/// </remarks>
PF_API PFPlatformLocalStorageSetHandlers(
    _In_ PFLocalStorageHooks* hooks
) noexcept;

PF_API PFPlatformIsGRTSAvailable(
    _Out_ bool* isGRTSAvailable
) noexcept;

enum class PFPlatformType
{
    Unknown,
    Windows,
    Xbox,
    Linux,
    Nintendo,
    PlayStation,
    iOS,
    Android,
};

PF_API PFPlatformGetPlatformType(
    _Out_ PFPlatformType* platformType
) noexcept;

#if HC_PLATFORM == HC_PLATFORM_GDK
/// <summary>
/// For use by the GameSave API.  Titles should not call this directly.
/// </summary>
/// <param name="gameSaveContext">GameSave context pointer.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFPlatformGetGameSaveContext(
    _Outptr_ void** gameSaveContext
) noexcept;
#endif

}
