# Brief: Nekta Top — data-derived producer ranking

**For:** a Claude Code session with live Songstats API access.
**From:** Nekta (Jacob Eloquin). Paste this whole file as your opening prompt.

---

## 0. What you are building

A Nekta-branded alternative to the DJ Mag Top 100: a **ranking of electronic
music producers, segmented by genre, derived from data rather than from a public
vote.**

DJ Mag's list is a popularity poll. It rewards marketing spend and fan
mobilisation, it is widely considered gameable, and it does not publish its
weighting. Nekta's credibility depends on being the opposite: every rank
traceable to a figure, every weighting published.

Deliverables are listed in §7. Read §1 before you touch the API.

---

## 1. Data integrity rules — non-negotiable

These exist because an earlier session was asked for this ranking without API
access, and the correct outcome was to refuse rather than produce plausible
numbers. Hold the same line.

1. **Never fabricate, estimate, extrapolate or "fill in" a metric.** Every
   number in every deliverable traces to a specific API response.
2. **Stamp every figure** with the metric name, the platform, and the UTC
   timestamp of the pull. Write these into the dataset, not just the prose.
3. **Never mix vintages.** A figure from last week and a figure from 2024 do not
   belong in the same column. If you cannot pull all artists in a cohort within
   the same window, say so and stop.
4. **Never mix metrics.** Spotify monthly listeners, Last.fm scrobbles, Apple
   Music plays and SoundCloud plays are different quantities. Do not coerce them
   into one "streams" column.
5. **Absent is absent.** If Songstats has no value for an artist, the cell is
   `null` with a reason — never interpolated, never a neighbour's value.
6. **Report coverage honestly.** Every deliverable states what fraction of the
   candidate pool had complete data, and names the artists that did not.
7. **Do not commit the API key.** Read it from the environment. Add any local
   credential file to `.gitignore` before the first run.
8. **Raw dumps are reviewed before they are committed.** Write them to a
   scratch directory first and report their size.

If a rule here blocks the deliverable, surface that to Jacob rather than
working around it.

---

## 2. Resolve this fork first — do not assume

The request is for "top streamed **producer** in each genre". Songstats tracks
**artist profiles** — streams, playlists, charts, Shazams against an artist
entity. It does not natively track **production credits.**

In dance music these diverge sharply: a producer's biggest work often appears
under another artist's name, and the ghost-production economy is invisible to an
artist-level lookup.

So decide, with Jacob, which list this is:

- **(A) Performer chart.** Rank artist entities by their own streaming. Songstats
  alone is sufficient. Fast, defensible, but it is a better-sourced DJ Mag — and
  "producer" is then the wrong word in the title.
- **(B) True producer list.** Join streaming data to production credits from a
  credits source (Jaxsta, MusicBrainz, Beatport release credits, label
  metadata). Materially harder. Nobody publishes this well, which is exactly why
  it is worth doing.

**Your first task:** read the live Songstats API documentation, confirm what it
actually exposes today, and report back to Jacob which of (A) or (B) is
achievable with the access you have, at what cost in effort. Do not pick for
him. Do not assume my description of the API is current — verify it.

---

## 3. The two inputs that make this list Nekta's

Raw streaming alone produces a boring and slightly dishonest list: it ranks
catalogue size and playlist luck. Two additions fix it.

### 3.1 DJ support (from Songstats)

Songstats carries DJ-support / tracklist data — which DJs are actually playing a
track. For electronic music this is a far better proxy for **scene standing**
than monthly listeners, and it is the signal that separates a producer the scene
respects from one with a big back catalogue.

This matters directly: see the benchmark in §6, where a scene-respected
bassline producer is ranked 3rd by a human but would place far lower on pure
streams. A blended score that includes DJ support is likely to reconcile the two
— test whether it does.

### 3.2 Live footprint (from Nekta's own data)

Nekta already pulls live event listings from the Resident Advisor GraphQL API
(`app/api/events/route.ts`, keyed by RA area IDs across ~24 cities). Nobody
else building a chart has a per-city live-booking feed sitting next to their
streaming data.

Billing frequency, city spread and venue tier are a legitimate third axis, and
they are the component that makes this list structurally impossible for a
competitor to copy. Treat the RA pipeline as an input, not decoration.

---

## 4. Genre taxonomy

Anchor to **Beatport's genre tree** wherever possible, because Songstats carries
Beatport data and the join will be clean. Then collapse Beatport's long tail
into Nekta's editorial set.

Starting proposal — needs Jacob's sign-off before you rank anything, because
the taxonomy determines every rank beneath it:

House · Tech House · Melodic House & Techno · Techno · Minimal / Deep Tech ·
Drum & Bass · UK Garage / Bassline · Dubstep & Bass · Trance · Psytrance ·
Hard Techno / Hard Dance · Afro House · Amapiano · Breakbeat / Jungle ·
Disco / Nu-Disco · Electronica / Leftfield

