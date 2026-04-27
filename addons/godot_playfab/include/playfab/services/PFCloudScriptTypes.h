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
/// CloudScriptRevisionOption enum.
/// </summary>
enum class PFCloudScriptCloudScriptRevisionOption : uint32_t
{
    Live,
    Latest,
    Specific
};

/// <summary>
/// PFCloudScriptExecuteCloudScriptRequest data model.
/// </summary>
typedef struct PFCloudScriptExecuteCloudScriptRequest
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
    /// The name of the CloudScript function to execute.
    /// </summary>
    _Null_terminated_ const char* functionName;

    /// <summary>
    /// (Optional) Object that is passed in to the function as the first argument.
    /// </summary>
    PFJsonObject functionParameter;

    /// <summary>
    /// (Optional) Generate a 'player_executed_cloudscript' PlayStream event containing the results of
    /// the function execution and other contextual information. This event will show up in the PlayStream
    /// debugger console for the player in Game Manager.
    /// </summary>
    _Maybenull_ bool const* generatePlayStreamEvent;

    /// <summary>
    /// (Optional) Option for which revision of the CloudScript to execute. 'Latest' executes the most
    /// recently created revision, 'Live' executes the current live, published revision, and 'Specific'
    /// executes the specified revision. The default value is 'Specific', if the SpeificRevision parameter
    /// is specified, otherwise it is 'Live'.
    /// </summary>
    _Maybenull_ PFCloudScriptCloudScriptRevisionOption const* revisionSelection;

    /// <summary>
    /// (Optional) The specivic revision to execute, when RevisionSelection is set to 'Specific'.
    /// </summary>
    _Maybenull_ int32_t const* specificRevision;

} PFCloudScriptExecuteCloudScriptRequest;

/// <summary>
/// PFCloudScriptScriptExecutionError data model.
/// </summary>
typedef struct PFCloudScriptScriptExecutionError
{
    /// <summary>
    /// (Optional) Error code, such as CloudScriptNotFound, JavascriptException, CloudScriptFunctionArgumentSizeExceeded,
    /// CloudScriptAPIRequestCountExceeded, CloudScriptAPIRequestError, or CloudScriptHTTPRequestError.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* error;

    /// <summary>
    /// (Optional) Details about the error.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* message;

    /// <summary>
    /// (Optional) Point during the execution of the script at which the error occurred, if any.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* stackTrace;

} PFCloudScriptScriptExecutionError;

/// <summary>
/// PFCloudScriptLogStatement data model.
/// </summary>
typedef struct PFCloudScriptLogStatement
{
    /// <summary>
    /// (Optional) Optional object accompanying the message as contextual information.
    /// </summary>
    PFJsonObject data;

    /// <summary>
    /// (Optional) 'Debug', 'Info', or 'Error'.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* level;

    /// <summary>
    /// (Optional) Message property.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* message;

} PFCloudScriptLogStatement;

/// <summary>
/// PFCloudScriptExecuteCloudScriptResult data model.
/// </summary>
typedef struct PFCloudScriptExecuteCloudScriptResult
{
    /// <summary>
    /// Number of PlayFab API requests issued by the CloudScript function.
    /// </summary>
    int32_t aPIRequestsIssued;

    /// <summary>
    /// (Optional) Information about the error, if any, that occurred during execution.
    /// </summary>
    _Maybenull_ PFCloudScriptScriptExecutionError const* error;

    /// <summary>
    /// ExecutionTimeSeconds property.
    /// </summary>
    double executionTimeSeconds;

    /// <summary>
    /// (Optional) The name of the function that executed.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* functionName;

    /// <summary>
    /// (Optional) The object returned from the CloudScript function, if any.
    /// </summary>
    PFJsonObject functionResult;

    /// <summary>
    /// (Optional) Flag indicating if the FunctionResult was too large and was subsequently dropped from
    /// this event. This only occurs if the total event size is larger than 350KB.
    /// </summary>
    _Maybenull_ bool const* functionResultTooLarge;

    /// <summary>
    /// Number of external HTTP requests issued by the CloudScript function.
    /// </summary>
    int32_t httpRequestsIssued;

    /// <summary>
    /// (Optional) Entries logged during the function execution. These include both entries logged in
    /// the function code using log.info() and log.error() and error entries for API and HTTP request
    /// failures.
    /// </summary>
    _Maybenull_ _Field_size_(logsCount) PFCloudScriptLogStatement const* const* logs;

    /// <summary>
    /// Count of logs
    /// </summary>
    uint32_t logsCount;

    /// <summary>
    /// (Optional) Flag indicating if the logs were too large and were subsequently dropped from this
    /// event. This only occurs if the total event size is larger than 350KB after the FunctionResult
    /// was removed.
    /// </summary>
    _Maybenull_ bool const* logsTooLarge;

    /// <summary>
    /// MemoryConsumedBytes property.
    /// </summary>
    uint32_t memoryConsumedBytes;

    /// <summary>
    /// Processor time consumed while executing the function. This does not include time spent waiting
    /// on API calls or HTTP requests.
    /// </summary>
    double processorTimeSeconds;

    /// <summary>
    /// The revision of the CloudScript that executed.
    /// </summary>
    int32_t revision;

} PFCloudScriptExecuteCloudScriptResult;

