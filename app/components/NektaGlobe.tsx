'use client'

import { useEffect, useRef, useCallback, useState } from 'react'

const LAND_B64 = "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"
const MAP_W = 720, MAP_H = 360

async function buildLandMap(): Promise<Uint8Array> {
  const bin = atob(LAND_B64)
  const bytes = new Uint8Array(bin.length)
  for (let i = 0; i < bin.length; i++) bytes[i] = bin.charCodeAt(i)
  const ds = new DecompressionStream('deflate')
  const writer = ds.writable.getWriter()
  writer.write(bytes); writer.close()
  const chunks: Uint8Array[] = []
  const reader = ds.readable.getReader()
  while (true) { const { done, value } = await reader.read(); if (done) break; chunks.push(value) }
  const total = chunks.reduce((s, c) => s + c.length, 0)
  const result = new Uint8Array(total)
  let off = 0; for (const c of chunks) { result.set(c, off); off += c.length }
  return result
}

function isLand(map: Uint8Array, lat: number, lon: number): boolean {
  while (lon > 180) lon -= 360; while (lon < -180) lon += 360
  const col = Math.min(MAP_W-1, Math.max(0, Math.floor((lon+180)/360*MAP_W)))
  const row = Math.min(MAP_H-1, Math.max(0, Math.floor((90-lat)/180*MAP_H)))
  return map[row*MAP_W+col] === 1
}

interface Hotspot { name: string; desc: string; lat: number; lon: number; tier: 0|1|2 }
interface EventData { id: string; title: string; startTime: string; venue?: { name: string }; artists?: { name: string }[] }

// Precise GPS coordinates verified
const HOTSPOTS: Hotspot[] = [
  { name: 'London',      desc: 'Fabric · Egg · Printworks',         lat: 51.51,  lon: -0.13,   tier: 2 },
  { name: 'Berlin',      desc: 'Berghain · Tresor · Watergate',     lat: 52.52,  lon: 13.40,   tier: 2 },
  { name: 'Ibiza',       desc: 'DC10 · Pacha · Amnesia',            lat: 38.91,  lon: 1.43,    tier: 2 },
  { name: 'Amsterdam',   desc: 'Shelter · Awakenings · ADE',        lat: 52.37,  lon: 4.90,    tier: 1 },
  { name: 'Paris',       desc: 'Rex Club · Concrete · Batofar',     lat: 48.86,  lon: 2.35,    tier: 1 },
  { name: 'Bristol',     desc: 'Motion · Marble Factory · Thekla',  lat: 51.45,  lon: -2.59,   tier: 1 },
  { name: 'Barcelona',   desc: 'Razzmatazz · Input · Apolo',        lat: 41.39,  lon: 2.15,    tier: 1 },
  { name: 'Detroit',     desc: 'Movement Festival · CODA',          lat: 42.33,  lon: -83.05,  tier: 1 },
  { name: 'Chicago',     desc: 'Smart Bar · Spy Bar',               lat: 41.88,  lon: -87.63,  tier: 1 },
  { name: 'New York',    desc: 'Output · House of Yes · Nowadays',  lat: 40.71,  lon: -74.01,  tier: 1 },
  { name: 'Rio',         desc: 'Fosfobox · Green Valley',           lat: -22.91, lon: -43.17,  tier: 1 },
  { name: 'Sao Paulo',   desc: 'D-Edge · Lov · The Week',           lat: -23.55, lon: -46.63,  tier: 1 },
  { name: 'Bangkok',     desc: 'De Commune · Onyx · Beam',          lat: 13.75,  lon: 100.50,  tier: 1 },
  { name: 'Koh Phangan', desc: 'Full Moon Party · Shiva Moon',      lat: 9.73,   lon: 100.01,  tier: 1 },
  { name: 'Melbourne',   desc: 'Revolver · Brown Alley · 161',      lat: -37.81, lon: 144.96,  tier: 1 },
  { name: 'Tokyo',       desc: 'Womb · Contact · Dommune',          lat: 35.69,  lon: 139.69,  tier: 1 },
  { name: 'Bali',        desc: 'Potato Head · La Favela',           lat: -8.34,  lon: 115.09,  tier: 0 },
  { name: 'Cape Town',   desc: 'Shimmy Beach · Cause Effect',       lat: -33.93, lon: 18.42,   tier: 0 },
  { name: 'Prague',      desc: 'Ankali · Cross Club',               lat: 50.08,  lon: 14.44,   tier: 0 },
  { name: 'Zurich',      desc: 'Hive · Zukunft · Supermarket',      lat: 47.38,  lon: 8.54,    tier: 0 },
  { name: 'Dubai',       desc: 'White Dubai · Soho Garden',         lat: 25.20,  lon: 55.27,   tier: 0 },
  { name: 'Seoul',       desc: 'Cakeshop · Contra · Boogie Nights', lat: 37.57,  lon: 126.98,  tier: 0 },
  { name: 'LA',          desc: 'Exchange · Catch One · EDC',        lat: 34.05,  lon: -118.24, tier: 0 },
  { name: 'Toronto',     desc: 'Coda · Rebel · Velvet Underground', lat: 43.65,  lon: -79.38,  tier: 0 },
  { name: 'Bogota',      desc: 'Baum · Video Club · Armando',       lat: 4.71,   lon: -74.07,  tier: 0 },
]

