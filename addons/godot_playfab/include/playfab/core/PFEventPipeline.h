// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFPal.h>
#include <playfab/core/PFEntity.h>
#include <httpClient/httpClient.h>

extern "C"
{

typedef struct PFEventPipeline* PFEventPipelineHandle;

/// <summary>
/// Default value for maximum events per batch for a PlayStream Event pipeline if none is provided.
/// </summary>
const uint32_t PFPlayStreamEventPipelineMaxEventsPerBatchDefault = 5;

/// <summary>
/// Default value for maximum wait time for a PlayStream Event pipeline if none is provided.
/// </summary>
const uint32_t PFPlayStreamEventPipelineMaxWaitTimeInSecondsDefault = 3;

/// <summary>
/// Default value for poll delay time for a PlayStream Event pipeline if none is provided.
/// </summary>
const uint32_t PFPlayStreamEventPipelinePollDelayInMsDefault = 10;

/// <summary>
/// Default value for maximum events per batch for a Telemetry Event pipeline if none is provided.
/// </summary>
const uint32_t PFTelemetryEventPipelineMaxEventsPerBatchDefault = 5;

/// <summary>
/// Default value for maximum wait time for a Telemetry Event pipeline if none is provided.
/// </summary>
const uint32_t PFTelemetryEventPipelineMaxWaitTimeInSecondsDefault = 3;

/// <summary>
/// Default value for poll delay time for a Telemetry Event pipeline if none is provided.
/// </summary>
const uint32_t PFTelemetryEventPipelinePollDelayInMsDefault = 3000;

/// <summary>
/// Default value for attempting a retry of event emission when losing connection.
/// </summary>
const bool PFTelemetryEventPipelineRetryOnDisconnectDefault = true;

/// <summary>
/// Default value for the maximum amount of events in the event buffer.
/// </summary>
const size_t PFEventPipelineBufferSizeDefault = 1024;

/// <summary>
/// Event to be uploaded to PlayFab.
/// </summary>
struct PFEvent
{
    /// <summary>
    /// Entity associated with the event. If null, the event will apply to the calling entity.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// The namespace in which the event is defined.
    /// </summary>
    _Null_terminated_ const char* eventNamespace;

    /// <summary>
    /// The name of the event.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// Client assigned identifier associated with event. Not required but may be useful for tracking or tracing.
    /// If a clientId is not provided, the SDK will automatically assign one.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* clientId;

    /// <summary>
    /// Arbitrary JSON data associated with the event.
    /// </summary>
    _Null_terminated_ const char* payloadJson;
};

/// <summary>
/// Event that has been uploaded to PlayFab. Associates the service assigned ID with the clientId provided when the event was emitted.
/// </summary>
struct PFUploadedEvent
{
    /// <summary>
    /// Id assigned by the client prior to the event being uploaded.
    /// </summary>
    _Null_terminated_ const char* clientId;

    /// <summary>
    /// Unique Id assigned by the PlayFab server for the event.
    /// </summary>
    _Null_terminated_ const char* serviceId;
};

/// <summary>
/// Configuration struct that defines the properties required for using Telemetry Key.
/// </summary>
struct PFEventPipelineTelemetryKeyConfig
{
    /// <summary>
    /// The Key created to send PlayFab event requests without an associated entity.
    /// </summary>
    _Null_terminated_ const char* telemetryKey;

    /// <summary>
    /// Service Config to be used with telemetry key.
    /// </summary>
    PFServiceConfigHandle serviceConfigHandle;
};

/// <summary>
/// Defines the Event Pipeline configuration.
/// </summary>
struct PFEventPipelineConfig
{
    /// <summary>
    /// The maximum number events that will be batched before writing them to PlayFab. If sent null, default value will be used (5 for Telemetry / 5 for PlayStream).
    /// </summary>
    _Maybenull_ uint32_t* maxEventsPerBatch;

    /// <summary>
    /// The maximum time the pipeline will wait before sending out an incomplete batch. If sent null, default value will be used (3 for Telemetry / 3 for PlayStream).
    /// </summary>
    _Maybenull_ uint32_t* maxWaitTimeInSeconds;

    /// <summary>
    /// How long the pipeline will wait to read from the event buffer again after emptying it. If sent null, default value will be used (10 for Telemetry / 10 for PlayStream).
    /// </summary>
    _Maybenull_ uint32_t* pollDelayInMs;

    /// <summary>
    /// The event pipeline will send events using GZIP compression with the level specified. If sent null, no compression will be made.
    /// </summary>
    _Maybenull_ HCCompressionLevel* compressionLevel;

    /// <summary>
    /// The event pipeline will retry sending events that failed due to lost connection. If sent null, default behavior will be to retry (true) (not available for PlayStream).
    /// </summary>
    _Maybenull_ bool* retryOnDisconnect;

    /// <summary>
    /// The limit of the amount of events in the pipeline's buffer. If sent null, default value will be used (1024 for either type).
    /// </summary>
    _Maybenull_ size_t* bufferSize;
};

/// <summary>
/// A handler invoked when the SDK successfully uploads a batch of events.
/// </summary>
/// <param name="context">Optional context pointer to data used by the event handler.</param>
/// <param name="eventPipelineUploadedEvents">The array of PFEventPipelineUploadedEvents that was uploaded.</param>
/// <param name="eventPipelineUploadedEventsCount">The number of entries in the eventPipelineUploadedEvents array.</param>
/// <remarks>
/// Arguments besides context are owned by the SDK and only guaranteed to be valid within the callback.
/// </remarks>
typedef void CALLBACK PFEventPipelineBatchUploadSucceededEventHandler(
    _In_opt_ void* context,
    _In_ PFUploadedEvent const* const* eventPipelineUploadedEvents,
    _In_ size_t eventPipelineUploadedEventsCount
);

/// <summary>
/// A handler invoked when the SDK has attempted but failed to upload a batch of event pipeline events.
/// It is up to the client to resubmit failed events as necessary using PFEventPipelineEmitEvent, though depending on the nature
/// of the failure, resubmitting directly may not resolve the issue.
/// </summary>
/// <param name="context">Optional context pointer to data used by the event handler.</param>
/// <param name="translatedUploadError">Translated upload error.</param>
/// <param name="errorMessage">Message describing the upload error.</param>
/// <param name="failedEvents">Events that were in the failed batch.</param>
/// <param name="failedEventsCount">The number of entries in the failedEvents array.</param>
/// <remarks>
/// Arguments besides context are owned by the SDK and only guaranteed to be valid within the callback.
/// </remarks>
typedef void CALLBACK PFEventPipelineBatchUploadFailedEventHandler(
    _In_opt_ void* context,
    _In_ HRESULT translatedUploadError,
    _In_ const char* errorMessage,
    _In_ PFEvent const* const* failedEvents,
    _In_ size_t failedEventsCount
);

/// <summary>
/// Creates an event pipeline to upload Telemetry events using a Telemetry Key.
/// </summary>
/// <param name="eventPipelineTelemetryKeyConfig">The Configuration to use if telemetry key logging will be used.</param>
/// <param name="queue">The async queue where background work will be scheduled and where event callbacks will be invoked.</param>
/// <param name="eventPipelineBatchUploadedEventHandler">Optional handler that will be invoked when a batch of events is uploaded.</param>
/// <param name="eventPipelineBatchFailedEventHandler">Optional handler that will be invoked when uploading a batch of events fails.</param>
/// <param name="handlerContext">Optional pointer to data used by the event handlers.</param>
/// <param name="eventPipelineHandle">Handle to the created pipeline. Needs to be closed with PFEventPipelineCloseHandle when it isn't needed.</param>
/// <returns>Result code for this API operation.  Possible values are S_OK, E_INVALIDARG, E_PF_NOT_INITIALIZED or E_FAIL.</returns>
PF_API PFEventPipelineCreateTelemetryPipelineHandleWithKey(
    _In_ PFEventPipelineTelemetryKeyConfig* eventPipelineTelemetryKeyConfig,
    _In_opt_ XTaskQueueHandle queue,
    _In_opt_ PFEventPipelineBatchUploadSucceededEventHandler* eventPipelineBatchUploadedEventHandler,
    _In_opt_ PFEventPipelineBatchUploadFailedEventHandler* eventPipelineBatchFailedEventHandler,
    _In_opt_ void* handlerContext,
    _Out_ PFEventPipelineHandle* eventPipelineHandle
) noexcept;

/// <summary>
/// Creates an event pipeline to upload Telemetry events using an Entity.
/// </summary>
/// <param name="entityHandle">The Entity whose token will be used to make the PlayFab service requests in the background.</param>
/// <param name="queue">The async queue where background work will be scheduled and where event callbacks will be invoked.</param>
/// <param name="eventPipelineBatchUploadedEventHandler">Optional handler that will be invoked when a batch of events is uploaded.</param>
/// <param name="eventPipelineBatchFailedEventHandler">Optional handler that will be invoked when uploading a batch of events fails.</param>
/// <param name="handlerContext">Optional pointer to data used by the event handlers.</param>
/// <param name="eventPipelineHandle">Handle to the created pipeline. Needs to be closed with PFEventPipelineCloseHandle when it isn't needed.</param>
/// <returns>Result code for this API operation.  Possible values are S_OK, E_INVALIDARG, E_PF_NOT_INITIALIZED or E_FAIL.</returns>
PF_API PFEventPipelineCreateTelemetryPipelineHandleWithEntity(
    _In_ PFEntityHandle entityHandle,
    _In_opt_ XTaskQueueHandle queue,
    _In_opt_ PFEventPipelineBatchUploadSucceededEventHandler* eventPipelineBatchUploadedEventHandler,
    _In_opt_ PFEventPipelineBatchUploadFailedEventHandler* eventPipelineBatchFailedEventHandler,
    _In_opt_ void* handlerContext,
    _Out_ PFEventPipelineHandle* eventPipelineHandle
) noexcept;

/// <summary>
/// Creates an event pipeline to upload PlayStream events using an Entity.
/// </summary>
/// <param name="entityHandle">The Entity whose token will be used to make the PlayFab service requests in the background.</param>
/// <param name="queue">The async queue where background work will be scheduled and where event callbacks will be invoked.</param>
/// <param name="eventPipelineBatchUploadedEventHandler">Optional handler that will be invoked when a batch of events is uploaded.</param>
/// <param name="eventPipelineBatchFailedEventHandler">Optional handler that will be invoked when uploading a batch of events fails.</param>
/// <param name="handlerContext">Optional pointer to data used by the event handlers.</param>
/// <param name="eventPipelineHandle">Handle to the created pipeline. Needs to be closed with PFEventPipelineCloseHandle when it isn't needed.</param>
/// <returns>Result code for this API operation.  Possible values are S_OK, E_INVALIDARG, E_PF_NOT_INITIALIZED or E_FAIL.</returns>
PF_API PFEventPipelineCreatePlayStreamPipelineHandle(
    _In_ PFEntityHandle entityHandle,
    _In_opt_ XTaskQueueHandle queue,
    _In_opt_ PFEventPipelineBatchUploadSucceededEventHandler* eventPipelineBatchUploadedEventHandler,
    _In_opt_ PFEventPipelineBatchUploadFailedEventHandler* eventPipelineBatchFailedEventHandler,
    _In_opt_ void* handlerContext,
    _Out_ PFEventPipelineHandle* eventPipelineHandle
) noexcept;

/// <summary>
/// Duplicates a PFEventPipelineHandle.
/// </summary>
/// <param name="eventPipelineHandle">EventPipeline handle to duplicate.</param>
/// <param name="duplicatedHandle">The duplicated handle.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Both the duplicated handle and the original handle need to be closed with PFEventPipelineCloseHandle when they are no longer needed.
/// </remarks>
PF_API PFEventPipelineDuplicateHandle(
    _In_ PFEventPipelineHandle eventPipelineHandle,
    _In_ PFEventPipelineHandle* duplicatedEventPipelineHandle
) noexcept;

/// <summary>
/// Closes a PFEventPipelineHandle.
/// </summary>
/// <param name="eventPipelineHandle">EventPipeline handle to close.</param>
/// <returns>Result code for this API operation.</returns>
PF_API_(void) PFEventPipelineCloseHandle(
    _In_ PFEventPipelineHandle eventPipelineHandle
) noexcept;

/// <summary>
/// Adds a event to a pipelines buffer to be uploaded. The API will return synchronously and the event
/// will be uploaded in the background based on the pipeline configuration.
/// </summary>
/// <param name="eventPipelineHandle">The EventPipeline to submit the event to.</param>
/// <param name="event">Event to upload.</param>
/// <returns>Result code for this API operation. Possible values are S_OK, E_PF_NOT_INITIALIZED, or E_FAIL.</returns>
PF_API PFEventPipelineEmitEvent(
    _In_ PFEventPipelineHandle eventPipelineHandle,
    _In_ PFEvent const* event
) noexcept;

/// <summary>
/// Adds an entity to an existing pipeline. All events emitted will be linked to this entity.
/// </summary>
/// <param name="eventPipelineHandle">The existing EventPipeline handle to add the uploading entity.</param>
/// <param name="entityHandle">The Entity whose token will be used to make the PlayFab service requests in the background.</param>
/// <returns>Result code for this API operation. Possible values are S_OK, E_PF_NOT_INITIALIZED or E_FAIL.</returns>
PF_API PFEventPipelineAddUploadingEntity(
    _In_ PFEventPipelineHandle eventPipelineHandle,
    _In_ PFEntityHandle entityHandle
) noexcept;

/// <summary>
/// Remove an entity from an existing pipeline. If a valid Telemetry Key Configuration was added at pipeline creation
/// it will switch to it.
/// </summary>
/// <param name="eventPipelineHandle">The existing EventPipeline handle to remove the uploading entity.</param>
/// <returns>Result code for this API operation. Possible values are S_OK, E_PF_NOT_INITIALIZED, or E_FAIL.</returns>
PF_API PFEventPipelineRemoveUploadingEntity(
    _In_ PFEventPipelineHandle eventPipelineHandle
) noexcept;

/// <summary>
/// Update an existing pipeline configuration.
/// </summary>
/// <param name="eventPipelineHandle">The existing EventPipeline handle to add the uploading entity.</param>
/// <param name="eventPipelineConfig">Struct that contains the new configuration of the event pipeline.</param>
/// <returns>Result code for this API operation. Possible values are S_OK, E_PF_NOT_INITIALIZED, or E_FAIL.</returns>
PF_API PFEventPipelineUpdateConfiguration(
    _In_ PFEventPipelineHandle eventPipelineHandle,
    _In_ PFEventPipelineConfig eventPipelineConfig
) noexcept;

}
