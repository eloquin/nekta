# Why NFT ticketing struggled, and what's working now

*Research note for Nekta, 11 Nov 2026. Method caveat: the research environment could run web searches but could not open most pages directly (Solana Compass, Cointelegraph and The Defiant were blocked). Claims below come from search-result excerpts of the listed sources. Treat figures as company-reported unless stated otherwise. **Uncertain** marks claims I could not confirm.*

## 1. The headline case: KYD Labs (Solana)

KYD Labs was founded by Ahmed Nimale and David Barrick, both ex-Ticketmaster/Live Nation/Vivid Seats. It raised a $7M seed led by a16z crypto in May 2025, and it is probably the most important signal here.

- **Accelerate (May 2025), "The Rise of the People's Ticketmaster":** Nimale argued that Ticketmaster's real moat is not software. It is **upfront cash advances to venues**, given in exchange for long exclusive contracts and control of the fan data. (Source: Solana Compass AI summary, so this is the argument, not a quote.)
- **Breakpoint (Dec 2025), "NFT Ticketing Doesn't Work and We're Selling to Ticketmaster":** KYD said it had moved all its tickets off compressed NFTs (cNFTs) to a new standard, the **TICKS protocol**. The widely repeated line "NFT ticketing dies today" comes from Solana Compass's *AI-generated* notes, so treat the exact wording as **uncertain**. The same notes say NFTs turned tickets into "collectibles sold to venues who need liquidity", that resale protections were easy to get around, and that infrastructure costs were high. These are AI paraphrases, not verified quotes.
- **TIX:** This is a DeFi lending and settlement layer. Venues borrow against future ticket sales, and outside lenders earn yield from real ticket cash flows, with **no exclusivity required**. Reported traction is about 300k tickets, $8–10M in sales (the sources conflict), and about $2M in venue financing with zero defaults. All of that is self-reported. Cointelegraph said mainnet was expected by mid-2026. I could not confirm it launched (**uncertain**). Nimale is now described as CEO of TIX. Commenting on the Live Nation–DOJ settlement, he said venues "depend on these companies for financing and advances", which makes real competition "nearly impossible" (via TheWrap excerpt).

**Takeaway:** the best-funded Solana ticketing team concluded that the token was never the product. The product was the **money flow** (who funds the venue) and the **settlement rail**. "On-chain" survives as plumbing. "NFT" as the user-facing idea was dropped.

## 2. What happened to the others

| Project | What it tried | What happened |
|---|---|---|
| **GET Protocol → OPEN Ticketing** | White-label NFT tickets on Polygon, wallet hidden from fans | Real volume: 1M on-chain tickets by Oct 2021, "4M+" claimed later. Rebranded to OPEN ($OPN) in March 2024. The token fell about 89% in 2025. Current operating status is **uncertain**. |
| **Ticketmaster (Flow, Ethereum)** | 2022: commemorative NFTs *linked to* tickets on Flow (e.g. 70k+ Super Bowl LVI keepsakes). 2023: token-gated presales (Avenged Sevenfold "Deathbats Club"). | Stayed a marketing add-on. Ticketmaster's real anti-fraud tool is **SafeTix**, which is not blockchain at all. |
| **Coachella / Tomorrowland (2022)** | NFT lifetime passes ("Coachella Keys", about $1.47M sold) and collectibles via FTX US, on Solana | When FTX collapsed in Nov 2022, holders who hadn't withdrawn lost access. Custodial risk wiped out the "you own it" promise. Coachella tried NFTs again with OpenSea in 2024. |
| **Tokenproof** | Wallet-based check-in and token-gating | Tech and team acquired by Yuga Labs in Dec 2024. Ticketing was not a standalone business. |
| **YellowHeart, SeatlabNFT, XP (Solana)** | NFT ticket marketplaces / capped resale | No shutdown news found, but no meaningful recent traction found either (**uncertain**). Coverage is mostly from 2022–24. |
| **POAP** | Free proof-of-attendance badges (e.g. Warner Music 2022) | Survives as a niche "I was there" layer, not as ticketing. |
| **HUG** | Not found as a ticketing product. Only appears as a Web3 community issuing POAPs (**uncertain**). | — |
| **DICE** (not crypto) | Tickets locked to phone and app, activated near showtime, resale only through a face-value waitlist | Widely credited with sharply cutting touting at club and indie level. **Central database plus policy.** |