function buildDots(map: Uint8Array) {
  const dots: { lat: number; lon: number }[] = []
  const STEP = 0.9
  for (let lat = -90+STEP/2; lat < 90; lat += STEP) {
    const num = Math.max(1, Math.round(360*Math.cos(lat*Math.PI/180)/STEP))
    for (let i = 0; i < num; i++) {
      const lon = -180 + (i+0.5)*360/num
      if (isLand(map, lat, lon)) dots.push({ lat, lon })
    }
  }
  return dots
}

const SIZE = 580, GLOB_R = 270, MAX_THETA = 0.48

// Project lat/lon to screen xy — returns null if on back of globe
function makeProjector(rotLon: number, rotTheta: number, scale: number) {
  const CX = SIZE/2, CY = SIZE/2, R = GLOB_R*scale
  return function project(lat: number, lon: number) {
    const dLon = ((lon - rotLon + 540) % 360) - 180
    if (Math.abs(dLon) >= 90) return null
    const la = lat*Math.PI/180, lo = dLon*Math.PI/180
    const th = rotTheta
    const x = Math.cos(la)*Math.sin(lo)
    const y = Math.sin(la), z = Math.cos(la)*Math.cos(lo)
    const y2 = y*Math.cos(th)+z*Math.sin(th)
    const z2 = -y*Math.sin(th)+z*Math.cos(th)
    if (z2 < 0.01) return null
    return { x: CX+x*R, y: CY-y2*R, z: z2 }
  }
}

function findClickedHotspot(mx: number, my: number, rotLon: number, rotTheta: number, scale: number): Hotspot | null {
  const project = makeProjector(rotLon, rotTheta, scale)
  let best: Hotspot | null = null, bestD = 9999
  for (const h of HOTSPOTS) {
    const p = project(h.lat, h.lon)
    if (!p) continue
    const d = Math.hypot(mx - p.x, my - p.y)
    // Hit radius proportional to tier and depth
    const hitR = (h.tier === 2 ? 16 : h.tier === 1 ? 13 : 10) * scale * p.z
    if (d < hitR && d < bestD) { bestD = d; best = h }
  }
  return best
}

