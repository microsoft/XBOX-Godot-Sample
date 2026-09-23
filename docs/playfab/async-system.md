# PlayFab async operations - signals, dispatch, and shutdown

The PlayFab addon exposes native XAsync, Party, and Multiplayer operations as one-shot Godot signals. This page defines the lifecycle contract that docs and samples should rely on.

## One-shot completion signals

- Every public `*_async` method returns a `Signal` that can be awaited directly.
- The signal emits exactly once with a `PlayFabResult`.
- Successful completions set `PlayFabResult.ok` and place Godot-native data in `PlayFabResult.data`.
- Failures still emit the signal. Check `ok` first; on failure inspect `code` (machine-readable error string), `message` (human-readable description), and `hresult` (native HRESULT) on the same `PlayFabResult` instead of waiting for a second callback.

## Main-thread completion delivery

API-visible completion work stays on the Godot main thread:

- Native SDK completions are queued into the shared PlayFab task queue and are finalized when `PlayFab.dispatch()` drains that queue.
- Immediate or synchronous failures use Godot `call_deferred` before emitting, so callers can connect or await the returned signal safely.
- Party and Multiplayer state-change batches are processed by their services from `PlayFab.dispatch()`, and their pending operation signals follow the same one-shot result rule.

## Dispatch ownership

`playfab/runtime/embed_dispatch` defaults to `true`. On builds with the extension frame callback enabled, the addon calls `PlayFab.dispatch()` once per process frame while PlayFab is initialized.

Disable embedded dispatch only when your project needs to own the pump. In that mode, call `PlayFab.dispatch()` from the main thread every frame while PlayFab, Party, or Multiplayer work is in flight:

```gdscript
func _process(_delta: float) -> void:
    if PlayFab.is_initialized():
        PlayFab.dispatch()
```

Do not run `dispatch()` from a worker thread. If dispatch is not pumped, async signals, Party state changes, and Multiplayer lobby/matchmaking events will not be delivered. Each call returns the number of completion work items processed, which is useful when a title wants to observe manual pump progress.

## Matchmaking ticket creation

`PlayFab.multiplayer.create_match_ticket_async()` resolves only after the returned `PlayFabMatchTicket.ticket_id` is non-empty. The native ticket handle may exist locally while the SDK is still assigning the id, but that half-created handle is not surfaced through the completion result or `get_match_tickets()`.

## Shutdown and cancellation

Party and Multiplayer scoped shutdown reject new work immediately but settle outstanding completion signals only after their SDK cleanup succeeds. Native contexts remain alive until their own completion or `PartyManager::Cleanup()` / `PFMultiplayerUninitialize()`; finishing one dispatch batch is not a lifetime fence. Reentrant shutdown defers cleanup until the current batch unwinds. Cleanup failures return explicit errors and retain contexts for a subsequent scoped shutdown retry. These scoped calls leave the PlayFab root runtime, accounts, and saves initialized.

Party shutdown uses two phases after successful Cleanup: silently invalidate all retained network/endpoint/chat handles and Party user registries, then emit peer/network notifications and pending results. This includes private partial networks and operations still awaiting create completion. The first callback already sees every retained peer disconnected and Party uninitialized; it cannot access a different network's old SDK handles. `release_local_user_async()` rejects with `party_shutting_down` throughout shutdown, including notifications and failed-cleanup retry. Failed Cleanup preserves initialized ownership and emits no shutdown terminal notifications.

Party host/join failures retain their original diagnostic result while rolling back any partial network. A failed/stalled rollback stays pending until native destruction or scoped cleanup establishes safe ownership. Title-level deadlines should invoke `PlayFab.party.shutdown_async()` rather than assuming a cancelled await stopped the SDK call. Existing callers must continue to handle the eventual one-shot result.

### Finalizer contract

Every `PlayFabSignalXAsyncContext::finalize(XAsyncBlock *)` implementation must short-circuit before result extraction or service/cache mutation when `get_runtime()->is_shutting_down()` or `get_pending_signal()->was_cancel_requested()` is true. The finalizer completes its pending signal with `PlayFabResult::cancelled(...)` and returns, so shutdown and explicit cancellation do not continue the success path after the runtime has started tearing down.

If a future finalizer must perform native cleanup during shutdown, keep the cancelled-result gate first and document the cleanup-only exception both inline and in this section.
