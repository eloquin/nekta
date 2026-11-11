# Should Nekta host dubs "on the blockchain"?

*Research note, 11 November 2026. Written for a non-technical founder. Claims I couldn't confirm from primary sources are marked **(uncertain)**. Some official documentation sites, including docs.audius.co, were blocked by the network during research, so parts of this note rest on search-result summaries rather than the full pages.*

## Short answer

No. Nobody, Audius included, stores audio files *on* a blockchain, because it's far too expensive and slow. Encrypting a file and putting it on decentralised storage protects the file while it's stored and sent, but it can't stop someone recording the track when it plays, and some of those storage networks make it impossible to take a track down. Managers and labels trust promo platforms that give **stream-only playback, expiring personal links, access logs and per-recipient watermarks**. The best design for Nekta is a hybrid: the Solana dub card is the *key*, and the audio stays off-chain, encrypted, streamed and watermarked for each holder.

## 1. How Audius actually works

- **Audio isn't on-chain.** Audius audio lives on node operators' servers. These were originally "Content Nodes" using IPFS-style content addressing, with "Discovery Nodes" doing the indexing. The blockchain holds token, staking and ownership data, not the files.
- **Rebrand.** The protocol was renamed the **Open Audio Protocol**, and an Open Audio Foundation now sits behind it. Under the 1.0 release, content and discovery nodes were merged into staked "validator" nodes, with storage proofs added. Sources disagree on the date: one says late 2025, Solana Compass says October 2025, and another says January 2026 **(exact date uncertain)**. The Audius consumer app still runs. In 2025 it added "Artist Coins" and signed a licensing deal with Downtown.
- **Gated content.** Artists can gate a track behind holding a collectible, a USDC purchase, or another condition. According to Audius's developer docs, the gate works through **signatures**: a trusted server or node checks that you qualify and signs your request, and the storage node refuses unsigned requests (it returns HTTP 401). That makes it **access control by the node, not cryptography**. Audius's 2020 whitepaper proposed per-user encryption, and one third-party glossary says tracks are encrypted at rest, but I couldn't confirm that in current Audius documentation **(uncertain; assume gated audio isn't encrypted at rest unless verified)**.
- **Incidents.** In July 2022 a bug in contract initialisation let an attacker pass a malicious governance proposal and drain about 18 million AUDIO tokens, worth about $6M on paper, though roughly $1.1M was realised. This was a smart-contract failure, not an audio leak. I found no documented leak of gated audio **(absence of evidence only)**.

**Lesson for Nekta:** the leading "music on blockchain" platform uses the chain for ownership and payments, and an ordinary server-side check to protect gated audio.

## 2. Decentralised storage options

| Option | Public by default? | Can you delete? | Cost (indicative) | Latency for streaming |
|---|---|---|---|---|
| **Arweave** | Yes | No. Built to be permanent. Gateways can choose not to serve a file, but the data stays on the network. | One-off fee, historically about $3–8/GB (2023 figure) | Moderate, depends on the gateway |
| **Irys** (formerly Bundlr) | Yes | Permanent tier: no. It also has a cheaper temporary tier. | Whitepaper lists about $2.50/GB permanent **(undated)** | Moderate |
| **IPFS / Filecoin** | Yes. Anyone with the content ID can fetch the file. | Only by "unpinning" everywhere. You can't force other copies to disappear. | Cheap via pinning services | Variable, often slow without a CDN |
| **Shadow Drive (GenesysGo)** | Yes, by URL | Yes, deletable | Paid in SHDW tokens | Reasonable |