## 3. Root causes

**Industry structure.** Primary inventory is controlled by whoever has the venue contract. Ticketmaster wins those contracts with advances and exclusivity, not better tech. A better ticket format can't get in if the venue is locked into a multi-year deal. KYD's pivot is a direct response to this.

**Economics.** Promoters need cash *before* the show. NFT ticketing offered collectibles and resale royalties, which arrive late, are small, and are uncertain. On-chain royalties can also be skipped on marketplaces that don't enforce them (this happened across the NFT market in 2022–23). Someone also has to pay for minting and infrastructure. Solana cNFTs made minting nearly free, but integration, support and door hardware still cost money.

**UX.** Wallets, seed phrases, gas, and "connect your wallet" at checkout all add friction. At the door, staff need a fast, offline-tolerant scan. Every successful project hid the wallet completely, which means the NFT itself added little that the fan could see.

**Do the security claims hold up?**
- *Screenshot fraud:* an NFT does **not** stop it. A static QR code tied to an NFT can be screenshotted like any other. What stops screenshots is **rotating, time-based codes plus an app tied to the device and account**, as with SafeTix (codes refresh about every 15s) and DICE. Ticketmaster's own CIO framed the entry token as "the relationship with the account", not the barcode.
- *Scams:* most fraud happens off-platform (Venmo/Zelle sellers on social media). On-chain provenance only helps if buyers check it, and only if all transfers go through the official system.
- *"Can't resell it":* this is a **policy**, and any database can enforce it. DICE proves it. A blockchain adds *public verifiability* and *portability across apps*, but a non-transferable token is just a database row everyone can read.
- *Identity:* tying a ticket to a real person still needs KYC or account binding off-chain.

**Regulation.**
- **New York** (Arts & Cultural Affairs Law §25.30): operators can't offer *only* non-transferable paperless tickets. Buyers must be offered a transferable option. A "soulbound" ticket can't be the only option in NY.
- **UK:** in Nov 2025 the government committed to banning resale above face value (plus unavoidable fees) and capping resale-platform fees. Whether it is now in force is **uncertain**.
- **EU:** rules vary by country. Several countries (e.g. Ireland, France, Belgium, Italy) restrict above-face-value resale (**from memory, not verified here**).
- Price caps help platforms that can *enforce* them. Programmable caps are a genuine fit for a blockchain, but a database does the same job.

**Stigma.** The 2022 crash, FTX (which hit Coachella and Tomorrowland directly) and scams made "NFT" a negative word for fans, artists and venues. Most survivors stopped saying it.

## 4. Where the thesis is right

- **Provenance and portability:** a ticket or pass that any app can verify, and that outlives one vendor, is real value. Today's ticket data is locked inside Ticketmaster.
- **Programmable resale rules:** caps, cuts for artists and promoters on official resale, and transfer windows. These are easier to make consistent and auditable on a shared ledger, especially as UK/EU price caps arrive.
- **Ownership after the night:** proof-of-attendance and collectibles that build a fan history are something a database ticket doesn't do well across platforms.
- **Financing:** KYD/TIX shows the most interesting on-chain angle is **capital for venues and promoters**, not the ticket image.

**What makes it work:** invisible embedded wallets (email or phone login); rotating QR codes on top of the token; account and device binding; resale only through official channels with enforced caps; royalties paid on that official resale; a POAP-style "afterlife"; and never leading with the word "NFT".

