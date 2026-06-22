#ifndef GDK_EDITION_H
#define GDK_EDITION_H

// GDK edition gate
// ----------------
// `grdk.h` exposes `_GRDK_EDITION`, the 6-digit edition of the GDK headers being
// compiled against (e.g. 251001 = October 2025, 260400 = April 2026). It
// is self-contained but is NOT pulled in transitively by the public GDK headers,
// so include it explicitly here.
#include <grdk.h>

#ifndef _GRDK_EDITION
// Older or repackaged SDKs that predate the edition macro: assume the newest
// behavior so the default (vcpkg) build keeps full functionality.
#define _GRDK_EDITION 999999
#endif

// `XGameActivation.h` (the unified activation API) was introduced in the
// April 2026 GDK (edition 260400). The October 2025 GDK ships only the older
// `XGameProtocol` + `XGameInvite` APIs that XGameActivation later superseded, so
// gate the activation backend on this edition.
#define GDK_EDITION_HAS_XGAME_ACTIVATION (_GRDK_EDITION >= 260400)

#endif // GDK_EDITION_H