Open questions to put to Jacob:

- Is Amapiano in scope? It is adjacent rather than electronic-native, but it is
  significant in several of Nekta's cities.
- Tech House split from House, or merged? Splitting is truer to the scene;
  merging avoids one genre dominating the overall list.
- Multi-genre producers: single primary genre, or allowed to place in several?
  Recommend a single primary with a declared secondary, so the per-genre lists
  stay readable.

---

## 5. Scoring

Define "top" explicitly and publish the definition alongside the list.

**Per-genre ranking** is ranked within genre, so raw magnitudes are fine.

**The overall Top 100 needs cross-genre normalisation.** Genre audience sizes
differ by an order of magnitude. Rank the overall list on raw streams and house
and bass will fill it while techno and UKG never appear — which would be both
boring and wrong. Normalise within genre (percentile or z-score) before
combining, so the overall list measures *standing within one's own scene*, not
genre size.

Recommended components, weights to be agreed with Jacob and then **published on
the page**:

| Component | Source | Why |
|---|---|---|
| Streaming reach | Songstats | audience size |
| Streaming velocity (rolling 90d) | Songstats | momentum, not legacy catalogue |
| DJ support | Songstats | scene standing (§3.1) |
| Beatport chart performance | Songstats | genre-native commercial signal |
| Live footprint | Nekta / RA | the differentiator (§3.2) |

Windows: a rolling 90-day window for velocity, 12 months for weight. State both.

Anti-gaming: publish the weighting, log every snapshot so ranks are reproducible
at a date, and exclude any metric a label can buy outright without the signal
being visible in the others.

---

## 6. Benchmark: compare the data against a human list

Jacob's own top 7, by instinct, no data:

1. Skrillex
2. Sammy Virji
3. MPH
4. Fred again..
5. Chase & Status
6. Anyma
7. Chris Lake

Tasks:

- **Disambiguate MPH.** Several artists record under that name. The intended one
  is the UK bassline / garage producer. Confirm the correct Songstats entity ID
  before pulling anything, and state which entity you used.
- Pull the agreed metrics for all seven, in one window.
- Produce the stream-ranked ordering, the DJ-support-ranked ordering, and the
  blended ordering. Show all three next to Jacob's.
- **Quantify the divergence and explain it.** The hypothesis to test: his list is
  scene-weighted rather than stream-weighted, and DJ support will track his
  ordering more closely than streams do. Report whether the data supports that
  or contradicts it. A clean contradiction is a useful result — say so plainly.
- Note that these seven span roughly six genres, so in a genre-segmented list
  most of them never compete with each other. Flag whether Jacob wants a flat
  overall ranking, per-genre rankings, or both.

---

## 7. Deliverables

1. `data/nekta-top/<ISO-date>/` — raw API responses, one file per artist or
   batch, untouched, with a `manifest.json` recording endpoint, parameters and
   UTC pull time per file.
2. `data/nekta-top/<ISO-date>/dataset.csv` — the normalised table, one row per
   artist, every column named with its metric and platform, `null` where absent.
3. `docs/nekta-top-methodology.md` — taxonomy, metric definitions, windows,
   weights, normalisation, exclusions, and the coverage statement from §1.6.
   This is a public-facing document: it is the thing that makes the list
   credible rather than just another poll.
4. `docs/nekta-top-benchmark.md` — the §6 comparison, with the three orderings
   and the divergence analysis.
5. The ranking itself, as data (JSON or CSV) that a page can render — not
   hand-written into a component.

Build 1–4 before anyone designs a page. The page is easy; the defensible
dataset is the product.

---

## 8. Context you may want

- Repo: `github.com/eloquin/nekta`, a Next.js 16 app. **Read `AGENTS.md` and
  `CLAUDE.md` first** — this Next version has breaking changes from training
  data, and the repo instructs you to read `node_modules/next/dist/docs/`
  before writing app code.
- `app/api/events/route.ts` is the existing RA events integration — the live
  data referenced in §3.2.
- There is no artist, genre or streaming data in the repo today. You are
  building that layer from scratch.
- `engine/` is an unrelated C++ audio engine (the Nekta Sampler). Ignore it.

## 9. Where to push back

If any of the following is true, say so rather than delivering around it:

- Songstats does not expose a metric this brief assumes.
- The genre taxonomy cannot be joined cleanly to the data.
- Coverage is too thin in a genre to rank it honestly — in which case publish
  that genre as "insufficient data" rather than ranking three artists and
  implying it is a scene.
- The blended score is doing nothing the raw streams don't already do. If the
  added components aren't changing the ordering, the methodology is theatre and
  should be simplified.
