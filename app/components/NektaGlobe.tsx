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
  const STEP = 1.1
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

  const [activeHotspot, setActiveHotspot] = useState<Hotspot | null>(null)
  const [spinning, setSpinning] = useState(true)
  const [events, setEvents] = useState<Record<string, EventData[]>>({})
  const [visible, setVisible] = useState(false)
  const [loading, setLoading] = useState(true)

  useEffect(() => { spinningRef.current = spinning }, [spinning])

  const flyTo = useCallback((h: Hotspot) => {
    targetLonRef.current = h.lon
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

      const dotR = Math.max(0.9, 1.5*scaleRef.current)

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
        const pulse = Math.sin(pulseRef.current + h.lat*0.3) * 0.5 + 0.5
        const ts = h.tier===2 ? 1 : h.tier===1 ? 0.75 : 0.55

        // Outer ring
        const ringR = (7+pulse*9)*ts*scaleRef.current*p.z
        ctx.beginPath(); ctx.arc(p.x, p.y, ringR, 0, Math.PI*2)
        ctx.strokeStyle = `rgba(255,140,30,${((0.28-pulse*0.22)*p.z).toFixed(2)})`
        ctx.lineWidth = 0.9; ctx.stroke()

        // Glow halo
        const glowR = (5+pulse*4)*ts*scaleRef.current*p.z
        const grd = ctx.createRadialGradient(p.x, p.y, 0, p.x, p.y, glowR)
        grd.addColorStop(0, `rgba(255,210,90,${(0.95*p.z).toFixed(2)})`)
        grd.addColorStop(0.45, `rgba(255,110,15,${(0.5*p.z).toFixed(2)})`)
        grd.addColorStop(1, 'rgba(255,65,10,0)')
        ctx.beginPath(); ctx.arc(p.x, p.y, glowR, 0, Math.PI*2)
        ctx.fillStyle = grd; ctx.fill()

        // Bright core
        const coreR = (2+pulse*0.7)*ts*scaleRef.current*p.z
        ctx.beginPath(); ctx.arc(p.x, p.y, coreR, 0, Math.PI*2)
        ctx.fillStyle = `rgba(255,245,190,${Math.min(1, p.z*1.15).toFixed(2)})`
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