/// <summary>
/// PFCloudScriptExecuteCloudScriptServerRequest data model.
/// </summary>
typedef struct PFCloudScriptExecuteCloudScriptServerRequest
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
    /// The name of the CloudScript function to execute.
    /// </summary>
    _Null_terminated_ const char* functionName;

    /// <summary>
    /// (Optional) Object that is passed in to the function as the first argument.
    /// </summary>
    PFJsonObject functionParameter;

    /// <summary>
    /// (Optional) Generate a 'player_executed_cloudscript' PlayStream event containing the results of
    /// the function execution and other contextual information. This event will show up in the PlayStream
    /// debugger console for the player in Game Manager.
    /// </summary>
    _Maybenull_ bool const* generatePlayStreamEvent;

    /// <summary>
    /// The unique user identifier for the player on whose behalf the script is being run.
    /// </summary>
    _Null_terminated_ const char* playFabId;

    /// <summary>
    /// (Optional) Option for which revision of the CloudScript to execute. 'Latest' executes the most
    /// recently created revision, 'Live' executes the current live, published revision, and 'Specific'
    /// executes the specified revision. The default value is 'Specific', if the SpeificRevision parameter
    /// is specified, otherwise it is 'Live'.
    /// </summary>
    _Maybenull_ PFCloudScriptCloudScriptRevisionOption const* revisionSelection;

    /// <summary>
    /// (Optional) The specivic revision to execute, when RevisionSelection is set to 'Specific'.
    /// </summary>
    _Maybenull_ int32_t const* specificRevision;

} PFCloudScriptExecuteCloudScriptServerRequest;

/// <summary>
/// PFCloudScriptExecuteEntityCloudScriptRequest data model. Executes CloudScript with the entity profile
/// that is defined in the request.
/// </summary>
typedef struct PFCloudScriptExecuteEntityCloudScriptRequest
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
    /// (Optional) The optional entity to perform this action on. Defaults to the currently logged in
    /// entity.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// The name of the CloudScript function to execute.
    /// </summary>
    _Null_terminated_ const char* functionName;

    /// <summary>
    /// (Optional) Object that is passed in to the function as the first argument.
    /// </summary>
    PFJsonObject functionParameter;

    /// <summary>
    /// (Optional) Generate a 'entity_executed_cloudscript' PlayStream event containing the results of
    /// the function execution and other contextual information. This event will show up in the PlayStream
    /// debugger console for the player in Game Manager.
    /// </summary>
    _Maybenull_ bool const* generatePlayStreamEvent;

    /// <summary>
    /// (Optional) Option for which revision of the CloudScript to execute. 'Latest' executes the most
    /// recently created revision, 'Live' executes the current live, published revision, and 'Specific'
    /// executes the specified revision. The default value is 'Specific', if the SpecificRevision parameter
    /// is specified, otherwise it is 'Live'.
    /// </summary>
    _Maybenull_ PFCloudScriptCloudScriptRevisionOption const* revisionSelection;

    /// <summary>
    /// (Optional) The specific revision to execute, when RevisionSelection is set to 'Specific'.
    /// </summary>
    _Maybenull_ int32_t const* specificRevision;

} PFCloudScriptExecuteEntityCloudScriptRequest;

