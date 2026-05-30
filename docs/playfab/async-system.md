# PlayFab async system

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

Do not run `dispatch()` from a worker thread. If dispatch is not pumped, async signals, Party state changes, and Multiplayer lobby/matchmaking events will not be delivered.

## Matchmaking ticket creation

`PlayFab.multiplayer.create_match_ticket_async()` resolves only after the returned `PlayFabMatchTicket.ticket_id` is non-empty. The native ticket handle may exist locally while the SDK is still assigning the id, but that half-created handle is not surfaced through the completion result or `get_match_tickets()`.

## Shutdown and cancellation

`PlayFab.shutdown()` cancels outstanding Party and Multiplayer pending signals before native teardown and rejects new Party/Multiplayer work while shutdown is in progress. Existing callers should still await or connect their signals and handle a cancellation-style `PlayFabResult` instead of assuming the signal disappears.

### Finalizer contract

Every `PlayFabSignalXAsyncContext::finalize(XAsyncBlock *)` implementation must short-circuit before result extraction or service/cache mutation when `get_runtime()->is_shutting_down()` or `get_pending_signal()->was_cancel_requested()` is true. The finalizer completes its pending signal with `PlayFabResult::cancelled(...)` and returns, so shutdown and explicit cancellation do not continue the success path after the runtime has started tearing down.

If a future finalizer must perform native cleanup during shutdown, keep the cancelled-result gate first and document the cleanup-only exception both inline and in this section.
