# The Word app notes

This is a native AppLoad/QML app for the reMarkable Paper Pro Move. It follows
the device and packaging conventions in [docs/PLATFORM.md](docs/PLATFORM.md).

## Current state

- Desktop preview and AppLoad frontend are the same `ui/Board.qml`.
- The backend is offline and loads the full imported KJV corpus from `data/bible.bin`,
  with a small demo fallback in `src/BibleData.cpp`.
- The frontend/backend protocol uses one compact JSON state message.
- The layout is portrait-first but has a side-by-side landscape mode.
- No animations, gradients, dense fills, or network dependency should be added.
- Notes are intentionally not part of the current product.

## Next data step

If the dictionary is enriched later, keep the UI-facing state stable:
`words`, `selectedWord`, `strongId`, `relatedEntries`, and the navigation fields.
Preserve source/license metadata with the imported data.
