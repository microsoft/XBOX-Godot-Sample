#include <string>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <algorithm>
#include <functional>

#ifdef PLAYFAB_SAMPLE_SWITCH
#include "../Switch/SwitchPch.h"
#endif // PLAYFAB_SAMPLE_SWITCH

#ifdef PLAYFAB_SAMPLE_PLAYSTATION4
#include "../PS4/PSPch.h"
#endif // PLAYFAB_SAMPLE_PLAYSTATION4

#ifdef PLAYFAB_SAMPLE_PLAYSTATION5
#include "../PS5/PSPch.h"
#endif // PLAYFAB_SAMPLE_PLAYSTATION5

#include <Party.h>
#include <Party_c.h>
#include <PFUtils.h>
#include <PFMultiplayer.h>
#include <PFLobby.h>
#include <EntityHandle.h>

#include <playfab/services/PFServices.h>

#include "Player.h"

#if HC_PLATFORM == HC_PLATFORM_GDK
#include "../WinGDK/GDKPch.h"
#endif