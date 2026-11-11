# Nekta Cards on Solana: Technical and Product Plan

_Draft dated 11 Nov 2026. All prices are **estimates**. Check them with each vendor before quoting to investors. SOL is assumed at about $180._

**What the cards are:** collectible **DJ cards**, collectible **dub cards** and collectible **access passes**. Fans who supported an artist early claim a card for a tour. It carries perks set up with promoters (backstage, skip the queue, guest list, a welcome drink) to reward day-one fans and help venues sell out.

**Repo facts that shape this plan:** the app runs Next.js 16.2.1 with React 19.2. It has no DB, no auth and no cards yet. Run `npm install` and read `node_modules/next/dist/docs/` before writing code (per AGENTS.md). In Next 16, `middleware.ts` is now `proxy.ts`. Keep minting out of request handlers.

**Naming rule:** in the product UI, marketing and the pitch, say **"cards"** and **"passes"**, never "NFTs", "tokens" or "mint". Use the on-chain terms only in engineering docs and in the investor technical appendix.

---

## 1. Asset standard: Bubblegum v2 compressed cards with Metaplex Core collections

| | Bubblegum v2 (cNFT) | Metaplex Core | Token-2022 |
|---|---|---|---|
| Cost per mint (est.) | ~0.00001–0.00002 SOL. Tree rent is a one-off ~1.5–8 SOL per 1M leaves | ~0.0029 SOL rent (≈$0.50) | ~0.004+ SOL |
| 30k mints/month | **≈$50–110** | ≈$15,700 | ≈$20k+ |
| Collections | Core collection is the parent | Native | Group/member extensions, weak tooling |
| Royalties | Inherited from the Core collection Royalties plugin | Royalties plugin | Transfer hook, custom work |
| Soulbound / freeze | v2 supports freeze/thaw and non-transferable | Freeze delegates | NonTransferable extension |
| Mutable on-chain state | No (every change is a tx) | Attributes plugin | Metadata fields |

**Decision:**
- **Use Bubblegum v2 for all mass-issued cards.** Each card type is its own **Core collection**: `Nekta DJ Cards S1`, `Nekta Dub Cards`, `Nekta Passes — <Artist> <Tour>`.
- **DJ and dub cards are transferable.** Put a 5–7.5% royalty on the collection, split between artist and Nekta.
- **Passes are non-transferable (soulbound).** Nekta holds a burn delegate, so a pass can be revoked or reissued after fraud or a lost account.
- **Use Core only for rare 1/1 "legendary" cards.** Its attributes can hold extras like `signed_by` and `played_at`.
- **Don't store perk state on-chain.** Section 4 explains why.

**Metadata:**
- Art and JSON for each **template** (artist × season × rarity) are uploaded once to Arweave via Irys.
- The serial number goes in the name, e.g. `Ben UFO — S1 Holo #0042`.
- Rarity tiers: Common, Rare, Holo, 1/1.

**Dub cards** (working definition, confirm with the founder): a card for one track ID or set moment, e.g. "the unreleased ID at Fabric, 14 Nov". It can unlock a gated audio snippet. The schema keeps `kind` and `attributes` generic, so this can change without a migration.

## 2. Wallets: recommend Privy

**Why Privy:**
- Email, Apple and Google login, with a Solana wallet created silently at sign-up.
- Non-custodial for Nekta, and users can export their key. That avoids lock-in and helps the regulatory story.
- Server wallets for Nekta's treasury and fee payer.
- Native Solana fee sponsorship.
- Stripe owns it, so it's stable and an on-ramp is possible later.

**Alternatives considered:**
- **Turnkey:** cheaper infrastructure, but we'd build all the auth UX ourselves. Plan B.
- **Dynamic:** strong, but aimed more at connecting existing wallets.
- **Web3Auth:** EVM-first.
- **Phantom embedded:** Phantom branding breaks the "invisible" goal.
- **Crossmint:** most turnkey, but the deepest lock-in and per-mint markups.

