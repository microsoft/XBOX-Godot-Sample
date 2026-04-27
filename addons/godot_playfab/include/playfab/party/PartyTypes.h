#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/// <summary>
/// A macro abstracting the nothrow convention for Party library function calls.
/// </summary>
#define party_no_throw throw()

/// <summary>
/// A macro abstracting the calling convention for Party library callbacks.
/// </summary>
#define PARTY_CALLBACK __stdcall

/// <summary>
/// A typedef for cross-platform safe Party library types.
/// </summary>
/// <remarks>
/// The size of a bool is implementation defined, so PartyBool is used to enable serialization.
///</remarks>
typedef uint8_t PartyBool;

/// <summary>
/// A typedef for cross-platform safe Party library types.
/// </summary>
typedef _Return_type_success_(return == c_partyErrorSuccess) uint32_t PartyError;

/// <summary>
/// An error code that represents a party library operation completed successfully.
/// </summary>
static const PartyError c_partyErrorSuccess = 0;

/// <summary>
/// An error code that represents a failure to load the party module.
/// </summary>
static const PartyError c_partyErrorFailedLoadingPartyModule = 0x10D0;

/// <summary>
/// Macro to help with determining whether a Party function call succeeded or not.
/// </summary>
#define PARTY_SUCCEEDED(partyError) ((partyError) == c_partyErrorSuccess)

/// <summary>
/// Macro to help with determining whether a Party function call succeeded or not
/// </summary>
#define PARTY_FAILED(partyError) (!PARTY_SUCCEEDED((partyError)))

/// <summary>
/// A typedef for cross-platform safe Party library types.
/// </summary>
typedef _Null_terminated_ const char * PartyString;

#ifdef __cplusplus
}
#endif
