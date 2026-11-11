# Solana funding for Nekta: research and pitch plan

_Prepared November 2026. Figures come from public and third-party sources. Check them with the contact before relying on them._

## 1. Who might be behind "$50k to $5M"

No single program called a "Solana Growth Fund" offers exactly $50k–$5M. The range most likely describes one of these:

| Source | What it is | Typical size | How you get in |
|---|---|---|---|
| **Solana Ventures** | Venture arm tied to Solana Labs. Invests only in Solana projects: consumer, NFTs, gaming, DeFi, infrastructure. | Directories disagree. Reported ranges run from about $50k–$150k up to $1M–$10M, with an average around $6M. | Warm intro (like this LinkedIn contact) |
| **Solana Foundation grants** | Non-profit. Gives milestone-based grants, mostly for public goods and open source, plus **convertible grants** that can turn into equity. | Grants average about $40k. Convertible grants are larger. | Rolling application on solana.org |
| **Foundation × YC grant** | For YC founders building on Solana | Up to $50k | Get into YC |
| **Colosseum** | Official Solana accelerator ($60M fund) | $250k pre-seed for each accepted team | Win or place in a Colosseum hackathon or its rolling "Eternal" challenge. You can't apply directly. |
| **Superteam** (regional chapters) | Community grants and bounties | Up to $10k | Apply through the local Superteam |
| **Ecosystem VCs** (e.g. Multicoin, Jump, Big Brain, Alliance-style funds) | Independent funds focused on Solana | $250k–$5M seed | Warm intro |

**What we don't know yet:** the contact's name and which fund they are at. The $50k–$5M range fits **Solana Ventures** best, or an ecosystem fund that pairs small grants with larger equity cheques. Ask them three things on the first call:
1. Is the money a grant, a convertible grant, a SAFE with a token warrant, or equity?
2. What milestones unlock each tranche?
3. Do they need the core product to run on Solana, or is one feature on Solana enough?

## 2. What they will look for

- **Real on-chain usage.** Funds measure wallets, transactions and retained users, not decks. A small working pilot beats a big migration plan.
- **Consumer apps with distribution.** Solana's current story centres on consumer apps, payments and ticketing. A nightlife/festival app with a real audience fits that story well.
- **A good reason to be on-chain.** The best-known Solana ticketing company, KYD Labs (Ticketmaster alumni; Charli XCX, Travis Scott), said publicly that "NFT ticketing doesn't work" as a collectible pitch. It moved to a protocol built around **financing venues and promoters (TIX)**. Expect the fund to ask: *what does the blockchain do for a raver that a database can't?*
- **The team and the traction.** Monthly users, cities, partners (promoters, venues, festivals).

## 3. Due-diligence risk in the current codebase

`app/api/events/route.ts` pulls listings from **Resident Advisor's private GraphQL API** using browser-spoofed headers. Investors will ask where the data comes from. Before pitching, either:
- get a data partnership (RA, Dice, Skiddle, or promoters directly), or
- move to promoter-submitted events, which is also better for on-chain claims because the promoter signs the event.

## 4. Moving the cards on-chain (proposed)

_The repo doesn't have a "cards" feature yet. This assumes cards are per-event or per-user cards in Nekta, like a rave passport. Adjust if they're something else._

### Principle: users shouldn't notice the blockchain
- Mint as **compressed NFTs** (Metaplex Bubblegum) or **Metaplex Core** assets. Minting costs fractions of a cent, so millions of cards are affordable.
- **Embedded wallets** (Privy, Dynamic, Phantom embedded, or Web3Auth). Users sign in with email or Apple, and a wallet is created silently.
- **Nekta pays the fees** through a fee-payer/relayer. Users never hold SOL.
- Card artwork and metadata go to Arweave or Irys. Fast reads still come from our own DB plus a Helius/Triton DAS indexer.

### What going on-chain unlocks (lead the pitch with these)
1. **Proof-of-attendance "rave passport":** a verifiable history of events attended, scanned at the door or claimed by GPS plus a promoter-signed QR code.
2. **Promoter and artist perks:** holders of card X get presale access, guestlist spots or merch drops. Any partner can check this without integrating with our backend.
3. **Rewards and payments:** Solana Pay / USDC for tickets and drinks, with loyalty points tied to the cards.
4. **Optional later step, ticketing:** resale with price caps and royalties. Partner with KYD/TICKS here rather than rebuilding it.

### Phased plan
| Phase | Scope | Rough effort | Use for the pitch |
|---|---|---|---|
| 0 | Fix data source and ship cards off-chain | 2–4 wks | Traction baseline |
| 1 | Devnet pilot: embedded wallet + compressed-NFT card for one event | 2–3 wks | Live demo for the fund call |
| 2 | Mainnet: a real event or festival partner, with claim at the door | 4–6 wks | First on-chain metrics |
| 3 | Perks, gating and Solana Pay | 6–8 wks | Retention and revenue story |

## 5. Next steps
1. Get the contact's name and fund from Jack, then look up their recent Solana deals.
2. Write a one-page deck: problem → Nekta (event discovery + card marketplace) → collectible cards and passes on Solana → traction → ask.
3. Build the Phase 1 devnet demo before the call if possible.
4. In parallel, join the next Colosseum hackathon as a backup route to $250k.
5. Apply for a Solana Foundation or Superteam grant for the open-source parts, e.g. a proof-of-attendance claim program.

## Sources
- [Solana Ventures – VC Sheet](https://www.vcsheet.com/fund/solana) · [vcbacked](https://www.vcbacked.co/directory/investors/solana-ventures) · [f4.fund](https://f4.fund/firms/solana-ventures) · [Waveup](https://hub.waveup.com/funds/solana-ventures)
- [Solana Foundation $50k YC grants](https://coinfomania.com/solana-empowers-yc-builders-with-50k-grants-from-solanafndn/)
- [Foundation defends builder support / grant sizes](https://coinpaper.com/15701/solana-foundation-defends-builder-support-as-ecosystem-growth-faces-scrutiny)
- [How to apply to Solana Foundation grants 2026 (third-party guide)](https://hub.causo.ai/guides/how-to-apply-to-solana-foundation-grants-2026)
- [Colosseum raises $60M – The Block](https://www.theblock.co/post/301795/solana-accelerator-colosseum-first-fund) · [5th cohort](https://solanacompass.com/news/colosseum-admits-21-startups-to-its-5th-accelerator-cohort-drawn-from-the-frontier-hackathon-and-eternal-sprint)
- [KYD Labs: "NFT ticketing doesn't work" – Breakpoint 25](https://solanacompass.com/learn/breakpoint-25/nft-ticketing-doesnt-work-and-were-selling-to-ticketmaster-kyd-labs) · [TIX protocol](https://yellow.com/news/tix-protocol-launches-solana-based-financing-network-for-live-events-industry)
- [Onchain entertainment ticketing (cNFT costs)](https://paragraph.xyz/@billysunshine.eth/onchain-ticketing)