**How custody and fees work:**
- **Most flows need no user signature.** Nekta's server mints straight to the user's address.
- **Users sign only to transfer or gift a card.** Nekta's fee payer covers the fee, through Privy sponsorship or a self-hosted **Kora** relayer.
- **Fee payer safety:** keep it in a separate hot wallet with a low balance, auto-top it up from a cold treasury, and alert below 2 SOL.
- **Ownership is chain truth, not Privy truth.** The `wallets` table is provider-agnostic.

## 3. Earning and claiming

**Early-supporter score** (stored per evidence source):
1. **Nekta follow date.** Start recording follows now, since it becomes the most valuable signal over time.
2. **Door-scan attendance** at the artist's shows.
3. **Promoter-uploaded lists.** Hashed emails or phones from Dice, RA or Skiddle exports.
4. **Spotify top-artists and Last.fm scrobbles.** Supplements only. Spotify doesn't expose follow dates, and SoundCloud's API is closed to new apps.

A drop sets the score threshold for each tier. For example, a pass needs ≥2 attended shows **or** a promoter-list match **or** a follow at least 6 months before the tour was announced.

**Claim channels:**
- **Claim links** `/claim/[code]`: single-use or capped, with an expiry.
- **Venue QR:** a code on the screen behind the DJ that changes every 30s, which proves the fan was there.
- **Auto-airdrop:** to eligible fans when a tour drop opens.

**Anti-sybil:**
- One verified phone per pass, with SMS asked only at pass claim.
- Account age, device fingerprinting and rate limits.
- Passes are non-transferable, so farming accounts has no resale payoff.

## 4. Perk redemption at the door

**The database is the source of truth, with an on-chain anchor:**
- Per-drink on-chain writes would cost a transaction each, slow down a loud door, and leak fans' drinking onto a public ledger.
- Redemptions live in Postgres.
- A nightly job writes a Merkle root of the night's redemptions on-chain (≈$0.001). That gives promoters, venues and investors proof that can't be tampered with.

**Rotating QR (defeats screenshots):**
- When a pass is issued, the fan's phone creates a non-extractable key bound to that pass.
- The pass screen shows a QR signed by that key, refreshed every 10s, with an animated holo border.
- A screenshot expires within 20s.
- A QR copied to another phone fails, because only one device is allowed per pass and re-binding needs SMS plus a cooldown.

**Scanner PWA** `/scan/[eventId]`:
- Staff log in with a role-scoped invite.
- Before doors, the PWA downloads a signed list of valid passes and perks, so it **works with no signal**. Redemptions sync when the connection returns.
- **Alcohol perks show a full-screen "CHECK ID 18+".** Staff must tap to confirm before redeeming.

## 5. Architecture and data model

**Stack:**
- Vercel (Next 16), Neon Postgres with Drizzle, and Inngest for the mint queue.
- Helius for RPC, asset lookups (DAS) and ownership webhooks.
- A nightly reconcile job checks the DB against the chain.

**Where data lives:**
- **On-chain:** ownership, collections, royalties, the soulbound flag and redemption anchors.
- **Off-chain:** eligibility, perks, redemptions, device keys, personal data and art cache.

**Tables:** `users`, `wallets`, `artists`, `promoters`, `promoter_members`, `card_templates` (kind: dj|dub|pass, season, tour, rarity, max_supply, transferable), `drops` (eligibility rule, claim mode), `claim_codes`, `cards` (owner_address, asset_id, serial, status), `eligibility_evidence`, `perks` (backstage|skip_queue|guestlist|drink, quantity, requires_id_check), `device_keys`, `redemptions`, `anchors`.