/// <summary>
/// PFCloudScriptExecuteFunctionRequest data model. Executes an Azure Function with the profile of the
/// entity that is defined in the request.
/// </summary>
typedef struct PFCloudScriptExecuteFunctionRequest
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
    /// (Optional) The optional entity to perform this action on. Defaults to the currently logged in
    /// entity.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// The name of the CloudScript function to execute.
    /// </summary>
    _Null_terminated_ const char* functionName;

    /// <summary>
    /// (Optional) Object that is passed in to the function as the FunctionArgument field of the FunctionExecutionContext
    /// data structure.
    /// </summary>
    PFJsonObject functionParameter;

    /// <summary>
    /// (Optional) Generate a 'entity_executed_cloudscript_function' PlayStream event containing the
    /// results of the function execution and other contextual information. This event will show up in
    /// the PlayStream debugger console for the player in Game Manager.
    /// </summary>
    _Maybenull_ bool const* generatePlayStreamEvent;

} PFCloudScriptExecuteFunctionRequest;

/// <summary>
/// PFCloudScriptFunctionExecutionError data model.
/// </summary>
typedef struct PFCloudScriptFunctionExecutionError
{
    /// <summary>
    /// (Optional) Error code, such as CloudScriptAzureFunctionsExecutionTimeLimitExceeded, CloudScriptAzureFunctionsArgumentSizeExceeded,
    /// CloudScriptAzureFunctionsReturnSizeExceeded or CloudScriptAzureFunctionsHTTPRequestError.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* error;

    /// <summary>
    /// (Optional) Details about the error.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* message;

    /// <summary>
    /// (Optional) Point during the execution of the function at which the error occurred, if any.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* stackTrace;

} PFCloudScriptFunctionExecutionError;

/// <summary>
/// PFCloudScriptExecuteFunctionResult data model.
/// </summary>
typedef struct PFCloudScriptExecuteFunctionResult
{
    /// <summary>
    /// (Optional) Error from the CloudScript Azure Function.
    /// </summary>
    _Maybenull_ PFCloudScriptFunctionExecutionError const* error;

    /// <summary>
    /// The amount of time the function took to execute.
    /// </summary>
    int32_t executionTimeMilliseconds;

    /// <summary>
    /// (Optional) The name of the function that executed.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* functionName;

    /// <summary>
    /// (Optional) The object returned from the function, if any.
    /// </summary>
    PFJsonObject functionResult;

    /// <summary>
    /// (Optional) Flag indicating if the FunctionResult was too large and was subsequently dropped from
    /// this event.
    /// </summary>
    _Maybenull_ bool const* functionResultTooLarge;

} PFCloudScriptExecuteFunctionResult;

/// <summary>
/// PFCloudScriptListFunctionsRequest data model. A title can have many functions, ListHttpFunctions
/// will return a list of all the currently registered HTTP triggered functions for a given title.
/// </summary>
typedef struct PFCloudScriptListFunctionsRequest
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

} PFCloudScriptListFunctionsRequest;

/// <summary>
/// PFCloudScriptEventHubFunctionModel data model.
/// </summary>
typedef struct PFCloudScriptEventHubFunctionModel
{
    /// <summary>
    /// (Optional) The connection string for the event hub.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* connectionString;

    /// <summary>
    /// (Optional) The name of the event hub that triggers the Azure Function.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eventHubName;

    /// <summary>
    /// (Optional) The name the function was registered under.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* functionName;

} PFCloudScriptEventHubFunctionModel;

/// <summary>
/// PFCloudScriptListEventHubFunctionsResult data model.
/// </summary>
typedef struct PFCloudScriptListEventHubFunctionsResult
{
    /// <summary>
    /// (Optional) The list of EventHub triggered functions that are currently registered for the title.
    /// </summary>
    _Maybenull_ _Field_size_(functionsCount) PFCloudScriptEventHubFunctionModel const* const* functions;

    /// <summary>
    /// Count of functions
    /// </summary>
    uint32_t functionsCount;

} PFCloudScriptListEventHubFunctionsResult;

/// <summary>
/// PFCloudScriptRegisterEventHubFunctionRequest data model. A title can have many functions, RegisterEventHubFunction
/// associates a function name with an event hub name and connection string.
/// </summary>
typedef struct PFCloudScriptRegisterEventHubFunctionRequest
{
    /// <summary>
    /// A connection string for the namespace of the event hub for the Azure Function.
    /// </summary>
    _Null_terminated_ const char* connectionString;

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
    /// The name of the event hub for the Azure Function.
    /// </summary>
    _Null_terminated_ const char* eventHubName;

    /// <summary>
    /// The name of the function to register.
    /// </summary>
    _Null_terminated_ const char* functionName;

} PFCloudScriptRegisterEventHubFunctionRequest;

#pragma pop_macro("IN")

}