export function NektaGlobe() {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const rotLonRef = useRef(10), targetLonRef = useRef(10)
  const rotThetaRef = useRef(0), targetThetaRef = useRef(0)
  const scaleRef = useRef(1), targetScaleRef = useRef(1)
  const pointerDownRef = useRef(false)
  const lastXRef = useRef(0), lastYRef = useRef(0)
  const pinchRef = useRef(0), rafRef = useRef(0)
  const didDragRef = useRef(false)
  const dotsRef = useRef<{ lat: number; lon: number }[]>([])
  const pulseRef = useRef(0)
  const spinningRef = useRef(true)
  const activeHotspotRef = useRef<Hotspot | null>(null)
  const activeScaleRef = useRef(1)

  const [activeHotspot, setActiveHotspot] = useState<Hotspot | null>(null)
  const [spinning, setSpinning] = useState(true)
  const [events, setEvents] = useState<Record<string, EventData[]>>({})
  const [visible, setVisible] = useState(false)
  const [loading, setLoading] = useState(true)

  useEffect(() => { spinningRef.current = spinning }, [spinning])
  useEffect(() => { activeHotspotRef.current = activeHotspot }, [activeHotspot])

  const flyTo = useCallback((h: Hotspot) => {
    // Smart rotation: find shortest angular path to city
    // Normalise current lon to same range, then pick direction that covers least distance
    const current = rotLonRef.current
    const target = h.lon
    // Difference mod 360, then shift to -180..180 range for shortest path
    let diff = ((target - current) % 360 + 360) % 360
    if (diff > 180) diff -= 360
    targetLonRef.current = current + diff
    targetThetaRef.current = Math.max(-MAX_THETA, Math.min(MAX_THETA, -(h.lat*Math.PI/180)*0.3))
    setSpinning(false)
    setActiveHotspot(h)
  }, [])

  const dismiss = useCallback(() => {
    setActiveHotspot(null)
    setSpinning(true)
  }, [])

  useEffect(() => {
    fetch('/api/events').then(r => r.json()).then(setEvents).catch(() => {})
  }, [])

  // Spacebar + Escape
  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.target !== document.body && e.target !== document.documentElement) return
      if (e.code === 'Space') { e.preventDefault(); setSpinning(s => !s) }
      if (e.code === 'Escape') dismiss()
    }
    window.addEventListener('keydown', onKey)
    return () => window.removeEventListener('keydown', onKey)
  }, [dismiss])

  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) return
    const ctx = canvas.getContext('2d')!
    canvas.width = SIZE; canvas.height = SIZE

    buildLandMap().then(map => {
      dotsRef.current = buildDots(map)
      setLoading(false)
      setTimeout(() => setVisible(true), 100)
    })

    function draw() {
      if (spinningRef.current && !pointerDownRef.current) targetLonRef.current += 0.1
      rotLonRef.current += (targetLonRef.current - rotLonRef.current) * 0.06
      rotThetaRef.current += (targetThetaRef.current - rotThetaRef.current) * 0.06
      scaleRef.current += (targetScaleRef.current - scaleRef.current) * 0.08
      pulseRef.current += 0.04
      // Ease activeScaleRef toward 2 if active, 1 if not
      const targetAS = activeHotspotRef.current ? 2.2 : 1
      activeScaleRef.current += (targetAS - activeScaleRef.current) * 0.08

      const R = GLOB_R * scaleRef.current
      const CX = SIZE/2, CY = SIZE/2
      const project = makeProjector(rotLonRef.current, rotThetaRef.current, scaleRef.current)

      ctx.clearRect(0, 0, SIZE, SIZE)

      // Sphere bg
      const bg = ctx.createRadialGradient(CX-40, CY-50, 10, CX, CY, R)
      bg.addColorStop(0, '#180604'); bg.addColorStop(0.65, '#0c0202'); bg.addColorStop(1, '#040101')
      ctx.beginPath(); ctx.arc(CX, CY, Math.min(R, SIZE/2-1), 0, Math.PI*2)
      ctx.fillStyle = bg; ctx.fill()

      ctx.save()
      ctx.beginPath(); ctx.arc(CX, CY, Math.min(R-1, SIZE/2-2), 0, Math.PI*2); ctx.clip()

      const dotR = Math.max(0.7, 1.2*scaleRef.current)

      // Land dots
      for (const dot of dotsRef.current) {
        const p = project(dot.lat, dot.lon)
        if (!p) continue
        ctx.beginPath(); ctx.arc(p.x, p.y, dotR*(0.55+p.z*0.45), 0, Math.PI*2)
        ctx.fillStyle = `rgba(255,55,10,${Math.min(1, 0.5+p.z*0.45).toFixed(2)})`
        ctx.fill()
      }

      // Hotspot dots — drawn ON TOP of land dots
      for (const h of HOTSPOTS) {
        const p = project(h.lat, h.lon)
        if (!p) continue
        const isActive = activeHotspotRef.current?.name === h.name
        const pulse = Math.sin(pulseRef.current + h.lat*0.3) * 0.5 + 0.5
        const ts = h.tier===2 ? 1 : h.tier===1 ? 0.75 : 0.55
        // Active dot uses animated scale, inactive dots slightly shrink when something else is active
        const as = isActive ? activeScaleRef.current : (activeHotspotRef.current ? 0.7 : 1)

        // Outer ring — bigger and brighter when active
        const ringR = (7+pulse*9)*ts*scaleRef.current*p.z*as
        const ringAlpha = isActive ? (0.55-pulse*0.2)*p.z : (0.28-pulse*0.22)*p.z
        ctx.beginPath(); ctx.arc(p.x, p.y, ringR, 0, Math.PI*2)
        ctx.strokeStyle = `rgba(255,140,30,${Math.max(0,ringAlpha).toFixed(2)})`
        ctx.lineWidth = isActive ? 1.5 : 0.9; ctx.stroke()

        // Second ring on active dot — extra pulse
        if (isActive) {
          const ring2R = (12+pulse*14)*ts*scaleRef.current*p.z
          ctx.beginPath(); ctx.arc(p.x, p.y, ring2R, 0, Math.PI*2)
          ctx.strokeStyle = `rgba(255,180,60,${((0.2-pulse*0.18)*p.z).toFixed(2)})`
          ctx.lineWidth = 1; ctx.stroke()
        }

        // Glow halo
        const glowR = (5+pulse*4)*ts*scaleRef.current*p.z*as
        const grd = ctx.createRadialGradient(p.x, p.y, 0, p.x, p.y, glowR)
        grd.addColorStop(0, `rgba(255,210,90,${(isActive?1:0.95)*p.z})`)
        grd.addColorStop(0.45, `rgba(255,110,15,${(0.5*p.z).toFixed(2)})`)
        grd.addColorStop(1, 'rgba(255,65,10,0)')
        ctx.beginPath(); ctx.arc(p.x, p.y, glowR, 0, Math.PI*2)
        ctx.fillStyle = grd; ctx.fill()

        // Bright core — doubles in size when active
        const coreR = (2+pulse*0.7)*ts*scaleRef.current*p.z*as
        ctx.beginPath(); ctx.arc(p.x, p.y, coreR, 0, Math.PI*2)
        ctx.fillStyle = isActive
          ? `rgba(255,255,220,${Math.min(1, p.z*1.2).toFixed(2)})`
          : `rgba(255,245,190,${Math.min(1, p.z*1.15).toFixed(2)})`
        ctx.fill()
      }

      ctx.restore()

      // Limb darkening
      const limb = ctx.createRadialGradient(CX, CY, R*0.5, CX, CY, R)
      limb.addColorStop(0, 'rgba(0,0,0,0)'); limb.addColorStop(1, 'rgba(0,0,0,0.88)')
      ctx.beginPath(); ctx.arc(CX, CY, Math.min(R, SIZE/2-1), 0, Math.PI*2)
      ctx.fillStyle = limb; ctx.fill()

      // Atmosphere rim
      const rim = ctx.createRadialGradient(CX, CY, R*0.88, CX, CY, R*1.06)
      rim.addColorStop(0, 'rgba(255,65,32,0)'); rim.addColorStop(1, 'rgba(255,65,32,0.13)')
      ctx.beginPath(); ctx.arc(CX, CY, Math.min(R*1.06, SIZE/2), 0, Math.PI*2)
      ctx.fillStyle = rim; ctx.fill()

      rafRef.current = requestAnimationFrame(draw)
    }

    rafRef.current = requestAnimationFrame(draw)
    return () => cancelAnimationFrame(rafRef.current)
  }, [])

  // Click handler — finds closest hotspot to click point
  const onCanvasClick = useCallback((e: React.MouseEvent<HTMLCanvasElement>) => {
    if (didDragRef.current) return
    const rect = (e.target as HTMLCanvasElement).getBoundingClientRect()
    const mx = (e.clientX - rect.left) * (SIZE / rect.width)
    const my = (e.clientY - rect.top) * (SIZE / rect.height)
    const hit = findClickedHotspot(mx, my, rotLonRef.current, rotThetaRef.current, scaleRef.current)
    if (hit) flyTo(hit)
    else dismiss()
  }, [flyTo, dismiss])

  const onPointerDown = useCallback((e: React.PointerEvent) => {
    pointerDownRef.current = true; didDragRef.current = false
    lastXRef.current = e.clientX; lastYRef.current = e.clientY
    ;(e.target as HTMLElement).setPointerCapture(e.pointerId)
  }, [])
  const onPointerUp = useCallback(() => { pointerDownRef.current = false }, [])
  const onPointerMove = useCallback((e: React.PointerEvent) => {
    if (!pointerDownRef.current) return
    const dx = e.clientX-lastXRef.current, dy = e.clientY-lastYRef.current
    if (Math.abs(dx)>3||Math.abs(dy)>3) didDragRef.current = true
    const sp = 0.4/scaleRef.current
    targetLonRef.current -= dx*sp
    targetThetaRef.current = Math.max(-MAX_THETA, Math.min(MAX_THETA, targetThetaRef.current-dy*sp*0.008))
    lastXRef.current = e.clientX; lastYRef.current = e.clientY
  }, [])
  const onWheel = useCallback((e: React.WheelEvent) => {
    e.preventDefault()
    if (e.ctrlKey||e.metaKey) {
      targetScaleRef.current = Math.max(0.5, Math.min(4, targetScaleRef.current*(1-e.deltaY*0.008)))
    } else if (Math.abs(e.deltaX)>Math.abs(e.deltaY)) {
      targetLonRef.current -= e.deltaX*0.3
    } else {
      targetThetaRef.current = Math.max(-MAX_THETA, Math.min(MAX_THETA, targetThetaRef.current-e.deltaY*0.002))
    }
  }, [])
  const lastTouchRef = useRef<{x:number,y:number}|null>(null)
  const onTouchStart = useCallback((e: React.TouchEvent) => {
    e.preventDefault(); didDragRef.current = false
    if (e.touches.length===1){pointerDownRef.current=true;lastTouchRef.current={x:e.touches[0].clientX,y:e.touches[0].clientY}}
    if (e.touches.length===2){pointerDownRef.current=false;pinchRef.current=Math.hypot(e.touches[1].clientX-e.touches[0].clientX,e.touches[1].clientY-e.touches[0].clientY)}
  }, [])
  const onTouchMove = useCallback((e: React.TouchEvent) => {
    e.preventDefault()
    if (e.touches.length===1&&lastTouchRef.current&&pointerDownRef.current){
      const dx=e.touches[0].clientX-lastTouchRef.current.x, dy=e.touches[0].clientY-lastTouchRef.current.y
      if (Math.abs(dx)>3||Math.abs(dy)>3) didDragRef.current=true
      const sp=0.4/scaleRef.current
      targetLonRef.current-=dx*sp
      targetThetaRef.current=Math.max(-MAX_THETA,Math.min(MAX_THETA,targetThetaRef.current-dy*sp*0.008))
      lastTouchRef.current={x:e.touches[0].clientX,y:e.touches[0].clientY}
    }
    if (e.touches.length===2&&pinchRef.current>0){
      const nd=Math.hypot(e.touches[1].clientX-e.touches[0].clientX,e.touches[1].clientY-e.touches[0].clientY)
      targetScaleRef.current=Math.max(0.5,Math.min(4,targetScaleRef.current*(nd/pinchRef.current)))
      pinchRef.current=nd
    }
  }, [])
  const onTouchEnd = useCallback((e: React.TouchEvent) => {
    if (!didDragRef.current&&e.changedTouches.length===1){
      const rect=canvasRef.current!.getBoundingClientRect()
      const mx=(e.changedTouches[0].clientX-rect.left)*(SIZE/rect.width)
      const my=(e.changedTouches[0].clientY-rect.top)*(SIZE/rect.height)
      const hit=findClickedHotspot(mx,my,rotLonRef.current,rotThetaRef.current,scaleRef.current)
      if (hit) flyTo(hit); else dismiss()
    }
    pointerDownRef.current=false; lastTouchRef.current=null; pinchRef.current=0
  }, [flyTo, dismiss])

  const cityEvents = activeHotspot ? (events[activeHotspot.name] ?? []) : []

  return (
    <div style={S.wrap}>
      <div style={S.brand}>
        <div style={S.brandName}>NEKTA</div>
        <div style={S.brandTag}>THE SCENE &gt; THE STREAMS</div>
      </div>

      <button style={S.spinBtn} onClick={() => setSpinning(s => !s)}>
        {spinning ? '⏸' : '▶'} <span style={{fontSize:'7px'}}>{spinning?'PAUSE':'SPIN'}</span>
      </button>

      <div style={{position:'relative',width:SIZE,height:SIZE,touchAction:'none'}}>
        {loading && <div style={S.loader}>LOADING...</div>}
        <canvas ref={canvasRef}
          style={{display:'block',cursor:'crosshair',opacity:visible?1:0,transition:'opacity 1.2s ease'}}
          onClick={onCanvasClick}
          onPointerDown={onPointerDown} onPointerUp={onPointerUp} onPointerMove={onPointerMove}
          onWheel={onWheel} onTouchStart={onTouchStart} onTouchMove={onTouchMove} onTouchEnd={onTouchEnd}
        />
      </div>

      {/* City sidebar — left side */}
      <div style={S.sidebar}>
        {HOTSPOTS.map(h => (
          <button key={h.name} onClick={() => flyTo(h)}
            style={{...S.si, opacity:activeHotspot?.name===h.name?1:0.45, background:activeHotspot?.name===h.name?'rgba(255,65,32,0.1)':'none'}}>
            <span style={{...S.dot,...(h.tier===2?S.dotBig:h.tier===1?S.dotMid:{})}}/>
            <span style={S.label}>{h.name}</span>
            {events[h.name]?.length > 0 && <span style={S.badge}>{events[h.name].length}</span>}
          </button>
        ))}
      </div>

      {/* Event panel */}
      {activeHotspot && (
        <div style={S.panel}>
          <div style={S.panelHeader}>
            <div>
              <div style={S.panelCity}>{activeHotspot.name}</div>
              <div style={S.panelDesc}>{activeHotspot.desc}</div>
            </div>
            <button onClick={dismiss} style={S.closeBtn}>✕</button>
          </div>
          {cityEvents.length > 0 ? (
            <div style={S.eventsWrap}>
              <div style={S.eventsTitle}>THIS WEEK</div>
              {cityEvents.map(ev => (
                <div key={ev.id} style={S.eventRow}>
                  <div style={S.eventName}>{ev.title}</div>
                  <div style={S.eventMeta}>
                    {ev.venue?.name}{ev.venue?.name && ' · '}
                    {new Date(ev.startTime).toLocaleDateString('en-GB',{weekday:'short',day:'numeric',month:'short'})}
                  </div>
                  {ev.artists && ev.artists.length > 0 && (
                    <div style={S.eventArtists}>{ev.artists.slice(0,3).map((a:any) => a.name).join(' · ')}</div>
                  )}
                </div>
              ))}
            </div>
          ) : (
            <div style={S.noEvents}>No live data · check RA for listings</div>
          )}
        </div>
      )}

      <div style={S.hint}>click a dot · drag · scroll to tilt · ctrl+scroll to zoom · space to pause</div>
    </div>
  )
}