**Proposed file and route tree:**
```
proxy.ts                         # Next16 (was middleware): gate /dashboard, /scan by role
app/
  (fan)/cards/page.tsx           # my cards binder
  (fan)/cards/[cardId]/page.tsx  # card detail + rotating pass QR
  (fan)/claim/[code]/page.tsx
  (dashboard)/dashboard/page.tsx
  (dashboard)/dashboard/drops/new/page.tsx
  (dashboard)/dashboard/drops/[dropId]/page.tsx
  (dashboard)/dashboard/events/[eventId]/redemptions/page.tsx
  scan/[eventId]/page.tsx        # promoter scanner PWA (offline)
  api/
    auth/session/route.ts
    claims/route.ts  claims/[code]/route.ts
    cards/route.ts   cards/[cardId]/device-key/route.ts
    drops/route.ts   drops/[dropId]/route.ts  drops/[dropId]/holders/route.ts
    perks/route.ts
    scan/[eventId]/manifest/route.ts  scan/redeem/route.ts  scan/sync/route.ts
    connect/spotify/callback/route.ts
    webhooks/helius/route.ts
    inngest/route.ts
lib/
  db/  auth/  solana/(umi, bubblegum, core, feePayer, anchor)  qr/  eligibility/  jobs/
scripts/create-tree.ts  scripts/create-collection.ts  scripts/upload-template.ts
```

## 6. Promoter and artist dashboard
- **Create a drop:** artist, art, rarity split, supply, eligibility rule, claim mode and window.
- **Define perks** for each event: type, quantity, ID check.
- **Holders view:** count by tier and score distribution. CSV export shares contact details only for fans who opted in (GDPR).
- **Live door view:** redemptions per minute and flagged conflicts.
- **Artists** get a read-only view plus a royalties ledger.

## 7. Roadmap

| Phase | Scope | Effort |
|---|---|---|
| **A. Devnet investor demo** | Google login → silent wallet → scan a venue QR → DJ card flips in 3D and pins to the user's spot on the globe → "Proof" link to Solana Explorer → a second phone scans the fan's rotating pass QR and redeems "Skip the queue". | **2–3 wks, 1 dev** |
| **B. Mainnet pilot, one promoter** | Real tour pass drop for 200–1,000 fans, dashboard v1, offline scanner, SMS at claim, nightly anchors, legal review of perk T&Cs. | **6–8 wks, 2 devs + designer** |
| **C. Scale** | Self-serve promoters, dub cards with gated audio, gifting, Spotify/Last.fm scoring, royalty-enforced resale through a partner marketplace, sell-through analytics for venues. | **3–4 months** |

**Monthly cost estimates.** Assumptions: 40% of users active monthly, about 3 cards per active user.

| Line | 10k users | 100k users |
|---|---|---|
| Card mint fees | ~$20–45 | ~$200–450 |
| Tree rent (one-off) | ~$270–1,450 once | 1–2 trees |
| Helius | $49 | $499–999 |
| Privy | ~$0–500 | ~$1.5k–5k (negotiate) |
| SMS at pass claims | ~$50–150 | ~$500–1,500 |
| DB, queue, hosting | ~$100–250 | ~$600–1,500 |
| Art storage | <$20 | <$100 |
| **Total (est.)** | **≈$300–1,000/mo** | **≈$3.3k–9.5k/mo** |

For comparison, the same volume using Core for every card would cost about $6k/mo at 10k users and $60k/mo at 100k.

## 8. Risks
- **Securities.**
  - Never market cards as investments. No "floor price" talk, and no Nekta-run marketplace before legal sign-off.
  - Soulbound passes help the "utility, not investment" case.
  - Under EU MiCA, avoid large runs of identical items.
  - Get a UK FCA memo before mainnet.
- **Gambling.** Never sell random paid packs of tradeable cards. Rarity should come from free drops or disclosed odds.
- **Consumer law (UK DMCC Act, EU).**
  - Clear perk T&Cs that cover cancellations.
  - Promoters must back perks contractually.
- **Alcohol.** "Free drinks" can count as an irresponsible promotion in the UK, so offer "one welcome drink (soft option available)". Use 18+/21+ gating and an ID check at the door.
- **Wallet lock-in.** Mitigated by key export, chain-truth ownership and Turnkey as plan B.
- **NFT stigma.** Say "cards" everywhere and hide blockchain UI behind a "Proof" link.
- **"Why chain?"**
  - Portable proof of fandom that any promoter can verify.
  - Artist royalties.
  - Tamper-evident perk records.
  - Don't pitch speculation.
- **Data source.** RA scraping in `app/api/events/route.ts` is a due-diligence risk. Pass events should come from promoters.
