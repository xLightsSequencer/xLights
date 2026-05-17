# Phase I — Import Effects (iPad)

I-1 / I-2 / I-3 / I-4 all complete. Only I-5 (`.lms`/`.las`) is
parked, pending vendor request.

## I-5 — LMS / LAS (parked)

- Hoist `.lms` / `.las` parsing from
  `LMSImportChannelMapDialog.cpp` to
  `src-core/import_export/LmsImporter.{h,cpp}`.
- Add formats to the iPad picker.
- LOREdit conversion (`LOREdit.{h,cpp}`) is referenced by both
  LMS and LOR S5 paths — investigate whether to hoist with LMS
  or leave for a later S5 import phase.

Distant third format behind `.xsq`/`.xsqz` and `.sup`; pull in when
a vendor request lands.

## Carried-forward deferrals from I-2

- Model-blending toggle. Niche.
- `.xsq` / `.xsqz` UTType registration in the iPad app's
  `LSItemContentTypes` (Info.plist edit) — file picker today
  works via extension matching.
- Apply still runs with hardcoded `convertRender=false`.
- Removing the desktop dialog's per-instance `norm` /
  `aggressive` / `regex` lambdas. Still work; clean up only if
  drift becomes annoying.
