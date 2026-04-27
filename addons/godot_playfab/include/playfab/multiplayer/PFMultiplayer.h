#pragma once

#include <playfab/core/PFEntity.h>

#ifndef __cplusplus
#error "This header requires C++"
#endif // end if not defined __cplusplus

#ifdef __cplusplus
extern "C"
{
#endif

#define PFMULTIPLAYER_API __stdcall

#if defined(__linux__)
#define PFMULTIPLAYER_API_ATTRIBUTES __attribute__((visibility("default")))
#else
#define PFMULTIPLAYER_API_ATTRIBUTES
#endif

typedef const struct PFMultiplayer* PFMultiplayerHandle;
typedef uint32_t PFMultiplayerMemoryType;

/// <summary>
/// A processor mask that represents all processors available to the title.
/// </summary>
constexpr uint64_t PFMultiplayerAnyProcessor = 0xffffffffffffffff;

#pragma pack(push, 8)

/// <summary>
/// Threads that Multiplayer library uses for internal purposes.
/// </summary>
/// <seealso cref="PFMultiplayerSetThreadAffinityMask" />
enum class PFMultiplayerThreadId : uint32_t
{
    /// <summary>
    /// Represents Multiplayer library internal networking threads
    /// </summary>
    Networking = 0,
};

/// <summary>
/// Initialization configuration for the Multiplayer library.
/// </summary>
/// <seealso cref="XTaskQueueCreate" />
/// <seealso cref="XTaskQueueDispatch()" />
/// <nyi />
struct MultiplayerInitializationConfiguration
{
    /// <summary>
    /// The Title ID provided must be the same Title ID used to acquire the PlayFab EntityHandle.
    /// </summary>
    _Null_terminated_ const char * titleId;

    /// <summary>
    /// XTaskQueueHandle to be used for processing Multiplayer library tasks.
    /// </summary>
    /// <remarks>
    /// <para>
    /// If given a 'nullptr' XTaskQueueHandle, the Multiplayer library will create and manage its own thread to process
    /// Matchmaking and Lobby tasks by default. If the title prefers to create and manage its own threads, it must
    /// provide an XTaskQueueHandle. The Multiplayer library will rely on the title calling
    /// <see cref="XTaskQueueDispatch()" /> on the provided XTaskQueueHandle to process the associated tasks.
    /// </para>
    /// <para>
    /// The title must keep the handle open and valid for the lifetime of the Multiplayer library. Calling
    /// <see cref="XTaskQueueCloseHandle()" /> prior to <see cref="PFMultiplayerUninitialize()" /> will result in
    /// undefined behavior.
    /// </para>
    /// <para>
    /// To create an XTaskQueueHandle, call <see cref="XTaskQueueCreate" />. When creating the XTaskQueue, the
    /// <see cref="XTaskQueueDispatchMode" /> for the Work port should be set to
    /// <see cref="XTaskQueueDispatchMode::Manual" />. The dispatch mode for the Completion port is ignored by the
    /// Multiplayer library.
    /// <see cref="XTaskQueueDispatchMode::ThreadPool" />, <see cref="XTaskQueueDispatchMode::SerializedThreadPool" />,
    /// and <see cref="XTaskQueueDispatchMode::Immediate" /> dispatch modes are not supported and will result in
    /// undefined behavior.
    /// </para>
    /// </remarks>
    XTaskQueueHandle multiplayerTaskQueue;
};

/// <summary>
/// A callback invoked every time a new memory buffer must be dynamically allocated by the Multiplayer library.
/// </summary>
/// <remarks>
/// This callback is optionally installed using the <see cref="PFMultiplayerSetMemoryCallbacks()" /> method.
/// <para>
/// The callback must allocate and return a pointer to a contiguous block of memory of the specified size that will
/// remain valid until the title's corresponding <see cref="PFMultiplayerFreeMemoryCallback" /> is invoked to release
/// it. If this is not possible, the callback must return nullptr to fail the allocation. Memory allocation failures are
/// sometimes considered benign but will usually cause current Multiplayer library operation(s) to fail.
/// </para>
/// <para>
/// Every non-nullptr returned by this method will be subsequently passed to the corresponding
/// <see cref="PFMultiplayerFreeMemoryCallback" /> once the memory is no longer needed.
/// </para>
/// </remarks>
/// <param name="size">
/// The size of the allocation to be made. This value will never be zero.
/// </param>
/// <param name="memoryTypeId">
/// An opaque identifier representing the Multiplayer library internal category of memory being allocated. Generally,
/// this value should be ignored but can be used to assist in diagnosing and debugging Multiplayer memory allocations.
/// </param>
/// <returns>
/// A pointer to an allocated block of memory of the specified size, or nullptr if the allocation failed.
/// </returns>
/// <seealso cref="PFMultiplayerFreeMemoryCallback" />
/// <seealso cref="PFMultiplayerSetMemoryCallbacks" />
typedef
_Ret_maybenull_
_Post_writable_byte_size_(size) void*
(__stdcall* PFMultiplayerAllocateMemoryCallback)(
    size_t size,
    uint32_t memoryTypeId
    );

/// <summary>
/// A callback invoked every time a previously allocated memory buffer is no longer needed by the Multiplayer library
/// and can be freed.
/// </summary>
/// <remarks>
/// This callback is optionally installed using the <see cref="PFMultiplayerSetMemoryCallbacks()" /> method.
/// <para>
/// The callback is invoked whenever the Multiplayer library has finished using a memory buffer previously returned by
/// the title's corresponding <see cref="PFMultiplayerAllocateMemoryCallback" />, so that the title can free the memory
/// buffer.
/// </para>
/// </remarks>
/// <param name="pointer">
/// A pointer to a memory buffer previously allocated. This value will never be nullptr.
/// </param>
/// <param name="memoryTypeId">
/// An opaque identifier representing the Multiplayer library internal category of memory being allocated. Generally,
/// this value should be ignored but can be used to assist in diagnosing and debugging Multiplayer memory allocations.
/// </param>
/// <seealso cref="PFMultiplayerAllocateMemoryCallback" />
/// <seealso cref="PFMultiplayerSetMemoryCallbacks" />
typedef
void
(__stdcall* PFMultiplayerFreeMemoryCallback)(
    _Post_invalid_ void * pointer,
    uint32_t memoryTypeId
    );

#pragma pack(pop)

/// <summary>
/// Get the human-readable form of an error generated by the PlayFab Multiplayer library.
/// </summary>
/// <remarks>
/// These error messages are not localized and are only intended for developers, i.e. these error messages are not
/// intended to be shown to users via UI.
/// </remarks>
/// <param name="error">
/// An error code.
/// </param>
/// <returns>
/// The human-readable error message. The memory for the returned string remains valid for the lifetime of the
/// process.
/// </returns>
PFMULTIPLAYER_API_ATTRIBUTES
_Null_terminated_
const char *
PFMULTIPLAYER_API
PFMultiplayerGetErrorMessage(
    HRESULT error
    ) noexcept;

/// <summary>
/// Optionally configures the memory allocation and freeing callbacks the Multiplayer library should use.
/// </summary>
/// <remarks>
/// Using **PFMemSetFunctions** from the PlayFab Core SDK will establish universal custom memory allocation and
/// freeing callbacks across all PlayFab SDK components, including Multiplayer. Using this function allows the title to
/// implement custom memory allocation and freeing callbacks that override the respective memory callbacks of the
/// PlayFab Core SDK. This method allows the title to install custom memory allocation functions in order to service all 
/// requests by the Multiplayer library for new memory buffers instead of using its default allocation functions.
/// <para>
/// The <paramref name="allocateMemoryCallback" /> and <paramref name="freeMemoryCallback" /> parameters must both be
/// non-null.
/// </para>
/// <para>
/// To use this method, it must be called before any other Multiplayer method. This method cannot be called again for
/// the lifetime of this process.
/// </para>
/// </remarks>
/// <param name="allocateMemoryCallback">
/// A pointer to the custom allocation callback to use.
/// </param>
/// <param name="freeMemoryCallback">
/// A pointer to the custom freeing callback to use.
/// </param>
/// <returns>
/// <c>S_OK</c> if the call succeeded or an error code otherwise. The human-readable form of the error code can be
/// retrieved via <see cref="PFMultiplayerGetErrorMessage()" />.
/// </returns>
/// <seealso cref="PFMultiplayerAllocateMemoryCallback" />
/// <seealso cref="PFMultiplayerFreeMemoryCallback" />
PFMULTIPLAYER_API_ATTRIBUTES
HRESULT
PFMULTIPLAYER_API
PFMultiplayerSetMemoryCallbacks(
    _In_ PFMultiplayerAllocateMemoryCallback allocateMemoryCallback,
    _In_ PFMultiplayerFreeMemoryCallback freeMemoryCallback
) noexcept;

/// <summary>
/// Optionally configures the processor on which internal Multiplayer library threads will run.
/// </summary>
/// <remarks>
/// This method enables the title to configure the processor affinity for internal Multiplayer library threads of a
/// given type.
/// <para>
/// This method may be called at any time before or after <see cref="PFMultiplayerInitialize()" /> and will take effect
/// immediately. Thread processor settings are persisted across calls to <see cref="PFMultiplayerUninitialize()" /> and
/// PFMultiplayerInitialize(). When there are more than 64 cores present, this method always applies to processor group
/// 0.
/// </para>
/// <para>
/// In order to specify any processor, pass <c>PFMultiplayerAnyProcessor</c> as the
/// <paramref name="threadAffinityMask" /> parameter. This is also the default value the Multiplayer library will use if
/// this method is never called.
/// </para>
/// </remarks>
/// <param name="threadId">
/// The type of internal Multiplayer library thread to configure processor affinity.
/// </param>
/// <param name="threadAffinityMask">
/// The affinity mask for this type of Multiplayer library thread.
/// </param>
/// <returns>
/// <c>S_OK</c> if the call succeeded or an error code otherwise. The human-readable form of the error code can be
/// retrieved via <see cref="PFMultiplayerGetErrorMessage()" />.
/// </returns>
/// <seealso cref="PFMultiplayerThreadId" />
PFMULTIPLAYER_API_ATTRIBUTES
HRESULT
PFMULTIPLAYER_API
PFMultiplayerSetThreadAffinityMask(
    PFMultiplayerThreadId threadId,
    uint64_t threadAffinityMask
    ) noexcept;

/// <summary>
/// Initializes an instance of the PlayFab Multiplayer library.
/// </summary>
/// <remarks>
/// This must be called before any other method, aside from <see cref="PFMultiplayerSetMemoryCallbacks()" />, and
/// <see cref="PFMultiplayerSetThreadAffinityMask()" />. PFMultiplayerInitialize() cannot be called again without a
/// subsequent <see cref="PFMultiplayerUninitialize()" /> call.
/// <para>
/// Every call to PFMultiplayerInitialize() should have a corresponding PFMultiplayerUninitialize() call.
/// </para>
/// <para>
/// Apps using the Microsoft Game Core version of the Multiplayer library will need to wait for the Game Core Networking
/// stack to be initialized prior to calling this method. Determining the status of the network stack can be done using
/// the Game Core XNetworkingGetConnectivityHint API.
/// </para>
/// <para>
/// The Title ID provided to <paramref name="initializationConfiguration" /> must be the same Title ID used to
/// acquire the PlayFab Entity Handle.
/// </para>
/// </remarks>
/// <param name="initializationConfiguration">
/// The Multiplayer library initialization configuration.
/// </param>
/// <param name="handle">
/// The output handle of the newly initialized PFMultiplayer API instance.
/// </param>
/// <returns>
/// <c>S_OK</c> if the call succeeded or an error code otherwise. The human-readable form of the error code can be
/// retrieved via <see cref="PFMultiplayerGetErrorMessage()" />.
/// </returns>
/// <seealso cref="PFMultiplayerSetEntityToken" />
/// <seealso cref="PFMultiplayerUninitialize" />
/// <seealso cref="PFMultiplayerSetMemoryCallbacks" />
/// <seealso cref="PFMultiplayerSetThreadAffinityMask" />
PFMULTIPLAYER_API_ATTRIBUTES
HRESULT
PFMULTIPLAYER_API
PFMultiplayerInitialize(
    _In_ const MultiplayerInitializationConfiguration * initializationConfiguration,
    _Outptr_ PFMultiplayerHandle * handle
    ) noexcept;

/// <summary>
/// Immediately reclaims all resources associated with all Multiplayer library objects.
/// </summary>
/// <remarks>
/// If local users were participating in a Lobby, they are removed (it appears to remote lobby clients as if network
/// connectivity to these users has been lost), so best practice is to call <c>PFLobbyLeave</c> on all lobbies and wait
/// for the corresponding <c>PFLobbyLeaveLobbyCompletedStateChange</c> to have the local users exit any existing
/// lobbies.
/// <para>
/// This method is not thread-safe and may not be called concurrently with other Multiplayer library methods. After
/// calling this method, all Multiplayer library state is invalidated.
/// </para>
/// <para>
/// Every call to <see cref="PFMultiplayerInitialize()" /> should have a corresponding PFMultiplayerUninitialize() call.
/// </para>
/// </remarks>
/// <param name="handle">
/// The handle of the PFMultiplayer API instance.
/// </param>
/// <returns>
/// <c>S_OK</c> if the call succeeded or an error code otherwise. The human-readable form of the error code can be
/// retrieved via <see cref="PFMultiplayerGetErrorMessage()" />.
/// </returns>
/// <seealso cref="PFMultiplayerInitialize" />
PFMULTIPLAYER_API_ATTRIBUTES
HRESULT
PFMULTIPLAYER_API
PFMultiplayerUninitialize(
    _Post_invalid_ PFMultiplayerHandle handle
    ) noexcept;

/// <summary>
/// Sets the token that should be used for authentication when performing library actions on behalf of an entity. If a
/// token has previously been set for the entity, this replaces its previous token.
/// </summary>
/// <remarks>
/// This method takes a PlayFab Entity Key as <paramref name="entity" /> and a PlayFab Entity Token as
/// <paramref name="token" />. When the library performs operations on behalf of an entity that require authentication
/// or authorization, such as creating or updating a lobby, the library will look up a token associated with the entity
/// to use for the operation. If no token has previously been set for the entity, the operation will synchronously fail.
/// During the asynchronous operation, the PlayFab service will validate that the token is valid, is not expired, is
/// associated with the Entity ID provided, and is authorized to perform the operation. If these conditions aren't met,
/// the operation will fail.
/// <para>
/// A PlayFab Entity Key and Entity Token can be obtained from the output of a PlayFab login operation and then provided
/// as input to this method.
/// </para>
/// <para>
/// The provided <paramref name="entity" /> and <paramref name="token" /> must have been acquired using the same PlayFab
/// Title ID that was passed to <see cref="PFMultiplayerInitialize()" />.
/// </para>
/// <para>
/// The Multiplayer library makes a copy of the supplied PlayFab Entity Token for use in subsequent operations that
/// require authentication or authorization of the local user, such as <c>PFMultiplayerCreateAndJoinLobby</c>. If the
/// token provided to this call is expired or otherwise invalid, operations that require a valid token will fail. A new,
/// valid token can be provided to the Multiplayer library by calling this method again using the same entity key.
/// </para>
/// <para>
/// The caller is responsible for monitoring the expiration of the entity token provided to this method. When the token
/// is nearing or past the expiration time a new token should be obtained by performing a PlayFab login operation and
/// provided to the Multiplayer library by calling this method again. It is recommended to acquire a new token when the
/// previously supplied token is halfway through its validity period. On platforms that may enter a low power state or
/// otherwise cause the application to pause execution for a long time, preventing the token from being refreshed before
/// it expires, the token should be checked for expiration once execution resumes.
/// </para>
/// <para>
/// No synchronous validation is performed on these values.
/// </para>
/// </remarks>
/// <param name="handle">
/// The handle of the PFMultiplayer API instance.
/// </param>
/// <param name="entity">
/// The PlayFab Entity Key to associate with a token.
/// </param>
/// <param name="token">
/// The PlayFab Entity token to associate with an entity.
/// </param>
/// <returns>
/// <c>S_OK</c> if creating the local user succeeded or an error code otherwise. The human-readable form of the error
/// code can be retrieved via <see cref="PFMultiplayerGetErrorMessage()" />.
/// </returns>
/// <seealso cref="PFMultiplayerInitialize" />
PFMULTIPLAYER_API_ATTRIBUTES
HRESULT
PFMULTIPLAYER_API
PFMultiplayerSetEntityToken(
    PFMultiplayerHandle handle,
    const PFEntityKey * entity,
    _Null_terminated_ const char * entityToken
    ) noexcept;

#ifdef __cplusplus
}
#endif