const S: Record<string,React.CSSProperties> = {
  wrap:{width:'100%',minHeight:'100svh',background:'#0d0a09',display:'flex',alignItems:'center',justifyContent:'center',position:'relative',overflow:'hidden',fontFamily:'monospace'},
  brand:{position:'absolute',top:'24px',left:'24px',zIndex:10},
  brandName:{fontSize:'14px',fontWeight:700,letterSpacing:'0.22em',color:'#ff4120'},
  brandTag:{fontSize:'9px',color:'rgba(255,65,32,0.4)',letterSpacing:'0.12em',marginTop:'4px'},
  spinBtn:{position:'absolute',top:'24px',right:'20px',fontSize:'11px',color:'rgba(255,65,32,0.5)',background:'none',border:'1px solid rgba(255,65,32,0.2)',borderRadius:'4px',padding:'4px 10px',cursor:'pointer',zIndex:10,display:'flex',alignItems:'center',gap:'5px'},
  loader:{position:'absolute',inset:0,display:'flex',alignItems:'center',justifyContent:'center',color:'rgba(255,65,32,0.4)',fontSize:'10px',letterSpacing:'0.15em'},
  sidebar:{position:'absolute',left:'16px',top:'50%',transform:'translateY(-50%)',display:'flex',flexDirection:'column',gap:'3px',maxHeight:'80vh',overflowY:'auto',zIndex:10},
  si:{display:'flex',alignItems:'center',gap:'7px',cursor:'pointer',padding:'3px 8px',borderRadius:'4px',border:'none',transition:'all 0.2s'},
  dot:{width:'6px',height:'6px',borderRadius:'50%',background:'#ff4120',flexShrink:0,display:'inline-block'},
  dotMid:{width:'8px',height:'8px'},
  dotBig:{width:'11px',height:'11px',boxShadow:'0 0 8px #ff4120'},
  label:{fontSize:'9px',color:'#ff7050',letterSpacing:'0.08em',textTransform:'uppercase' as const,whiteSpace:'nowrap' as const},
  badge:{marginLeft:'2px',fontSize:'8px',background:'rgba(255,65,32,0.25)',color:'#ff7050',padding:'1px 4px',borderRadius:'3px'},
  panel:{position:'absolute',bottom:'28px',left:'50%',transform:'translateX(-50%)',background:'rgba(10,4,2,0.97)',border:'1px solid rgba(255,65,32,0.35)',borderRadius:'8px',padding:'14px 18px',minWidth:'280px',maxWidth:'360px',zIndex:20},
  panelHeader:{display:'flex',justifyContent:'space-between',alignItems:'flex-start'},
  panelCity:{fontSize:'12px',fontWeight:700,color:'#ff4120',letterSpacing:'0.18em',textTransform:'uppercase' as const},
  panelDesc:{fontSize:'9px',color:'rgba(255,130,100,0.55)',marginTop:'3px',letterSpacing:'0.05em'},
  closeBtn:{background:'none',border:'none',color:'rgba(255,65,32,0.5)',cursor:'pointer',fontSize:'14px',padding:'0 0 0 12px',lineHeight:1},
  eventsWrap:{marginTop:'12px',borderTop:'1px solid rgba(255,65,32,0.15)',paddingTop:'10px'},
  eventsTitle:{fontSize:'8px',color:'rgba(255,65,32,0.45)',letterSpacing:'0.18em',marginBottom:'8px'},
  eventRow:{marginBottom:'8px',paddingBottom:'8px',borderBottom:'1px solid rgba(255,65,32,0.08)'},
  eventName:{fontSize:'10px',color:'rgba(255,200,150,0.9)',letterSpacing:'0.04em',lineHeight:'1.4'},
  eventMeta:{fontSize:'8px',color:'rgba(255,100,50,0.5)',marginTop:'2px',letterSpacing:'0.06em'},
  eventArtists:{fontSize:'8px',color:'rgba(255,150,80,0.6)',marginTop:'2px',letterSpacing:'0.04em'},
  noEvents:{fontSize:'9px',color:'rgba(255,65,32,0.3)',marginTop:'10px',letterSpacing:'0.06em'},
  hint:{position:'absolute',bottom:'10px',left:'50%',transform:'translateX(-50%)',fontSize:'8px',color:'rgba(255,65,32,0.2)',letterSpacing:'0.1em',textTransform:'uppercase',whiteSpace:'nowrap'},
}