**Tickets or identity layer?** The evidence points to the **fan identity and loyalty layer**. Ticket *issuing* is a contract fight with incumbents. *Verifiable attendance history* ("this person went to 40 techno nights") is something nobody owns yet. Promoters would use it for presales, guest lists and perks, and DICE/Ticketmaster can't easily share it across platforms.

## Implications for Nekta

1. **Don't compete on issuing tickets at first.** Plug in alongside DICE, RA, Shotgun or a promoter's existing ticketing, and build the pass and perk layer on top.
2. **Pitch access passes and DJ cards as membership, not "NFTs".** Backstage, skip-the-queue, guest list and drink perks are utility promoters already understand. Use Solana cNFTs or a similar cheap standard underneath, with an invisible wallet.
3. **Security comes from the door app, not the chain.** Rotating QR, device binding and staff scanning must work offline. The chain only proves provenance.
4. **Make "no resale" or "capped resale" a promoter setting**, and design for NY's transferable-option rule and UK face-value caps from day one.
5. **Your moat is attendance history.** Verified "I was there" data powers presales, perks and loyalty across promoters, and promoters will pay for that.
6. **Watch KYD/TIX.** If promoter financing on-chain works, Nekta's attendance and demand data could feed into it later (e.g. demand signals for advances).

## Sources

*All viewed via search-result excerpts only; direct page fetches were blocked.*

- https://solanacompass.com/learn/breakpoint-25/nft-ticketing-doesnt-work-and-were-selling-to-ticketmaster-kyd-labs (AI-generated notes)
- https://solanacompass.com/learn/accelerate-25/ship-or-die-at-accelerate-2025-the-rise-of-the-peoples-ticketmaster (AI-generated notes)
- https://thedefiant.io/news/nfts-and-web3/solana-ticketing-platform-kyd-labs-launches-tix
- https://cointelegraph.com/news/tix-defi-onchain-settlement-live-event-ticketing
- https://kydlabs.notion.site/EXT-Memo-TIX-Credit-Protocol-30d98bd7f93a809a8a72f8892ebbbea3
- https://dev.coindesk.com/consensus-toronto-2025-coverage/2025/05/15/a16z-crypto-leads-usd7m-round-in-kyd-labs-that-aims-to-transform-ticketing-industry
- https://www.thewrap.com/creative-content/music/live-nation-ticketmaster-settlement-explained-whats-next/
- https://musically.com/2023/08/02/kyd-labs-talks-web3-ticketing-fan-loyalty-and-winning-industry-trust/
- https://medium.com/get-protocol/faq-opn-migration-072144543596
- https://decrypt.co/148254/get-protocol-raises-4-5-million-take-ticketmaster-nft-tickets
- https://coincodex.com/crypto/open-ticketing-ecosystem/
- https://www.theblock.co/post/166794/ticketmaster-taps-dapper-labs-flow-blockchain-to-release-tickets-as-nfts-sbj
- https://techcrunch.com/?p=1828729 (SafeTix)
- https://blog.incogni.com/is-ticketmaster-safe/
- https://www.billboard.com/pro/coachella-nfts-stuck-ftx-exchange-bankruptcy
- https://www.digitalmusicnews.com/2024/03/05/coachella-is-trying-nft-concert-benefits-again/
- https://cointelegraph.com/news/yuga-labs-tokenproof-acquisition-nft-innovation
- https://www.digitalmusicnews.com/2022/04/14/warner-music-group-poap-partnership/
- https://medium.com/tixel/how-to-resell-a-dice-ticket-f8369f233460
- https://tickts.co.uk/blog/tickts-vs-dice (competitor page)
- https://nysenate.gov/legislation/laws/ACA/25.30
- https://www.musicbusinessworldwide.com/uk-moves-to-ban-resale-of-tickets-above-original-price/
- https://news.pollstar.com/2025/11/19/uk-makes-plans-to-ban-for-profit-ticket-resale-official-reactions/
- https://solana.com/news/case-study-xp