Shadow Drive was still being promoted as V2 in October 2024. I found no 2025–26 status update **(uncertain whether it's actively maintained)**.

Key point: on Arweave, an unencrypted dub is effectively **published to the world forever**. Even an encrypted copy stays permanently, so if the encryption scheme or a key is ever compromised, it can't be pulled.

## 3. Encryption and token-gated keys

The idea is to encrypt the file on the uploader's device and release the decryption key only to wallets that hold a given token. Lit Protocol offers this, with Solana support, through a decentralised network of key-holders ("threshold encryption"). Lit's token fell about 99% from its October 2025 high by July 2026 **(project health uncertain; don't build a hard dependency on it without checking)**.

**The analog hole.** Encryption protects a track while it's stored and while it's sent. Once a holder presses play, the audio is decoded on their device, and they can record it with a cable, a screen recorder or a phone. No encryption scheme prevents this. The only thing that helps afterwards is **knowing who leaked it**, which means watermarking.

## 4. What the industry actually uses for promos

- **Haulix**: per-recipient inaudible watermarks, 10–15 per track, using technology licensed from Fraunhofer. If a leak appears, you send Haulix the pirated file and it identifies the recipient.
- **DISCO**: stream-only mode, password, link expiry, per-track stream and download stats, and a paid watermarking add-on. DISCO's own FAQ admits that a **shared public link can't trace an individual**. Only personalised links can.
- **Inflyte** (ProLink): recipient-specific links, stream-only or download, expiry and access protection, engagement reports. It deliberately doesn't offer public links.
- **Label Worx / PromoJukeBox**: a unique code watermarked into every downloaded file.
- **LabelRadar** is mainly for demo submissions and **Label Engine** for promo email and feedback. I found no evidence that either watermarks files **(uncertain)**.
- **SoundCloud private links and Dropbox/Drive** are widely used because they're easy, but there's no watermark and anyone who has the link can forward it.
- **BMAT, Audible Magic and Digimarc** are content-recognition and watermark companies that labels use to *detect* tracks circulating online. I didn't verify their specific promo products in this research **(uncertain)**.

**What managers trust:** platforms where (a) recipients usually stream rather than download, (b) every link is personal and logged, (c) links expire or can be revoked, and (d) a leak can be traced to a person. Their confidence comes from deterrence and accountability, not from any claim that leaks are impossible.

## 5. "A link, not an upload": is a link safer?

Partly. A Dropbox link keeps the master out of *Nekta's* hands, but the trust just moves to Dropbox, and a plain link is usually shareable, permanent and untraceable. Options where Nekta never holds the master:

1. **Stream from the manager's storage via signed URL.** The manager keeps the file in their own bucket or Drive. Nekta issues short-lived signed links only to card holders. The catch is that there's no watermark, and Nekta's servers still see the audio in transit.
2. **Watermarked preview transcode.** Nekta fetches the master once, makes a low-bitrate (e.g. 128–192 kbps) per-holder watermarked stream, and deletes the master. Holders never get the WAV, so a leak is low quality *and* traceable. This is closest to what Haulix and DISCO do.
3. **Client-side encryption.** The manager encrypts in their browser and Nekta stores only ciphertext. Keys are released per holder by Nekta or a Lit-style network. Nekta can truthfully say it **cannot** listen to the file. The drawback is that per-holder watermarking then has to happen on the listener's device, which is harder and less robust **(feasibility uncertain)**.

## 6. Comparison

| | Blockchain / encrypted decentralised storage | Conventional secure promo platform (or a Nekta hybrid) |
|---|---|---|
| **Security at rest** | Strong if encrypted. A public file if not. | Strong (encrypted cloud storage, access control) |
| **Leak tracing** | None by itself. Same file for everyone. | Per-recipient watermarks identify the leaker. |
| **Revocation** | Hard. A key already released can't be taken back, and cached files persist. | Instant: kill the link or session. |
| **Takedown / deletion** | Impossible on Arweave or Irys permanent storage. Unreliable on IPFS. | Simple delete |
| **Cost** | Upfront per GB, paid in crypto, plus key-network fees | Cloud storage and CDN, cheap at dub-sized volumes |
| **UX for managers** | Wallets, unfamiliar tools, "forever" anxiety | Familiar: upload or link, see who listened |
| **Copyright / legal** | DMCA notices can't remove the data. Nekta could be blamed for infringing or leaked content it can't remove. | Normal DMCA / notice-and-takedown compliance |
| **Playback ("analog hole")** | Not protected | Not protected, but deterred by watermarks |

## 7. Recommendation for Nekta's dub cards

Don't put audio on-chain or on permanent storage. Use **the card as the key**:

1. **On Solana:** the compressed-NFT dub card records ownership and the rights metadata (title, artist, expiry), never the audio and never a public audio URL.
2. **Off-chain:** audio sits in private, encrypted cloud storage. Ideally Nekta keeps only a watermarkable preview and never the master (option 2), or holds ciphertext only (option 3) for managers who want that.
3. **Playback:** the holder signs in with their Privy wallet. Nekta checks on-chain that they hold the card and issues a short-lived streaming session with a **watermark unique to that holder**. The app is stream-only, with no downloads.
4. **Revocation:** when a card is burned, expires or is transferred, the next ownership check fails and access stops. A manager can also hit "pull track" to delete the file everywhere, which is impossible on Arweave.
5. **Logging:** every play is logged per wallet, so managers see who listened, when and how often.

This is roughly how Audius's own gating works: a signature check guards ordinary storage. Nekta would add the watermarking and revocation that promo platforms like Haulix and DISCO offer.

**Pitch to managers:** "Your master never leaves your hands. You share a link or upload once, and we turn it into a stream-only, lower-quality preview. Every DJ hears a copy invisibly stamped with their identity, so if it leaks, you'll know who leaked it. Access is tied to a dub card you control. Burn it, let it expire, or hit 'pull track', and it's gone instantly. You see every play. Nothing is put on a blockchain where it could never be deleted. The blockchain only records who holds the key."

## Sources

- Open Audio Protocol announcement: https://openaudio.substack.com/p/introducing-the-open-audio-protocol ; https://blog.audius.co/posts/openaudioprotocol
- Open Audio Foundation: https://audius.org ; https://openaudiofoundation.org/token
- Solana Compass, Audius: https://solanacompass.com/projects/audius
- Audius gated access docs (fetch blocked; read via search summary): https://docs.audius.co/developers/guides/gate-release-access ; https://docs.audius.org/developers/api/stream-track
- Audius availability guide: https://blog.audius.co/article/guide-to-audius-availability-settings
- Audius whitepaper: https://www.makingascene.org/wp-content/uploads/2022/01/AudiusWhitepaper.pdf
- Audius–Downtown deal: https://www.digitalmusicnews.com/2025/04/30/downtown-licenses-content-to-audius/
- 2022 hack: https://decrypt.co/105913/how-audius-was-hacked-6m-ethereum-tokens ; https://cointelegraph.com/news/hacker-drains-1-08m-from-audius-following-passing-of-malicious-proposal
- AR.IO content moderation: https://docs.ar.io/gateways/moderation ; Arweave blacklist: https://docs.arweave.org/developers/mining/overview/blacklist
- IPFS moderation (USENIX Security 2024): https://usenix.org/conference/usenixsecurity24/presentation/sokoto
- Arweave/Irys pricing: https://ardrive.io/?p=744 ; https://irys.xyz/assets/IrysWhitepaper.pdf ; https://ardrive.io/articles/can-data-really-be-stored-forever/
- Shadow Drive V2: https://solanacompass.com/learn/Solfate/decentralized-cloud-storage-on-the-solana-network-introducing-shadow-drive-v2-64
- Lit Protocol on Solana: https://spark.litprotocol.com/solana/ ; https://coinmarketcap.com/cmc-ai/lit-protocol/what-is
- Haulix watermarking: https://support.haulix.com/help/watermarking-explained ; https://haulixdaily.com/2014/07/haulix-web-based-watermarking-process
- DISCO watermarking FAQ: https://help.disco.ac/articles/71674-watermarking-faq ; https://school.disco.ac/article/watermarking
- Inflyte ProLink and public links: https://inflytehelp.groovehq.com/help/introducing-inflyte-prolink ; https://inflytehelp.groovehq.com/help/why-inflyte-doesnt-allow-public-links
- Label Worx watermarking: https://support.label-worx.com/hc/en-us/articles/115006001309-How-do-I-enable-Watermarking-on-my-PromoWorx-campaigns
- Label Engine promo tools: https://help.label-engine.com/en/articles/4681444-what-type-of-promotion-tools-does-label-engine-provide
- LabelRadar: https://greenroom.beatport.com/labels/labelradar
