#ifndef PLAYFAB_GDK_EDITION_H
#define PLAYFAB_GDK_EDITION_H

// GDK edition gate (PlayFab addon)
// --------------------------------
// `grdk.h` exposes `_GRDK_EDITION`, the 6-digit edition of the GDK headers being
// compiled against (e.g. 251001 = October 2025, 260400 = April 2026). It
// is self-contained but is NOT pulled in transitively by the public GDK/PlayFab
// headers, so include it explicitly here.
#include <grdk.h>

#ifndef _GRDK_EDITION
// Older or repackaged SDKs that predate the edition macro: assume the newest
// behavior so the default (vcpkg) build keeps full functionality.
#define _GRDK_EDITION 999999
#endif

// Several PlayFab struct fields shipped with the April 2026 GDK (edition 260400)
// and are absent from the October 2025 GDK PlayFab headers:
//   * PFLobbyCreateConfiguration::restrictInvitesToLobbyOwner
//   * PFLobbyArrangedJoinConfiguration::restrictInvitesToLobbyOwner
//   * PFInventoryInventoryItem::startDate
//   * PFInventoryRedemptionSuccess::expirationTimestamp
// Gate the code that reads/writes those fields on this edition.
#define PLAYFAB_GDK_HAS_APRIL_2026_FIELDS (_GRDK_EDITION >= 260400)

#endif // PLAYFAB_GDK_EDITION_H
